#ifndef TRAINER_H
#define TRAINER_H

#include "deck.h"



class SqliteDatabase;

class Trainer
{
public:
    Trainer();

    void import(const std::string &filename);
    uint32_t fetch(uint32_t count);

    bool process_key(int key, bool &repaint_panel);

    const Deck &get_deck() const;

private:
    std::shared_ptr<SqliteDatabase> database;
    Deck deck;
};

#endif // TRAINER_H
