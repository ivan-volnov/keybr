#include "app.h"
#include <iostream>
#include <libs/argh.h>
#include "trainer.h"
#include "utility/tools.h"
#include "config.h"


void run(int argc, char *argv[])
{
    argh::parser cmdl(argc, argv, argh::parser::SINGLE_DASH_IS_MULTIFLAG);
    {
        auto flags = cmdl.flags();
        for (const char *flag : {"S", "stats", "s", "sound", "import", "clear_removed"}) {
            flags.erase(flag);
        }
        if (!flags.empty() || !cmdl.params().empty() || cmdl.pos_args().size() > 1) {
            std::cout << "Usage: " << std::filesystem::path(argv[0]).filename().string() << " [options]\n\n"
                         "Optional arguments:\n"
                         "-h --help               show this help message and exit\n"
                         "-S --stats              show stats and exit\n"
                         "-s --sound              read aloud the current phrase while typing\n"
                         "   --import             import cards from anki\n"
                         "   --clear_removed      remove cards that have been removed from anki deck\n"
                      << std::flush;
            return;
        }
    }
    Config::instance().set_sound_enabled(cmdl[{"s", "sound"}]);
    auto trainer = std::make_shared<Trainer>();
    if (cmdl[{"S", "stats"}]) {
        trainer->show_stats();
        return;
    }
    if (cmdl["import"]) {
        auto count = trainer->anki_import();
        std::cout << "Successfully imported " << count << " cards" << std::endl;
        return;
    }
    if (cmdl["clear_removed"]) {
        auto count = trainer->anki_clear_removed();
        std::cout << "Successfully removed " << count << " cards" << std::endl;
        return;
    }
    if (!trainer->load()) {
        std::cout << "No cards to study. Please import some" << std::endl;
        return;
    }
    {
        App app;
        app.run(trainer);
    }
    trainer->show_stats();
}


int main(int argc, char *argv[])
{
#ifndef NDEBUG
    if (tools::am_I_being_debugged()) {
        run(argc, argv);
        return 0;
    }
#endif
    try {
        run(argc, argv);
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
