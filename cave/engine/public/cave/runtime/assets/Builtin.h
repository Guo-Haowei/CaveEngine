// =============================================================================
// File: cave/runtime/assets/Builtin.h
// =============================================================================
#pragma once
#include "cave/core/ids/Guid.h"

namespace cave {

struct CheckerboardInfo {
    static constexpr uint32_t kCheckerCountPerAxis = 4;
    static constexpr uint32_t kCellSizePx = 32;
    static constexpr uint32_t kTextureSizePx = kCheckerCountPerAxis * kCellSizePx;
};

namespace builtin {

constexpr const char GUID1[] = "00000000-0000-0000-0000000000000001";
constexpr const char GUID2[] = "00000000-0000-0000-0000000000000002";
constexpr const char GUID3[] = "00000000-0000-0000-0000000000000003";
constexpr const char GUID4[] = "00000000-0000-0000-0000000000000004";
constexpr const char GUID5[] = "00000000-0000-0000-0000000000000005";
constexpr const char GUID6[] = "00000000-0000-0000-0000000000000006";
constexpr const char GUID7[] = "00000000-0000-0000-0000000000000007";
constexpr const char GUID8[] = "00000000-0000-0000-0000000000000008";
constexpr const char GUID9[] = "00000000-0000-0000-0000000000000009";
constexpr const char GUID10[] = "00000000-0000-0000-0000000000000010";
constexpr const char GUID11[] = "00000000-0000-0000-0000000000000011";
constexpr const char GUID12[] = "00000000-0000-0000-0000000000000012";
constexpr const char GUID13[] = "00000000-0000-0000-0000000000000013";
constexpr const char GUID14[] = "00000000-0000-0000-0000000000000014";
constexpr const char GUID15[] = "00000000-0000-0000-0000000000000015";
constexpr const char GUID16[] = "00000000-0000-0000-0000000000000016";
constexpr const char GUID17[] = "00000000-0000-0000-0000000000000017";
constexpr const char GUID18[] = "00000000-0000-0000-0000000000000018";
constexpr const char GUID19[] = "00000000-0000-0000-0000000000000019";
constexpr const char GUID20[] = "00000000-0000-0000-0000000000000020";

}  // namespace builtin

}  // namespace cave
