#include "deck.h"
#include <random>


// TODO: we can use audio to speak current phrase!



Phrase::Phrase(uint64_t id, const std::string &phrase, const std::string &translation) :
    id(id), phrase(phrase), translation(translation)
{

}

size_t Phrase::size() const
{
    return phrase.size();
}

uint64_t Phrase::current_errors(int64_t pos) const
{
    auto it = stats.find(pos);
    return it == stats.end() ? 0 : it->second.current_errors;
}

bool Phrase::has_current_errors() const
{
    for (auto &stat : stats) {
        if (stat.second.current_errors > 0) {
            return true;
        }
    }
    return false;
}

char Phrase::get_symbol(int64_t pos) const
{
    return phrase.at(pos);
}

const std::string &Phrase::get_translation() const
{
    return translation;
}

void Phrase::add_error(int64_t pos)
{
    ++stats[pos].current_errors;
}

size_t Deck::size() const
{
    return phrases.size();
}

const Phrase &Deck::current_phrase() const
{
    return phrases.at(phrase_idx);
}

const Phrase &Deck::get_phrase(size_t idx) const
{
    return phrases.at(idx);
}

size_t Deck::get_symbol_idx() const
{
    return symbol_idx;
}

size_t Deck::get_phrase_idx() const
{
    return phrase_idx;
}

bool Deck::process_key(int key, bool &repaint_panel)
{
    repaint_panel = false;
    if (symbol_idx >= current_phrase().size()) {
        if (key == ' ') {
            ++phrase_idx;
            symbol_idx = 0;
            repaint_panel = true;
        }
        else {
            phrases.at(phrase_idx).add_error(-1);
        }
        return true;
    }
    if (current_phrase().get_symbol(symbol_idx) == key) {
        if (++symbol_idx == current_phrase().size()) {
            repaint_panel = true;
            if (phrase_idx + 1 >= size()) {
                return false;
            }
        }
    }
    else {
        phrases.at(phrase_idx).add_error(symbol_idx);
    }
    return true;
}

void Deck::shuffle()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(phrases.begin(), phrases.end(), g);
}
