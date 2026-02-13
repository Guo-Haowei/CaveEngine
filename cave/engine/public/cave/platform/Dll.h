#pragma once

namespace cave {

class Dll {
public:
    Dll() = default;
    ~Dll();

    bool Load(const char* p_path);
    void Unload();
    void* GetSymbol(const char* p_name) const;

    Dll(const Dll&) = delete;
    Dll& operator=(const Dll&) = delete;

    Dll(Dll&&) noexcept;
    Dll& operator=(Dll&&) noexcept;

private:
    void* m_handle = nullptr;
};

}  // namespace cave
