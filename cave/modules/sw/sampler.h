#pragma once
#include "engine/math/geomath.h"

namespace cave {

struct Color {
    uint8_t r, g, b, a;
};

template<class T>
class SwTexture {
public:
    struct CreateInfo {
        int width;
        int height;
        const void* data;
    };

    void create(const CreateInfo& info) {
        DEV_ASSERT(info.width != 0 && info.height != 0);
        m_width = info.width;
        m_height = info.height;
        m_buffer.resize(m_width * m_height);
        memcpy(&m_buffer[0], info.data, sizeof(T) * m_width * m_height);
    }

    void resize(int width, int height) {
        m_width = width;
        m_height = height;
        m_buffer.resize(m_width * m_height);
    }

    void clear(const T& clearValue) {
        std::fill(m_buffer.begin(), m_buffer.end(), clearValue);
    }

    const void* getData() const { return m_buffer.data(); }

public:
    int m_width = 0;
    int m_height = 0;
    std::vector<T> m_buffer;
};

}  // namespace cave
