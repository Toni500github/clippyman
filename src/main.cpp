#include <getopt.h>
#include <unistd.h>
#include <chrono>
#include <cstdlib>
#include <thread>

#include "EventData.hpp"
#include "fmt/base.h"
#include "util.hpp"

#include "clipboard/unix/ClipboardListenerUnix.hpp"

#if PLATFORM_XORG
# include "clipboard/x11/ClipboardListenerX11.hpp"
#elif PLATFORM_WAYLAND
# include "clipboard/wayland/ClipboardListenerWayland.hpp"
#endif

void CopyCallback(const CopyEvent &event)
{
    fmt::println("Copied: {}", event.content);
}

bool input = false;

bool parseargs(int argc, char* argv[])
{
    int opt = 0;
    int option_index = 0;
    opterr = 1; // re-enable since before we disabled for "invalid option" error
    const char *optstring = "-Vhi";

    static const struct option opts[] = {
        {"input", no_argument, 0, 'i'},
        {0,0,0,0}
    };

    optind = 0;
    while ((opt = getopt_long(argc, argv, optstring, opts, &option_index)) != -1)
    {
        switch (opt)
        {
            case 0:
                break;

            case 'i':
                input = true; break;

            default:
                return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
#if PLATFORM_XORG
    CClipboardListenerX11 clipboardListener;
    clipboardListener.AddCopyCallback(CopyCallback);
#elif PLATFORM_WAYLAND
    CClipboardListenerWayland clipboardListener;
    clipboardListener.AddCopyCallback(CopyCallback);
#endif

    CClipboardListenerUnix clipboardListenerUnix;
    clipboardListenerUnix.AddCopyCallback(CopyCallback);

    if (!parseargs(argc, argv))
        return EXIT_FAILURE;

    bool piped = isatty(fileno(stdin));
    if (piped || input)
    {
        if (!piped)
            fmt::println("Type the text to copy into clipboard, then press enter and CTRL+D");

        clipboardListenerUnix.PollClipboard();
        return EXIT_SUCCESS;
    }

#if !PLATFORM_UNIX
    while (true)
    {
        clipboardListener.PollClipboard();
        debug("POLLING");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#endif

    return EXIT_SUCCESS;
}
