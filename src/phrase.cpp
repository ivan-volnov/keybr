#include "phrase.h"



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

int64_t Phrase::cumulative_errors() const
{
    int64_t result = 0;
    for (const auto &stat : stats) {
        result += stat.second.cumulative_errors + stat.second.current_errors;
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
    if (errors <= 0 && delay > 80000 && delay < 1500000) {
        stat.current_delay.add(delay);
    }
}
