#include <chrono>

#include "example_base.h"
#include "application.h"
#include "engine/core/os/threads.h"
#include "engine/runtime/engine.h"

namespace cave {
using namespace std::chrono;

ExampleBase::ExampleBase() {}

int ExampleBase::run() {
    engine::InitializeCore();
    rs::initialize();

    m_renderTarget.create({ m_width, m_height, true, true });
    rs::setRenderTarget(&m_renderTarget);
    rs::setSize(m_width, m_height);

    postInit();

    app::initialize();

    for (;;) {
        app::pollEvents();
        if (app::shouldQuit()) {
            break;
        }

        // render
        update();
        app::drawPixels(m_width,
                        m_height,
                        m_renderTarget.getColorBuffer().getData());

    }

    thread::RequestShutdown();

    app::finalize();
    rs::finalize();
    engine::FinalizeCore();

    return 0;
}

}  // namespace cave
