#ifdef __linux__

#include "clipboard/wayland/ClipboardListenerWayland.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "util.hpp"

static void *m_handle;
const char* const tempname = "/clippyman-buffer-XXXXXX";

LIB_SYMBOL(wl_display*, wl_display_connect, const char *name);
LIB_SYMBOL(void, wl_display_disconnect, wl_display *display);
LIB_SYMBOL(int, wl_display_roundtrip, wl_display *display);

CClipboardListenerWayland::CClipboardListenerWayland()
{
    m_handle = LOAD_LIBRARY("libwayland-client.so");
    if (!m_handle)
        die("Failed to load Wayland libraries");

    LOAD_LIB_SYMBOL(m_handle, wl_display*, wl_display_connect, const char *name);
    LOAD_LIB_SYMBOL(m_handle, void, wl_display_disconnect, wl_display *display);
    LOAD_LIB_SYMBOL(m_handle, int, wl_display_roundtrip, wl_display *display);

    m_display = cf_wl_display_connect(NULL);
    if (!m_display)
        die("Failed to connect to wayland display!");

    const char* env = getenv("TMPDIR");
    if (env != NULL)
    {
        if (strlen(env) > PATH_MAX - strlen(tempname))
            die("TMPDIR has too long of a path");

        m_path = env;
    }

    m_path += tempname;
    m_fd = mkstemp(m_path.data());
    if (m_fd == -1)
        die("Failed to create temporary file for copy buffer");

    if (!config.arg_search)
    {
        close(STDIN_FILENO);
        main_waycopy(m_display, wl_options, STDIN_FILENO);
        main_waypaste(m_display, m_fd);
    }
}

CClipboardListenerWayland::~CClipboardListenerWayland()
{
    cf_wl_display_disconnect(m_display);
    if (unlink(m_path.c_str()) == -1)
        warn("Failed to remove temporary file '{}", m_path);
}

void CClipboardListenerWayland::AddCopyCallback(const std::function<void(const CopyEvent&)>& func)
{
    m_CopyEventCallbacks.push_back(func);
}

void CClipboardListenerWayland::PollClipboard()
{
    cf_wl_display_roundtrip(m_display);

    // for checking duplicated every 50ms
    // instead of:
    // * opening the file
    // * get each line of the file
    // * compare it with the m_LastClipboardContent
    // * do the rest
    // We could instead:
    // * check the file attributes
    // * check if the last time we checked the file is either newer or older than the current
    // * do the rest
    // Optimization moment
    /* -----------------------------------------------------
     * Benchmark           Time             CPU   Iterations
     * -----------------------------------------------------
     *  stat              844 ns          842 ns       827300
     *  fstream          2199 ns         2193 ns       318829
     */
    struct stat attrib;
    if (stat(m_path.c_str(), &attrib) != 0)
        return;
    if (attrib.st_mtim.tv_sec <= m_lastModifiedFileTime)
        return;
    m_lastModifiedFileTime = attrib.st_mtim.tv_sec;
    debug("the file is newer");

    std::ifstream f(m_path);
    if (!f.is_open())
        die("temp file was deleted");

    CopyEvent   copyEvent;
    std::string line;
    while (std::getline(f, line))
    {
        copyEvent.content += line;
        copyEvent.content += "\n";
    }

    if (!copyEvent.content.empty() && copyEvent.content.back() == '\n')
        copyEvent.content.pop_back();

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
    truncate(m_path.c_str(), 0);
}

/*void CClipboardListenerWayland::CopyToClipboard(const std::string& str) const
{
    ftruncate(m_fd, 0);
    if (write(m_fd, str.c_str(), str.size()) < 0)
        die("Failed to write to temporary file");

    info("Copied into clipboard!");
}*/

#endif
