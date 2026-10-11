#include "OpenGlPipelineStateManager.h"

#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "OpenGlDefines.h"
#include "OpenGlHelpers.h"

#include <fstream>

namespace cave {
#include "shader_resource_defines.slang.h"
}  // namespace cave

namespace cave::render {

namespace fs = std::filesystem;

struct TextureSlot {
    const char* name;
    int slot;
};

static constexpr TextureSlot s_textureSots[] = {
#define TEXTURE_2D(NAME, SLOT) TextureSlot{ #NAME, SLOT },
    SRV_DEFINES
#undef TEXTURE_2D
};

OpenGlPipelineState::~OpenGlPipelineState() {
    if (programId) {
        glDeleteProgram(programId);
    }
}

// @TODO: refactor this. Shader will be included as const char* directly
static std::string ReadFileToString(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return {};  // return empty string on failure

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static auto ProcessShader(const fs::path& p_path, int p_depth) -> Result<std::string> {
    constexpr int max_depth = 100;
    if (p_depth >= max_depth) {
        return CAVE_ERROR(ErrorCode::ERR_COMPILATION_FAILED, "circular includes in file '{}'!", p_path.string());
    }

    std::string source = ReadFileToString(p_path.string());

    std::string final_string;
    std::stringstream ss(source);
    for (std::string line; std::getline(ss, line);) {
        constexpr const char pattern[] = "#include";
        if (line.find(pattern) == 0) {
            const char* lineStr = line.c_str();
            const char* quote1 = strchr(lineStr, '"');
            const char* quote2 = strrchr(lineStr, '"');
            if (!(quote1 && quote2 && (quote1 != quote2))) {
                const char* left = strchr(lineStr, '<');
                const char* right = strrchr(lineStr, '>');
                if (left && right && (left < right)) {
                    // skip line
                    continue;
                }
                CRASH_NOW_MSG("should not reach here");
            }
            std::string file_to_include(quote1 + 1, quote2);

            fs::path new_path = p_path;
            new_path.remove_filename();
            new_path = new_path / file_to_include;

            auto res = ProcessShader(new_path, p_depth + 1);
            if (!res) {
                return CAVE_ERROR(res.error());
            }

            final_string.append(*res);
        } else {
            final_string.append(line);
        }

        final_string.push_back('\n');
    }

    return final_string;
}

static auto CreateShader(std::string_view shader_name, GLenum shader_type) -> Result<GLuint> {
    String file{ shader_name };
    file.append(".glsl");
    fs::path fullpath = fs::path{ ROOT_FOLDER } / "cave" / "shader" / "opengl_generated" / file;

    auto result = ProcessShader(fullpath, 0);
    if (!result) {
        return CAVE_ERROR(result.error());
    }

    String fullsource = std::move(*result);
    const char* sources[] = { fullsource.c_str() };

    GLuint shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, sources, nullptr);
    glCompileShader(shader_id);

    GLint status = GL_FALSE, length = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        Vector<char> buffer(length + 1);
        glGetShaderInfoLog(shader_id, length, nullptr, buffer.data());
        LOG_FATAL(LogChannel::Render, "[glsl] failed to compile shader_id '{}'\ndetails:\n{}", shader_name, buffer.data());
        glDeleteShader(shader_id);
        return CAVE_ERROR(ErrorCode::ERR_COMPILATION_FAILED, "[glsl] failed to compile shader_id '{}'", shader_name);
    }

    if (status == GL_FALSE) {
        glDeleteShader(shader_id);
        return CAVE_ERROR(ErrorCode::ERR_COMPILATION_FAILED, "failed to compile shader '{}'", shader_name);
    }

    return shader_id;
}

auto OpenGlPipelineStateManager::graphicsPipeline(const PipelineStateDesc& pipeline_state_desc) -> Result<Owner<PipelineState>> {
    return createPipelineImpl(pipeline_state_desc);
}

auto OpenGlPipelineStateManager::computePipeline(const PipelineStateDesc& pipeline_state_desc) -> Result<Owner<PipelineState>> {
    return createPipelineImpl(pipeline_state_desc);
}

auto OpenGlPipelineStateManager::createPipelineImpl(const PipelineStateDesc& pipeline_state_desc) -> Result<Owner<PipelineState>> {
    GLuint program_id = glCreateProgram();
    Vector<GLuint> shaders;
    auto create_shader_helper = [&](std::string_view path, GLenum type) {
        auto res = CreateShader(path, type);
        if (res) {
            glAttachShader(program_id, *res);
            shaders.push_back(*res);
        }
        return res;
    };

    ON_SCOPE_EXIT([&]() {
        for (GLuint id : shaders) {
            glDeleteShader(id);
        }
    });

    switch (pipeline_state_desc.type) {
        case PipelineStateType::GRAPHICS: {
            Result<GLuint> result(0);
            do {
                if (!pipeline_state_desc.vs.empty()) {
                    result = create_shader_helper(pipeline_state_desc.vs, GL_VERTEX_SHADER);
                    if (!result) { break; }
                }
#if !USING(USE_GLES3)
                if (!pipeline_state_desc.gs.empty()) {
                    result = create_shader_helper(pipeline_state_desc.gs, GL_GEOMETRY_SHADER);
                    if (!result) { break; }
                }
#endif
                if (!pipeline_state_desc.ps.empty()) {
                    result = create_shader_helper(pipeline_state_desc.ps, GL_FRAGMENT_SHADER);
                    if (!result) { break; }
                }
            } while (0);
            if (!result) {
                return CAVE_ERROR(result.error());
            }
        } break;
#if !USING(USE_GLES3)
        case PipelineStateType::COMPUTE: {
            DEV_ASSERT(!pipeline_state_desc.cs.empty());
            auto result = create_shader_helper(pipeline_state_desc.cs, GL_COMPUTE_SHADER);
            if (!result) {
                return CAVE_ERROR(result.error());
            }
        } break;
#endif
        default:
            CRASH_NOW();
            break;
    }

    DEV_ASSERT(!shaders.empty());

    glLinkProgram(program_id);
    GLint status = GL_FALSE;
    GLint length = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        std::vector<char> buffer(length + 1);
        glGetProgramInfoLog(program_id, length, nullptr, buffer.data());
        if (status == GL_TRUE) {
#if 0
            LOG_WARN("[glsl] warning\ndetails:\n{}", buffer.data());
#endif
        } else {
            LOG_FATAL("[glsl] failed to link program\ndetails:\n{}", buffer.data());
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE);
        }
    }

    if (status == GL_FALSE) {
        glDeleteProgram(program_id);
        program_id = 0;
    }

    auto program = MakeOwner<OpenGlPipelineState>(pipeline_state_desc);
    program->programId = program_id;

    glUseProgram(program_id);

    // set textures
    for (int i = 0; i < 15; ++i) {
        auto name = std::format("u_Texture{}", i);
        const int location = glGetUniformLocation(program_id, name.c_str());
        if (location != -1) {
            glUniform1i(location, i);
        }
    }

    // set uniforms
    for (uint32_t i = 0; i < std::size(s_textureSots); ++i) {
        const int location = glGetUniformLocation(program_id, s_textureSots[i].name);
        if (location != -1) {
            glUniform1i(location, s_textureSots[i].slot);
        }
    }

    glUseProgram(0);

    return program;
}

}  // namespace cave::render
