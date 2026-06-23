#pragma once
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class BatController final : public ::cave::NativeScript {
public:
    void onCreate() override;
    void onUpdate(float dt) override;

private:
};

}  // namespace super_cave_boy
