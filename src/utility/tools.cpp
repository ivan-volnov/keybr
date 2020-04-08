#include "tools.h"
#include <sys/clonefile.h>
#include <filesystem>

std::string tools::to_string(std::chrono::weekday day)
{
    switch (static_cast<unsigned int>(day))
    {
    case 0:
        return "Sunday";
    case 1:
        return "Monday";
    case 2:
        return "Tuesday";
    case 3:
        return "Wednesday";
    case 4:
        return "Thursday";
    case 5:
        return "Friday";
    case 6:
        return "Saturday";
    default:
        throw std::runtime_error("Wrong weekday value");
    }
}

void tools::string_replace(std::string &str, const std::string &src, const std::string &dst)
{
    size_t pos = 0;
    while ((pos = str.find(src, pos)) != std::string::npos) {
        str.replace(pos, src.size(), dst);
        pos += dst.size();
    }
}

void tools::clone_file(const std::string &src, const std::string &dst)
{
    if (std::filesystem::exists(dst)) {
        std::filesystem::remove(dst);
    }
    if (clonefile(src.c_str(), dst.c_str(), CLONE_NOOWNERCOPY) != 0) {
        throw std::runtime_error("Error clonefile: " + std::to_string(errno) + " from " + src + " to " + dst);
    };
}
