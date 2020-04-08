#include "config.h"
#include <unistd.h>
#include <pwd.h>


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
    const auto backup_path = get_app_path().append("backup");
    if (!std::filesystem::exists(backup_path)) {
        std::filesystem::create_directory(backup_path);
    }
}

bool Config::is_sound_enabled() const
{
    return sound_enabled;
}

void Config::set_sound_enabled(bool value)
{
    sound_enabled = value;
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

std::filesystem::path Config::get_backup_path() const
{
    return get_app_path().append("backup");
}
