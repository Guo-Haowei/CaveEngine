#pragma once
#include "engine/private/core/dynamic_variable/dynamic_variable_manager.h"
#include "engine/private/runtime/framework/Application.h"
#include "engine/private/runtime/framework/Engine.h"

#define DEFINE_DVAR
#include "engine/private/runtime/framework/CommonDvars.h"
#undef DEFINE_DVAR
#define DEFINE_DVAR
#include "engine/private/renderer/graphics_dvars.h"
#undef DEFINE_DVAR

namespace cave {

#if USING(ENABLE_DVAR)
static constexpr const char* DVAR_CACHE_FILE = "dynamic_variables.cache";

extern void RegisterExtraDvars();

static void InitializeDvars(const std::vector<std::string>& p_commands) {
    // 1) Register dvars
#define REGISTER_DVAR
#include "engine/private/runtime/framework/CommonDvars.h"
#undef REGISTER_DVAR
#define REGISTER_DVAR
#include "engine/private/renderer/graphics_dvars.h"
#undef REGISTER_DVAR
    RegisterExtraDvars();

    // 2) Deserialize dvars
    DynamicVariableManager::Deserialize(DVAR_CACHE_FILE);
    // 3) Parse from command line, so command line will override cache
    DynamicVariableManager::Parse(p_commands);
}
#define INITIALIZE_DVARS(CMD) ::cave::InitializeDvars(CMD)
#define FINALIZE_DVARS()      ::cave::DynamicVariableManager::Serialize(DVAR_CACHE_FILE)

#else
#define INITIALIZE_DVARS(...) (void)0
#define FINALIZE_DVARS()      (void)0
#endif

extern Application* CreateApp();
extern void DestroyApp(Application* p_app);

#ifdef EMPTY_APPLICATION
Application* CreateApp() { return nullptr; }
void DestroyApp(Application*) {}

void RegisterExtraDvars() {}
#endif

// @TODO: refactor this
static auto SaveCommandLine(int p_argc, const char** p_argv) {
    std::vector<std::string> command_line;
    for (int i = 1; i < p_argc; ++i) {
        command_line.push_back(p_argv[i]);
    }
    return command_line;
}

int Main(int p_argc, const char** p_argv) {
    engine::InitializeCore();
    INITIALIZE_DVARS(SaveCommandLine(p_argc, p_argv));

    ON_SCOPE_EXIT([&]() {
        FINALIZE_DVARS();
        engine::FinalizeCore();
    });

    Application* app = CreateApp();
    if (!app) {
        LOG_ERROR("Failed to create application");
        return 1;
    }

    if (auto res = app->Initialize(); !res) {
        LOG_ERROR("{}", ToString(res.error()));
        DestroyApp(app);
        return 1;
    }

    Application::Run(app);
    app->Finalize();
    DestroyApp(app);
    return 0;
}

}  // namespace cave
