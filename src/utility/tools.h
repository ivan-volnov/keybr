#ifndef TOOLS_H
#define TOOLS_H

#include <string>


namespace tools {

std::string weekday_to_string(uint32_t day);
void string_replace(std::string &str, const std::string &src, const std::string &dst);
void clone_file(const std::string &src, const std::string &dst);
bool am_I_being_debugged();

} // namespace tools


#endif // TOOLS_H
