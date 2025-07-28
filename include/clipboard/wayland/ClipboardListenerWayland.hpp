#ifndef _CLIPBOARD_LISTENER_WAYLAND_HPP_
#define _CLIPBOARD_LISTENER_WAYLAND_HPP_

#ifdef __linux__

#include <string>
#include <vector>

#include "clipboard/ClipboardListener.hpp"

extern "C" {
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "clipboard/wayland/wayclip/common.h"
}

inline struct wc_options wl_options;

class CClipboardListenerWayland : public CClipboardListener
{
public:
    CClipboardListenerWayland();
    ~CClipboardListenerWayland();

    void AddCopyCallback(const std::function<void(const CopyEvent&)>& func) override;
    void PollClipboard() override;
    //    void CopyToClipboard(const std::string& str) const override;

private:
    std::vector<std::function<void(const CopyEvent&)>> m_CopyEventCallbacks;

    wl_display* m_display = nullptr;

    std::string m_path{ "/tmp" };

    unsigned int m_lastModifiedFileTime = 0;

    int m_fd = 0;

    std::string m_LastClipboardContent;
};

#endif  // __linux__
#endif  // !_CLIPBOARD_LISTENER_WAYLAND_HPP_
