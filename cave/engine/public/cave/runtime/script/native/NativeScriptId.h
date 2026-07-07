// =============================================================================
// File: cave/runtime/script/native/NativeScriptId.h
// =============================================================================
#pragma once
#include "cave/core/ids/GenId.h"

namespace cave {

class NativeScript;

struct NativeScriptId {
    uint32_t manager_id{ 0 };
    GenId<NativeScript> local_id{};

    bool valid() const {
        return manager_id != 0;
    }
};

}  // namespace cave
