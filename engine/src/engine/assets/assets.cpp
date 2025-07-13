#include "assets.h"

#include <yaml-cpp/yaml.h>

#include "engine/core/io/file_access.h"
#include "engine/systems/serialization/serialization.h"

namespace my {

std::vector<Guid> SpriteSheetAsset::GetDependencies() const {
    return { image_id };
}

// @TODO: write to .tmp then rename, because renaming it atomic
Result<void> SpriteSheetAsset::SaveToDisk(const AssetMetaData& p_meta) const {
    // meta
    {
        auto res = p_meta.SaveToDisk(this);
        if (!res) {
            return HBN_ERROR(res.error());
        }
    }

    // file
    auto res = FileAccess::Open(p_meta.path, FileAccess::WRITE);
    if (!res) {
        return HBN_ERROR(res.error());
    }
    auto file = *res;

    serialize::SerializeYamlContext ctx;

    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "image_id" << YAML::Value << image_id.ToString();
    out << YAML::Key << "separtion" << YAML::Value;
    serialize::SerializeYaml(out, separation, ctx);
    out << YAML::Key << "offset" << YAML::Value;
    serialize::SerializeYaml(out, offset, ctx);
    out << YAML::EndMap;

    if (!out.good()) {
        return HBN_ERROR(ErrorCode::ERR_PARSE_ERROR, "error: {}", out.GetLastError());
    }

    const char* string = out.c_str();
    const size_t len = strlen(string);
    const size_t written = file->WriteBuffer(string, len);
    DEV_ASSERT(written == len);

    return Result<void>();
}

}  // namespace my
