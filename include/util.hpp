#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include "fmt/base.h"
#include "fmt/color.h"

#define BOLD_COLOR(x) (fmt::emphasis::bold | fmt::fg(x))

template <typename... Args>
void error(const std::string_view fmt, Args&&... args) noexcept
{
    fmt::print(stderr, BOLD_COLOR(fmt::rgb(fmt::color::red)), "ERROR\033[0m: {}\n",
               fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
}

template <typename... Args>
void die(const std::string_view fmt, Args&&... args) noexcept
{
    fmt::print(stderr, BOLD_COLOR(fmt::rgb(fmt::color::red)), "FATAL\033[0m: {}\n",
               fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
    std::exit(1);
}

template <typename... Args>
void debug(const std::string_view fmt, Args&&... args) noexcept
{
#if DEBUG
    fmt::print(BOLD_COLOR((fmt::rgb(fmt::color::hot_pink))), "[DEBUG]: {}\n",
               fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
#endif
}

template <typename... Args>
void warn(const std::string_view fmt, Args&&... args) noexcept
{
    fmt::print(BOLD_COLOR((fmt::rgb(fmt::color::yellow))), "WARNING\033[0m: {}\n",
               fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
}

template <typename... Args>
void info(const std::string_view fmt, Args&&... args) noexcept
{
    fmt::print(BOLD_COLOR((fmt::rgb(fmt::color::cyan))), "INFO\033[0m: {}\n",
               fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
}

#endif  // !_UTIL_HPP_
