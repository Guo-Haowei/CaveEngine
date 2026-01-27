namespace cave {

class IDocument;

struct IEditCommand {
    virtual ~IEditCommand() = 0;
    virtual const char* Label() const = 0;
    virtual bool Do(IDocument& p_doc) = 0;
    virtual bool Undo(IDocument& p_doc) = 0;
    virtual bool CanCoalesceWith(IEditCommand& p_cmd) const = 0;
    virtual void CoalesceFrom(std::unique_ptr<IEditCommand>&& p_cmd) = 0;
};

}  // namespace cave
