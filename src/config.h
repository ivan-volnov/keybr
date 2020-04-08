#ifndef CONFIG_H
#define CONFIG_H


#include <filesystem>

class Config
{
public:
    static Config &instance();

    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    std::filesystem::path get_app_path() const;
    std::filesystem::path get_backup_path() const;

    std::string get_db_filepath() const;
    std::string get_backup_db_filepath() const;

    bool is_sound_enabled() const;
    void set_sound_enabled(bool value);

private:
    Config();
    std::filesystem::path app_path;
    bool sound_enabled = false;
};

#endif // CONFIG_H
