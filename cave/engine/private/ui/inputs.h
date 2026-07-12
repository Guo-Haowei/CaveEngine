#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/core/math/Vec.h"
#include "cave/core/string/StringUtils.h"

namespace cave::ui {

constexpr float kDefaultColumnWidth = 150.0f;

bool CheckBox(const char* name,
              bool& val,
              float column_width = kDefaultColumnWidth);

bool TextBox(const char* label,
             char* buf_ptr,
             uint32_t buf_cap,
             bool enter_returns_true,
             float column_width);

bool TextBox(const char* label,
             std::string& str,
             bool enter_returns_true = true,
             float column_width = kDefaultColumnWidth);

template<size_t N>
bool TextBox(const char* label,
             FixedString<N>& str,
             bool enter_returns_true = true,
             float column_width = kDefaultColumnWidth) {
    char buf[N]{};
    StringUtils::strcpy(buf, str.c_str());
    const bool dirty = TextBox(label, buf, N, enter_returns_true, column_width);
    if (dirty) {
        str.assign(buf);
    }
    return dirty;
}

bool InputInt(const char* label,
              int& out,
              float column_width = kDefaultColumnWidth);

bool InputFloat(const char* label,
                float& out,
                float column_width = kDefaultColumnWidth);

bool DragInt(const char* label,
             int& out,
             float speed,
             int min,
             int max,
             float column_width = kDefaultColumnWidth);

bool DragFloat(const char* label,
               float& out,
               float speed,
               float min,
               float max,
               float column_width = kDefaultColumnWidth);

bool Float2(const char* label,
            math::Vec2f& out,
            float reset_value = 0.0f,
            float column_width = kDefaultColumnWidth);

bool Float3(const char* label,
            math::Vec3f& out,
            float reset_value = 0.0f,
            float column_width = kDefaultColumnWidth);

bool ColorPicker3(const char* label,
                  math::Vec3f& out,
                  float column_width = kDefaultColumnWidth);

bool ColorPicker4(const char* label,
                  math::Vec4f& out,
                  float column_width = kDefaultColumnWidth);

bool ToggleButton(const char* str_id, bool& value);

bool DrawBitMask32(const char* str_id, uint32_t& value);

}  // namespace cave::ui
