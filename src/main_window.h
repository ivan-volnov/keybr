#ifndef MAINWINDOW_H
#define MAINWINDOW_H


struct _win_st;
using WINDOW = struct _win_st;
class Deck;
struct Symbol;


class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

public:
    void resize(int height, int width);
    void paint(const Deck &deck);

private:
    void paint(const Symbol &symbol, bool is_grey);

public:
    WINDOW *window;
};

#endif // MAINWINDOW_H
