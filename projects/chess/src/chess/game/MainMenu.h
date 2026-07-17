#pragma once
#include "cave/runtime/script/native/NativeScript.h"

namespace chess {

class ChessGameMode;

class MainMenu final : public cave::NativeScript {
private:
    void alwaysRun(cave::SceneCommandWriter& writer) override;

    void start() override;
    void destroy() override;

private:
    uint64_t m_local_listener = 0;
    uint64_t m_online_listener = 0;
    uint64_t m_quit_listener = 0;
};

}  // namespace chess
