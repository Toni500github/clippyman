#ifndef _EVENTDATA_HPP_
#define _EVENTDATA_HPP_

#include <string>

struct CopyEvent
{
    std::string index;
    unsigned int saved_time_secs;
    std::string content;
};

struct PasteEvent
{
    std::string content;
};

#endif
