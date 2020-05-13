#include "tools.h"
#include <sys/clonefile.h>
#include <filesystem>
#include <sys/sysctl.h>
#include <unistd.h>

std::string tools::weekday_to_string(uint32_t day)
{
    switch (day)
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

void tools::clone_file(const std::string &src, const std::string &dst)
{
    if (std::filesystem::exists(dst)) {
        std::filesystem::remove(dst);
    }
    if (clonefile(src.c_str(), dst.c_str(), CLONE_NOOWNERCOPY) != 0) {
        throw std::runtime_error("Error clonefile: " + std::to_string(errno) + " from " + src + " to " + dst);
    };
}

bool tools::am_I_being_debugged()
{
    struct kinfo_proc info;
    info.kp_proc.p_flag = 0;
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    auto size = sizeof(info);
    const bool ok = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0) == 0;
    assert(ok);
    return ok && (info.kp_proc.p_flag & P_TRACED) != 0;
}
