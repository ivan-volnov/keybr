#include "trainer.h"
#include <iostream>
#include <regex>
#include <sqlite_database/sqlite_database.h>
#include "utility/anki_client.h"
#include "utility/tools.h"
#include "config.h"


constexpr int64_t total_phrases = 10;
constexpr int64_t last_n_delay_revisions = 10;
constexpr double uppercase_delay_multiplier = 0.4;
constexpr auto anki_query = "\"deck:En::Vocabulary Profile\" is:due -is:new -is:suspended";


Trainer::Trainer() :
    random_generator(std::random_device{}())
{

}

bool Trainer::load()
{
    if (!fetch()) {
        return false;
    }
    if (Config::instance().is_sound_enabled()) {
        speech = std::make_unique<SpeechEngine>();
        say_current_phrase();
    }
    return true;
}

uint64_t Trainer::anki_import(const std::string &query)
{
    AnkiClient anki;
    auto notes = anki.request("findNotes", {{"query", query.empty() ? anki_query : query}});
    notes = anki.request("notesInfo", {{"notes", std::move(notes)}});
    auto transaction = database->begin_transaction();

    auto sql = database->create_query();
    sql << "INSERT INTO keybr_phrases (phrase, translation) VALUES";
    sql.add_array(2) << "\n";
    sql << "ON CONFLICT (phrase) DO UPDATE SET translation = excluded.translation";

    auto sql_chars = database->create_query();
    sql_chars << "INSERT INTO keybr_phrase_chars (phrase_id, pos, ch) VALUES";
    sql_chars.add_array(3);

    auto sql_select = database->create_query();
    sql_select << "SELECT p.id\n"
                  "FROM keybr_phrases p\n"
                  "WHERE phrase = ?\n"
                  "AND NOT EXISTS (\n"
                  "    SELECT 1\n"
                  "    FROM keybr_phrase_chars\n"
                  "    WHERE phrase_id = p.id\n"
                  ")";

    uint64_t result = 0;
    for (const auto &note : notes) {
        const auto &fields = note.at("fields");
        auto phrase = fields.at("Front").at("value").get<std::string>();
        auto translation = fields.at("Back").at("value").get<std::string>();
        tools::string_replace(phrase, ", etc.", "");
        tools::string_replace(phrase, ", etc", "");
        sql.clear_bindings()
           .bind(phrase)
           .bind(translation)
           .step();
        if (sql_select.clear_bindings().bind(phrase).step()) {
            const auto phrase_id = sql_select.get_int64();
            for (int64_t pos = -1; pos < static_cast<int64_t>(phrase.size()); ++pos) {
                sql_chars.clear_bindings()
                         .bind(phrase_id)
                         .bind(pos)
                         .bind(std::string(1, pos < 0 ? ' ' : phrase[pos]))
                         .step();
            }
            ++result;
        }
    }
    return result;
}

void Trainer::show_stats() const
{
    constexpr int64_t column_width = 16;
    auto sql = database->create_query();
    sql << "SELECT\n"
           "    round(avg(wpm) FILTER (WHERE today), 2),\n"
           "    time(sum(delay) FILTER (WHERE today) / 1000000, 'unixepoch'),\n"
           "    round(avg(wpm), 2),\n"
           "    time(sum(delay) / 1000000, 'unixepoch')\n"
           "FROM (\n"
           "    SELECT\n"
           "        row_number() OVER win AS row_number,\n"
           "        date(create_date, 'localtime') = date('now', 'localtime') AS today,\n"
           "        delay,\n"
           "        60000000.0 / sum(delay) OVER win AS wpm\n"
           "    FROM keybr_stat_delays\n"
           "    WINDOW win AS (ORDER BY create_date, phrase_char_id ROWS 5 PRECEDING)\n"
           ") a\n"
           "WHERE row_number > 5";
    sql.step();
    if (sql.is_null()) {
        sql.skip().skip();
    }
    else {
        std::cout << "\n- Today:" << std::endl;
        std::cout << "    " << std::left << std::setw(column_width) << "Average Speed: " << sql.get_string() << " wpm" << std::endl;
        std::cout << "    " << std::left << std::setw(column_width) << "Total Time: " << sql.get_string() << std::endl;
    }
    std::cout << "\n- All time:" << std::endl;
    std::cout << "    " << std::left << std::setw(column_width) << "Average Speed: " << sql.get_string() << " wpm" << std::endl;
    std::cout << "    " << std::left << std::setw(column_width) << "Total Time: " << sql.get_string() << std::endl;
    sql.reset();
    sql << "SELECT\n"
           "    count(DISTINCT c.phrase_id) FILTER (WHERE EXISTS (SELECT 1 FROM keybr_stat_delays WHERE phrase_char_id = c.id)),\n"
           "    count(DISTINCT c.phrase_id)\n"
           "FROM keybr_phrase_chars c";
    sql.step();
    const auto seen = sql.get_int64();
    const auto total = sql.get_int64();
    const auto unseen = total - seen;
    std::cout << "\n- Other stats:" << std::endl;
    if (unseen > 0) {
        std::cout << "    " << std::left << std::setw(column_width) << "Unseen phrases: " << unseen << std::endl;
    }
    std::cout << "    " << std::left << std::setw(column_width) << "Total phrases: " << total << std::endl;
}

bool Trainer::fetch()
{
    int64_t to_fetch = total_phrases - phrases.size();
    if (to_fetch > 0) {
        to_fetch -= fetch(to_fetch, LearnStrategy::ReviseErrors);
    }
    if (to_fetch > 0) {
        const auto s = to_fetch / 2 - count(LearnStrategy::ReviseSlow);
        if (s > 0) {
            to_fetch -= fetch(s, LearnStrategy::ReviseSlow);
        }
    }
    if (to_fetch > 0) {
        fetch(to_fetch, LearnStrategy::Random);
    }
    std::shuffle(phrases.begin(), phrases.end(), random_generator);
    return !phrases.empty();
}

uint64_t Trainer::fetch(uint64_t count, LearnStrategy strategy)
{
    if (!count) {
        return 0;
    }
    auto sql = database->create_query();
    sql << "SELECT p.id, p.phrase, p.translation, group_concat(c.id, ','), group_concat(coalesce(er.errors, 0), ',')\n"
           "FROM keybr_phrases p\n"
           "JOIN keybr_phrase_chars c ON c.phrase_id = p.id\n"
           "LEFT JOIN (\n"
           "    SELECT phrase_char_id, sum(errors) AS errors\n"
           "    FROM keybr_stat_errors\n"
           "    GROUP BY phrase_char_id\n"
           ") er ON er.phrase_char_id = c.id\n";
    switch (strategy) {
    case LearnStrategy::Random :
        sql << "WHERE p.id NOT IN";
        sql.add_array(phrases.size()) << "\n";
        sql << "GROUP BY p.id\n"
               "ORDER BY EXISTS (\n"
               "    SELECT 1\n"
               "    FROM keybr_stat_delays\n"
               "    WHERE phrase_char_id = c.id\n"
               "), EXISTS (\n"
               "    SELECT 1\n"
               "    FROM keybr_stat_delays\n"
               "    WHERE phrase_char_id = c.id\n"
               "    AND date(create_date, 'localtime') = date('now', 'localtime')\n"
               "), RANDOM()\n";
        break;
    case LearnStrategy::ReviseErrors :
        sql << "WHERE p.id NOT IN";
        sql.add_array(phrases.size()) << "\n";
        sql << "GROUP BY p.id\n"
               "HAVING sum(er.errors) > 0\n"
               "ORDER BY sum(er.errors) DESC\n";
        break;
    case LearnStrategy::ReviseSlow :
        sql << "LEFT JOIN (\n"
               "    SELECT\n"
               "        phrase_char_id,\n"
               "        avg(CASE WHEN cc.ch = lower(cc.ch)\n"
               "            THEN delay\n"
               "            ELSE delay * " << uppercase_delay_multiplier << "\n"
               "            END) FILTER (WHERE row_number <= " << last_n_delay_revisions << ") AS delay\n"
               "    FROM (\n"
               "        SELECT\n"
               "            phrase_char_id,\n"
               "            delay,\n"
               "            row_number() OVER (PARTITION BY phrase_char_id ORDER BY create_date DESC) AS row_number\n"
               "        FROM keybr_stat_delays\n"
               "    )\n"
               "    JOIN keybr_phrase_chars cc ON cc.id = phrase_char_id\n"
               "    GROUP BY phrase_char_id\n"
               ") a ON a.phrase_char_id = c.id\n"
               "WHERE p.id NOT IN";
        sql.add_array(phrases.size()) << "\n";
        sql << "GROUP BY p.id\n"
               "ORDER BY avg(a.delay) DESC\n";
        break;
    }
    sql << "LIMIT ?";
    for (const auto &phrase : phrases) {
        sql.bind(phrase.get_id());
    }
    sql.bind(count);
    uint64_t result = 0;
    while (sql.step()) {
        phrases.push_back({sql.get_uint64(),
                           sql.get_string(),
                           sql.get_string(),
                           sql.get_int64_array(),
                           sql.get_int64_array(),
                           strategy});
        ++result;
    }
    return result;
}

void Trainer::say_current_phrase() const
{
    if (!speech) {
        return;
    }
    auto phrase = current_phrase().get_phrase_text();
    phrase = std::regex_replace(phrase, std::regex("\\bsb\\b"), "somebody");
    phrase = std::regex_replace(phrase, std::regex("\\bsth\\b"), "something");
    phrase = std::regex_replace(phrase, std::regex("\\bswh\\b"), "somewhere");
    if (phrase != "or" &&
        phrase != "believe it or not" &&
        phrase != "or so" &&
        phrase != "more or less")
    {
        phrase = std::regex_replace(phrase, std::regex("\\bor\\b"), ",");
    }
    tools::string_replace(phrase, "(", "");
    tools::string_replace(phrase, ")", "");
    if (phrase == "read, read, read") {
        phrase = "read, red, red";
    }
    speech->say(phrase);
}

uint64_t Trainer::count(LearnStrategy strategy) const
{
    uint64_t result = 0;
    for (const auto &phrase : phrases) {
        if (phrase.get_strategy() == strategy) {
            ++result;
        }
    }
    return result;
}

bool Trainer::process_key(int key, bool &repaint_panel)
{
    using namespace std::chrono;
    const auto now = steady_clock::now();
    const auto delay = key_ts.time_since_epoch().count() > 0 ? duration_cast<microseconds>(now - key_ts).count() : 0;
    repaint_panel = false;
    if (current_symbol() == key) {
        phrases.at(phrase_idx).add_stat(symbol_idx, 0, delay);
        if (symbol_idx++ < 0) {                                         // on the space after the phrase
            repaint_panel = true;
            say_current_phrase();
        }
        else if (symbol_idx >= current_phrase().size()) {               // on the last symbol of the phrase
            if (++phrase_idx >= static_cast<int64_t>(phrases.size())) { // on the last phrase
                if (!load_next_exercise()) {
                    return false;
                }
                phrase_idx = 0;
                symbol_idx = 0;
                key_ts = {};
                repaint_panel = true;
                say_current_phrase();
                return true;
            }
            symbol_idx = -1;
        }
    }
    else if (!phrase_idx && !symbol_idx && key == ' ') {                // ignore space error on the first symbol of the first phrase
        assert(key_ts == std::chrono::steady_clock::time_point{});
        return true;
    }
    else {
        phrases.at(phrase_idx).add_stat(symbol_idx, 1, 0);
    }
    key_ts = now;
    return true;
}

bool Trainer::load_next_exercise()
{
    auto transaction = database->begin_transaction();
    auto sql_delays = database->create_query();
    sql_delays << "INSERT INTO keybr_stat_delays (phrase_char_id, delay) VALUES";
    sql_delays.add_array(2);
    auto sql_errors = database->create_query();
    sql_errors << "INSERT INTO keybr_stat_errors (phrase_char_id, errors) VALUES";
    sql_errors.add_array(2);
    for (auto phrase = phrases.begin(); phrase != phrases.end();) {
        if (phrase->save(sql_errors, sql_delays)) {
            phrase = phrases.erase(phrase);
        }
        else {
            ++phrase;
        }
    }
    return fetch();
}
