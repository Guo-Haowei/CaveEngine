#pragma once
#include "cave/core/reflection/Meta.h"

#if USING(USE_EDITOR)
#include <imgui/imgui.h>

namespace cave {

enum class AssetType : uint32_t;

template<HasEnumTraits T>
bool DrawEnumDropDown(std::string_view name, T& enum_type, float column_width) {
    bool dirty = false;
    if constexpr (!std::same_as<T, AssetType>) {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, column_width);
        ImGui::TextUnformatted(name.data());
        ImGui::NextColumn();

        constexpr int count = static_cast<int>(T::Count);
        std::vector<const char*> items;
        items.reserve(count);
        for (int i = 0; i < count; ++i) {
            items.push_back(EnumTraits<T>::s_mappings[i].data());
        }

        int selected = static_cast<int>(enum_type);
        std::string id = std::format("##{}{}", name, selected);
        if (ImGui::Combo(id.c_str(), &selected, items.data(), count)) {
            enum_type = static_cast<T>(selected);
            dirty = true;
        }

        ImGui::Columns(1);
    }

    return dirty;
}

template<typename T>
bool FieldMeta<T>::DrawEditor(void* p_object, float p_column_width) const {
    if constexpr (HasEnumTraits<T>) {
        T& enum_value = GetData<T>(p_object);
        return DrawEnumDropDown<T>(name, enum_value, p_column_width);
    } else {
        return false;
    }
}

}  // namespace cave
#endif
