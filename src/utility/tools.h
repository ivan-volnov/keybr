#ifndef TOOLS_H
#define TOOLS_H

#include <string>


namespace tools {

std::string weekday_to_string(uint32_t day);
void clone_file(const std::string &src, const std::string &dst);
bool am_I_being_debugged();

std::string clear_string(const std::string &string);
std::string clear_string(const std::string &string, bool &changed);

} // namespace tools


#endif // TOOLS_H
