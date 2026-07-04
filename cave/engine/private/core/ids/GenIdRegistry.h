#pragma once
#include "cave/core/ids/GenId.h"

namespace cave {

template<typename T>
class GenIdRegistry {
    struct Slot {
        uint32_t gen{ GenId<T>::kInitialGen };
        std::unique_ptr<T> storage{ nullptr };
    };

public:
    using IdT = GenId<T>;

    IdT create(std::unique_ptr<T>&& data) {
        IdT id = allocate();
        Slot& slot = slots_[id.index];
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

        slots_[id.index].storage = std::move(data);
        return true;
    }

    T* resolve(IdT id) {
        return isAlive(id) ? slots_[id.index].storage.get() : nullptr;
    }

    const T* resolve(IdT id) const {
        return isAlive(id) ? slots_[id.index].storage.get() : nullptr;
    }

    bool isAlive(IdT id) const {
        if (id.index >= static_cast<uint32_t>(slots_.size())) {
            return false;
        }

        const Slot& slot = slots_[id.index];
        if (slot.gen != id.gen) {
            return false;
        }
        return slot.storage != nullptr;
    }

protected:
    IdT allocate() {
        uint32_t index;
        if (free_.empty()) {
            index = static_cast<uint32_t>(slots_.size());
            slots_.emplace_back();
        } else {
            index = free_.back();
            free_.pop_back();
            DEV_ASSERT(slots_[index].storage == nullptr);
        }

        return { index, slots_[index].gen };
    }

    void free(IdT id) {
        Slot& slot = slots_[id.index];
        ++slot.gen;
        slot.storage.reset();
        free_.push_back(id.index);
    }

protected:
    std::vector<Slot> slots_;
    std::vector<uint32_t> free_;
};

}  // namespace cave
