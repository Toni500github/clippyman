#include <chrono>
#include <cstdlib>
#include <thread>

#include "EventData.hpp"
#include "fmt/base.h"
#include "util.hpp"

#if PLATFORM_XORG
# include "clipboard/x11/ClipboardListenerX11.hpp"
#elif PLATFORM_WAYLAND
# include "clipboard/wayland/ClipboardListenerWayland.hpp"
#endif

void CopyCallback(const CopyEvent &event) {
    fmt::println("Copied: {}", event.content);
}

int main()
{
#if PLATFORM_XORG
    CClipboardListenerX11 clipboardListener;
#elif PLATFORM_WAYLAND
    CClipboardListenerWayland clipboardListener;
#endif

    clipboardListener.AddCopyCallback(CopyCallback);

    while (true)
    {
        clipboardListener.PollClipboard();
        debug("POLLING");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
