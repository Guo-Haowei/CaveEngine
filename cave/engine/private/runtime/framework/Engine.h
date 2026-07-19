#pragma once

namespace cave {
class MetaRegistry;
}  // namespace cave

namespace cave::engine {

bool InitializeCore();

void FinalizeCore();

const MetaRegistry& GetMetaRegistry();

}  // namespace cave::engine
