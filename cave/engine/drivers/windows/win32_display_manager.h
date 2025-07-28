#include "engine/drivers/windows/win32_prerequisites.h"
#include "engine/runtime/display_manager.h"

namespace cave {

class Win32DisplayManager : public IDisplayManager {
public:
    Win32DisplayManager()
        : IDisplayManager("Win32DisplayManager") {}

    void FinalizeImpl() final;

    bool ShouldClose() final;

    std::tuple<int, int> GetWindowSize() final;
    std::tuple<int, int> GetWindowPos() final;

    void BeginFrame() final;

    LRESULT WndProc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);

    HWND GetHwnd() const { return m_hwnd; }
    void* GetNativeWindow() final { return m_hwnd; }

    std::string_view GetTitle() override;
    void SetTitle(std::string_view p_title) override;

private:
    auto InitializeWindow(const WindowSpecfication& p_spec) -> Result<void> final;
    void InitializeKeyMapping() final;

    WNDCLASSEXW m_wndClass{};
    HWND m_hwnd{};
    bool m_shouldQuit{ false };
    std::string m_title;
};

}  // namespace cave
