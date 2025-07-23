#pragma once
#include "engine/core/dynamic_variable/dynamic_variable_manager.h"
#include "engine/core/string/string_builder.h"
#include "engine/runtime/application.h"
#include "engine/runtime/engine.h"

#define DEFINE_DVAR
#include "engine/renderer/graphics_dvars.h"
#undef DEFINE_DVAR
#define DEFINE_DVAR
#include "engine/runtime/common_dvars.h"
#undef DEFINE_DVAR

namespace cave {

[[maybe_unused]] static constexpr const char* DVAR_CACHE_FILE = "dynamic_variables.cache";

static void RegisterCommonDvars() {
#define REGISTER_DVAR
#include "engine/runtime/common_dvars.h"
#undef REGISTER_DVAR
}

static void RegisterRenderDvars() {
#define REGISTER_DVAR
#include "engine/renderer/graphics_dvars.h"
#undef REGISTER_DVAR
}

extern Application* CreateApplication();
extern void RegisterExtraDvars();

static auto SaveCommandLine(int p_argc, const char** p_argv) {
    std::vector<std::string> command_line;
    // m_appName = p_argv[0];
    for (int i = 1; i < p_argc; ++i) {
        command_line.push_back(p_argv[i]);
    }
    return command_line;
}

int Main(int p_argc, const char** p_argv) {
    int result = 0;
    {
        engine::InitializeCore();

#if USING(ENABLE_DVAR)
        RegisterCommonDvars();
        RegisterRenderDvars();
        RegisterExtraDvars();
        DynamicVariableManager::Deserialize(DVAR_CACHE_FILE);
        // parse happens after deserialization, so command line will override cache
        DynamicVariableManager::Parse(SaveCommandLine(p_argc, p_argv));
#endif

        Application* app = CreateApplication();
        DEV_ASSERT(app);

        if (auto res = app->Initialize(); !res) {
            StringStreamBuilder builder;
            builder << res.error();
            LOG_ERROR("{}", builder.ToString());
        } else {
            Application::Run(app);
        }

        app->Finalize();
        delete app;

#if USING(ENABLE_DVAR)
        DynamicVariableManager::Serialize(DVAR_CACHE_FILE);
#endif

        engine::FinalizeCore();
    }
    return result;
}

}  // namespace cave
