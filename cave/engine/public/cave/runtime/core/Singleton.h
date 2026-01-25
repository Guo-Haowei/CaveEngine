#pragma once
#include <cassert>
#include "NonCopyable.h"

namespace cave {

template<typename T>
class Singleton : public NonCopyable {
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    Singleton() {
        assert(s_singleton == nullptr);
        s_singleton = static_cast<T*>(this);
    }

    virtual ~Singleton() {
        assert(s_singleton);
        s_singleton = nullptr;
    }

    static T& GetSingleton() {
        assert(s_singleton);
        return *s_singleton;
    }

    static T* GetSingletonPtr() { return s_singleton; }

protected:
    inline static T* s_singleton;
};

}  // namespace cave