#ifndef PHRASE_h
#define PHRASE_h

#include <string>
#include <map>


enum class LearnStrategy {
    Random,
    ReviseErrors,
    ReviseSlow
};

class Trainer;

class Phrase
{
    friend class Trainer;

    struct Stats
    {
        int64_t cumulative_errors = 0;
        int64_t current_errors = 0;
        int64_t current_delay = 0;
    };

public:
    Phrase(uint64_t id, const std::string &phrase, const std::string &translation, LearnStrategy strategy);
    size_t size() const;

    int64_t current_errors(int64_t pos) const;
    int64_t cumulative_errors() const;

    char get_symbol(int64_t pos) const;
    const std::string &get_translation() const;

    void add_stat(int64_t pos, int64_t errors, int64_t delay);

private:
    uint64_t id;
    std::string phrase;
    std::string translation;
    LearnStrategy strategy;
    std::map<int64_t, Stats> stats;
};



#endif // PHRASE_h
