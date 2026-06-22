#pragma once
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class SnakeController final : public ::cave::NativeScript {
public:
    void onCreate() override;
    void onUpdate(float dt) override;

private:
    float elapsed_ = 0.0f;
    int facing_x_ = -1;  // or 1
};

}  // namespace super_cave_boy
