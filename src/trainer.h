#ifndef TRAINER_H
#define TRAINER_H

#include "phrase.h"
#include "utility/speech_engine.h"
#include <chrono>
#include <random>



class SqliteDatabase;


class TrainerDeck
{
public:
    size_t phrase_count() const { return phrases.size(); }
    char current_symbol() const { return phrases.at(phrase_idx).get_symbol(symbol_idx); }
    const Phrase &current_phrase() const { return phrases.at(phrase_idx); }
    const Phrase &get_phrase(int64_t idx) const { return phrases.at(idx); }
    int64_t get_symbol_idx() const { return symbol_idx; }
    int64_t get_phrase_idx() const { return phrase_idx; }

protected:
    std::vector<Phrase> phrases;
    int64_t symbol_idx = 0;
    int64_t phrase_idx = 0;
};

class Trainer : public TrainerDeck
{
public:
    Trainer();

    bool load();
    uint64_t anki_import(const std::string &query);
    void show_stats() const;

    bool process_key(int key, bool &repaint_panel);

private:
    bool fetch();
    uint64_t fetch(uint64_t count, LearnStrategy strategy);
    bool load_next_exercise();
    uint64_t count_db_phrases() const;
    void say_current_phrase() const;
    uint64_t count(LearnStrategy strategy) const;

private:
    std::shared_ptr<SqliteDatabase> database;
    std::unique_ptr<SpeechEngine> speech;
    std::mt19937 random_generator;
    std::chrono::steady_clock::time_point key_ts{};
};

#endif // TRAINER_H
