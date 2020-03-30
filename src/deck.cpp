#include "deck.h"
#include <random>



Phrase::Phrase(uint64_t id, const std::string &phrase, const std::string &translation, bool is_revision) :
    id(id), phrase(phrase), translation(translation), is_revision(is_revision)
{

}

size_t Phrase::size() const
{
    return phrase.size();
}

int64_t Phrase::current_errors(int64_t pos) const
{
    auto it = stats.find(pos);
    return it == stats.end() ? 0 : it->second.current_errors;
}

bool Phrase::has_current_errors() const
{
    for (const auto &stat : stats) {
        if (stat.second.current_errors > 0) {
            return true;
        }
    }
    return false;
}

int64_t Phrase::errors() const
{
    int64_t result = 0;
    for (const auto &stat : stats) {
        result += stat.second.errors + stat.second.current_errors;
    }
    return result;
}

char Phrase::get_symbol(int64_t pos) const
{
    return pos < 0 ? ' ' : phrase.at(pos);
}

const std::string &Phrase::get_translation() const
{
    return translation;
}

void Phrase::add_stat(int64_t pos, int64_t errors, int64_t delay)
{
    auto &stat = stats[pos];
    stat.current_errors += errors;
    stat.current_delay.add(delay);
}

size_t Deck::size() const
{
    return phrases.size();
}

const Phrase &Deck::current_phrase() const
{
    return phrases.at(phrase_idx);
}

char Deck::current_symbol() const
{
    return phrases.at(phrase_idx).get_symbol(symbol_idx);
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

bool Deck::process_key(int key, bool &repaint_panel, int64_t delay)
{
    repaint_panel = false;
    int64_t errors = 0;
    int64_t idx = symbol_idx;
    if (symbol_idx >= current_phrase().size()) {
        // on the space after the phrase
        idx = -1;
        if (key == ' ') {
            ++phrase_idx;
            symbol_idx = 0;
            repaint_panel = true;
        }
        else {
            errors = 1;
        }
    }
    else if (current_phrase().get_symbol(symbol_idx) == key) {
        // into the phrase
        if (++symbol_idx == current_phrase().size()) {
            // on the last symbol of the phrase
            repaint_panel = true;
            if (phrase_idx + 1 >= size()) {
                phrases.at(phrase_idx).add_stat(idx, 0, delay);
                return false;
            }
        }
    }
    else {
        errors = 1;
    }
    phrases.at(phrase_idx).add_stat(idx, errors, delay);
    return true;
}

void Deck::shuffle()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(phrases.begin(), phrases.end(), g);
}
