#pragma once

namespace cave {

struct ProjectInfo {
    std::string project_root;

    // parsed from manifest
    bool is_2d = false;
    int version = 1;
    std::string name;
    std::string thumbnail;
};

}  // namespace cave
