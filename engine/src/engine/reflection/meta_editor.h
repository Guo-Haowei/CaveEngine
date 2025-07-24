#pragma once
#include "engine/reflection/meta.h"

#if USING(USE_EDITOR)
#include <imgui/imgui.h>

namespace cave {

template<HasEnumTraits T>
bool DrawEnumDropDown(std::string_view p_name, T& p_enum, float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_name.data());
    ImGui::NextColumn();

    constexpr int count = static_cast<int>(T::Count);
    std::vector<const char*> items;
    items.reserve(count);
    for (int i = 0; i < count; ++i) {
        items.push_back(EnumTraits<T>::s_mappings[i].data());
    }

    bool dirty = false;
    int selected = static_cast<int>(p_enum);
    std::string id = std::format("##{}{}", p_name, selected);
    if (ImGui::Combo(id.c_str(), &selected, items.data(), count)) {
        p_enum = static_cast<T>(selected);
        dirty = true;
    }

    ImGui::Columns(1);
    return dirty;
}

template<typename T>
bool FieldMeta<T>::DrawEditor(void* p_object, float p_column_width) {
    if constexpr (HasEnumTraits<T>) {
        T& enum_value = GetData<T>(p_object);
        return DrawEnumDropDown<T>(name, enum_value, p_column_width);
    } else {
        return false;
    }
}

}  // namespace cave
#endif
