#pragma once
#include "DocumentBase.h"

namespace cave {

class MaterialDocument : public DocumentBase {
public:
    MaterialDocument(IApplication& app, const Guid& guid);
};

}  // namespace cave
