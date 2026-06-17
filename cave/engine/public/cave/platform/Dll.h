#pragma once

namespace cave {

class Dll {
public:
    Dll() = default;
    ~Dll();

    bool load(const char* path);
    void unload();
    void* symbol(const char* name) const;

    Dll(const Dll&) = delete;
    Dll& operator=(const Dll&) = delete;

    Dll(Dll&&) noexcept;
    Dll& operator=(Dll&&) noexcept;

private:
    void* handle_{ nullptr };
};

}  // namespace cave
