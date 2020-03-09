#ifndef DECK_H
#define DECK_H

#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>



class Phrase
{
    struct Stats
    {
        const uint64_t cumulative_errors{};
        const std::chrono::microseconds cumulative_delay{};
        uint64_t current_errors{};
        std::chrono::microseconds current_delay{};
    };

public:
    Phrase(uint64_t id, const std::string &phrase, const std::string &translation);
    size_t size() const;

    uint64_t current_errors(int64_t pos) const;
    char get_symbol(int64_t pos) const;
    const std::string &get_translation() const;

    void add_error(int64_t pos);

private:
    uint64_t id;
    std::string phrase;
    std::string translation;
    std::unordered_map<int64_t, Stats> stats;
};


class Trainer;

class Deck
{
    friend class Trainer;

public:
    size_t size() const;

    const Phrase &current_phrase() const;
    const Phrase &get_phrase(size_t idx) const;
    size_t get_symbol_idx() const;
    size_t get_phrase_idx() const;

    bool process_key(int key, bool &repaint_panel);

private:
    void shuffle();

private:
    std::vector<Phrase> phrases;
    size_t symbol_idx = 0;
    size_t phrase_idx = 0;
};



#endif // DECK_H
