#pragma once
#include "cave/core/math/Vector.h"

namespace cave::ui {

constexpr float kDefaultColumnWidth = 150.0f;

bool CheckBox(const char* name,
              bool& val,
              float column_width = kDefaultColumnWidth);

bool TextBox(const char* label,
             char* buf_ptr,
             uint32_t buf_size,
             bool enter_returns_true = true,
             float column_width = kDefaultColumnWidth,
             float text_box_width = 0);

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

}  // namespace cave::ui
