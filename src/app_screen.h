#ifndef APP_SCREEN_H
#define APP_SCREEN_H

#include <memory>

class TranslationWindow;
class MainWindow;
class Trainer;
class TrainerDeck;
struct _win_st;
using WINDOW = struct _win_st;


class AppScreen
{
public:
    AppScreen(const std::shared_ptr<Trainer> &trainer);
    ~AppScreen();

    void run();

private:
    void init_colors();
    void paint(const TrainerDeck &deck);

private:
    WINDOW *win;
    std::unique_ptr<TranslationWindow> translation_window;
    std::unique_ptr<MainWindow> main_window;

    std::shared_ptr<Trainer> trainer;
};

#endif // APP_SCREEN_H
