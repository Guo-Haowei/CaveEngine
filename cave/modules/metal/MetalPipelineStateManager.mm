#include "MetalPipelineStateManager.h"

#import <Metal/Metal.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

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

// Pipeline descriptions may specify either "primitive" or "primitive.vs".
// Resolve those to metal_generated/primitive.vs.metal.
static auto ReadMetalSource(std::string_view shader_name, std::string_view stage) -> Result<std::string> {
    std::string filename(shader_name);
    const std::string stage_suffix = "." + std::string(stage);

    const bool has_stage_suffix = filename.size() >= stage_suffix.size() && filename.compare(filename.size() - stage_suffix.size(), stage_suffix.size(), stage_suffix) == 0;

    if (!has_stage_suffix) {
        filename += stage_suffix;
    }
    filename += ".metal";

    const auto path = std::filesystem::path(ROOT_FOLDER) / "cave/shader/metal_generated" / filename;

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Could not open Metal shader source '{}'", path.string());
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

static id<MTLLibrary> CompileMetalSource(id<MTLDevice> device, const std::string& source, NSError** error) {
    NSString* source_text = [[NSString alloc] initWithBytes:source.data() length:source.size() encoding:NSUTF8StringEncoding];
    if (!source_text) {
        return nil;
    }

    return [device newLibraryWithSource:source_text options:nil error:error];
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
    case PixelFormat::R16G16_FLOAT:
        return MTLPixelFormatRG16Float;
    case PixelFormat::R16G16B16_FLOAT:
        return MTLPixelFormatRGB16Float;
    case PixelFormat::R16G16B16A16_FLOAT:
        return MTLPixelFormatRGBA16Float;
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
    if (!device) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal device unavailable");
    }

    NSError* error = nil;

    auto vs_source = ReadMetalSource(desc.vs, "vs");
    if (!vs_source) {
        return CAVE_ERROR(vs_source.error());
    }

    id<MTLLibrary> vs_library = CompileMetalSource(device, *vs_source, &error);
    if (!vs_library) {
        return CAVE_ERROR(ErrorCode::ERR_COMPILATION_FAILED, "Failed compiling vertex shader '{}': {}", desc.vs, error.localizedDescription.UTF8String);
    }

    id<MTLFunction> vs = [vs_library newFunctionWithName:@"vs_main"];
    if (!vs) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Metal vertex entry point 'vs_main' missing in '{}'", desc.vs);
    }

    id<MTLLibrary> ps_library = nil;
    id<MTLFunction> ps = nil;

    if (!desc.ps.empty()) {
        auto ps_source = ReadMetalSource(desc.ps, "ps");
        if (!ps_source) {
            return CAVE_ERROR(ps_source.error());
        }

        ps_library = CompileMetalSource(device, *ps_source, &error);
        if (!ps_library) {
            return CAVE_ERROR(ErrorCode::ERR_COMPILATION_FAILED, "Failed compiling pixel shader '{}': {}", desc.ps, error.localizedDescription.UTF8String);
        }

        ps = [ps_library newFunctionWithName:@"ps_main"];
        if (!ps) {
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Metal pixel entry point 'ps_main' missing in '{}'", desc.ps);
        }
    }

    MTLRenderPipelineDescriptor* pd = [MTLRenderPipelineDescriptor new];
    pd.vertexFunction = vs;
    pd.fragmentFunction = ps;

    const auto target_count = std::min<uint32_t>(desc.num_render_targets, 8);
    for (uint32_t i = 0; i < target_count; ++i) {
        const MTLPixelFormat format = ToPixelFormat(desc.rtv_formats[i]);
        if (format == MTLPixelFormatInvalid) {
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Unsupported Metal render target format at slot {}", i);
        }
        pd.colorAttachments[i].pixelFormat = format;
    }

    if (desc.dsv_format == PixelFormat::D32_FLOAT || desc.dsv_format == PixelFormat::D24_UNORM_S8_UINT) {
        pd.depthAttachmentPixelFormat = ToPixelFormat(desc.dsv_format);
        if (pd.depthAttachmentPixelFormat == MTLPixelFormatInvalid) {
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Unsupported Metal depth format");
        }
    }

    if (desc.input_layout_desc) {
        MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];

        for (const auto& e : desc.input_layout_desc->elements) {
            if (e.input_slot >= 31) {
                continue;
            }

            auto* attribute = vd.attributes[e.input_slot];
            attribute.format = ToVertexFormat(e.format);
            if (attribute.format == MTLVertexFormatInvalid) {
                return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Unsupported Metal vertex format for input slot {}", e.input_slot);
            }

            attribute.offset = e.aligned_byte_offset;
            attribute.bufferIndex = e.input_slot;

            // Cave currently uses one vertex stream per input slot.
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
            MTLStencilDescriptor* stencil_desc = [MTLStencilDescriptor new];
            stencil_desc.stencilCompareFunction = ToCompare(face.stencilFunc);
            stencil_desc.stencilFailureOperation = ToStencilOp(face.stencilFailOp);
            stencil_desc.depthFailureOperation = ToStencilOp(face.stencilDepthFailOp);
            stencil_desc.depthStencilPassOperation = ToStencilOp(face.stencilPassOp);
            stencil_desc.readMask = read_mask;
            stencil_desc.writeMask = write_mask;
            return stencil_desc;
        };

        depth_desc.frontFaceStencil = make_face(ds.frontFace, ds.stencilReadMask, ds.stencilWriteMask);
        depth_desc.backFaceStencil = make_face(ds.backFace, ds.stencilReadMask, ds.stencilWriteMask);
    }

    id<MTLDepthStencilState> depth_state = [device newDepthStencilStateWithDescriptor:depth_desc];
    if (!depth_state) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal depth/stencil state creation failed");
    }

    id<MTLRenderPipelineState> state = [device newRenderPipelineStateWithDescriptor:pd error:&error];
    if (!state) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal render pipeline creation failed: {}", error.localizedDescription.UTF8String);
    }

    auto result = MakeOwner<MetalPipelineState>(desc);
    result->state = (__bridge_retained void*)state;
    result->depth_state = (__bridge_retained void*)depth_state;
    return result;
}

auto MetalPipelineStateManager::computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal device unavailable");
    }

    auto source = ReadMetalSource(desc.cs, "cs");
    if (!source) {
        return CAVE_ERROR(source.error());
    }

    NSError* error = nil;
    id<MTLLibrary> library = CompileMetalSource(device, *source, &error);
    if (!library) {
        return CAVE_ERROR(ErrorCode::ERR_COMPILATION_FAILED, "Failed compiling compute shader '{}': {}", desc.cs, error.localizedDescription.UTF8String);
    }

    id<MTLFunction> function = [library newFunctionWithName:@"cs_main"];
    if (!function) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Metal compute entry point 'cs_main' missing in '{}'", desc.cs);
    }

    id<MTLComputePipelineState> state = [device newComputePipelineStateWithFunction:function error:&error];
    if (!state) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal compute pipeline creation failed: {}", error.localizedDescription.UTF8String);
    }

    auto result = MakeOwner<MetalPipelineState>(desc);
    result->state = (__bridge_retained void*)state;
    return result;
}

} // namespace cave::render
