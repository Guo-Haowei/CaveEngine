#pragma once

namespace cave {
class MetaRegistry;
}  // namespace cave

namespace cave::engine {

bool InitializeCore();

void FinalizeCore();

MetaRegistry& GetComponentRegistry();

}  // namespace cave::engine
