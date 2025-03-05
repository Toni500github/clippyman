#ifndef _CLIPBOARD_LISTENER_WAYLAND_HPP_
#define _CLIPBOARD_LISTENER_WAYLAND_HPP_

#if PLATFORM_WAYLAND

#include <string>
#include <vector>

#include "clipboard/ClipboardListener.hpp"

extern "C" {
    #include "clipboard/wayland/wayclip/common.h"
    #include <wayland-client.h>
    #include <wayland-client-protocol.h>
}

class CClipboardListenerWayland : public CClipboardListener
{
public:
    CClipboardListenerWayland();
    ~CClipboardListenerWayland();

    /*
     * Registers a callback for when the user copies something.
     */
    void AddCopyCallback(const std::function<void(const CopyEvent&)>& func) override;

    void PollClipboard() override;

private:
    std::vector<std::function<void(const CopyEvent&)>> m_CopyEventCallbacks;

    wl_display* m_display = nullptr;

    struct options m_options = {
        "text/plain;charset=utf-8",
        NULL,
        false,
        false
    };

    int m_EventSelectionChange;

    pid_t m_pid = 0;

    std::string m_LastClipboardContent;
};

#endif  // PLATFORM_WAYLAND

#endif // !_CLIPBOARD_LISTENER_WAYLAND_HPP_
