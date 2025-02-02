#if PLATFORM_WAYLAND

#include <wayland-client-core.h>
#include "util.hpp"

#include "clipboard/wayland/ClipboardListenerWayland.hpp"
#include "clipboard/wayland/wlr-data-control-unstable-v1.h"

CClipboardListenerWayland::CClipboardListenerWayland()
{
    m_display = wl_display_connect(NULL);
    if (!m_display)
        die("Failed to connect to wayland display!");

    m_registry = wl_display_get_registry(m_display);
    if (!m_registry)
        die("Failed to get registry!");


}

/*
 * Registers a callback for when the user copies something.
 */
void CClipboardListenerWayland::AddCopyCallback(const std::function<void(const CopyEvent&)>& func)
{
    m_CopyEventCallbacks.push_back(func);
}

void CClipboardListenerWayland::PollClipboard()
{}

CClipboardListenerWayland::~CClipboardListenerWayland()
{
    if (m_display)
        wl_display_disconnect(m_display);
}

#endif // PLATFORM_WAYLAND
