#include "deck.h"
#include "3rdparty/json.hpp"
#include <fstream>
#include <sstream>
#include <random>
#include "sqlite_database.h"
#include "config.h"


// TODO: we can use audio to speak current phrase!



const Phrase &Deck::current_phrase() const
{
    return phrases.at(phrase_idx);
}

Phrase &Deck::current_phrase()
{
    return phrases.at(phrase_idx);
}

uint64_t Phrase::current_errors(int64_t pos) const
{
    auto it = stats.find(pos);
    return it == stats.end() ? 0 : it->second.current_errors;
}

size_t Phrase::size() const
{
    return phrase.size();
}

bool Deck::process_key(int key, bool &repaint_panel)
{
    repaint_panel = false;
    if (symbol_idx >= current_phrase().size()) {
        if (key == ' ') {
            if (++phrase_idx >= phrases.size()) {
                return false;
            }
            symbol_idx = 0;
            repaint_panel = true;
        }
        else {
            ++current_phrase().stats[-1].current_errors;
        }
        return true;
    }
    if (current_phrase().phrase.at(symbol_idx) == key) {
        repaint_panel = ++symbol_idx == current_phrase().size();
    }
    else {
        ++current_phrase().stats[symbol_idx].current_errors;
    }
    return true;
}

void Deck::shuffle()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(phrases.begin(), phrases.end(), g);
}



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
    sql << "INSERT OR IGNORE INTO keybr_phrases (phrase, translation) VALUES(?,?)";
    for (auto &card : json) {
        sql.clear_bindings();
        sql.bind(card["keyword"].get<std::string>());
        sql.bind(card["translation"].get<std::string>());
        sql.step();
    }
}

void Trainer::fetch(uint32_t count, Deck &deck)
{
    auto sql = database->create_query();
    sql << "SELECT id, phrase, translation\n"
           "FROM keybr_phrases\n"
           "WHERE id IN (\n"
           "    SELECT id\n"
           "    FROM keybr_phrases\n"
           "    ORDER BY RANDOM()\n"
           "    LIMIT ?\n"
           ")";
    sql.bind(count);
    while (sql.step()) {
        deck.phrases.push_back({sql.get_uint64(),
                                sql.get_string(),
                                sql.get_string()});
    }
}
