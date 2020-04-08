#ifndef TRAINER_DATA_H
#define TRAINER_DATA_H

#include "phrase.h"


class SqliteDatabase;


class TrainerData
{
public:
    TrainerData();

    int64_t phrase_count() const;
    char current_symbol() const;
    const Phrase &current_phrase() const;
    const Phrase &get_phrase(int64_t idx) const;
    int64_t get_symbol_idx() const;
    int64_t get_phrase_idx() const;

protected:
    std::shared_ptr<SqliteDatabase> database;
    std::vector<Phrase> phrases;
    int64_t symbol_idx = 0;
    int64_t phrase_idx = 0;
};

#endif // TRAINER_DATA_H
