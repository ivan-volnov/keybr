#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdint>


struct _win_st;
using WINDOW = struct _win_st;
class TrainerDeck;
typedef unsigned int chtype;


class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

public:
    void resize(int height, int width);
    void paint(const TrainerDeck &deck);

private:
    void paint(chtype ch, uint64_t errors, bool is_grey);

public:
    WINDOW *window;
};

#endif // MAINWINDOW_H
