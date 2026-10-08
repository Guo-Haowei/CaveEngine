#include "MetalPipelineStateManager.h"

#import <Metal/Metal.h>
#include <filesystem>

namespace cave::render {

MetalPipelineState::~MetalPipelineState() {
    if (state)
        CFRelease(state);
    if (depth_state)
        CFRelease(depth_state);
}

static MTLCompareFunction ToCompare(ComparisonFunc f) {
    switch (f) {
    case ComparisonFunc::NEVER:
        return MTLCompareFunctionNever;
    case ComparisonFunc::LESS:
        return MTLCompareFunctionLess;
    case ComparisonFunc::EQUAL:
        return MTLCompareFunctionEqual;
    case ComparisonFunc::LESS_EQUAL:
        return MTLCompareFunctionLessEqual;
    case ComparisonFunc::GREATER:
        return MTLCompareFunctionGreater;
    case ComparisonFunc::NOT_EQUAL:
        return MTLCompareFunctionNotEqual;
    case ComparisonFunc::GREATER_EQUAL:
        return MTLCompareFunctionGreaterEqual;
    case ComparisonFunc::ALWAYS:
        return MTLCompareFunctionAlways;
    default:
        return MTLCompareFunctionAlways;
    }
}

static MTLStencilOperation ToStencilOp(StencilOp op) {
    switch (op) {
    case StencilOp::KEEP:
        return MTLStencilOperationKeep;
    case StencilOp::REPLACE:
        return MTLStencilOperationReplace;
    default:
        return MTLStencilOperationKeep;
    }
}

// Current asset convention: desc.vs/ps/cs are Metal function names. Keep the
// library location in one place until Cave's shader asset path is finalized.
static id<MTLLibrary> LoadLibrary(id<MTLDevice> device, NSError** error) {
    NSString* path = [NSString stringWithUTF8String:(std::filesystem::path(ROOT_FOLDER) / "cave/shader/metal/CaveShaders.metallib").string().c_str()];
    return [device newLibraryWithURL:[NSURL fileURLWithPath:path] error:error];
}

static MTLVertexFormat ToVertexFormat(PixelFormat format) {
    switch (format) {
    case PixelFormat::R32G32_FLOAT:
        return MTLVertexFormatFloat2;
    case PixelFormat::R32G32B32_FLOAT:
        return MTLVertexFormatFloat3;
    case PixelFormat::R32G32B32A32_SINT:
        return MTLVertexFormatInt4;
    case PixelFormat::R32G32B32A32_FLOAT:
        return MTLVertexFormatFloat4;
    default:
        return MTLVertexFormatInvalid;
    }
}

static NSUInteger VertexFormatSize(PixelFormat format) {
    switch (format) {
    case PixelFormat::R32G32_FLOAT:
        return 2 * sizeof(float);
    case PixelFormat::R32G32B32_FLOAT:
        return 3 * sizeof(float);
    case PixelFormat::R32G32B32A32_SINT:
    case PixelFormat::R32G32B32A32_FLOAT:
        return 4 * sizeof(uint32_t);
    default:
        return 0;
    }
}

static MTLPixelFormat ToPixelFormat(PixelFormat format) {
    switch (format) {
    case PixelFormat::R8G8B8A8_UINT:
        return MTLPixelFormatRGBA8Unorm;
    case PixelFormat::R32_FLOAT:
        return MTLPixelFormatR32Float;
    case PixelFormat::R32G32_FLOAT:
        return MTLPixelFormatRG32Float;
    case PixelFormat::R32G32B32A32_FLOAT:
        return MTLPixelFormatRGBA32Float;
    case PixelFormat::D32_FLOAT:
        return MTLPixelFormatDepth32Float;
    case PixelFormat::D24_UNORM_S8_UINT:
        return MTLPixelFormatDepth24Unorm_Stencil8;
    default:
        return MTLPixelFormatInvalid;
    }
}

auto MetalPipelineStateManager::graphicsPipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal device unavailable");
    NSError* error = nil;
    id<MTLLibrary> library = LoadLibrary(device, &error);
    if (!library)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Could not load CaveShaders.metallib: {}", error.localizedDescription.UTF8String);
    NSString* vs_name = [NSString stringWithUTF8String:std::string(desc.vs).c_str()];
    NSString* ps_name = [NSString stringWithUTF8String:std::string(desc.ps).c_str()];
    id<MTLFunction> vs = [library newFunctionWithName:vs_name];
    id<MTLFunction> ps = desc.ps.empty() ? nil : [library newFunctionWithName:ps_name];
    if (!vs || (!desc.ps.empty() && !ps))
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Metal entry point missing (vs='{}', ps='{}')", desc.vs, desc.ps);

    MTLRenderPipelineDescriptor* pd = [MTLRenderPipelineDescriptor new];
    pd.vertexFunction = vs;
    pd.fragmentFunction = ps;
    const auto target_count = std::min<uint32_t>(desc.num_render_targets, 8);
    for (uint32_t i = 0; i < target_count; ++i) {
        MTLPixelFormat format = ToPixelFormat(desc.rtv_formats[i]);
        if (format == MTLPixelFormatInvalid)
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Unsupported Metal render target format at slot {}", i);
        pd.colorAttachments[i].pixelFormat = format;
    }
    if (desc.dsv_format == PixelFormat::D32_FLOAT || desc.dsv_format == PixelFormat::D24_UNORM_S8_UINT) {
        pd.depthAttachmentPixelFormat = ToPixelFormat(desc.dsv_format);
        if (pd.depthAttachmentPixelFormat == MTLPixelFormatInvalid)
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Unsupported Metal depth format");
    }
    if (desc.input_layout_desc) {
        MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];
        for (const auto& e : desc.input_layout_desc->elements) {
            if (e.input_slot >= 31)
                continue;
            auto* a = vd.attributes[e.input_slot];
            a.format = ToVertexFormat(e.format);
            if (a.format == MTLVertexFormatInvalid)
                return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Unsupported Metal vertex format for input slot {}", e.input_slot);
            a.offset = e.aligned_byte_offset;
            a.bufferIndex = e.input_slot;
            // Cave currently creates one vertex stream per input slot.
            vd.layouts[e.input_slot].stride = VertexFormatSize(e.format);
            vd.layouts[e.input_slot].stepFunction = e.input_slot_class == InputClassification::PER_VERTEX_DATA ? MTLVertexStepFunctionPerVertex : MTLVertexStepFunctionPerInstance;
            vd.layouts[e.input_slot].stepRate = std::max(1u, e.instance_data_step_rate);
        }
        pd.vertexDescriptor = vd;
    }
    MTLDepthStencilDescriptor* depth_desc = [MTLDepthStencilDescriptor new];
    const DepthStencilDesc default_depth{};
    const DepthStencilDesc& ds = desc.depth_stencil_desc ? *desc.depth_stencil_desc : default_depth;
    depth_desc.depthCompareFunction = ds.depthEnabled ? ToCompare(ds.depthFunc) : MTLCompareFunctionAlways;
    depth_desc.depthWriteEnabled = ds.depthEnabled;
    if (ds.stencilEnabled) {
        auto make_face = [](const StencilOpDesc& face, uint8_t read_mask, uint8_t write_mask) {
            MTLStencilDescriptor* sd = [MTLStencilDescriptor new];
            sd.stencilCompareFunction = ToCompare(face.stencilFunc);
            sd.stencilFailureOperation = ToStencilOp(face.stencilFailOp);
            sd.depthFailureOperation = ToStencilOp(face.stencilDepthFailOp);
            sd.depthStencilPassOperation = ToStencilOp(face.stencilPassOp);
            sd.readMask = read_mask;
            sd.writeMask = write_mask;
            return sd;
        };
        depth_desc.frontFaceStencil = make_face(ds.frontFace, ds.stencilReadMask, ds.stencilWriteMask);
        depth_desc.backFaceStencil = make_face(ds.backFace, ds.stencilReadMask, ds.stencilWriteMask);
    }
    id<MTLDepthStencilState> depth_state = [device newDepthStencilStateWithDescriptor:depth_desc];
    if (!depth_state)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal depth/stencil state creation failed");
    id<MTLRenderPipelineState> state = [device newRenderPipelineStateWithDescriptor:pd error:&error];
    if (!state)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal render pipeline creation failed: {}", error.localizedDescription.UTF8String);
    auto result = MakeOwner<MetalPipelineState>(desc);
    result->state = (__bridge_retained void*)state;
    result->depth_state = (__bridge_retained void*)depth_state;
    return result;
}

auto MetalPipelineStateManager::computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal device unavailable");
    NSError* error = nil;
    id<MTLLibrary> library = LoadLibrary(device, &error);
    if (!library)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Could not load CaveShaders.metallib: {}", error.localizedDescription.UTF8String);
    NSString* name = [NSString stringWithUTF8String:std::string(desc.cs).c_str()];
    id<MTLFunction> function = [library newFunctionWithName:name];
    if (!function)
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Metal compute entry point missing: {}", desc.cs);
    id<MTLComputePipelineState> state = [device newComputePipelineStateWithFunction:function error:&error];
    if (!state)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal compute pipeline creation failed: {}", error.localizedDescription.UTF8String);
    auto result = MakeOwner<MetalPipelineState>(desc);
    result->state = (__bridge_retained void*)state;
    return result;
}

} // namespace cave::render
