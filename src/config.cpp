#include "config.h"
#include <unistd.h>
#include <pwd.h>
#include <ctime>
#include "utility/tools.h"


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
    const auto backup_path = get_backup_path();
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

std::string Config::get_db_filepath() const
{
    return get_app_path().append("keybr.db");
}

std::string Config::get_backup_db_filepath() const
{
    const auto now = std::time(nullptr);
    auto weekday = tools::weekday_to_string(std::localtime(&now)->tm_wday);
    weekday.resize(3);
    weekday[0] = std::tolower(weekday[0]);
    return get_backup_path().append("keybr_backup_" + weekday + ".db");
}
