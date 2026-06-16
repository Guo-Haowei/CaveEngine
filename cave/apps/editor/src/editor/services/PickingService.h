#pragma once
#include "cave/runtime/intent/IIntentHandler.h"

#include "IPickConsumer.h"

namespace cave {

struct AppServices;
struct EditorServices;

class PickingService final : public IIntentHandler {
public:
    PickingService(AppServices& app_services,
                   EditorServices& editor_services);
    ~PickingService();

    void pick(math::Vec2f point_win);

    void addConsumer(IPickConsumer* consumer);
    void removeConsumer(IPickConsumer* consumer);

    bool handleIntent(Intent& intent) override;

    DebugId debugId() const override { return debug_id_; }

private:
    void raycast(const PickData& data);

    AppServices& app_services_;
    EditorServices& editor_services_;
    const DebugId debug_id_;

    std::vector<IPickConsumer*> m_consumers;
};

}  // namespace cave
