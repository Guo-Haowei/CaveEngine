// =============================================================================
// File: public/cave/core/ids/ViewId.h
// =============================================================================
#pragma once
#include "GenId.h"

namespace cave {

namespace internal {
class View {};
}  // namespace internal

using ViewId = GenId<internal::View>;

}  // namespace cave
