#ifndef TRANSLATIONWINDOW_H
#define TRANSLATIONWINDOW_H


struct _win_st;
using WINDOW = struct _win_st;
class Deck;


class TranslationWindow
{
public:
    TranslationWindow();
    ~TranslationWindow();

public:
    void resize(int height, int width);
    void paint(const Deck &deck);

public:
    WINDOW *window;
};

#endif // TRANSLATIONWINDOW_H
