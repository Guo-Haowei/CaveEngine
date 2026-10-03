#pragma once

namespace cave::os {

void RevealInFolder(const std::filesystem::path& path);

Option<std::filesystem::path> OpenFileDialog(const std::vector<const char*>& filters);

bool OpenSaveDialog(std::filesystem::path& inout_path);

}  // namespace cave::os
