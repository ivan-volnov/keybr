#include "appwindow.h"
#include <unistd.h>
#include <iostream>
#include <sys/sysctl.h>
#include "3rdparty/argparse.hpp"
#include "trainer.h"



bool AmIBeingDebugged()
{
    struct kinfo_proc info;
    info.kp_proc.p_flag = 0;
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    auto size = sizeof(info);
    const bool ok = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0) == 0;
    assert(ok);
    return ok && (info.kp_proc.p_flag & P_TRACED) != 0;
}



int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("keybr");
    program.add_argument("--import")
           .help("Import cards from json file");
    program.add_argument("--anki_import")
           .help("Import cards from anki")
           .default_value(false)
           .implicit_value(true);
    program.add_argument("--sound")
           .help("Read aloud the current phrase")
           .default_value(false)
           .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        std::cout << program;
        return 1;
    }

    try {
        const auto filename = program.get<std::string>("--import");
        Trainer trainer;
        std::cout << "Successfully imported " << trainer.import(filename) << " cards" << std::endl;
        return 0;
    }
    catch (const std::logic_error &) {
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 0;
    }

    if (program["--anki_import"] == true) {
        try {
            Trainer trainer;
            std::cout << "Successfully imported " << trainer.anki_import() << " cards" << std::endl;
        }
        catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        return 0;
    }

    if (AmIBeingDebugged()) {
        AppScreen app;
        app.set_sound_enabled(program["--sound"] == true);
        app.run();
    }
    else {
        try {
            AppScreen app;
            app.set_sound_enabled(program["--sound"] == true);
            app.run();
        }
        catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}
