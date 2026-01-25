#ifdef PRECOMPILED_HEADER_INCLUDED
#error "this file should only be included once"
#endif
#define PRECOMPILED_HEADER_INCLUDED

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cave/runtime/core/typedefs.h"
#include "cave/runtime/core/ErrorMacros.h"
#include "cave/runtime/core/Print.h"

// include it after error_macros.h
#include "engine/private/core/base/optional.h"
