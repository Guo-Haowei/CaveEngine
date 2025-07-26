#pragma once
#include "engine/core/base/singleton.h"
#include "engine/input/input_router.h"
#include "engine/math/vector.h"
#include "engine/runtime/module.h"

namespace cave {

using StringId = std::string;
#define STR_ID(x) (x)

class InputManager : public Singleton<InputManager>,
                     public Module,
                     public MouseButtonBase {
public:
    InputManager()
        : Module("InputManager") {}

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    void SetButton(MouseButton p_button, bool p_pressed);

    void BeginFrame();
    void EndFrame();

    void PushInputHandler(IInputHandler* p_input_handler);
    IInputHandler* PopInputHandler();

    // Should only use for lua binding
    bool IsActionPressed(StringId p_name);
    bool IsActionJustPressed(StringId p_name);
    bool IsActionJustReleased(StringId p_name);

    Vector2f MouseMove();
    const Vector2f& GetCursor() const { return m_cursor; }

protected:
    bool IsKeyDown(KeyCode p_key);
    bool IsKeyPressed(KeyCode p_key);
    bool IsKeyReleased(KeyCode p_key);
    Vector2f GetWheel() const;

    void SetKey(KeyCode p_key, bool p_pressed);
    void SetCursor(float p_x, float p_y);
    void SetWheel(double p_x, double p_y);

    KeyArray m_keys;
    KeyArray m_prevKeys;

    Vector2f m_cursor{ 0, 0 };
    Vector2f m_prevCursor{ 0, 0 };

    double m_wheelX{ 0 };
    double m_wheelY{ 0 };

    bool m_mouseMoved{ false };

    InputRouter m_router;

    std::unordered_map<StringId, uint16_t> m_input_binding;

    friend class GlfwDisplayManager;
    friend class Win32DisplayManager;
};

};  // namespace cave
