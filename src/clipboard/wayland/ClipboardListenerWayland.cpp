#include <wayland-client-core.h>
#if PLATFORM_WAYLAND

#include <unistd.h>
#include <cstring>
#include <cerrno>

#include "clipboard/wayland/ClipboardListenerWayland.hpp"
#include "util.hpp"

CClipboardListenerWayland::CClipboardListenerWayland()
{
    m_display = wl_display_connect(NULL);
    if (!m_display)
        die("Failed to connect to wayland display!");

    /*m_pid = fork();
    debug("pid = {}", m_pid);

    if (m_pid < 0)
        die("Failed to fork(): {}", strerror(errno));
    else if (m_pid == 0) {
        close(STDIN_FILENO);
        main_waycopy(m_display, m_options);
    }*/
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
    debug("calling {} with m_pid = {}", __func__, m_pid);

    if (m_pid == 0)
        wl_display_roundtrip(m_display);
}

CClipboardListenerWayland::~CClipboardListenerWayland()
{
    wl_display_disconnect(m_display);
}

#endif // PLATFORM_WAYLAND
