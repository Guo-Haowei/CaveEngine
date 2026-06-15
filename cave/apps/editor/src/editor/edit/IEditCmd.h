#pragma once
#include <memory>

namespace cave {

class IDocument;

class IEditCmd {
public:
    virtual ~IEditCmd() = default;
    virtual const char* label() const = 0;
    virtual bool apply(IDocument& doc) = 0;
    virtual bool undo(IDocument& doc) = 0;
    virtual bool canCoalesceWith(const IEditCmd* cmd) const = 0;
    virtual void coalesceFrom(std::unique_ptr<IEditCmd> cmd) = 0;
};

}  // namespace cave
