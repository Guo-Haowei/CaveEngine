#include "engine/core/dynamic_variable/dynamic_variable_begin.h"

// project
DVAR_STRING(project, DVAR_FLAG_NONE, "Open project at start", "");

// IO
DVAR_BOOL(verbose, DVAR_FLAG_NONE, "Print verbose log", true);

#include "engine/core/dynamic_variable/dynamic_variable_end.h"
