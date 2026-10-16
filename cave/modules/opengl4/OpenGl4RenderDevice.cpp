#include "OpenGl4RenderDevice.h"

#include <imgui/backends/imgui_impl_opengl3.h>

#include "cave/runtime/framework/IApplication.h"

#include "../opengl_common/OpenGlHelpers.h"
#include "../opengl_common/OpenGlPrerequisites.h"
#include "../opengl_common/OpenGlResources.h"
#include "engine/private/runtime/display/GlfwDisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"

namespace cave::render {

void APIENTRY DebugCallback(GLenum, GLenum, unsigned int, GLenum, GLsizei, const char*, const void*);

auto OpenGl4RenderDevice::InitializeInternal() -> Result<void> {
    auto display_manager = dynamic_cast<GlfwDisplayService*>(m_app->services().display_service);
    DEV_ASSERT(display_manager);
    if (!display_manager) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "display manager is nullptr");
    }
    m_window = display_manager->GetGlfwWindow();

    if (gladLoadGL() == 0) {
        LOG_FATAL("[glad] failed to import gl functions");
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE);
    }

    LOG_TRACE("[opengl] renderer: {}", (const char*)glGetString(GL_RENDERER));
    LOG_TRACE("[opengl] version: {}", (const char*)glGetString(GL_VERSION));

    if (m_enable_validation_layer) {
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(DebugCallback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            LOG_TRACE("[opengl] debug callback enabled");
        }
    }

    // query capabilities
    {
        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        if (major < 4 || minor < 3) {
            m_capabilities.support_cs = false;
        }
    }

    if (ImGuiService* imgui = m_app->services().imgui) {
        imgui->setRenderCallbacks(
            []() {
                ImGui_ImplOpenGL3_Init();
                ImGui_ImplOpenGL3_CreateDeviceObjects();
            },
            []() {
                ImGui_ImplOpenGL3_Shutdown();
            });
    }

    return Result<void>();
}

auto OpenGl4RenderDevice::createStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<Ref<GpuStructuredBuffer>> {
    GLuint handle = 0;
    glGenBuffers(1, &handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, p_desc.element_count * p_desc.element_size, p_desc.initial_data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    auto buffer = MakeRef<OpenGlStructuredBuffer>(p_desc);
    buffer->handle = handle;
    return buffer;
}

void OpenGl4RenderDevice::updateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) {
    auto buffer = reinterpret_cast<const OpenGlStructuredBuffer*>(p_buffer);
    if (DEV_VERIFY(buffer)) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->handle);
        float* ptr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        DEV_ASSERT(ptr);
        memcpy(ptr + p_desc.offset, p_desc.initial_data, p_desc.element_count * p_desc.element_size);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void APIENTRY DebugCallback(GLenum p_source,
                            GLenum p_type,
                            uint32_t p_id,
                            GLenum p_severity,
                            GLsizei,
                            const char* p_message,
                            const void*) {
    switch (p_id) {
        case 131185:
        case 131204:
            return;
    }

    const char* source = "GL_DEBUG_SOURCE_OTHER";
    switch (p_source) {
        case GL_DEBUG_SOURCE_API:
            source = "GL_DEBUG_SOURCE_API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            source = "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            source = "GL_DEBUG_SOURCE_SHADER_COMPILER";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            source = "GL_DEBUG_SOURCE_THIRD_PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            source = "GL_DEBUG_SOURCE_APPLICATION";
            break;
        default:
            break;
    }

    const char* type = "GL_DEBUG_TYPE_OTHER";
    switch (p_type) {
        case GL_DEBUG_TYPE_ERROR:
            type = "GL_DEBUG_TYPE_ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            type = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            type = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            type = "GL_DEBUG_TYPE_PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            type = "GL_DEBUG_TYPE_PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_MARKER:
            type = "GL_DEBUG_TYPE_MARKER";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            [[fallthrough]];
        case GL_DEBUG_TYPE_POP_GROUP:
            return;
        default:
            break;
    }

    LogLevel level = LOG_LEVEL_INFO;
    switch (p_severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            level = LOG_LEVEL_ERROR;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            level = LOG_LEVEL_WARN;
            break;
        case GL_DEBUG_SEVERITY_LOW:
            return;
        default:
            break;
    }

    cave::LogImpl(level, std::format("[opengl] {}\n\t| id: {} | source: {} | type: {}", p_message, p_id, source, type));
}

}  // namespace cave::render
