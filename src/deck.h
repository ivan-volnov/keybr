#ifndef DECK_H
#define DECK_H

#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>



class Stats
{
public:
    const uint64_t cumulative_errors{};
    const std::chrono::microseconds cumulative_delay{};
    uint64_t current_errors{};
    std::chrono::microseconds current_delay{};
};



class Phrase
{
public:
    uint64_t current_errors(int64_t pos) const;
    size_t size() const;

    uint64_t id;
    std::string phrase;
    std::string translation;
    std::unordered_map<int64_t, Stats> stats;
};



class Deck
{
public:
    Phrase &current_phrase();
    const Phrase &current_phrase() const;

    bool process_key(int key, bool &repaint_panel);

private:
    void shuffle();

public:
    std::vector<Phrase> phrases;
    size_t phrase_idx = 0;
    size_t symbol_idx = 0;
};


class SqliteDatabase;

class Trainer
{
public:
    Trainer();

    void import(const std::string &filename);
    void fetch(uint32_t count, Deck &deck);

private:
    std::shared_ptr<SqliteDatabase> database;
};



#endif // DECK_H
