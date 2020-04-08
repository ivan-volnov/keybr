#include "translation_window.h"
#include <ncurses.h>
#include "global.h"
#include "trainer.h"

constexpr auto translation_h = 1;
constexpr auto translation_border = 1;


TranslationWindow::TranslationWindow()
{
    int height, width;
    getmaxyx(stdscr, height, width);
    window = newwin(translation_h, width - translation_border * 2, height - translation_h, translation_border);
    wbkgd(window, COLOR_PAIR(ColorScheme::ColorWindow));
}

TranslationWindow::~TranslationWindow()
{
    delwin(window);
}

void TranslationWindow::resize(int height, int width)
{
    wresize(window, translation_h, width - translation_border * 2);
    mvwin(window, height - translation_h, translation_border);
}

void TranslationWindow::paint(const TrainerDeck &deck)
{
    wclear(window);
    waddnstr(window, deck.current_phrase().get_translation().c_str(), -1);
    wnoutrefresh(window);
}
