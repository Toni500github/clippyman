#ifndef CLIPBOARD_LISTENER_X11_HPP_
#define CLIPBOARD_LISTENER_X11_HPP_

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "clipboard/ClipboardListener.hpp"

class CClipboardListenerX11 : public CClipboardListener {
public:
    CClipboardListenerX11();
    ~CClipboardListenerX11();

    /*
     * Registers a callback for when the user copies something.
     */
    void AddCopyCallback(const std::function<void(const CopyEvent &)> &func) override;

    void PollClipboard() override;
private:
    std::vector<std::function<void(const CopyEvent &)>> m_CopyEventCallbacks;

    xcb_connection_t *m_XCBConnection = nullptr;

    xcb_window_t m_Window;

    int m_EventSelectionChange;

    std::string m_LastClipboardContent;

    xcb_atom_t m_Clipboard, m_UTF8String, m_ClipboardProperty;
};

#endif
