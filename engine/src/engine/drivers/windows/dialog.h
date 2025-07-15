#pragma once

namespace cave {

std::string OpenFileDialog(const std::vector<const char*>& p_filters);

bool OpenSaveDialog(std::filesystem::path& p_inout_path);

}  // namespace cave
