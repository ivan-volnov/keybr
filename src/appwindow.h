#ifndef APPWINDOW_H
#define APPWINDOW_H

#include "deck.h"

class TranslationWindow;
class MainWindow;
struct _win_st;
using WINDOW = struct _win_st;


class AppScreen
{
public:
    AppScreen();
    ~AppScreen();

    void run();

private:
    void paint();

private:
    WINDOW *win;
    std::unique_ptr<TranslationWindow> translation_window;
    std::unique_ptr<MainWindow> main_window;

    Deck deck;
    Trainer trainer;
};

#endif // APPWINDOW_H
