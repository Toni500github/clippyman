#ifndef _EVENTDATA_HPP_
#define _EVENTDATA_HPP_

#include <string>
#include <vector>

struct CopyEvent
{
    std::vector<unsigned short> alphabet_indexes;
    std::string content;
};

struct PasteEvent
{
    std::string content;
};

#endif
