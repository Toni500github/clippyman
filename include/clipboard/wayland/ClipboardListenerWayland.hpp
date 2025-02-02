#ifndef CLIPBOARD_LISTENER_WAYLAND_HPP_
#define CLIPBOARD_LISTENER_WAYLAND_HPP_

#if PLATFORM_WAYLAND

#include <wayland-client.h>
#include <wayland-client-protocol.h>

#include "clipboard/ClipboardListener.hpp"

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

    wl_registry* m_registry = nullptr;

    int m_EventSelectionChange;

    std::string m_LastClipboardContent;
};

#endif  // PLATFORM_WAYLAND

#endif // !CLIPBOARD_LISTENER_WAYLAND_HPP_
