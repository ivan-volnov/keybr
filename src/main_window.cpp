#include "main_window.h"
#include <ncurses.h>
#include "color_scheme.h"
#include "trainer.h"


constexpr uint64_t border_h = 3;
constexpr uint64_t border_w = 4;
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

void MainWindow::paint(const TrainerDeck &deck)
{
    const auto width = getmaxx(window);
    wclear(window);
    wmove(window, 0, 0);
    int cursor_x = -1, cursor_y = -1;
    for (int i = 0; i < deck.phrase_count(); ++i) {
        auto &phrase = deck.get_phrase(i);
        for (int j = -1; j < phrase.size(); ++j) {
            if (j < 0 && !i) {                                              // skip space before the first phrase
                continue;
            }
            if (j == 0 && phrase.size() >= width - getcurx(window)) {       // add newline after the space before current phrase
                waddch(window, '\n');
            }
            if (i == deck.get_phrase_idx() && j == deck.get_symbol_idx()) { // locate the cursor pos while phrase painting
                getyx(window, cursor_y, cursor_x);
            }
            chtype ch = phrase.get_symbol(j);
            if (const auto errors = phrase.current_errors(j); errors > 0) {
                if (ch == ' ') {
                    // paint unicode symbol ␣ instead
                    waddch(window, 0xe2);
                    waddch(window, 0x90);
                    ch = 0xa3;
                }
                ch |= COLOR_PAIR(errors > 1 ? ColorScheme::ColorMultipleErrors : ColorScheme::ColorError);
            }
            else if (cursor_x < 0) {
                ch |= COLOR_PAIR(ColorScheme::ColorGray);
            }
            waddch(window, ch);
            // TODO: wrap long phrases by words
        }
    }
    wmove(window, cursor_y, cursor_x);
    wnoutrefresh(window);
}
