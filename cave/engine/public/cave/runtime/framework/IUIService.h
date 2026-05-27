// =============================================================================
// File: engine/public/cave/runtime/framework/IUIService.h
// =============================================================================
#pragma once
#include "cave/runtime/framework/IService.h"
#include "cave/ui/UIInput.h"
#include "cave/ui/UITypes.h"

namespace cave {

class IUIService : public IService {
public:
    using IService::IService;

    virtual void BeginFrame(const UIInput& p_input) = 0;
    virtual void EndFrame() = 0;

    virtual bool Button(UIId p_id, UIRect p_rect) = 0;
};

}  // namespace cave
