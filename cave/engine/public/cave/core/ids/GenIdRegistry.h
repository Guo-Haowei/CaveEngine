// =============================================================================
// File: cave/core/ids/GenIdRegistry.h
// =============================================================================
#pragma once
#include <string_view>

#include "cave/core/containers/Containers.h"
#include "cave/core/memory/Pointer.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/ids/GenId.h"

namespace cave {

template<typename T, typename PtrT = Owner<T>>
class GenIdRegistry {
    struct Slot {
        uint32_t gen{ GenId<T>::kInitialGen };
        PtrT storage{ nullptr };
        String debug_name;
    };

public:
    using IdT = GenId<T>;

    IdT create(PtrT&& data) {
        IdT id = allocate();
        Slot& slot = m_slots[id.index];
        DEV_ASSERT(slot.storage == nullptr);
        slot.storage = std::move(data);
        return id;
    }

    void destroy(IdT id) {
        if (!isAlive(id)) {
            return;
        }
        free(id);
    }

    bool replace(IdT id, PtrT&& data) {
        if (!data || !isAlive(id)) {
            return false;
        }

        m_slots[id.index].storage = std::move(data);
        return true;
    }

    T* resolve(IdT id) {
        return isAlive(id) ? m_slots[id.index].storage.get() : nullptr;
    }

    const T* resolve(IdT id) const {
        return isAlive(id) ? m_slots[id.index].storage.get() : nullptr;
    }

    bool isAlive(IdT id) const {
        if (id.index >= static_cast<uint32_t>(m_slots.size())) {
            return false;
        }

        const Slot& slot = m_slots[id.index];
        if (slot.gen != id.gen) {
            return false;
        }
        return slot.storage != nullptr;
    }

    std::string_view debugName(IdT id) const {
        if (DEV_VERIFY(id.index < m_slots.size())) {
            return m_slots[id.index].debug_name;
        }
        return "";
    }

protected:
    IdT allocate() {
        uint32_t index;
        if (m_free.empty()) {
            index = static_cast<uint32_t>(m_slots.size());
            m_slots.emplace_back();
        } else {
            index = m_free.back();
            m_free.pop_back();
            DEV_ASSERT(m_slots[index].storage == nullptr);
        }

        return { index, m_slots[index].gen };
    }

    void free(IdT id) {
        Slot& slot = m_slots[id.index];
        ++slot.gen;
        slot.storage.reset();
        m_free.push_back(id.index);
    }

protected:
    Vector<Slot> m_slots;
    Vector<uint32_t> m_free;
};

}  // namespace cave
