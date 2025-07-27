#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "linalg.h"
#include "sampler.h"

namespace cave::rs {

void loadTexture(Texture& texture, const char* path);

}  // namespace cave::rs
