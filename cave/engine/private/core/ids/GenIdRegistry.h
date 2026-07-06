#pragma once
#include "cave/core/ids/GenId.h"

namespace cave {

template<typename T>
class GenIdRegistry {
    struct Slot {
        uint32_t gen{ GenId<T>::kInitialGen };
        std::unique_ptr<T> storage{ nullptr };
        std::string debug_name;
    };

public:
    using IdT = GenId<T>;

    IdT create(std::unique_ptr<T>&& data) {
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

    bool replace(IdT id, std::unique_ptr<T>&& data) {
        DEV_ASSERT(data != nullptr);
        if (!isAlive(id) || !data) {
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
    std::vector<Slot> m_slots;
    std::vector<uint32_t> m_free;
};

}  // namespace cave
