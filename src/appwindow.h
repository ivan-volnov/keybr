#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <memory>

class TranslationWindow;
class MainWindow;
class Trainer;
struct _win_st;
using WINDOW = struct _win_st;


class AppScreen
{
public:
    AppScreen();
    ~AppScreen();

    void run();

    void set_sound_enabled(bool value);

private:
    void paint();

private:
    WINDOW *win;
    std::unique_ptr<TranslationWindow> translation_window;
    std::unique_ptr<MainWindow> main_window;

    std::unique_ptr<Trainer> trainer;
};

#endif // APPWINDOW_H
