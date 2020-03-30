#ifndef TRAINER_H
#define TRAINER_H

#include "deck.h"
#include <chrono>



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
    uint64_t fetch(uint64_t count, bool revise = false);
    void save(Phrase &phrase);

private:
    std::shared_ptr<SqliteDatabase> database;
    Deck deck;
    std::chrono::steady_clock::time_point key_ts{};
};

#endif // TRAINER_H
