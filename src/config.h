#ifndef CONFIG_H
#define CONFIG_H


#include <filesystem>

namespace argparse
{
class ArgumentParser;
}

class Config
{
public:
    static Config &instance();

    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    std::filesystem::path get_app_path() const;

    void read(argparse::ArgumentParser &program);

    bool is_sound_enabled() const;

private:
    Config();
    std::filesystem::path app_path;
    bool sound_enabled = false;
};

#endif // CONFIG_H
