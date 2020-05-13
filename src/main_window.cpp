#include "main_window.h"
#include <string_essentials/string_essentials.hpp>
#include <ncurses.h>
#include <sstream>
#include <iomanip>
#include "global.h"
#include "trainer.h"


constexpr int64_t border_h = 3;
constexpr int64_t border_w = 4;


MainWindow::MainWindow()
{
    int height, width;
    getmaxyx(stdscr, height, width);
    window = subwin(stdscr, height - border_h * 2, width - border_w * 2, border_h, border_w);
    wbkgd(window, COLOR_PAIR(ColorScheme::ColorWindow));
}

MainWindow::~MainWindow()
{
    delwin(window);
}

void MainWindow::resize(int height, int width)
{
    wresize(window, height - border_h * 2, width - border_w * 2);
}

void MainWindow::paint(const TrainerData &deck)
{
    const auto width = getmaxx(window);
    wclear(window);
    wmove(window, 0, 0);
    struct {
        int cursor_x = -1;
        int cursor_y = -1;
        int center_x;
        int center_y;
        int start_y;
    } cur_phr;
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
                if (word_len - j >= width - getcurx(window)) {              // if there is no space left to render current word
                    waddch(window, '\n');
                }
            }
            if (i == deck.get_phrase_idx()) {
                if (j == deck.get_symbol_idx()) {                           // locate the cursor pos while phrase painting
                    getyx(window, cur_phr.cursor_y, cur_phr.cursor_x);
                }
                if (j == phrase.size() / 2) {
                    getyx(window, cur_phr.center_y, cur_phr.center_x);
                }
                if (!j) {
                    cur_phr.start_y = getcury(window);
                }
            }
            ch = phrase.get_symbol(j);
            if (ch == ' ' && (phrase.current_errors(j) > 0 || (cur_phr.cursor_x < 0 && phrase.cumulative_errors(j) > 0))) {
                // render symbol ␣ instead
                waddch(window, 0xe2);
                waddch(window, 0x90);
                ch = 0xa3;
            }
            if (phrase.current_errors(j) > 0) {
                ch |= COLOR_PAIR(ColorScheme::ColorError);
                if (phrase.current_errors(j) > 1) {
                    ch |= A_STANDOUT;
                }
            }
            else if (cur_phr.cursor_x < 0) {
                ch |= COLOR_PAIR(phrase.cumulative_errors(j) > 0 ? ColorScheme::ColorErrorFixed
                                                                 : ColorScheme::ColorGray);
            }
            waddch(window, ch);
        }
    }
    auto translation = '[' + deck.current_phrase().get_translation() + ']';
    int tr_len = string_essentials::utf8::strlen(translation);
    if (tr_len > width) {
        const auto begin = string_essentials::utf8::next(translation.begin(), translation.end(), width - 1);
        const auto end = std::prev(translation.end());
        tr_len -= string_essentials::utf8::strlen(begin, end);
        translation.erase(begin, end);
    }
    if (cur_phr.cursor_y > cur_phr.center_y) {
        wmove(window, cur_phr.cursor_y, 0);
    }
    else if (cur_phr.cursor_y < cur_phr.center_y) {
        wmove(window, cur_phr.cursor_y, width - tr_len);
    }
    else {
        wmove(window, cur_phr.cursor_y, std::max(0, std::min(cur_phr.center_x - tr_len / 2, width - tr_len)));
    }
    winsertln(window);
    if (cur_phr.cursor_y >= cur_phr.start_y) {
        string_essentials::utf8::decoder decoder;
        for (uint8_t c : translation) {
            if (decoder.skip_symbol((ch = c))) {
                ch |= COLOR_PAIR(ColorScheme::ColorTranslation);
            }
            waddch(window, ch);
        }
    }
    wmove(window, cur_phr.cursor_y + 1, cur_phr.cursor_x);
    wnoutrefresh(window);
}
