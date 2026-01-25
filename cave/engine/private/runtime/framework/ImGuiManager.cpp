#include "ImGuiManager.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <filesystem>

#include "engine/assets/blob_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/input/key_code.h"
#include "engine/input/input_types.h"
#include "engine/runtime/framework/Application.h"
#include "engine/runtime/framework/IAssetManager.h"
#include "engine/runtime/framework/AssetRegistry.h"
#include "engine/runtime/framework/DisplayManager.h"
#include "engine/runtime/framework/VFS.h"

namespace cave {

namespace fs = std::filesystem;

auto ImguiManager::InitializeImpl() -> Result<void> {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    std::string_view engine_folder = StringUtils::BasePath(__FILE__);
    engine_folder = StringUtils::BasePath(engine_folder);
    engine_folder = StringUtils::BasePath(engine_folder);
    engine_folder = StringUtils::BasePath(engine_folder);
    engine_folder = StringUtils::BasePath(engine_folder);

    ImGuiIO& io = ImGui::GetIO();
    // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
    const float scale = 1.5f;
    const float base_font_size = 16.0f * scale;
    const float icon_font_size = base_font_size * 2.0f / 3.0f;

    {
        const std::string path = "@persist://fonts/DroidSans.ttf";
        auto res = m_app->GetAssetRegistry()->FindByPath<BlobAsset>(path).unwrap();
        BlobAsset* font = res.Get();

        if (DEV_VERIFY(font)) {
            ImFontConfig font_cfg;
            font_cfg.FontDataOwnedByAtlas = false;

            void* data = const_cast<char*>(font->GetBufferPoiner());
            if (!io.Fonts->AddFontFromMemoryTTF(data, (int)font->GetBufferLength(), base_font_size, &font_cfg)) {
                return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Failed to create font '{}'", path);
            }
        }
    }

    {
        const std::string path = "@persist://fonts/fa-solid-900.ttf";
        auto res = m_app->GetAssetRegistry()->FindByPath<BlobAsset>(path).unwrap();
        BlobAsset* font = res.Get();

        if (DEV_VERIFY(font)) {
            // merge in icons from Font Awesome
            static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            icons_config.GlyphMinAdvanceX = icon_font_size;
            icons_config.FontDataOwnedByAtlas = false;

            void* data = const_cast<char*>(font->GetBufferPoiner());
            if (!io.Fonts->AddFontFromMemoryTTF(data, (int)font->GetBufferLength(), base_font_size, &icons_config, icons_ranges)) {
                return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Failed to create font '{}'", path);
            }
        }
    }

    fs::path ini_path = m_app->GetVFS().Resolve("@user://imgui.ini");
    m_imguiSettingsPath = ini_path.string();
    LOG_VERBOSE("imgui settings path is '{}'", m_imguiSettingsPath);
    io.IniFilename = m_imguiSettingsPath.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);

    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.3805f, 0.381f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.2805f, 0.281f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    DEV_ASSERT(m_displayInitializeFunc);
    m_displayInitializeFunc();

    DEV_ASSERT(m_rendererInitializeFunc);
    m_rendererInitializeFunc();

    return Result<void>();
}

void ImguiManager::FinalizeImpl() {
    DEV_ASSERT(m_rendererFinalizeFunc);
    m_rendererFinalizeFunc();

    DEV_ASSERT(m_displayFinalizeFunc);
    m_displayFinalizeFunc();

    ImGui::DestroyContext();
}

void ImguiManager::BeginFrame() {
    if (DEV_VERIFY(m_displayBeginFrameFunc)) {
        m_displayBeginFrameFunc();
        ImGui::NewFrame();
    }
}

static int MouseButtonIndex(Key p_key) {
    switch (p_key) {
        case Key::LMB:
            return 0;  // ImGuiMouseButton_Left
        case Key::RMB:
            return 1;  // ImGuiMouseButton_Right
        case Key::MMB:
            return 2;  // ImGuiMouseButton_Middle
        default:
            return -1;
    }
}

ImGuiKey ImguiManager::ToImGuiKey(Key p_key) {
    switch (p_key) {
        case Key::W:
            return ImGuiKey_W;
        case Key::A:
            return ImGuiKey_A;
        case Key::S:
            return ImGuiKey_S;
        case Key::D:
            return ImGuiKey_D;

        case Key::LeftCtrl:
        case Key::RightCtrl:
            return ImGuiKey_ModCtrl;
        case Key::LeftShift:
        case Key::RightShift:
            return ImGuiKey_ModShift;
        case Key::LeftAlt:
        case Key::RightAlt:
            return ImGuiKey_ModAlt;

        case Key::Up:
            return ImGuiKey_UpArrow;
        case Key::Down:
            return ImGuiKey_DownArrow;
        case Key::Left:
            return ImGuiKey_LeftArrow;
        case Key::Right:
            return ImGuiKey_RightArrow;

        // Add more as needed
        default:
            return ImGuiKey_None;
    }
}

void ImguiManager::Feed(std::vector<InputEvent>& p_events) {
    ImGuiIO& io = ImGui::GetIO();

    for (auto& e : p_events) {
        if (e.consumed) {
            continue;
        }

        switch (e.type) {
            case InputEventType::MouseMove: {
                float x = static_cast<float>(e.x);
                float y = static_cast<float>(e.y);
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    auto [window_x, window_y] = m_app->GetDisplayManager()->GetWindowPos();
                    x += window_x;
                    y += window_y;
                }
                io.AddMousePosEvent(x, y);
            } break;
            case InputEventType::MouseWheel: {
                io.AddMouseWheelEvent(e.dx, e.dy);
            } break;
            case InputEventType::TextInput: {
                io.AddInputCharacter((unsigned int)e.code);
            } break;
            case InputEventType::ButtonDown:
            case InputEventType::ButtonUp: {
                const bool down = (e.type == InputEventType::ButtonDown);
                const Key k = static_cast<Key>(e.code);

                if (IsMouseButton(k)) {
                    if (const int idx = MouseButtonIndex(k); idx >= 0) {
                        io.AddMouseButtonEvent(idx, down);
                    }
                } else {
                    if (const ImGuiKey ik = ToImGuiKey(k); ik != ImGuiKey_None) {
                        io.AddKeyEvent(ik, down);
                    }
                }
            } break;
            default:
                break;
        }
    }
}

bool ImguiManager::WantKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImguiManager::WantMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImguiManager::WantTextInput() const {
    return ImGui::GetIO().WantTextInput;
}

}  // namespace cave
