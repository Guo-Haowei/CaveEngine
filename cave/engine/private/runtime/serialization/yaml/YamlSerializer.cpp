#include "YamlSerializer.h"

#include "cave/core/ids/Guid.h"

namespace cave {

ISerializer& YamlSerializer::beginArray(bool single_line) {
    IF_VALIDATE_SERIALIZER(checkEnter(SerializerState::Array));
    out_.SetSeqFormat(single_line ? YAML::Flow : YAML::Block);
    out_ << YAML::BeginSeq;
    return *this;
}

ISerializer& YamlSerializer::endArray() {
    IF_VALIDATE_SERIALIZER(checkExit(SerializerState::Array));
    out_ << YAML::EndSeq;
    return *this;
}

ISerializer& YamlSerializer::beginMap(bool single_line) {
    IF_VALIDATE_SERIALIZER(checkEnter(SerializerState::Map));
    out_.SetSeqFormat(single_line ? YAML::Flow : YAML::Block);
    out_ << YAML::BeginMap;
    return *this;
}

ISerializer& YamlSerializer::endMap() {
    IF_VALIDATE_SERIALIZER(checkExit(SerializerState::Map));
    out_ << YAML::EndMap;
    return *this;
}

ISerializer& YamlSerializer::beginKey(std::string_view key) {
    out_ << YAML::Key << key.data() << YAML::Value;
    return *this;
}

ISerializer& YamlSerializer::write(const int8_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const uint8_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const int16_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const uint16_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const int32_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const uint32_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const int64_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const uint64_t& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const bool& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const float& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const char* value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(std::string_view value) {
    out_ << value.data();
    return *this;
}

ISerializer& YamlSerializer::write(const std::string& value) {
    out_ << value;
    return *this;
}

ISerializer& YamlSerializer::write(const Guid& object) {
    return write(object.toString());
}

auto SaveYaml(std::string_view path, YamlSerializer& s) -> Result<void> {
    auto& emitter = s.emitter();

    if (!emitter.good()) {
        return CAVE_ERROR(ErrorCode::ERR_PARSE_ERROR, "error: {}", emitter.GetLastError());
    }

    auto res = FileAccess::Open(path, FileAccess::WRITE);
    if (!res) {
        return CAVE_ERROR(res.error());
    }
    auto file = *res;

    const char* string = emitter.c_str();
    const size_t len = strlen(string);
    const size_t written = file->WriteBuffer(string, len);
    DEV_ASSERT(written == len);

    return Result<void>();
}

}  // namespace cave
