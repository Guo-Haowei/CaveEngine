#include "PropertyEditors.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cave {

static constexpr VariantType kEditableVariantTypes[] = {
    VariantType::Int,
    VariantType::Float,
    VariantType::String,
    VariantType::Vec2f,
    VariantType::Vec3f,
    VariantType::Vec4f,
    VariantType::Vec2i,
    VariantType::Vec3i,
    VariantType::Vec4i,
};

static bool DrawVariantTypeCombo(const char* id, VariantType& type) {
    bool changed = false;

    std::string_view preview = EnumTraits<VariantType>::ToString(type);

    if (ImGui::BeginCombo(id, preview.data())) {
        for (VariantType candidate : kEditableVariantTypes) {
            const bool selected = candidate == type;

            // BUGFIX: use candidate, not type.
            std::string_view name = EnumTraits<VariantType>::ToString(candidate);

            if (ImGui::Selectable(name.data(), selected)) {
                type = candidate;
                changed = true;
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    return changed;
}

static bool DrawVariantValue(Variant& value) {
    switch (value.type()) {
        case VariantType::Int: {
            int v = value.asInt();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputInt("##value", &v)) {
                value = Variant(v);
                return true;
            }
            return false;
        }

        case VariantType::Float: {
            float v = value.asFloat();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputFloat("##value", &v)) {
                value = Variant(v);
                return true;
            }
            return false;
        }

        case VariantType::String: {
            char buffer[256]{};

            std::string_view text = value.asString();
            const size_t count = std::min(text.size(), sizeof(buffer) - 1);
            std::memcpy(buffer, text.data(), count);

            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputText("##value", buffer, sizeof(buffer))) {
                value = Variant(std::string_view(buffer));
                return true;
            }

            return false;
        }

        case VariantType::Vec2f: {
            math::Vec2f v = value.asVec2f();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputFloat2("##value", &v.x)) {
                value = Variant(v.x, v.y);
                return true;
            }
            return false;
        }

        case VariantType::Vec3f: {
            math::Vec3f v = value.asVec3f();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputFloat3("##value", &v.x)) {
                value = Variant(v.x, v.y, v.z);
                return true;
            }
            return false;
        }

        case VariantType::Vec4f: {
            math::Vec4f v = value.asVec4f();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputFloat4("##value", &v.x)) {
                value = Variant(v.x, v.y, v.z, v.w);
                return true;
            }
            return false;
        }

        case VariantType::Vec2i: {
            math::Vec2i v = value.asVec2i();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputInt2("##value", &v.x)) {
                value = Variant(v.x, v.y);
                return true;
            }
            return false;
        }

        case VariantType::Vec3i: {
            math::Vec3i v = value.asVec3i();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputInt3("##value", &v.x)) {
                value = Variant(v.x, v.y, v.z);
                return true;
            }
            return false;
        }

        case VariantType::Vec4i: {
            math::Vec4i v = value.asVec4i();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputInt4("##value", &v.x)) {
                value = Variant(v.x, v.y, v.z, v.w);
                return true;
            }
            return false;
        }

        default:
            ImGui::TextDisabled("<invalid>");
            return false;
    }
}

static std::string MakeUniqueVariantKey(const VariantMap& map, std::string_view base) {
    std::string key(base);

    if (!map.contains(key)) {
        return key;
    }

    for (int i = 1; i < 10000; ++i) {
        key = std::format("{}_{}", base, i);
        if (!map.contains(key)) {
            return key;
        }
    }

    return std::format("{}_{}", base, map.size());
}

bool DrawVariantMap(const char* label, VariantMap& map) {
    bool changed = false;

    const ImGuiTreeNodeFlags tree_flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (!ImGui::TreeNodeEx(label, tree_flags)) {
        return false;
    }

    std::optional<std::string> key_to_remove;
    std::vector<std::pair<std::string, std::string>> pending_renames;

    const ImGuiTableFlags table_flags =
        ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_PadOuterX;

    if (ImGui::BeginTable("##variant_map_table", 4, table_flags)) {
        ImGui::TableSetupColumn("remove", ImGuiTableColumnFlags_WidthFixed, 24.0f);
        ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("key", ImGuiTableColumnFlags_WidthFixed, 160.0f);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        int row_index = 0;

        for (auto& [key, value] : map) {
            ImGui::PushID(row_index++);

            ImGui::TableNextRow();

            // remove
            ImGui::TableSetColumnIndex(0);
            if (ImGui::SmallButton("-")) {
                key_to_remove = key;
            }

            // type
            ImGui::TableSetColumnIndex(1);
            VariantType selected_type = value.type();
            ImGui::SetNextItemWidth(-1.0f);
            if (DrawVariantTypeCombo("##type", selected_type)) {
                value = Variant();
                changed = true;
            }

            // key
            ImGui::TableSetColumnIndex(2);
            char key_buffer[128]{};
            const size_t key_count = std::min(key.size(), sizeof(key_buffer) - 1);
            std::memcpy(key_buffer, key.data(), key_count);

            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputText("##key", key_buffer, sizeof(key_buffer))) {
                std::string new_key = key_buffer;

                if (!new_key.empty() && new_key != key && !map.contains(new_key)) {
                    pending_renames.emplace_back(key, std::move(new_key));
                }
            }

            // value
            ImGui::TableSetColumnIndex(3);
            if (DrawVariantValue(value)) {
                changed = true;
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    for (const auto& [old_key, new_key] : pending_renames) {
        auto node = map.extract(old_key);
        if (!node.empty()) {
            node.key() = new_key;
            map.insert(std::move(node));
            changed = true;
        }
    }

    if (key_to_remove.has_value()) {
        map.erase(*key_to_remove);
        changed = true;
    }

    if (ImGui::SmallButton("+")) {
        map.emplace(MakeUniqueVariantKey(map, "param"), Variant(0.0f));
        changed = true;
    }

    ImGui::TreePop();

    return changed;
}

}  // namespace cave