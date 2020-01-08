#ifndef DECK_H
#define DECK_H

#include <string>
#include <vector>
#include <chrono>


struct Symbol
{
    char ch;
    uint64_t errors;
    std::chrono::microseconds delay;
};



class Phrase
{
public:
    std::vector<Symbol> symbols;
    std::string translation;
    Symbol delimiter{' '};
};



class Deck
{
public:
    Deck();

    Phrase &current_phrase();
    const Phrase &current_phrase() const;

    Symbol &current_symbol();
    const Symbol &current_symbol() const;

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
