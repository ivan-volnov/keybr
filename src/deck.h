#ifndef DECK_H
#define DECK_H

#include <string>
#include <vector>
#include <map>
#include "average.h"



class Trainer;

class Phrase
{
    friend class Trainer;

    struct Stats
    {
        int64_t cumulative_errors = 0;
        Average<uint64_t> avg_delay;
        int64_t current_errors = 0;
        Average<uint64_t> current_delay;
    };

public:
    Phrase(uint64_t id, const std::string &phrase, const std::string &translation, bool is_revision);
    size_t size() const;

    int64_t current_errors(int64_t pos) const;
    bool has_current_errors() const;
    int64_t cumulative_errors() const;

    char get_symbol(int64_t pos) const;
    const std::string &get_translation() const;

    void add_stat(int64_t pos, int64_t errors, int64_t delay);

private:
    uint64_t id;
    std::string phrase;
    std::string translation;
    bool is_revision;
    std::map<int64_t, Stats> stats;
};



class Deck
{
    friend class Trainer;

public:
    size_t size() const;

    const Phrase &current_phrase() const;
    char current_symbol() const;
    const Phrase &get_phrase(int64_t idx) const;
    int64_t get_symbol_idx() const;
    int64_t get_phrase_idx() const;

    bool process_key(int key, bool &repaint_panel, int64_t delay);

private:
    std::vector<Phrase> phrases;
    int64_t symbol_idx = 0;
    int64_t phrase_idx = 0;
};



#endif // DECK_H
