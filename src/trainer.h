#ifndef TRAINER_H
#define TRAINER_H

#include "deck.h"
#include "utility/speech_engine.h"
#include <chrono>
#include <random>



class SqliteDatabase;

class Trainer
{
public:
    Trainer();

    bool load();
    uint64_t anki_import();
    void show_stats() const;

    bool process_key(int key, bool &repaint_panel);

    const Deck &get_deck() const;

private:
    uint64_t fetch(uint64_t count, bool revise = false);
    void load_stats(const std::vector<uint64_t> &ids);
    bool load_more_samples();
    void save(Phrase &phrase);
    uint64_t count_db_phrases() const;
    void say_current_phrase() const;

private:
    std::shared_ptr<SqliteDatabase> database;
    Deck deck;
    std::chrono::steady_clock::time_point key_ts{};
    std::unique_ptr<SpeechEngine> speech;
    std::mt19937 random_generator;
};

#endif // TRAINER_H
