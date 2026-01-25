#pragma once

namespace cave {

class IGameMode;

//class ChessMode : public IGameMode {
//public:
//    using CreatorFn = std::function<std::unique_ptr<IGameMode>()>;
//
//    bool Register(std::string_view p_id, CreatorFn p_fn);
//
//    std::unique_ptr<IGameMode> Create(std::string_view p_id) const;
//
//    void ListIds(std::vector<std::string>& p_out_ids) const;
//
//private:
//    std::unordered_map<std::string, CreatorFn> m_creators;
//};

}  // namespace cave

