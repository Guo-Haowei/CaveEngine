#include "Sampler.h"

namespace cave::rs {

template<>
const Color TextureBase<Color>::sDefaultValue{ 0, 0, 0, 255 };
template<>
const float TextureBase<float>::sDefaultValue{ 1.0f };

}  // namespace cave::rs
