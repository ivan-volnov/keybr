#include "config.h"
#include <unistd.h>
#include <pwd.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include "utility/tools.h"


Config::Config() :
    json({})
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
    if (auto conf = get_config_filepath(); std::filesystem::exists(conf)) {
        std::ifstream(conf) >> json;
    }
    else {
        json["total_phrases"] = 15;
        json["last_n_delay_revisions"] = 10;
        json["uppercase_delay_multiplier"] = 0.4;
        json["starting_symbol_delay_multiplier"] = 0.9;
        json["anki_query"] = "\"deck:Vocabulary Profile\" -is:new -is:learn -is:suspended";
        json["anki_clear_query"] = "\"deck:Vocabulary Profile\"";
        json["max_current_errors"] = 5;
        json["daily_goal"] = 25;
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

Config::~Config()
{
    try {
        std::ofstream(get_config_filepath()) << std::setw(4) << json;
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
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

std::string Config::get_config_filepath() const
{
    return get_app_path().append("config.json");
}
