#pragma once
#include "gpu_resource.h"

namespace cave {

template<typename T>
static GpuBufferDesc CreateDesc(const std::vector<T>& p_data) {
    GpuBufferDesc desc{
        .element_size = sizeof(T),
        .element_count = static_cast<uint32_t>(p_data.size()),
        .offset = 0,
        .initial_data = p_data.data(),
    };
    return desc;
}

}  // namespace cave
