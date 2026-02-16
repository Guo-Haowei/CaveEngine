// =============================================================================
// File: public/cave/core/containers/Iterators.h
// =============================================================================
#pragma once
#include <cassert>
#include "cave/core/typedefs.h"

namespace cave {

#if USING(DEBUG_BUILD)
constexpr inline bool IS_DEBUG_BUILD = true;
#else
constexpr inline bool IS_DEBUG_BUILD = false;
#endif

inline void assert_out_of_range() { assert(0 && "index out of range"); }

constexpr std::size_t check_out_of_range(size_t i, size_t range) {
    return i < range ? i : (assert_out_of_range(), i);
}

constexpr std::size_t check_out_of_range_if_debug(size_t i, size_t range) {
    return IS_DEBUG_BUILD ? check_out_of_range(i, range) : i;
}

template<typename T>
class LinearIterator {
    using self_type = LinearIterator<T>;

public:
    explicit LinearIterator(T* ptr)
        : m_ptr(ptr) {}

    self_type operator++(int) {
        LinearIterator<T> tmp = *this;
        ++m_ptr;
        return tmp;
    }

    self_type operator--(int) {
        self_type tmp = *this;
        --m_ptr;
        return tmp;
    }

    self_type& operator++() {
        ++m_ptr;
        return *this;
    }

    self_type& operator--() {
        --m_ptr;
        return *this;
    }

    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }

    bool operator==(const self_type& rhs) const { return m_ptr == rhs.m_ptr; }
    bool operator!=(const self_type& rhs) const { return m_ptr != rhs.m_ptr; }

private:
    T* m_ptr = nullptr;
};

template<typename T, typename Base>
class ReverseIterator {
    using ThisType = ReverseIterator<T, Base>;

public:
    explicit ReverseIterator(Base base)
        : m_base_iterator(--base) {}

    ThisType operator++(int) {
        ThisType tmp = *this;
        --m_base_iterator;
        return tmp;
    }

    ThisType& operator++() {
        --m_base_iterator;
        return *this;
    }

    T& operator*() const { return m_base_iterator.operator*(); }
    T* operator->() const { return m_base_iterator.operator->(); }

    bool operator==(const ThisType& rhs) const { return m_base_iterator == rhs.m_base_iterator; }
    bool operator!=(const ThisType& rhs) const { return m_base_iterator != rhs.m_base_iterator; }

private:
    Base m_base_iterator;
};

}  // namespace cave
