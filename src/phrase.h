#ifndef PHRASE_h
#define PHRASE_h

#include <string>
#include <vector>


enum class LearnStrategy {
    Unseen,
    ReviseErrors,
    ReviseSlow
};

class Query;

class Phrase
{
    struct Stats
    {
        int64_t phrase_char_id = 0;
        int64_t cumulative_errors = 0;
        int64_t current_errors = 0;
        int64_t current_delay = 0;
    };

public:
    Phrase(uint64_t id, const std::u32string &phrase, const std::string &translation, const std::vector<int64_t> &char_ids, const std::vector<int64_t> &errors, LearnStrategy strategy);

    int64_t size() const;

    uint64_t get_id() const;
    char32_t get_symbol(int64_t pos) const;
    const std::u32string &get_phrase_text() const;
    const std::string &get_translation() const;
    LearnStrategy get_strategy() const;

    int64_t current_errors(int64_t pos) const;
    int64_t current_errors() const;

    void add_stat(int64_t pos, int64_t errors, int64_t delay);
    bool save(Query &sql_errors, Query &sql_delay);

private:
    uint64_t id;
    std::u32string phrase;
    std::string translation;
    LearnStrategy strategy;
    std::vector<Stats> stats;
};



#endif // PHRASE_h
