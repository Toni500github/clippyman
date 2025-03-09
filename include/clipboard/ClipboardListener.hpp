#ifndef CLIPBOARD_LISTENER_HPP_
#define CLIPBOARD_LISTENER_HPP_

#include <functional>
#include <vector>

#include "EventData.hpp"

/* The base class for ClipboardListeners, Keep in mind this is not supposed to be used directly.
 * If you want a functional CClipboardListener instance, use GetAppropriateClipboardListener().
 */
class CClipboardListener
{
public:
    /*
     * Registers a callback for when the user copies something.
     */
    virtual void AddCopyCallback(const std::function<void(const CopyEvent&)>& func) = 0;

    /*
     * Poll for clipboard events, depending on the windowing system this MAY block.
     */
    virtual void PollClipboard() = 0;

    /*
     *
     */
    void fillVectorAlphabeticIndex(const std::string& clipboardContent, std::vector<unsigned short>& alphabetic_index)
    {
        alphabetic_index.resize(26);

        for (char c : clipboardContent)
        {
            if (isalpha(c) && tolower(c))
                ++alphabetic_index[c-'a'];
        }
    }

private:
    bool isalpha(const unsigned c)
    {
        return (c|32)-'a' < 26;
    }

    bool tolower(char& c)
    {
        if (c-'A' < 26)
            c |= 32;
        return true;
    }
};

#endif
