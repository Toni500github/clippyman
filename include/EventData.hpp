#ifndef _EVENTDATA_HPP
#define _EVENTDATA_HPP

#include <string>

struct CopyEvent {
    std::string content;
};

struct PasteEvent {
    std::string content;
};

#endif
