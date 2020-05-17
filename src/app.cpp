#include "app.h"
#include <string_essentials/string_essentials.hpp>
#include <ncurses.h>
#include "trainer.h"



namespace ColorScheme {

constexpr auto Window       = "window";
constexpr auto Error        = "error";
constexpr auto ErrorFixed   = "error_fixed";
constexpr auto Gray         = "gray";
constexpr auto Translation  = "translation";

}



App::App()
{
    screen = std::make_shared<Screen>();
    screen->init_color(ColorScheme::Window, COLOR_BLACK, COLOR_WHITE);
    screen->init_color(ColorScheme::Error, COLOR_RED, COLOR_TRANSPARRENT);
    screen->init_color(ColorScheme::ErrorFixed, COLOR_LIGHT_YELLOW, COLOR_TRANSPARRENT);
    screen->init_color(ColorScheme::Gray, 251, COLOR_TRANSPARRENT);
    screen->init_color(ColorScheme::Translation, COLOR_BLUE, COLOR_TRANSPARRENT);
    screen->show_cursor(true);
}

void App::run(std::shared_ptr<Trainer> trainer)
{
    auto layout = screen->create<VerticalLayout>();
    layout->create<SimpleBorder>(3, 4)->create<MainWindow>(screen, std::move(trainer));
    screen->run_modal();
}



MainWindow::MainWindow(std::shared_ptr<Screen> screen, std::shared_ptr<Trainer> trainer) :
    screen_ptr(screen), trainer(trainer)
{
    leaveok(win, false);
}

void MainWindow::paint() const
{
    const auto width = getmaxx(win);
    wclear(win);
    wmove(win, 0, 0);
    struct {
        int cursor_x = -1;
        int cursor_y = -1;
        int center_x;
        int center_y;
        int start_y;
    } cur_phr;
    chtype ch;
    for (int64_t i = 0; i < trainer->phrase_count(); ++i) {
        auto &phrase = trainer->get_phrase(i);
        for (int j = -1; j < phrase.size(); ++j) {
            if (j < 0 && !i) {                                              // skip space before the first phrase
                continue;
            }
            if (j >= 0 && phrase.get_symbol(j - 1) == ' ') {                // add newline after the space before the phrase
                const auto space_index = phrase.get_phrase_text().find_first_of(' ', j);
                const auto word_len = space_index == std::string::npos ? phrase.size() : static_cast<int64_t>(space_index);
                if (word_len - j >= width - getcurx(win)) {                 // if there is no space left to render current word
                    waddch(win, '\n');
                }
            }
            if (i == trainer->get_phrase_idx()) {
                if (j == trainer->get_symbol_idx()) {                       // locate the cursor pos while phrase painting
                    getyx(win, cur_phr.cursor_y, cur_phr.cursor_x);
                }
                if (j == phrase.size() / 2) {
                    getyx(win, cur_phr.center_y, cur_phr.center_x);
                }
                if (!j) {
                    cur_phr.start_y = getcury(win);
                }
            }
            ch = phrase.get_symbol(j);
            if (ch == ' ' && (phrase.current_errors(j) > 0 || (cur_phr.cursor_x < 0 && phrase.cumulative_errors(j) > 0))) {
                // render symbol ␣ instead
                waddch(win, 0xe2);
                waddch(win, 0x90);
                ch = 0xa3;
            }
            if (phrase.current_errors(j) > 0) {
                ch |= get_color(ColorScheme::Error);
                if (phrase.current_errors(j) > 1) {
                    ch |= A_STANDOUT;
                }
            }
            else if (cur_phr.cursor_x < 0) {
                ch |= get_color(phrase.cumulative_errors(j) > 0 ? ColorScheme::ErrorFixed : ColorScheme::Gray);
            }
            waddch(win, ch);
        }
    }
    auto translation = '[' + trainer->current_phrase().get_translation() + ']';
    int tr_len = string_essentials::utf8::strlen(translation);
    if (tr_len > width) {
        const auto begin = string_essentials::utf8::next(translation.begin(), translation.end(), width - 1);
        const auto end = std::prev(translation.end());
        tr_len -= string_essentials::utf8::strlen(begin, end);
        translation.erase(begin, end);
    }
    if (cur_phr.cursor_y > cur_phr.center_y) {
        wmove(win, cur_phr.cursor_y, 0);
    }
    else if (cur_phr.cursor_y < cur_phr.center_y) {
        wmove(win, cur_phr.cursor_y, width - tr_len);
    }
    else {
        wmove(win, cur_phr.cursor_y, std::max(0, std::min(cur_phr.center_x - tr_len / 2, width - tr_len)));
    }
    winsertln(win);
    if (cur_phr.cursor_y >= cur_phr.start_y) {
        string_essentials::utf8::decoder decoder;
        for (uint8_t c : translation) {
            if (decoder.skip_symbol((ch = c))) {
                ch |= get_color(ColorScheme::Translation);
            }
            waddch(win, ch);
        }
    }
    wmove(win, cur_phr.cursor_y + 1, cur_phr.cursor_x);
    wnoutrefresh(win);
}

uint8_t MainWindow::process_key(char32_t ch, bool is_symbol)
{
    if (is_symbol) {
        if (ch == 27) { // escape
            return PleaseExitModal;
        }
        if (!trainer->process_key(ch)) {
            return PleaseExitModal;
        }
        return PleasePaint;
    }
    return 0;
}
