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

private:
    Config();
    std::filesystem::path app_path;
};

#endif // CONFIG_H
