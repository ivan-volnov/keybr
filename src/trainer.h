#ifndef TRAINER_H
#define TRAINER_H

#include "deck.h"
#include "speech_engine.h"
#include <chrono>



class SqliteDatabase;

class Trainer
{
public:
    Trainer();

    bool load();
    void set_sound_enabled(bool value);
    uint64_t import(const std::string &filename);
    uint64_t anki_import();

    bool process_key(int key, bool &repaint_panel);

    const Deck &get_deck() const;

private:
    uint64_t fetch(uint64_t count, bool revise = false);
    void save(Phrase &phrase);
    uint64_t count_db_phrases() const;

private:
    std::shared_ptr<SqliteDatabase> database;
    Deck deck;
    std::chrono::steady_clock::time_point key_ts{};
    std::unique_ptr<SpeechEngine> speech;
};

#endif // TRAINER_H
