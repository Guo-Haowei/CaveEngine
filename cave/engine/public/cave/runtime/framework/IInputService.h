// =============================================================================
// File: engine/public/cave/runtime/framework/IInputService.h
// =============================================================================
#pragma once
#include "cave/runtime/framework/IService.h"

namespace cave {

class IInputService : public IService {
public:
    IInputService(std::string_view p_name)
        : IService(p_name) {}
};

}  // namespace cave
