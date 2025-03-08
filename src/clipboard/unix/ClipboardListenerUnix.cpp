#include "clipboard/unix/ClipboardListenerUnix.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "util.hpp"

std::string in()
{
    if (feof(stdin))
        std::exit(0);

    char characters[UINT16_MAX] = { 0 };
    while (fgets(characters, sizeof(characters), stdin) != NULL)
        debug("WE ARE READING");

    debug("chars = {}", characters);

    std::string ret{characters};
    memset(characters, 0, sizeof(characters));

    if (!ret.empty() && ret.back() == '\n')
        ret.pop_back();
    return ret;
}

/*
 * Registers a callback for when the user copies something.
 */
void CClipboardListenerUnix::AddCopyCallback(const std::function<void(const CopyEvent&)>& func)
{
    m_CopyEventCallbacks.push_back(func);
}

void CClipboardListenerUnix::PollClipboard()
{
    std::string clipboardContent{in()};
    if (clipboardContent == m_LastClipboardContent)
        return;

    CopyEvent copyEvent{};
    copyEvent.content = clipboardContent;
    size_t pos = clipboardContent.find_first_not_of(' ');
    if (pos != clipboardContent.npos)
        copyEvent.index = std::toupper(clipboardContent.at(pos));
    else
        copyEvent.index = "Other";

    for (const auto& callback : m_CopyEventCallbacks)
        callback(copyEvent);

    m_LastClipboardContent = clipboardContent;
}
