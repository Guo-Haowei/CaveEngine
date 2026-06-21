#pragma once
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class SnakeController final : public ::cave::NativeScript {
public:
    void onCreate() override;
    void onUpdate(float dt) override;

private:
    float elapsed_ = 0.0f;
};

}  // namespace super_cave_boy
