#pragma once
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/dvar/DvarCache.h"
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

static void InitializeDvars(int p_argc, const char** p_argv) {
    // 1) Register dvars
#define REGISTER_DVAR
#include "engine/private/runtime/framework/CommonDvars.h"
#undef REGISTER_DVAR
#define REGISTER_DVAR
#include "engine/private/renderer/graphics_dvars.h"
#undef REGISTER_DVAR
    RegisterExtraDvars();

    std::vector<std::string_view> commands;
    commands.reserve(p_argc);
    // skip executable name

    std::string cmd_args;
    for (int i = 1; i < p_argc; ++i) {
        commands.push_back(p_argv[i]);
        cmd_args.push_back(' ');
        cmd_args += p_argv[i];
    }

    LOG_INFO(LogChannel::App, "command line:{}", cmd_args);

    // 2) Deserialize dvars
    DvarCache::deserialize(DVAR_CACHE_FILE);
    // 3) Parse from command line, so command line will override cache
    DvarCache::parse(commands);
}
#define INITIALIZE_DVARS(...) ::cave::InitializeDvars(__VA_ARGS__)
#define FINALIZE_DVARS()      ::cave::DvarCache::serialize(DVAR_CACHE_FILE)

#else
#define INITIALIZE_DVARS(...) (void)0
#define FINALIZE_DVARS()      (void)0
#endif

extern IApplication* CreateApp();
extern void DestroyApp(IApplication* p_app);

#ifdef EMPTY_APPLICATION
IApplication* CreateApp() { return nullptr; }
void DestroyApp(IApplication*) {}

void RegisterExtraDvars() {}
#endif

int Main(int p_argc, const char** p_argv) {
    engine::InitializeCore();
    INITIALIZE_DVARS(p_argc, p_argv);

    ON_SCOPE_EXIT([&]() {
        FINALIZE_DVARS();
        engine::FinalizeCore();
    });

    IApplication* app = CreateApp();
    if (!app) {
        LOG_ERROR("Failed to create application");
        return 1;
    }

    if (auto res = app->initialize(); !res) {
        LOG_ERROR("{}", ToString(res.error()));
        DestroyApp(app);
        return 1;
    }

    IApplication::run(app);
    app->finalize();
    DestroyApp(app);
    return 0;
}

}  // namespace cave
