#include <chrono>
#include <cstdlib>
#include <thread>

#include "EventData.hpp"
#include "fmt/base.h"
#include "util.hpp"

#if PLATFORM_XORG
# include "clipboard/x11/ClipboardListenerX11.hpp"
#endif

void CopyCallback(const CopyEvent &event) {
    fmt::println("Copied: {}", event.content);
}

int main()
{
#if PLATFORM_XORG
    CClipboardListenerX11 clipboardListener;

    clipboardListener.AddCopyCallback(CopyCallback);

    while (true)
    {
        clipboardListener.PollClipboard();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

#elif PLATFORM_WAYLAND
    die("Wayland currently WIP");
#endif

    return EXIT_SUCCESS;
}
