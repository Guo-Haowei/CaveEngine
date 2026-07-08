#ifdef PRECOMPILED_HEADER_INCLUDED
#error "this file should only be included once"
#endif
#define PRECOMPILED_HEADER_INCLUDED

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <span>
#include <thread>
#include <type_traits>

#include "cave/core/containers/Containers.h"
#include "cave/core/diagnostics/Log.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/typedefs.h"
#include "cave/core/math/Utils.h"
#include "cave/core/Option.h"
