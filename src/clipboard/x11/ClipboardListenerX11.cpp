#ifdef __linux__

#include "clipboard/x11/ClipboardListenerX11.hpp"

#include <dlfcn.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>

#include "EventData.hpp"
#include "config.hpp"
#include "util.hpp"

LIB_SYMBOL(xcb_intern_atom_cookie_t, xcb_intern_atom, xcb_connection_t *c,
           uint8_t only_if_exists, uint16_t name_len, const char *name);
LIB_SYMBOL(xcb_intern_atom_reply_t*, xcb_intern_atom_reply,
           xcb_connection_t *c, xcb_intern_atom_cookie_t cookie,
           xcb_generic_error_t **e);
LIB_SYMBOL(xcb_connection_t*, xcb_connect, const char*, int*);
LIB_SYMBOL(int, xcb_connection_has_error, xcb_connection_t*);
LIB_SYMBOL(xcb_setup_t*, xcb_get_setup, xcb_connection_t*);
LIB_SYMBOL(xcb_screen_iterator_t, xcb_setup_roots_iterator, const xcb_setup_t*);
LIB_SYMBOL(int32_t, xcb_generate_id, xcb_connection_t*);
LIB_SYMBOL(xcb_void_cookie_t, xcb_create_window,
           xcb_connection_t *c, uint8_t depth, xcb_window_t wid,
           xcb_window_t parent, int16_t x, int16_t y,
           uint16_t width, uint16_t height, uint16_t border_width,
           uint16_t _class, xcb_visualid_t visual,
           uint32_t value_mask, const void *value_list);
LIB_SYMBOL(int, xcb_flush, xcb_connection_t *c);
LIB_SYMBOL(void, xcb_disconnect, xcb_connection_t*);
LIB_SYMBOL(xcb_void_cookie_t, xcb_convert_selection,
           xcb_connection_t *c, xcb_window_t requestor,
           xcb_atom_t selection, xcb_atom_t target,
           xcb_atom_t property, xcb_timestamp_t time);
LIB_SYMBOL(xcb_get_property_cookie_t, xcb_get_property,
           xcb_connection_t* c, uint8_t _delete, xcb_window_t window,
           xcb_atom_t property, xcb_atom_t type, uint32_t long_offset,
           uint32_t long_length);
LIB_SYMBOL(xcb_get_property_reply_t*, xcb_get_property_reply,
           xcb_connection_t *c, xcb_get_property_cookie_t cookie,
           xcb_generic_error_t **e);
LIB_SYMBOL(void*, xcb_get_property_value,
           const xcb_get_property_reply_t *reply);
LIB_SYMBOL(xcb_generic_event_t*, xcb_wait_for_event, xcb_connection_t *c);
LIB_SYMBOL(xcb_void_cookie_t, xcb_send_event, xcb_connection_t *c, uint8_t propagate, xcb_window_t destination, uint32_t event_mask, const char *event);
LIB_SYMBOL(xcb_void_cookie_t, xcb_set_selection_owner, xcb_connection_t *c, xcb_window_t owner, xcb_atom_t selection, xcb_timestamp_t time);
LIB_SYMBOL(xcb_void_cookie_t, xcb_change_property, xcb_connection_t *c, uint8_t mode, xcb_window_t window, xcb_atom_t property, xcb_atom_t type, uint8_t format, uint32_t data_len, const void *data)

xcb_atom_t CClipboardListenerX11::getAtom(xcb_connection_t* connection, const std::string& name)
{
    xcb_intern_atom_cookie_t cookie = cf_xcb_intern_atom(connection, 0, name.size(), name.c_str());
    xcb_intern_atom_reply_t* reply  = cf_xcb_intern_atom_reply(connection, cookie, NULL);

    if (!reply)
        die("Failed to get atom with name \"{}\"", name);

    return reply->atom;
}

CClipboardListenerX11::CClipboardListenerX11()
{
    static void *m_handle = LOAD_LIBRARY("libxcb.so");
    if (!m_handle)
        die("Failed to load libxcb.so!");

    LOAD_LIB_SYMBOL(m_handle, xcb_intern_atom_cookie_t, xcb_intern_atom,
                    xcb_connection_t*, uint8_t, uint16_t, const char*);
    LOAD_LIB_SYMBOL(m_handle, xcb_intern_atom_reply_t*, xcb_intern_atom_reply,
                    xcb_connection_t*, xcb_intern_atom_cookie_t, xcb_generic_error_t**);
    LOAD_LIB_SYMBOL(m_handle, xcb_connection_t*, xcb_connect,
                    const char*, int*);
    LOAD_LIB_SYMBOL(m_handle, int, xcb_connection_has_error, xcb_connection_t*);
    LOAD_LIB_SYMBOL(m_handle, xcb_setup_t*, xcb_get_setup, xcb_connection_t*);
    LOAD_LIB_SYMBOL(m_handle, xcb_screen_iterator_t, xcb_setup_roots_iterator,
                    const xcb_setup_t*);
    LOAD_LIB_SYMBOL(m_handle, int32_t, xcb_generate_id, xcb_connection_t*);
    LOAD_LIB_SYMBOL(m_handle, xcb_void_cookie_t, xcb_create_window,
                    xcb_connection_t*, uint8_t, xcb_window_t, xcb_window_t,
                    int16_t, int16_t, uint16_t, uint16_t, uint16_t,
                    uint16_t, xcb_visualid_t, uint32_t, const void*);
    LOAD_LIB_SYMBOL(m_handle, int, xcb_flush, xcb_connection_t*);
    LOAD_LIB_SYMBOL(m_handle, void, xcb_disconnect, xcb_connection_t*);
    LOAD_LIB_SYMBOL(m_handle, xcb_void_cookie_t, xcb_convert_selection,
                    xcb_connection_t*, xcb_window_t, xcb_atom_t, xcb_atom_t,
                    xcb_atom_t, xcb_timestamp_t);
    LOAD_LIB_SYMBOL(m_handle, xcb_get_property_cookie_t, xcb_get_property,
           xcb_connection_t *c, uint8_t _delete, xcb_window_t window,
           xcb_atom_t property, xcb_atom_t type, uint32_t long_offset, uint32_t long_length);
    LOAD_LIB_SYMBOL(m_handle, xcb_get_property_reply_t*, xcb_get_property_reply,
                    xcb_connection_t*, xcb_get_property_cookie_t, xcb_generic_error_t**);
    LOAD_LIB_SYMBOL(m_handle, void*, xcb_get_property_value,
                    const xcb_get_property_reply_t*);
    LOAD_LIB_SYMBOL(m_handle, xcb_generic_event_t*, xcb_wait_for_event,
                    xcb_connection_t*);
    LOAD_LIB_SYMBOL(m_handle, xcb_void_cookie_t, xcb_send_event, xcb_connection_t *c, uint8_t propagate, xcb_window_t destination, uint32_t event_mask, const char *event);
    LOAD_LIB_SYMBOL(m_handle, xcb_void_cookie_t, xcb_change_property, xcb_connection_t *c, uint8_t mode, xcb_window_t window, xcb_atom_t property, xcb_atom_t type, uint8_t format, uint32_t data_len, const void *data);
    LOAD_LIB_SYMBOL(m_handle, xcb_void_cookie_t, xcb_set_selection_owner, xcb_connection_t *c, xcb_window_t owner, xcb_atom_t selection, xcb_timestamp_t time);

    m_XCBConnection = cf_xcb_connect(nullptr, nullptr);
    if (cf_xcb_connection_has_error(m_XCBConnection) != 0)
        die("Failed to connect to X11 display!");

    const xcb_setup_t*          setup  = cf_xcb_get_setup(m_XCBConnection);
    const xcb_screen_iterator_t iter   = cf_xcb_setup_roots_iterator(setup);
    const xcb_screen_t*         screen = iter.data;

    if (!screen)
        die("Failed to get X11 root window!");

    m_Window = cf_xcb_generate_id(m_XCBConnection);
    cf_xcb_create_window(m_XCBConnection, XCB_COPY_FROM_PARENT, m_Window, screen->root, 0, 0, 1, 1, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 0, NULL);
    cf_xcb_flush(m_XCBConnection);

    m_Clipboard         = getAtom(m_XCBConnection, config.primary_clip ? "PRIMARY" : "CLIPBOARD");
    m_UTF8String        = getAtom(m_XCBConnection, "UTF8_STRING");
    m_ClipboardProperty = getAtom(m_XCBConnection, "XCB_CLIPBOARD");
}

CClipboardListenerX11::~CClipboardListenerX11()
{
    if (m_XCBConnection)
        cf_xcb_disconnect(m_XCBConnection);
}

/*
 * Registers a callback for when the user copies something.
 */
void CClipboardListenerX11::AddCopyCallback(const std::function<void(const CopyEvent&)>& func)
{
    m_CopyEventCallbacks.push_back(func);
}

void CClipboardListenerX11::PollClipboard()
{
    /* Request the clipboard contents */
    cf_xcb_convert_selection(m_XCBConnection, m_Window, m_Clipboard, m_UTF8String, m_ClipboardProperty, XCB_CURRENT_TIME);
    cf_xcb_flush(m_XCBConnection);

    xcb_generic_event_t* event;
    if ((event = cf_xcb_wait_for_event(m_XCBConnection)))
    {
        xcb_get_property_cookie_t propertyCookie = cf_xcb_get_property(m_XCBConnection, 0, m_Window, m_ClipboardProperty,
                                                                    XCB_GET_PROPERTY_TYPE_ANY, 0, UINT16_MAX);

        xcb_generic_error_t*      error         = nullptr;
        xcb_get_property_reply_t* propertyReply = cf_xcb_get_property_reply(m_XCBConnection, propertyCookie, &error);

        if (error)
            die("Unknown libxcb error: {}", error->error_code);

        CopyEvent copyEvent{ reinterpret_cast<char*>(cf_xcb_get_property_value(propertyReply)) };

        /* Simple but fine approach */
        if (copyEvent.content == m_LastClipboardContent)
            goto end;

        if (copyEvent.content.find_first_not_of(' ') == std::string::npos)
            goto end;

        if (copyEvent.content[0] == '\0')
        {
            std::string tmp{ copyEvent.content };
            copyEvent.content.clear();
            for (char c : tmp)
            {
                if (c != '\0')
                    copyEvent.content += c;
            }

            if (copyEvent.content == m_LastClipboardContent)
                goto end;
        }

        m_LastClipboardContent = copyEvent.content;
        for (const auto& callback : m_CopyEventCallbacks)
            callback(copyEvent);

    end:
        free(propertyReply);
    }

    free(event);
}

static void runInBg(xcb_connection_t* m_XCBConnection, xcb_atom_t selection, xcb_atom_t target, xcb_atom_t property, const std::string& str)
{
    // handle selection requests in event loop
    while (true)
    {
        xcb_generic_event_t* event = cf_xcb_wait_for_event(m_XCBConnection);
        if (!event)
        {
            free(event);
            continue;
        }

        if ((event->response_type & ~0x80) == XCB_SELECTION_CLEAR)
        {
            // someone else took ownership of the clipboard, we should exit
            free(event);
            break;
        }
        else if ((event->response_type & ~0x80) == XCB_SELECTION_NOTIFY)
        {
            // check if we lost ownership
            xcb_selection_notify_event_t* notify = reinterpret_cast<xcb_selection_notify_event_t*>(event);
            if (notify->selection == selection && notify->property == XCB_NONE)
            {
                free(event);
                break;
            }
        }
        else if ((event->response_type & ~0x80) == XCB_SELECTION_REQUEST)
        {
            xcb_selection_request_event_t* request = reinterpret_cast<xcb_selection_request_event_t*>(event);
            if (request->target == target)
            {
                xcb_selection_notify_event_t notify_event = {};
                // setting the property
                cf_xcb_change_property(m_XCBConnection, XCB_PROP_MODE_REPLACE, request->requestor, property, target, 8,
                                    str.size(), str.c_str());
                notify_event.response_type = XCB_SELECTION_NOTIFY;
                notify_event.requestor     = request->requestor;
                notify_event.selection     = request->selection;
                notify_event.target        = request->target;
                notify_event.property      = property;
                notify_event.time          = request->time;

                cf_xcb_send_event(m_XCBConnection, false, request->requestor, XCB_EVENT_MASK_NO_EVENT,
                               reinterpret_cast<const char*>(&notify_event));
                cf_xcb_flush(m_XCBConnection);
                break;
            }
        }
        free(event);
    }
}

void CClipboardListenerX11::CopyToClipboard(const std::string& str) const
{
    xcb_intern_atom_cookie_t cookie_selection = cf_xcb_intern_atom(m_XCBConnection, 1, 9, "CLIPBOARD");
    xcb_intern_atom_cookie_t cookie_target    = cf_xcb_intern_atom(m_XCBConnection, 1, 11, "UTF8_STRING");
    xcb_intern_atom_cookie_t cookie_property  = cf_xcb_intern_atom(m_XCBConnection, 1, 9, "XSEL_DATA");

    xcb_intern_atom_reply_t* reply_selection = cf_xcb_intern_atom_reply(m_XCBConnection, cookie_selection, NULL);
    xcb_intern_atom_reply_t* reply_target    = cf_xcb_intern_atom_reply(m_XCBConnection, cookie_target, NULL);
    xcb_intern_atom_reply_t* reply_property  = cf_xcb_intern_atom_reply(m_XCBConnection, cookie_property, NULL);

    if (!reply_selection || !reply_target || !reply_property)
    {
        free(reply_selection);
        free(reply_target);
        free(reply_property);
        die("Failed to get reply atoms");
    }

    xcb_atom_t selection = reply_selection->atom;
    xcb_atom_t target    = reply_target->atom;
    xcb_atom_t property  = reply_property->atom;

    free(reply_selection);
    free(reply_target);
    free(reply_property);

    // set our window as the selection owner
    cf_xcb_set_selection_owner(m_XCBConnection, m_Window, selection, XCB_CURRENT_TIME);
    cf_xcb_flush(m_XCBConnection);
    if (!config.silent)
        info("Copied to clipboard! (maybe)");

    // run the task in the background, waiting for any event
    pid_t pid = fork();
    if (pid < 0)
        die("failed to fork(): {}", strerror(errno));
    else if (pid == 0)
        runInBg(m_XCBConnection, selection, target, property, str);
    else
        exit(0);
}

#endif  // __linux__
