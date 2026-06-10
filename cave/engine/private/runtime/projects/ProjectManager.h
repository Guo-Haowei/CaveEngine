#pragma once
#include "ProjectInfo.h"
//
//namespace cave {
//
//struct ProjectInfo {
//    std::string path;
//
//    // parsed from manifest
//    int version = 1;
//    std::string name;
//    std::string start_scene;
//    std::string thumbnail;
//};
//
//class ProjectManager {
//public:
//    void initialize();
//
//    std::span<const ProjectInfo> projects() const { return project_list_; }
//
//private:
//    auto scan(const std::filesystem::path& root) const -> std::vector<ProjectInfo>;
//    bool parse(const std::filesystem::path& path, ProjectInfo& out_info) const;
//
//    std::vector<ProjectInfo> project_list_{};
//};
//
//}  // namespace cave
