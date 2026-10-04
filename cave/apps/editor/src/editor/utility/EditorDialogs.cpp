#include "EditorDialogs.h"

// @TODO: refactor
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

CloseDecision AskCloseUnsaved(const char* title) {
#if USING(PLATFORM_WINDOWS)
    int result = MessageBoxA(
        NULL,
        "You have unsaved changes.\n\nDo you want to save before closing?",
        title,
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON1);

    switch (result) {
        case IDYES:
            return CloseDecision::Save;
        case IDNO:
            return CloseDecision::Discard;
        default:
            return CloseDecision::Cancel;
    }
#else
    unused(title);
    return CloseDecision::Discard;
#endif
}

}  // namespace cave
