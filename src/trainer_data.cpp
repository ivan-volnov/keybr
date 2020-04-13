#include "trainer_data.h"
#include <sqlite_database/sqlite_database.h>
#include "utility/tools.h"
#include "config.h"


const int64_t required_db_version = 4;


TrainerData::TrainerData()
{
    const auto db_filepath = Config::instance().get_db_filepath();
    if (std::filesystem::exists(db_filepath)) {
        tools::clone_file(db_filepath, Config::instance().get_backup_db_filepath());
    }
    database = SqliteDatabase::open(db_filepath);
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
        sql.reset();
        {
            auto transaction = database->begin_transaction();
            for (; db_version < required_db_version; ++db_version) {
                switch (db_version) {
                case 0:
                    database->exec("CREATE TABLE keybr_phrases (\n"
                                   "    id INTEGER PRIMARY KEY,\n"
                                   "    phrase TEXT UNIQUE,\n"
                                   "    translation TEXT NOT NULL\n"
                                   ")");
                    database->exec("CREATE TABLE keybr_phrase_chars (\n"
                                   "    id INTEGER PRIMARY KEY,\n"
                                   "    phrase_id INTEGER NOT NULL,\n"
                                   "    pos INTEGER NOT NULL,\n"
                                   "    ch TEXT NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_id) REFERENCES keybr_phrases(id)\n"
                                   ")");
                    database->exec("CREATE TABLE keybr_stat_errors (\n"
                                   "    id INTEGER PRIMARY KEY,\n"
                                   "    create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
                                   "    phrase_char_id INTEGER NOT NULL,\n"
                                   "    errors INTEGER NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_char_id) REFERENCES keybr_phrase_chars(id)\n"
                                   ")");
                    database->exec("CREATE TABLE keybr_stat_delays (\n"
                                   "    id INTEGER PRIMARY KEY,\n"
                                   "    create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
                                   "    phrase_char_id INTEGER NOT NULL,\n"
                                   "    delay INTEGER NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_char_id) REFERENCES keybr_phrase_chars(id)\n"
                                   ")");
                    database->exec("CREATE INDEX keybr_phrase_chars_phrase_id_idx ON keybr_phrase_chars(phrase_id)");
                    database->exec("CREATE INDEX keybr_stat_errors_char_id_idx ON keybr_stat_errors(phrase_char_id)");
                    database->exec("CREATE INDEX keybr_stat_delays_char_id_idx ON keybr_stat_delays(phrase_char_id)");
                    db_version = required_db_version - 1; // one step update to actual version
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
                case 3:
                    database->exec("ALTER TABLE keybr_stat_errors RENAME TO keybr_stat_errors_old");
                    database->exec("ALTER TABLE keybr_stat_delays RENAME TO keybr_stat_delays_old");
                    database->exec("CREATE TABLE keybr_stat_errors (\n"
                                   "    id INTEGER PRIMARY KEY,\n"
                                   "    create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
                                   "    phrase_char_id INTEGER NOT NULL,\n"
                                   "    errors INTEGER NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_char_id) REFERENCES keybr_phrase_chars(id)\n"
                                   ")");
                    database->exec("CREATE TABLE keybr_stat_delays (\n"
                                   "    id INTEGER PRIMARY KEY,\n"
                                   "    create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
                                   "    phrase_char_id INTEGER NOT NULL,\n"
                                   "    delay INTEGER NOT NULL,\n"
                                   "    FOREIGN KEY (phrase_char_id) REFERENCES keybr_phrase_chars(id)\n"
                                   ")");
                    database->exec("INSERT INTO keybr_stat_errors (create_date, phrase_char_id, errors)\n"
                                   "SELECT create_date, phrase_char_id, errors\n"
                                   "FROM keybr_stat_errors_old\n"
                                   "ORDER BY ROWID");
                    database->exec("INSERT INTO keybr_stat_delays (create_date, phrase_char_id, delay)\n"
                                   "SELECT create_date, phrase_char_id, delay\n"
                                   "FROM keybr_stat_delays_old\n"
                                   "ORDER BY ROWID");
                    database->exec("DROP TABLE keybr_stat_errors_old");
                    database->exec("DROP TABLE keybr_stat_delays_old");
                    database->exec("CREATE INDEX keybr_stat_errors_char_id_idx ON keybr_stat_errors(phrase_char_id)");
                    database->exec("CREATE INDEX keybr_stat_delays_char_id_idx ON keybr_stat_delays(phrase_char_id)");
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
