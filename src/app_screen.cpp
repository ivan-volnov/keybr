#include "app_screen.h"
#include <ncurses.h>
#include <locale.h>
#include "translation_window.h"
#include "main_window.h"
#include "global.h"
#include "trainer.h"
#include "utility/utf8_tools.h"



AppScreen::AppScreen(const std::shared_ptr<Trainer> &trainer) :
    trainer(trainer)
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
    timeout(-1);
    init_colors();
    bkgd(COLOR_PAIR(ColorScheme::ColorWindow));
    translation_window = std::make_unique<TranslationWindow>();
    main_window = std::make_unique<MainWindow>();
}

AppScreen::~AppScreen()
{
    translation_window.reset();
    main_window.reset();
    keypad(stdscr, false);
    echo();
    nocbreak();
    clear();
    endwin();
}

void AppScreen::run()
{
    paint(*trainer);
    int key, height, width;
    bool repaint_panel;
    tools::utf8::decoder decoder;
    char32_t unicode_key;
    while (true)
    {
        key = wgetch(stdscr);
        switch (key) {
        case 27: // escape
            return;
        case ERR:
            break;
        case KEY_RESIZE:
            getmaxyx(stdscr, height, width);
            if (is_term_resized(width, height)) {
                clear();
                translation_window->resize(height, width);
                main_window->resize(height, width);
                resize_term(height, width);
                paint(*trainer);
            }
            break;
        default:
            if (decoder.decode_symbol(key, unicode_key)) {
                if (!trainer->process_key(unicode_key, repaint_panel)) {
                    return;
                }
                if (repaint_panel) {
                    translation_window->paint(*trainer);
                }
                main_window->paint(*trainer);
                doupdate();
            }
            break;
        }
    }
}

void AppScreen::init_colors()
{
    start_color();
    use_default_colors();
    init_pair(ColorScheme::ColorWindow, COLOR_BLACK, COLOR_WHITE);
    init_pair(ColorScheme::ColorError, COLOR_RED, -1);
    init_pair(ColorScheme::ColorMultipleErrors, COLOR_WHITE, COLOR_RED);
    init_pair(ColorScheme::ColorGray, 251, -1); // https://jonasjacek.github.io/colors/
}

void AppScreen::paint(const TrainerData &deck)
{
    clear();
    int height, width;
    getmaxyx(stdscr, height, width);
    mvwhline(stdscr, height - 2, 1, '_', width - 2);
    wnoutrefresh(stdscr);
    translation_window->paint(deck);
    main_window->paint(deck);
    doupdate();
}
