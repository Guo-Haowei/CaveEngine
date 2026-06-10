#pragma once

namespace cave {

struct ProjectInfo {
    std::string path;

    // parsed from manifest
    int version = 1;
    std::string name;
    std::string start_scene;
    std::string thumbnail;
};

}  // namespace cave
