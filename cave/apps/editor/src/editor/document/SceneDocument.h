#pragma once
#include "DocumentBase.h"

namespace cave {

class SceneDocument : public DocumentBase {
public:
    SceneDocument(IApplication& app, const Guid& guid);

    bool save() override;
    bool saveAs(std::string_view new_path) override;
};

}  // namespace cave
