#include "engine/private/runtime/dvar/DvarBegin.h"

DVAR_BOOL(show_editor, DVAR_FLAG_CACHE, "Show editor", true);

DVAR_BOOL(is_world_2d, DVAR_FLAG_NONE, "Is 2D World", false);

DVAR_IVEC2(window_resolution, DVAR_FLAG_CACHE, "Request window resolution", 1920, 1080);

DVAR_STRING(last_opened_project, DVAR_FLAG_CACHE, "Last opened project path", "");

DVAR_BOOL(auto_open_last_project, DVAR_FLAG_NONE, "Automatically open last selected project", false);

#include "engine/private/runtime/dvar/DvarEnd.h"
