#include "app_screen.h"
#include <unistd.h>
#include <iostream>
#include <sys/sysctl.h>
#include "3rdparty/argparse.hpp"
#include "trainer.h"
#include "config.h"



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


void run_app(argparse::ArgumentParser &program)
{
    Config::instance().read(program);
    auto trainer = std::make_shared<Trainer>();
    if (program["--stats"] == true) {
        trainer->show_stats();
        return;
    }
    if (program["--import"] == true) {
        std::cout << "Successfully imported " << trainer->anki_import() << " cards" << std::endl;
        return;
    }
    if (!trainer->load()) {
        std::cout << "No cards to study. Please import some" << std::endl;
        return;
    }
    {
        AppScreen app(std::move(trainer));
        app.run();
    }
    trainer->show_stats();
}


int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("keybr");
    program.add_argument("--import")
           .help("Import cards from anki")
           .default_value(false)
           .implicit_value(true);
    program.add_argument("--sound")
           .help("Read aloud the current phrase")
           .default_value(false)
           .implicit_value(true);
    program.add_argument("--stats")
           .help("Show stats")
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

    if (AmIBeingDebugged()) {
        run_app(program);
    }
    else {
        try {
            run_app(program);
        }
        catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}
