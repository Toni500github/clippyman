#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <string>
//#include "toml++/toml.hpp"

class Config
{
public:
    bool search = false;
    bool terminal_input = false;
    std::string path;
};

extern Config config;

#endif // _CONFIG_HPP_
