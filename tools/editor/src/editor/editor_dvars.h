#include "engine/core/dynamic_variable/dynamic_variable_begin.h"

DVAR_STRING(last_open_scene, DVAR_FLAG_CACHE, "Last scene opened", "");

DVAR_BOOL(show_editor, DVAR_FLAG_CACHE, "Show editor", true);

DVAR_BOOL(is_world_2d, DVAR_FLAG_NONE, "Is 2D World", false);

#include "engine/core/dynamic_variable/dynamic_variable_end.h"
