#pragma once

namespace cave {

class IApplication;

class Module {
public:
    Module(std::string_view p_name)
        : m_initialized(false), m_name(p_name) {}
    virtual ~Module() = default;

    auto Initialize() -> Result<void>;
    void Finalize();

    IApplication* GetApp() { return m_app; }
    void SetApp(IApplication* p_app) { m_app = p_app; }

    std::string_view GetName() const { return m_name; }

protected:
    virtual auto InitializeImpl() -> Result<void> = 0;
    virtual void FinalizeImpl() = 0;

    bool m_initialized{ false };
    std::string_view m_name;
    IApplication* m_app;
};

template<class T>
class ModuleCreateRegistry {
public:
    using CreateFunc = T* (*)();

    static void RegisterCreateFunc(CreateFunc p_func) {
        s_createFunc = p_func;
    }

    inline static CreateFunc s_createFunc = nullptr;
};

}  // namespace cave
