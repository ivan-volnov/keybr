#include "phrase.h"
#include <sqlite_database/sqlite_database.h>



constexpr int64_t max_current_errors = 5;


Phrase::Phrase(uint64_t id, const std::u32string &phrase, const std::string &translation, const std::vector<int64_t> &char_ids, const std::vector<int64_t> &errors, LearnStrategy strategy) :
    id(id), phrase(phrase), translation(translation), strategy(strategy)
{
    const auto stat_size = phrase.size() + 1;
    if (char_ids.size() != stat_size || errors.size() != stat_size) {
        throw std::runtime_error("Got wrong size of char_ids or errors arrays from db");
    }
    stats.reserve(stat_size);
    for (size_t i = 0; i < stat_size; ++i) {
        stats.push_back({char_ids.at(i), errors.at(i)});
    }
}

int64_t Phrase::size() const
{
    return phrase.size();
}

uint64_t Phrase::get_id() const
{
    return id;
}

char32_t Phrase::get_symbol(int64_t pos) const
{
    return pos < 0 ? ' ' : phrase.at(pos);
}

const std::u32string &Phrase::get_phrase_text() const
{
    return phrase;
}

const std::string &Phrase::get_translation() const
{
    return translation;
}

LearnStrategy Phrase::get_strategy() const
{
    return strategy;
}

int64_t Phrase::current_errors(int64_t pos) const
{
    return stats.at(pos + 1).current_errors;
}

int64_t Phrase::current_errors() const
{
    int64_t result = 0;
    for (const auto &stat : stats) {
        result += stat.current_errors;
    }
    return result;
}

void Phrase::add_stat(int64_t pos, int64_t errors, int64_t delay)
{
    auto &stat = stats.at(pos + 1);
    stat.current_errors += errors;
    if (errors <= 0 && delay > 80000 && delay < 1500000) {
        stat.current_delay = delay;
    }
}

bool Phrase::save(Query &sql_errors, Query &sql_delay)
{
    bool has_errors = false;
    for (auto &stat : stats) {
        if (stat.current_errors > max_current_errors) {
            stat.current_errors = max_current_errors;
        }
        if (stat.current_errors <= 0 && stat.cumulative_errors > 0) {
            stat.current_errors = -1;
        }
        if (stat.current_errors) {
            sql_errors.clear_bindings()
                    .bind(stat.phrase_char_id)
                    .bind(stat.current_errors)
                    .step();
            stat.cumulative_errors += stat.current_errors;
        }
        if (stat.current_delay > 0) {
            sql_delay.clear_bindings()
                     .bind(stat.phrase_char_id)
                     .bind(stat.current_delay)
                     .step();
        }
        stat.current_errors = 0;
        stat.current_delay = 0;
        if (stat.cumulative_errors > 0) {
            has_errors = true;
        }
    }
    if (has_errors) {
        strategy = LearnStrategy::ReviseErrors;
        return false;
    }
    return true;
}
