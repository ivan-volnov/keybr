#include "main_window.h"
#include <ncurses.h>
#include "color_scheme.h"
#include "deck.h"


constexpr uint64_t border_h = 1;
constexpr uint64_t border_w = 2;
constexpr uint64_t translation_h = 1;


MainWindow::MainWindow()
{
    int height, width;
    getmaxyx(stdscr, height, width);
    window = newwin(height - border_h * 2 - translation_h, width - border_w * 2, border_h, border_w);
    wbkgd(window, COLOR_PAIR(ColorScheme::ColorWindow));
}

MainWindow::~MainWindow()
{
    delwin(window);
}

void MainWindow::resize(int height, int width)
{
    wresize(window, height - border_h * 2 - translation_h, width - border_w * 2);
}

void MainWindow::paint(const Deck &deck)
{
    const auto width = getmaxx(window);
    wclear(window);
    wmove(window, 0, 0);
    int cursor_x = -1, cursor_y = -1;
    for (int i = 0; i < deck.phrases.size(); ++i) {
        if (i) {
            paint(deck.phrases[i - 1].delimiter, false);
        }
        auto &phrase = deck.phrases[i];
        if (i && phrase.symbols.size() >= width - getcurx(window)) {
            waddch(window, '\n');
        }
        for (int j = 0; j < phrase.symbols.size(); ++j) {
            if (i == deck.phrase_idx && j == deck.symbol_idx) {
                getyx(window, cursor_y, cursor_x);
            }
            paint(phrase.symbols[j], cursor_x < 0);
            // TODO: wrap long phrases by words
        }
        if (i == deck.phrase_idx && deck.symbol_idx >= deck.current_phrase().symbols.size()) {
            getyx(window, cursor_y, cursor_x);
        }
    }
    wmove(window, cursor_y, cursor_x);
    wnoutrefresh(window);
}

void MainWindow::paint(const Symbol &symbol, bool is_grey)
{
    chtype ch = symbol.ch;
    if (symbol.errors) {
        if (ch == ' ') {
            waddch(window, 0xe2);
            waddch(window, 0x90);
            ch = 0xa3;
        }
        ch |= COLOR_PAIR(symbol.errors > 1 ? ColorScheme::ColorMultipleErrors : ColorScheme::ColorError);
    }
    else if (is_grey) {
        ch |= COLOR_PAIR(ColorScheme::ColorGray);
    }
    waddch(window, ch);
}
