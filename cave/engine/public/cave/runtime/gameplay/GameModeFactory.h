// =============================================================================
// File: engine/public/cave/runtime/gameplay/GameModeFactory.h
// =============================================================================
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace cave {

class IGameMode;

class GameModeFactory {
public:
    using CreateFn = IGameMode* (*)();
    using DestroyFn = void (*)(IGameMode*);
    using GameModeRef = std::unique_ptr<IGameMode, DestroyFn>;

    struct CreatorEntry {
        CreateFn create;
        DestroyFn destroy;
    };

    bool Register(std::string_view p_id, CreateFn p_create_fn, DestroyFn p_destroy_fn);

    GameModeRef Create(std::string_view p_id) const;

    void ListIds(std::vector<std::string>& p_out_ids) const;

private:
    std::unordered_map<std::string, CreatorEntry> m_creators;
};

}  // namespace cave
