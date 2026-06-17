#pragma once
#include "DocumentBase.h"

namespace cave {

class MaterialDocument : public DocumentBase {
public:
    MaterialDocument(EngineServices& services, const Guid& guid);
};

}  // namespace cave
