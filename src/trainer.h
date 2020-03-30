#ifndef TRAINER_H
#define TRAINER_H

#include "deck.h"



class SqliteDatabase;

class Trainer
{
public:
    Trainer();

    void load();
    void import(const std::string &filename);

    bool process_key(int key, bool &repaint_panel);

    const Deck &get_deck() const;

private:
    void fetch(uint32_t count, bool revise = false);
    void save(Phrase &phrase);

private:
    std::shared_ptr<SqliteDatabase> database;
    Deck deck;
};

#endif // TRAINER_H
