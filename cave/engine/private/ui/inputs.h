#pragma once
#include "cave/core/math/Vector.h"

namespace cave::ui {

constexpr float DEFAULT_COLUMN_WIDTH = 150.0f;

bool CheckBox(const char* p_name,
              bool& p_val,
              float p_column_width = DEFAULT_COLUMN_WIDTH);

bool TextBox(const char* p_label,
             char* p_buf_ptr,
             uint32_t p_buf_size,
             float p_text_width = DEFAULT_COLUMN_WIDTH,
             float p_text_box_width = 0,
             bool p_enter_returns_true = true);

bool InputInt(const char* p_label,
              int& p_out,
              float p_column_width = DEFAULT_COLUMN_WIDTH);

bool InputFloat(const char* p_label,
                float& p_out,
                float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DragInt(const char* p_label,
             int& p_out,
             float p_speed,
             int p_min,
             int p_max,
             float p_column_width = DEFAULT_COLUMN_WIDTH);

bool DragFloat(const char* p_label,
               float& p_out,
               float p_speed,
               float p_min,
               float p_max,
               float p_column_width = DEFAULT_COLUMN_WIDTH);

bool Float2(const char* p_label,
            math::Vector2f& p_out,
            float p_reset_value = 0.0f,
            float p_column_width = DEFAULT_COLUMN_WIDTH);

bool Float3(const char* p_label,
            math::Vector3f& p_out,
            float p_reset_value = 0.0f,
            float p_column_width = DEFAULT_COLUMN_WIDTH);

bool ColorPicker3(const char* p_label,
                  math::Vector3f& p_out,
                  float p_column_width = DEFAULT_COLUMN_WIDTH);

bool ColorPicker4(const char* p_label,
                  math::Vector4f& p_out,
                  float p_column_width = DEFAULT_COLUMN_WIDTH);

bool ToggleButton(const char* p_str_id, bool& p_value);

}  // namespace cave::ui
