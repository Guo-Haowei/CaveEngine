#include "ImGuiManager.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <filesystem>

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/input/InputTypes.h"
#include "cave/runtime/input/KeyCode.h"

#include "engine/private/assets/blob_asset.h"
#include "engine/private/core/string/StringUtils.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

namespace fs = std::filesystem;

auto ImguiManager::InitializeImpl() -> Result<void> {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

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
    // clang-format off
    switch (p_key) {
        case Key::Tab:            return ImGuiKey_Tab;
        case Key::Left:           return ImGuiKey_LeftArrow;
        case Key::Right:          return ImGuiKey_RightArrow;
        case Key::Up:             return ImGuiKey_UpArrow;
        case Key::Down:           return ImGuiKey_DownArrow;

        case Key::PageUp:         return ImGuiKey_PageUp;
        case Key::PageDown:       return ImGuiKey_PageDown;
        case Key::Home:           return ImGuiKey_Home;
        case Key::End:            return ImGuiKey_End;
        case Key::Insert:         return ImGuiKey_Insert;
        case Key::Delete:         return ImGuiKey_Delete;
        case Key::Backspace:      return ImGuiKey_Backspace;
        case Key::Space:          return ImGuiKey_Space;
        case Key::Enter:          return ImGuiKey_Enter;
        case Key::Escape:         return ImGuiKey_Escape;
        case Key::LeftCtrl:       return ImGuiKey_LeftCtrl;
        case Key::LeftShift:      return ImGuiKey_LeftShift;
        case Key::LeftAlt:        return ImGuiKey_LeftAlt;
        case Key::LeftSuper:      return ImGuiKey_LeftSuper;
        case Key::RightCtrl:      return ImGuiKey_RightCtrl;
        case Key::RightShift:     return ImGuiKey_RightShift;
        case Key::RightAlt:       return ImGuiKey_RightAlt;
        case Key::RightSuper:     return ImGuiKey_RightSuper;
        case Key::Menu:           return ImGuiKey_Menu;

        case Key::_0:             return ImGuiKey_0;
        case Key::_1:             return ImGuiKey_1;
        case Key::_2:             return ImGuiKey_2;
        case Key::_3:             return ImGuiKey_3;
        case Key::_4:             return ImGuiKey_4;
        case Key::_5:             return ImGuiKey_5;
        case Key::_6:             return ImGuiKey_6;
        case Key::_7:             return ImGuiKey_7;
        case Key::_8:             return ImGuiKey_8;
        case Key::_9:             return ImGuiKey_9;

        case Key::A:              return ImGuiKey_A;
        case Key::B:              return ImGuiKey_B;
        case Key::C:              return ImGuiKey_C;
        case Key::D:              return ImGuiKey_D;
        case Key::E:              return ImGuiKey_E;
        case Key::F:              return ImGuiKey_F;
        case Key::G:              return ImGuiKey_G;
        case Key::H:              return ImGuiKey_H;
        case Key::I:              return ImGuiKey_I;
        case Key::J:              return ImGuiKey_J;
        case Key::K:              return ImGuiKey_K;
        case Key::L:              return ImGuiKey_L;
        case Key::M:              return ImGuiKey_M;
        case Key::N:              return ImGuiKey_N;
        case Key::O:              return ImGuiKey_O;
        case Key::P:              return ImGuiKey_P;
        case Key::Q:              return ImGuiKey_Q;
        case Key::R:              return ImGuiKey_R;
        case Key::S:              return ImGuiKey_S;
        case Key::T:              return ImGuiKey_T;
        case Key::U:              return ImGuiKey_U;
        case Key::V:              return ImGuiKey_V;
        case Key::W:              return ImGuiKey_W;
        case Key::X:              return ImGuiKey_X;
        case Key::Y:              return ImGuiKey_Y;
        case Key::Z:              return ImGuiKey_Z;

        case Key::F1:             return ImGuiKey_F1;
        case Key::F2:             return ImGuiKey_F2;
        case Key::F3:             return ImGuiKey_F3;
        case Key::F4:             return ImGuiKey_F4;
        case Key::F5:             return ImGuiKey_F5;
        case Key::F6:             return ImGuiKey_F6;
        case Key::F7:             return ImGuiKey_F7;
        case Key::F8:             return ImGuiKey_F8;
        case Key::F9:             return ImGuiKey_F9;
        case Key::F10:            return ImGuiKey_F10;
        case Key::F11:            return ImGuiKey_F11;
        case Key::F12:            return ImGuiKey_F12;
        case Key::F13:            return ImGuiKey_F13;
        case Key::F14:            return ImGuiKey_F14;
        case Key::F15:            return ImGuiKey_F15;
        case Key::F16:            return ImGuiKey_F16;
        case Key::F17:            return ImGuiKey_F17;
        case Key::F18:            return ImGuiKey_F18;
        case Key::F19:            return ImGuiKey_F19;
        case Key::F20:            return ImGuiKey_F20;
        case Key::F21:            return ImGuiKey_F21;
        case Key::F22:            return ImGuiKey_F22;
        case Key::F23:            return ImGuiKey_F23;
        case Key::F24:            return ImGuiKey_F24;

        case Key::Apostrophe:     return ImGuiKey_Apostrophe;
        case Key::Comma:          return ImGuiKey_Comma;
        case Key::Minus:          return ImGuiKey_Minus;
        case Key::Period:         return ImGuiKey_Period;
        case Key::Slash:          return ImGuiKey_Slash;
        case Key::Semicolon:      return ImGuiKey_Semicolon;
        case Key::Equal:          return ImGuiKey_Equal;
        case Key::LeftBracket:    return ImGuiKey_LeftBracket;
        case Key::Backslash:      return ImGuiKey_Backslash;
        case Key::RightBracket:   return ImGuiKey_RightBracket;
        case Key::GraveAccent:    return ImGuiKey_GraveAccent;

        case Key::CapsLock:       return ImGuiKey_CapsLock;
        case Key::ScrollLock:     return ImGuiKey_ScrollLock;
        case Key::NumLock:        return ImGuiKey_NumLock;
        case Key::PrintScreen:    return ImGuiKey_PrintScreen;
        case Key::Pause:          return ImGuiKey_Pause;

        case Key::Keypad0:        return ImGuiKey_Keypad0;
        case Key::Keypad1:        return ImGuiKey_Keypad1;
        case Key::Keypad2:        return ImGuiKey_Keypad2;
        case Key::Keypad3:        return ImGuiKey_Keypad3;
        case Key::Keypad4:        return ImGuiKey_Keypad4;
        case Key::Keypad5:        return ImGuiKey_Keypad5;
        case Key::Keypad6:        return ImGuiKey_Keypad6;
        case Key::Keypad7:        return ImGuiKey_Keypad7;
        case Key::Keypad8:        return ImGuiKey_Keypad8;
        case Key::Keypad9:        return ImGuiKey_Keypad9;

        case Key::KeypadDecimal:  return ImGuiKey_KeypadDecimal;
        case Key::KeypadDivide:   return ImGuiKey_KeypadDivide;
        case Key::KeypadMultiply: return ImGuiKey_KeypadMultiply;
        case Key::KeypadSubtract: return ImGuiKey_KeypadSubtract;
        case Key::KeypadAdd:      return ImGuiKey_KeypadAdd;
        case Key::KeypadEnter:    return ImGuiKey_KeypadEnter;
        case Key::KeypadEqual:    return ImGuiKey_KeypadEqual;

        default:                  return ImGuiKey_None;
    }
    // clang-format on
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
