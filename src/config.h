#ifndef CONFIG_H
#define CONFIG_H


#include <filesystem>
#include <libs/json.hpp>

class Config
{
public:
    ~Config();
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    static Config &instance();

    template<typename T>
    static T get(const std::string &key)
    {
        auto &value = instance().json[key];
        if (value.is_null()) {
            value = T{};
        }
        return value.get<T>();
    }

    template<typename T>
    static T get(const std::string &key, const std::string &inner_key)
    {
        auto &value = instance().json[key][inner_key];
        if (value.is_null()) {
            value = T{};
        }
        return value.get<T>();
    }

    template<typename T>
    static void set(const std::string &key, T &&value)
    {
        instance().json[key] = std::forward<T>(value);
    }

    template<typename T>
    static void set(const std::string &key, const std::string &inner_key, T &&value)
    {
        instance().json[key][inner_key] = std::forward<T>(value);
    }

    std::filesystem::path get_app_path() const;
    std::filesystem::path get_backup_path() const;

    std::string get_db_filepath() const;
    std::string get_backup_db_filepath() const;
    std::string get_config_filepath() const;

    bool is_sound_enabled() const;
    void set_sound_enabled(bool value);

private:
    Config();
    std::filesystem::path app_path;
    bool sound_enabled = false;
    nlohmann::json json;
};

#endif // CONFIG_H
