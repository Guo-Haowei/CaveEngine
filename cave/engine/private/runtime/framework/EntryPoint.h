#pragma once
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/variant/DvarTable.h"
#include "engine/private/runtime/framework/Engine.h"

#define DEFINE_DVAR
#include "engine/private/runtime/framework/CommonDvars.h"
#undef DEFINE_DVAR
#define DEFINE_DVAR
#include "engine/private/renderer/graphics_dvars.h"
#undef DEFINE_DVAR

namespace cave {

extern IApplication* CreateApp();
extern void DestroyApp(IApplication* app);

#if USING(ENABLE_DVAR)
extern void RegisterExtraDvars();

static constexpr const char* kDvarCacheFile = "dynamic_variables.cache";

static void InitializeDvars(int argc, const char** argv) {
    // 1) Register dvars
#define REGISTER_DVAR
#include "engine/private/runtime/framework/CommonDvars.h"
#undef REGISTER_DVAR
#define REGISTER_DVAR
#include "engine/private/renderer/graphics_dvars.h"
#undef REGISTER_DVAR
    RegisterExtraDvars();

    Vector<std::string_view> commands;
    commands.reserve(argc);
    // skip executable name

    std::string cmd_args;
    for (int i = 1; i < argc; ++i) {
        commands.push_back(argv[i]);
        cmd_args.push_back(' ');
        cmd_args += argv[i];
    }

    LOG_INFO(LogChannel::App, "command line:{}", cmd_args);

    // 2) Deserialize dvars
    DvarTable::global().deserialize(kDvarCacheFile);
    // 3) Parse from command line, so command line will override cache
    DvarTable::global().parse(commands);
}
#endif

#ifdef EMPTY_APPLICATION
IApplication* CreateApp() { return nullptr; }
void DestroyApp(IApplication*) {}

void RegisterExtraDvars() {}
#endif

int Main(int argc, const char** argv) {
    engine::InitializeCore();
    InitializeDvars(argc, argv);

    IApplication* app = CreateApp();
    if (!app) {
        LOG_ERROR("Failed to create application");

        engine::FinalizeCore();
        return 1;
    }

    if (auto res = app->initialize(); !res) {
        LOG_ERROR("{}", ToString(res.error()));

        DestroyApp(app);
        DvarTable::global().serialize(kDvarCacheFile);
        engine::FinalizeCore();
        return 1;
    }

    IApplication::run(app);
    app->finalize();
    DestroyApp(app);

    DvarTable::global().serialize(kDvarCacheFile);
    engine::FinalizeCore();
    return 0;
}

}  // namespace cave
