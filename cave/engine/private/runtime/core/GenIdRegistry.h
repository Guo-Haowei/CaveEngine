#pragma once
#include "cave/runtime/core/GenId.h"

namespace cave {

template<typename T>
class GenIdRegistry {
    struct Slot {
        uint32_t gen{ GenId<T>::kInitialGen };
        std::unique_ptr<T> storage{ nullptr };
    };

public:
    using IdT = GenId<T>;

    IdT Create(std::unique_ptr<T>&& p_data) {
        IdT id = Alloc();
        Slot& slot = m_slots[id.index];
        DEV_ASSERT(slot.storage == nullptr);
        slot.storage = std::move(p_data);
        return id;
    }

    void Destroy(IdT p_id) {
        if (!IsAlive(p_id)) {
            return;
        }
        Free(p_id);
    }

    T* Resolve(IdT p_id) {
        return IsAlive(p_id) ? m_slots[p_id.index].storage.get() : nullptr;
    }

    const T* Resolve(IdT p_id) const {
        return IsAlive(p_id) ? m_slots[p_id.index].storage.get() : nullptr;
    }

    bool IsAlive(IdT p_id) const {
        if (p_id.index >= static_cast<uint32_t>(m_slots.size())) {
            return false;
        }

        const Slot& slot = m_slots[p_id.index];
        if (slot.gen != p_id.gen) {
            return false;
        }
        return slot.storage != nullptr;
    }

protected:
    IdT Alloc() {
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

    void Free(IdT p_id) {
        Slot& slot = m_slots[p_id.index];
        ++slot.gen;
        slot.storage.reset();
        m_free.push_back(p_id.index);
    }

protected:
    std::vector<Slot> m_slots;
    std::vector<uint32_t> m_free;
};

}  // namespace cave
