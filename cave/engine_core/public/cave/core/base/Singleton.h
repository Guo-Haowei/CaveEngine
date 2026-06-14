// =============================================================================
// File: cave/core/base/Singleton.h
// =============================================================================
#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/error/ErrorMacros.h"

namespace cave {

template<typename T>
class Singleton : public NonCopyable {
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    Singleton() {
        DEV_ASSERT(s_singleton == nullptr);
        s_singleton = static_cast<T*>(this);
    }

    virtual ~Singleton() {
        DEV_ASSERT(s_singleton);
        s_singleton = nullptr;
    }

    static T& singleton() {
        DEV_ASSERT(s_singleton);
        return *s_singleton;
    }

    static T* singletonPtr() { return s_singleton; }

protected:
    inline static T* s_singleton = nullptr;
};

}  // namespace cave