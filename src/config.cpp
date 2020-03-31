#include "config.h"
#include <unistd.h>
#include <pwd.h>
#include "3rdparty/argparse.hpp"


Config::Config()
{
    const char *homedir = getenv("HOME");
    if (homedir == nullptr) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    app_path.assign(homedir).append(".keybr");
    if (!std::filesystem::exists(app_path)) {
        std::filesystem::create_directory(app_path);
    }
}

bool Config::is_sound_enabled() const
{
    return sound_enabled;
}

Config &Config::instance()
{
    static Config _instance;
    return _instance;
}

std::filesystem::path Config::get_app_path() const
{
    return app_path;
}

void Config::read(argparse::ArgumentParser &program)
{
    sound_enabled = program["--sound"] == true;
}
