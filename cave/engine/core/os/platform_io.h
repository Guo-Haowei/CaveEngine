#pragma once

namespace cave::os {

void RevealInFolder(const std::filesystem::path& p_path);

Option<std::filesystem::path> OpenFileDialog(const std::vector<const char*>& p_filters);

bool OpenSaveDialog(std::filesystem::path& p_inout_path);

}  // namespace cave::os
