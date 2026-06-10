#pragma once

namespace cave {

struct ProjectInfo {
    std::string path;

    // parsed from manifest
    bool is_2d = false;
    int version = 1;
    std::string name;
    std::string start_scene;
    std::string thumbnail;
};

}  // namespace cave
