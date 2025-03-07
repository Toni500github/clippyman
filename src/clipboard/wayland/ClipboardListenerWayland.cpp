#if PLATFORM_WAYLAND

#include "clipboard/wayland/ClipboardListenerWayland.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "util.hpp"

CClipboardListenerWayland::CClipboardListenerWayland()
{
    m_display = wl_display_connect(NULL);
    if (!m_display)
        die("Failed to connect to wayland display!");

    close(STDIN_FILENO);
    main_waycopy(m_display, m_options);

    main_waypaste(m_display);
}

/*
 * Registers a callback for when the user copies something.
 */
void CClipboardListenerWayland::AddCopyCallback(const std::function<void(const CopyEvent&)>& func)
{
    m_CopyEventCallbacks.push_back(func);
}

void CClipboardListenerWayland::PollClipboard()
{
    wl_display_roundtrip(m_display);
}

CClipboardListenerWayland::~CClipboardListenerWayland()
{
    wl_display_disconnect(m_display);
}

#endif  // PLATFORM_WAYLAND
