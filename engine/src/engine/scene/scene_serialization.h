#pragma once

namespace cave {

class Scene;

[[nodiscard]] Result<void> SaveSceneBinary(const std::string& p_path, Scene& p_scene);
[[nodiscard]] Result<void> LoadSceneBinary(const std::string& p_path, Scene& p_scene);

}  // namespace cave
