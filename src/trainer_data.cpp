#include "trainer_data.h"
#include "utility/sqlite_database.h"
#include "config.h"


TrainerData::TrainerData()
{
    database = SqliteDatabase::open(Config::instance().get_app_path().append("keybr_db.sqlite"));
    const int64_t required_db_version = 3;
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
        {
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
                case 1:
                    database->exec("DELETE FROM keybr_stats WHERE pos < 0");
                    break;
                case 2:
                    database->exec("CREATE TABLE keybr_phrase_chars (\n"
                                   "    id INTEGER PRIMARY KEY,\n"
                                   "    phrase_id INTEGER NOT NULL,\n"
                                   "    pos INTEGER NOT NULL,\n"
                                   "    ch TEXT NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_id) REFERENCES keybr_phrases(id)\n"
                                   ")");
                    database->exec("CREATE TABLE keybr_stat_errors (\n"
                                   "    create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
                                   "    phrase_char_id INTEGER NOT NULL,\n"
                                   "    errors INTEGER NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_char_id) REFERENCES keybr_phrase_chars(id)\n"
                                   ")");
                    database->exec("CREATE TABLE keybr_stat_delays (\n"
                                   "    create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
                                   "    phrase_char_id INTEGER NOT NULL,\n"
                                   "    delay INTEGER NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_char_id) REFERENCES keybr_phrase_chars(id)\n"
                                   ")");
                    database->exec("CREATE INDEX keybr_phrase_chars_phrase_id_idx ON keybr_phrase_chars(phrase_id)");
                    database->exec("CREATE INDEX keybr_stat_errors_char_id_idx ON keybr_stat_errors(phrase_char_id)");
                    database->exec("CREATE INDEX keybr_stat_delays_char_id_idx ON keybr_stat_delays(phrase_char_id)");
                    {
                        auto sql_select = database->create_query();
                        sql_select << "SELECT id, phrase FROM keybr_phrases";
                        sql.reset() << "INSERT INTO keybr_phrase_chars (phrase_id, pos, ch) VALUES";
                        sql.add_array(3);
                        while (sql_select.step()) {
                            const auto phrase_id = sql_select.get_uint64();
                            const auto phrase = sql_select.get_string();
                            for (int64_t pos = -1; pos < static_cast<int64_t>(phrase.size()); ++pos) {
                                sql.clear_bindings()
                                   .bind(phrase_id)
                                   .bind(pos)
                                   .bind(std::string(1, pos < 0 ? ' ' : phrase[pos]))
                                   .step();
                            }
                        }
                    }
                    database->exec("INSERT INTO keybr_stat_delays\n"
                                   "SELECT s.create_date, c.id, s.delay\n"
                                   "FROM keybr_stats s\n"
                                   "JOIN keybr_phrase_chars c ON s.phrase_id = c.phrase_id AND s.pos = c.pos\n"
                                   "WHERE s.delay != 0");
                    database->exec("INSERT INTO keybr_stat_errors\n"
                                   "SELECT s.create_date, c.id, s.errors\n"
                                   "FROM keybr_stats s\n"
                                   "JOIN keybr_phrase_chars c ON s.phrase_id = c.phrase_id AND s.pos = c.pos\n"
                                   "WHERE s.errors != 0");
                    database->exec("DROP TABLE keybr_stats");
                    break;
                default:
                    throw std::runtime_error("Can't update database");
                }
            }
        }
        sql.reset() << "PRAGMA user_version =" << db_version << Query::step;
        database->exec("VACUUM");
    }
}

int64_t TrainerData::phrase_count() const
{
    return phrases.size();
}

char TrainerData::current_symbol() const
{
    return phrases.at(phrase_idx).get_symbol(symbol_idx);
}

const Phrase &TrainerData::current_phrase() const
{
    return phrases.at(phrase_idx);
}

const Phrase &TrainerData::get_phrase(int64_t idx) const
{
    return phrases.at(idx);
}

int64_t TrainerData::get_symbol_idx() const
{
    return symbol_idx;
}

int64_t TrainerData::get_phrase_idx() const
{
    return phrase_idx;
}
