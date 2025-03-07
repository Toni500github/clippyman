#if PLATFORM_WAYLAND

#include "clipboard/wayland/ClipboardListenerWayland.hpp"

#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <string>

#include "util.hpp"

const char* const tempname = "/clippyman-buffer-XXXXXX";

CClipboardListenerWayland::CClipboardListenerWayland()
{
    m_display = wl_display_connect(NULL);
    if (!m_display)
        die("Failed to connect to wayland display!");

    close(STDIN_FILENO);
    main_waycopy(m_display, m_options);

    const char *env = getenv("TMPDIR");
    if (env != NULL)
    {
        if (strlen(env) > PATH_MAX - strlen(tempname))
            die("TMPDIR has too long of a path");

        m_path = env;
    }
   
    m_path += tempname;
    int fd = mkstemp(m_path.data());
    if (fd == -1)
        die("Failed to create temporary file for copy buffer");

    main_waypaste(m_display, fd);
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

    // for checking duplicated
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
        die("temp file was deleted????");

    std::string clipboardContent;
    std::string line;
    while (std::getline(f, line))
    {
        clipboardContent += line;
        clipboardContent += "\n";
    }

    if (!clipboardContent.empty() && clipboardContent.back() == '\n')
        clipboardContent.pop_back();

    if (clipboardContent != m_LastClipboardContent && !clipboardContent.empty())
    {
        CopyEvent copyEvent{};
        copyEvent.content = clipboardContent;

        for (const auto& callback : m_CopyEventCallbacks)
            callback(copyEvent);

        m_LastClipboardContent = clipboardContent;
    }

    truncate(m_path.c_str(), 0);
}

CClipboardListenerWayland::~CClipboardListenerWayland()
{
    wl_display_disconnect(m_display);
    if (unlink(m_path.c_str()) == -1)
        warn("Failed to remove temporary file '{}", m_path);
}

#endif  // PLATFORM_WAYLAND
