#pragma once
#include "cave/runtime/script/native/NativeScript.h"

namespace chess {

class ChessGameMode;

class MainMenu final : public cave::NativeScript {
public:
    void alwaysRun(cave::SceneCommandWriter& writer) override;
};

}  // namespace chess
