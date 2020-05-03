#include "main_window.h"
#include <ncurses.h>
#include "global.h"
#include "trainer.h"
#include "utility/utf8_tools.h"


constexpr int64_t border_h = 3;
constexpr int64_t border_w = 4;
constexpr int64_t translation_h = 1;
constexpr int64_t translation_border = 1;
constexpr int64_t line_spacing = 1;


MainWindow::MainWindow()
{
    int height, width;
    getmaxyx(stdscr, height, width);
    window = subwin(stdscr, height - border_h * 2 - translation_h, width - border_w, border_h, border_w);
    stats_window = subwin(stdscr, translation_h, width - translation_border * 2, height - translation_h, translation_border);
    wbkgd(window, COLOR_PAIR(ColorScheme::ColorWindow));
    wbkgd(stats_window, COLOR_PAIR(ColorScheme::ColorWindow));
}

MainWindow::~MainWindow()
{
    delwin(stats_window);
    delwin(window);
}

void MainWindow::resize(int height, int width)
{
    wresize(window, height - border_h * 2 - translation_h, width - border_w);
    wresize(stats_window, translation_h, width - translation_border * 2);
    mvwin(stats_window, height - translation_h, translation_border);
}

void MainWindow::paint(const TrainerData &deck)
{
    const auto width = getmaxx(window);
    wclear(window);
    wmove(window, 0, 0);
    int cursor_x = -1, cursor_y = -1;
    int hint_x = -1, hint_y = -1;
    chtype ch;
    for (int64_t i = 0; i < deck.phrase_count(); ++i) {
        auto &phrase = deck.get_phrase(i);
        for (int j = -1; j < phrase.size(); ++j) {
            if (j < 0 && !i) {                                              // skip space before the first phrase
                continue;
            }
            if (j >= 0 && phrase.get_symbol(j - 1) == ' ') {                // add newline after the space before the phrase
                const auto space_index = phrase.get_phrase_text().find_first_of(' ', j);
                const auto word_len = space_index == std::string::npos ? phrase.size() : static_cast<int64_t>(space_index);
                if (word_len - j >= width - border_w - getcurx(window)) {   // if there is no space left to render current word
                    waddch(window, '\n');
                }
            }
            if (!getcurx(window)) {
                for (int k = 0; k < line_spacing; ++k) {
                    waddch(window, '\n');
                }
            }
            if (i == deck.get_phrase_idx()) {
                if (j == deck.get_symbol_idx()) {                           // locate the cursor pos while phrase painting
                    getyx(window, cursor_y, cursor_x);
                }
                if (j == 0) {
                    getyx(window, hint_y, hint_x);
                }
            }
            ch = phrase.get_symbol(j);
            if (const auto errors = phrase.current_errors(j); errors > 0) {
                if (ch == ' ') {
                    // render symbol ␣ instead
                    waddch(window, 0xe2);
                    waddch(window, 0x90);
                    ch = 0xa3;
                }
                ch |= COLOR_PAIR(ColorScheme::ColorError);
                if (errors > 1) {
                    ch |= A_STANDOUT;
                }
            }
            else if (cursor_x < 0) {
                ch |= COLOR_PAIR(ColorScheme::ColorGray);
            }
            waddch(window, ch);
        }
    }
    tools::utf8::decoder decoder;
    wmove(window, --hint_y, hint_x);
    int len = 0;
    for (uint8_t c : deck.current_phrase().get_translation()) {
        if (decoder.decode_symbol((ch = c))) {
            ch |= COLOR_PAIR(ColorScheme::ColorTranslation);
            ++len;
        }
        waddch(window, ch);
        if (len > width - hint_x - 1) {
            hint_y += 1 + line_spacing;
            hint_x = 0;
            len = 0;
            wmove(window, hint_y, hint_x);
        }
    }
    wmove(window, cursor_y, cursor_x);
    wnoutrefresh(window);
}

void MainWindow::paint_stats(const TrainerData &deck)
{
    const auto width = getmaxx(window);
    wclear(stats_window);
    waddnstr(stats_window, "stats:", width - translation_border * 2);
    wnoutrefresh(stats_window);
}
