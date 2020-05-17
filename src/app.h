#ifndef APP_H
#define APP_H

#include <tiled_ncurses/tiled_ncurses.hpp>


class Trainer;



class App
{
public:
    App();
    void run(std::shared_ptr<Trainer> trainer);

    App(const App &) = delete;
    App &operator=(const App &) = delete;

private:
    std::shared_ptr<Screen> screen;
};



class MainWindow : public CursesWindow
{
public:
    MainWindow(std::shared_ptr<Screen> screen, std::shared_ptr<Trainer> trainer);

public:
    void paint() const override;
    bool requires_cursor() const override;
    uint8_t process_key(char32_t ch, bool is_symbol) override;

private:
    std::weak_ptr<Screen> screen_ptr;
    std::shared_ptr<Trainer> trainer;
};

#endif // APP_H
