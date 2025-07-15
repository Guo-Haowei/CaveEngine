#include "serialization.h"

#include "engine/assets/guid.h"
#include "engine/core/io/file_access.h"

namespace cave {

void to_json(json& j, const Guid& p_guid) {
    j = p_guid.ToString();
}

void from_json(const json& j, Guid& p_guid) {
    auto res = Guid::Parse(j.get<std::string>());
    if (res) {
        p_guid = *res;
    }
}

auto Serialize(std::string_view p_path, const json& j) -> Result<void> {
    // if (!p_out.good()) {
    //     return HBN_ERROR(ErrorCode::ERR_PARSE_ERROR, "error: {}", p_out.GetLastError());
    // }

    auto res = FileAccess::Open(p_path, FileAccess::WRITE);
    if (!res) {
        return HBN_ERROR(res.error());
    }
    auto file = *res;

    std::string content = j.dump();
    const size_t written = file->WriteString(content);
    DEV_ASSERT(written == content.length());
    return Result<void>();
}

auto Deserialize(std::string_view p_path, json& j) -> Result<void> {
    auto res = FileAccess::Open(p_path, FileAccess::READ);
    if (!res) {
        return HBN_ERROR(res.error());
    }

    auto file = *res;

    const size_t size = file->GetLength();
    std::vector<char> buffer;
    buffer.resize(size);
    file->ReadBuffer(buffer.data(), size);
    buffer.push_back('\0');

    j = json::parse(buffer.data(), nullptr, false);
    if (j.is_discarded()) {
        return HBN_ERROR(ErrorCode::ERR_PARSE_ERROR);
    }
    return Result<void>();
}

}  // namespace cave
