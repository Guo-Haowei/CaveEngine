#pragma once
#include <memory>

namespace cave {

class IDocument;

class IEditCmd {
public:
    virtual ~IEditCmd() = default;
    virtual const char* Label() const = 0;
    virtual bool Do(IDocument& p_doc) = 0;
    virtual bool Undo(IDocument& p_doc) = 0;
    virtual bool CanCoalesceWith(const IEditCmd* p_cmd) const = 0;
    virtual void CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) = 0;
};

}  // namespace cave
