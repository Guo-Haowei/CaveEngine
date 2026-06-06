// =============================================================================
// File: cave/runtime/framework/IInputService.h
// =============================================================================
#pragma once
#include <memory>
#include "cave/core/Singleton.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/framework/IService.h"
#include "cave/runtime/input/IInputDevice.h"

namespace cave {

struct FrameTime;
class IInputConsumer;
class KeyState;

class IInputService : public IService,
                      public Singleton<IInputService> {
public:
    IInputService(std::string_view p_name)
        : IService(p_name) {}

    virtual void Tick(const FrameTime& p_time) = 0;

    virtual void Register(IInputConsumer* p_consumer) = 0;
    virtual void Unregister(IInputConsumer* p_consumer) = 0;

    virtual void AddDevice(std::unique_ptr<IInputDevice> p_device) = 0;

    virtual const KeyState& GetKeyState() const = 0;

    virtual bool IsActionPressed(int p_player, const StringId& p_action) const = 0;

    virtual bool IsActionJustPressed(int p_player, const StringId& p_action) const = 0;

    virtual bool IsActionJustReleased(int p_player, const StringId& p_action) const = 0;

    virtual float GetActionStrength(int p_player, const StringId& p_action) const = 0;

    // Convenience overloads for single-player default
    bool IsActionPressed(const StringId& p_action) const {
        return IsActionPressed(0, p_action);
    }

    bool IsActionJustPressed(const StringId& p_action) const {
        return IsActionJustPressed(0, p_action);
    }

    bool IsActionJustReleased(const StringId& p_action) const {
        return IsActionJustReleased(0, p_action);
    }

    float GetActionStrength(const StringId& p_action) const {
        return GetActionStrength(0, p_action);
    }
};

}  // namespace cave
