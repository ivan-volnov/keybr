#ifndef TRAINER_H
#define TRAINER_H

#include "trainer_data.h"
#include "utility/speech_engine.h"
#include <chrono>
#include <random>



class Trainer : public TrainerData
{
public:
    Trainer();

    bool load();
    uint64_t anki_import(const std::string &query);
    void show_stats() const;

    bool process_key(char32_t key);

private:
    bool fetch();
    uint64_t fetch(uint64_t count, LearnStrategy strategy);
    bool load_next_exercise();
    void say_current_phrase() const;
    uint64_t count(LearnStrategy strategy) const;

private:
    std::unique_ptr<SpeechEngine> speech;
    std::mt19937 random_generator;
    std::chrono::steady_clock::time_point key_ts{};
};

#endif // TRAINER_H
