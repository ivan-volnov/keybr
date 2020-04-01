#include "trainer.h"
#include <iostream>
#include <fstream>
#include <regex>
#include "utility/sqlite_database.h"
#include "utility/anki_client.h"
#include "config.h"


constexpr uint64_t revisions = 10;
constexpr uint64_t total_phrases = 10;


Trainer::Trainer() :
    database(SqliteDatabase::open(Config::instance().get_app_path().append("keybr_db.sqlite"))),
    random_generator(std::random_device{}())
{
    const int64_t required_db_version = 1;
    auto sql = database->create_query();
    sql << "PRAGMA user_version";
    if (!sql.step()) {
        throw std::runtime_error("Can't read database version");
    }
    auto db_version = sql.get_int64();
    if (db_version > required_db_version) {
        throw std::runtime_error("App is older than the database. Can't update");
    }
    if (db_version < required_db_version) {
        auto transaction = database->begin_transaction();
        for (; db_version < required_db_version; ++db_version) {
            switch (db_version) {
            case 0:
                database->exec("CREATE TABLE keybr_phrases(\n"
                               "    id INTEGER PRIMARY KEY,\n"
                               "    phrase TEXT UNIQUE,\n"
                               "    translation TEXT NOT NULL\n"
                               ")");
                database->exec("CREATE TABLE keybr_stats(\n"
                               "    id INTEGER PRIMARY KEY,\n"
                               "    create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
                               "    phrase_id INTEGER NOT NULL,\n"
                               "    pos INTEGER NOT NULL,\n"
                               "    errors INTEGER NOT NULL DEFAULT 0,\n"
                               "    delay INTEGER NOT NULL DEFAULT 0,\n"
                               "    ch TEXT NOT NULL,\n"
                               "    FOREIGN KEY (phrase_id) REFERENCES keybr_phrases(id)\n"
                               ")");
                database->exec("CREATE INDEX keybr_stats_phrase_id_idx ON keybr_stats(phrase_id)");
                break;
            default:
                throw std::runtime_error("Can't update database");
            }
        }
        sql.reset() << "PRAGMA user_version =" << db_version << Query::step;
    }
    database->exec("CREATE TEMPORARY TABLE keybr_tmp_phrase_ids(\n"
                   "    phrase_id INTEGER PRIMARY KEY\n"
                   ")");
}

bool Trainer::load()
{
    fetch(total_phrases - std::min(total_phrases, fetch(revisions, true)));
    if (!phrase_count()) {
        return false;
    }
    std::shuffle(phrases.begin(), phrases.end(), random_generator);
    if (Config::instance().is_sound_enabled()) {
        speech = std::make_unique<SpeechEngine>();
        say_current_phrase();
    }
    return true;
}

void string_replace(std::string &str, const std::string &from, const std::string &to)
{
    if (const auto pos = str.find(from); pos != std::string::npos) {
        str.erase(pos, from.size());
        str.insert(pos, to);
    }
}

uint64_t Trainer::anki_import()
{
    AnkiClient anki;
    auto notes = anki.request("findNotes", {{"query", "\"deck:En::Vocabulary Profile\" is:due -is:new -is:suspended"}});
    notes = anki.request("notesInfo", {{"notes", std::move(notes)}});
    auto transaction = database->begin_transaction();
    const auto count = count_db_phrases();
    auto sql = database->create_query();
    sql << "INSERT OR IGNORE INTO keybr_phrases (phrase, translation) VALUES";
    sql.add_array(2);
    for (const auto &note : notes) {
        const auto &fields = note.at("fields");
        auto phrase = fields.at("Front").at("value").get<std::string>();
        auto translation = fields.at("Back").at("value").get<std::string>();
        string_replace(phrase, ", etc.", "");
        string_replace(phrase, ", etc", "");
        sql.clear_bindings();
        sql.bind(phrase);
        sql.bind(translation);
        sql.step();
    }
    return count_db_phrases() - count;
}

void Trainer::show_stats() const
{
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
           "    FROM keybr_stats\n"
           "    WHERE delay > 0\n"
           "    AND errors <= 0\n"
           "    WINDOW win AS (ORDER BY id ROWS 5 PRECEDING)\n"
           ") a\n"
           "WHERE row_number > 5";
    sql.step();
    if (sql.is_null()) {
        sql.skip().skip();
    }
    else {
        std::cout << "\n- Today:" << std::endl;
        std::cout << "    Average Speed: " << sql.get_string() << " wpm" << std::endl;
        std::cout << "    Total Time: " << sql.get_string() << std::endl;
    }
    std::cout << "\n- All time:" << std::endl;
    std::cout << "    Average Speed: " << sql.get_string() << " wpm" << std::endl;
    std::cout << "    Total Time: " << sql.get_string() << std::endl;
}

uint64_t Trainer::fetch(uint64_t count, bool revise)
{
    if (!count) {
        return 0;
    }
    auto sql = database->create_query();
    sql << "SELECT p.id, p.phrase, p.translation\n"
           "FROM keybr_phrases p\n";
    if (revise) {
        sql << "JOIN keybr_stats s ON p.id = s.phrase_id\n";
    }
    sql << "WHERE p.id NOT IN (\n"
           "    SELECT phrase_id\n"
           "    FROM keybr_tmp_phrase_ids\n"
           ")\n";
    if (revise) {
        sql << "GROUP BY p.id\n"
               "ORDER BY sum(s.errors) DESC\n";
    }
    else {
        sql << "ORDER BY RANDOM()\n";
    }
    sql << "LIMIT ?";
    sql.bind(count);
    std::vector<uint64_t> ids;
    while (sql.step()) {
        const auto id = sql.get_uint64();
        ids.push_back(id);
        phrases.push_back({id, sql.get_string(), sql.get_string(), revise});
    }
    if (ids.empty()) {
        return 0;
    }
    load_stats(ids);
    return ids.size();
}

void Trainer::load_stats(const std::vector<uint64_t> &ids)
{
    auto sql = database->create_query();
    auto sql_inserter = database->create_query();
    for (size_t i = 0; i < ids.size();) {
        const auto chunk_size = std::min(ids.size() - i, static_cast<size_t>(1000));
        sql.reset() << "SELECT phrase_id, pos, errors, delay\n"
                       "FROM keybr_stats\n"
                       "WHERE phrase_id IN";
        sql.add_array(chunk_size);
        sql_inserter.reset() << "INSERT INTO keybr_tmp_phrase_ids (phrase_id) VALUES";
        sql_inserter.add_array(1, chunk_size);
        for (size_t j = 0; j < chunk_size; ++j) {
            sql.bind(ids[i]);
            sql_inserter.bind(ids[i++]);
        }
        while (sql.step()) {
            const auto phrase_id = sql.get_uint64();
            for (auto &phrase : phrases) {
                if (phrase.id != phrase_id) {
                    continue;
                }
                auto &stat = phrase.stats[sql.get_int64()];
                stat.cumulative_errors += sql.get_int64();
                const auto delay = sql.get_uint64();
                if (delay) {
                    stat.avg_delay.add(delay);
                }
            }
        }
        sql_inserter.step();
    }
}

void Trainer::save(Phrase &phrase)
{
    auto sql = database->create_query();
    sql << "INSERT INTO keybr_stats (phrase_id, pos, errors, delay, ch) VALUES";
    sql.add_array(5);
    for (auto &stat : phrase.stats) {
        const int64_t errors = phrase.is_revision && !stat.second.current_errors && stat.second.cumulative_errors >= 1 ? -1 : stat.second.current_errors;
        const uint64_t delay = stat.second.current_delay.value();
        stat.second.current_errors = 0;
        stat.second.current_delay.reset();
        if (!errors && !delay) {
            continue;
        }
        sql.clear_bindings();
        sql.bind(phrase.id);
        sql.bind(stat.first);
        sql.bind(errors);
        sql.bind(delay);
        sql.bind(std::string(1, phrase.get_symbol(stat.first)));
        sql.step();
        stat.second.cumulative_errors += errors;
        if (delay) {
            stat.second.avg_delay.add(delay);
        }
    }
}

uint64_t Trainer::count_db_phrases() const
{
    auto sql = database->create_query();
    sql << "SELECT count(*) FROM keybr_phrases";
    sql.step();
    return sql.get_uint64();
}

void Trainer::say_current_phrase() const
{
    if (!speech) {
        return;
    }
    auto phrase = current_phrase().phrase;
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
    string_replace(phrase, "(", "");
    string_replace(phrase, ")", "");
    if (phrase == "read, read, read") {
        phrase = "read, red, red";
    }
    speech->say(phrase);
}

bool Trainer::process_key(int key, bool &repaint_panel)
{
    using namespace std::chrono;
    const auto now = steady_clock::now();
    const auto delay = key_ts.time_since_epoch().count() > 0 ? duration_cast<microseconds>(now - key_ts).count() : 0;
    repaint_panel = false;
    if (current_symbol() == key) {
        phrases.at(phrase_idx).add_stat(symbol_idx, 0, delay);
        if (symbol_idx < 0) {                                   // on the space after the phrase
            ++phrase_idx;
            ++symbol_idx;
            repaint_panel = true;
            say_current_phrase();
        }
        else if (++symbol_idx >= current_phrase().size()) {     // on the last symbol of the phrase
            if (phrase_idx + 1 >= phrase_count()) {             // on the last phrase
                repaint_panel = true;
                return load_next_exercise();
            }
            symbol_idx = -1;
        }
    }
    else if (!phrase_idx && !symbol_idx && key == ' ') {        // ignore space error on the first symbol of the first phrase
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
    size_t erased = 0;
    bool has_revision = false;
    for (auto phrase = phrases.begin(); phrase != phrases.end();) {
        save(*phrase);
        if (phrase->cumulative_errors() > 0) {
            phrase->is_revision = true;
            ++phrase;
            has_revision = true;
        }
        else {
            phrase = phrases.erase(phrase);
            ++erased;
        }
    }
    if (erased > 1 && !has_revision) {
        erased -= fetch(1, true);
    }
    fetch(erased);
    if (phrases.empty()) {
        return false;
    }
    phrase_idx = 0;
    symbol_idx = 0;
    key_ts = {};
    std::shuffle(phrases.begin(), phrases.end(), random_generator);
    say_current_phrase();
    return true;
}
