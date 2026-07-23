#pragma once

namespace cave {

enum class CloseDecision {
    Save,
    Discard,
    Cancel,
};

CloseDecision AskCloseUnsaved(const char* title);

}  // namespace cave
