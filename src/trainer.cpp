#include "trainer.h"
#include <fstream>
#include "3rdparty/json.hpp"
#include "sqlite_database.h"
#include "config.h"



Trainer::Trainer() :
    database(SqliteDatabase::open(Config::instance().get_app_path().append("keybr_db.sqlite")))
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

void Trainer::load()
{
    fetch(1, true);
    fetch(59);
    deck.shuffle();
}

void Trainer::import(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open import file");
    }
    nlohmann::json json;
    file >> json;
    auto transaction = database->begin_transaction();
    auto sql = database->create_query();
    sql << "INSERT OR IGNORE INTO keybr_phrases (phrase, translation) VALUES";
    sql.add_array(2);
    for (auto &card : json) {
        sql.clear_bindings();
        sql.bind(card["keyword"].get<std::string>());
        sql.bind(card["translation"].get<std::string>());
        sql.step();
    }
}

void Trainer::fetch(uint32_t count, bool revise)
{
    if (!count) {
        return;
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
        deck.phrases.push_back({id, sql.get_string(), sql.get_string(), revise});
    }
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
            for (auto &phrase : deck.phrases) {
                if (phrase.id != phrase_id) {
                    continue;
                }
                auto &stat = phrase.stats[sql.get_uint64()];
                stat.errors += sql.get_int64();
                stat.avg_delay.add(sql.get_uint64());
            }
        }
        sql_inserter.step();
    }
}

void Trainer::save(Phrase &phrase)
{
    auto sql = database->create_query();
    sql << "INSERT INTO keybr_stats (phrase_id, pos, errors, delay) VALUES";
    sql.add_array(4);
    for (auto &stat : phrase.stats) {
        const int64_t errors = phrase.is_revision && !stat.second.current_errors && stat.second.errors >= 1 ? -1 : stat.second.current_errors;
        sql.clear_bindings();
        sql.bind(phrase.id);
        sql.bind(stat.first);
        sql.bind(errors);
        sql.bind(stat.second.current_delay);
        sql.step();
        stat.second.errors += errors;
        stat.second.avg_delay.add(stat.second.current_delay);
        stat.second.current_errors = 0;
        stat.second.current_delay = 0;
    }
}

bool Trainer::process_key(int key, bool &repaint_panel)
{
    if (deck.process_key(key, repaint_panel)) {
        return true;
    }
    auto transaction = database->begin_transaction();
    size_t erased = 0;
    bool has_revision = false;
    for (auto phrase = deck.phrases.begin(); phrase != deck.phrases.end();) {
        save(*phrase);
        if (phrase->errors() > 0) {
            phrase->is_revision = true;
            ++phrase;
            has_revision = true;
        }
        else {
            phrase = deck.phrases.erase(phrase);
            ++erased;
        }
    }
    if (erased > 1 && !has_revision) {
        fetch(1, true);
        --erased;
    }
    fetch(erased);
    if (deck.phrases.empty()) {
        return false;
    }
    deck.phrase_idx = 0;
    deck.symbol_idx = 0;
    deck.shuffle();
    return true;
}

const Deck &Trainer::get_deck() const
{
    return deck;
}
