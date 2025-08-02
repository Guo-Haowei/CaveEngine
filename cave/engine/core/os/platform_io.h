#pragma once

namespace cave::os {

void RevealInFolder(const std::filesystem::path& p_path);

Option<std::string> OpenFileDialog(const std::vector<const char*>& p_filters);

bool OpenSaveDialog(std::filesystem::path& p_inout_path);

}  // namespace cave::os
