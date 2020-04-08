#ifndef TOOLS_H
#define TOOLS_H

#include <chrono>
#include <string>


namespace tools {

std::string to_string(std::chrono::weekday day);
void string_replace(std::string &str, const std::string &src, const std::string &dst);
void clone_file(const std::string &src, const std::string &dst);

} // namespace tools


#endif // TOOLS_H
