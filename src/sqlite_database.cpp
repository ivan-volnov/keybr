#include "sqlite_database.h"
#include "3rdparty/sqlite3/sqlite3.h"
#include <iostream>



Query::Query(std::shared_ptr<SqliteDatabase> database) :
    database(std::move(database))
{

}

Query::~Query()
{
    sqlite3_finalize(stmt);
}

Query &Query::operator<<(const char *value)
{
    if (!value) {
        return *this;
    }
    if (!ss.str().empty()) {
        ss << ' ';
    }
    ss << value;
    return *this;
}

Query &Query::operator<<(const std::string &value)
{
    if (value.empty()) {
        return *this;
    }
    if (!ss.str().empty()) {
        ss << ' ';
    }
    ss << value;
    return *this;
}

void Query::prepare() MAYTHROW
{
    if (sqlite3_prepare_v2(database->db, ss.str().c_str(), ss.str().size(), &stmt, nullptr) != SQLITE_OK) {
        throw DatabaseException(database->db);
    }
    col_count = sqlite3_column_count(stmt);
}

Query &Query::bind(const char *str, bool constant) MAYTHROW
{
    if (!stmt) {
        prepare();
    }
    if (sqlite3_bind_text(stmt, ++bind_idx, str, -1, constant ? SQLITE_STATIC : SQLITE_TRANSIENT) != SQLITE_OK) {
        throw DatabaseException(database->db);
    }
    return *this;
}

Query &Query::bind(const std::string &str, bool constant) MAYTHROW
{
    if (!stmt) {
        prepare();
    }
    if (sqlite3_bind_text(stmt, ++bind_idx, str.c_str(), str.size(), constant ? SQLITE_STATIC : SQLITE_TRANSIENT) != SQLITE_OK) {
        throw DatabaseException(database->db);
    }
    return *this;
}

Query &Query::bind(int32_t value) MAYTHROW
{
    if (!stmt) {
        prepare();
    }
    if (sqlite3_bind_int(stmt, ++bind_idx, value) != SQLITE_OK) {
        throw DatabaseException(database->db);
    }
    return *this;
}

Query &Query::bind(uint32_t value) MAYTHROW
{
    return bind(static_cast<int64_t>(value));
}

Query &Query::bind(int64_t value) MAYTHROW
{
    if (!stmt) {
        prepare();
    }
    if (sqlite3_bind_int64(stmt, ++bind_idx, value) != SQLITE_OK) {
        throw DatabaseException(database->db);
    }
    return *this;
}

Query &Query::bind(uint64_t value) MAYTHROW
{
    if (value > std::numeric_limits<int64_t>::max()) {
        throw DatabaseException("Can't bind value. Sqlite doesn't support uint64 type");
    }
    return bind(static_cast<int64_t>(value));
}

Query &Query::bind() MAYTHROW
{
    if (!stmt) {
        prepare();
    }
    if (sqlite3_bind_null(stmt, ++bind_idx) != SQLITE_OK) {
        throw DatabaseException(database->db);
    }
    return *this;
}

bool Query::step() MAYTHROW
{
    if (!stmt) {
        prepare();
    }
    col_idx = 0;
    switch (sqlite3_step(stmt)) {
    case SQLITE_ROW:
        return true;
    case SQLITE_DONE:
        return false;
    default:
        throw DatabaseException(database->db);
    }
}

Query &Query::step(Query &query) MAYTHROW
{
    query.step();
    return query;
}

void Query::add_array(size_t size) MAYTHROW
{
    if (!ss.str().empty()) {
        ss << ' ';
    }
    for (int i = 0; i < size; ++i) {
        ss << (i ? ",?" : "(?");
    }
    ss << ')';
}



Query &Query::reset() noexcept
{
    sqlite3_finalize(stmt);
    ss = std::stringstream();
    stmt = nullptr;
    bind_idx = 0;
    col_idx = 0;
    col_count = 0;
    return *this;
}

Query &Query::clear_bindings() noexcept
{
    if (stmt) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    bind_idx = 0;
    col_idx = 0;
    col_count = 0;
    return *this;
}

bool Query::is_null() const noexcept
{
    return sqlite3_column_type(stmt, col_idx) == SQLITE_NULL;
}

std::string Query::get_string() MAYTHROW
{
    if (col_idx >= col_count) {
        throw DatabaseException("Column is out of range");
    }
    auto str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_idx++));
    return str ? str : "";
}

int32_t Query::get_int32() MAYTHROW
{
    if (col_idx >= col_count) {
        throw DatabaseException("Column is out of range");
    }
    return sqlite3_column_int(stmt, col_idx++);
}

uint32_t Query::get_uint32() MAYTHROW
{
    const auto value = get_int64();
    if (value < 0 || value > std::numeric_limits<uint32_t>::max()) {
        throw DatabaseException("uint32 value is out of range");
    }
    return value;
}

int64_t Query::get_int64() MAYTHROW
{
    if (col_idx >= col_count) {
        throw DatabaseException("Column is out of range");
    }
    return sqlite3_column_int64(stmt, col_idx++);
}

uint64_t Query::get_uint64() MAYTHROW
{
    const auto value = get_int64();
    if (value < 0) {
        throw DatabaseException("uint64 value is out of range");
    }
    return value;
}

std::shared_ptr<SqliteDatabase> Query::get_database() const noexcept
{
    return database;
}



Transaction::Transaction(std::shared_ptr<SqliteDatabase> database) MAYTHROW :
    database(std::move(database)), exception_count(std::uncaught_exceptions())
{
    this->database->exec("BEGIN");
}

Transaction::~Transaction() MAYTHROW
{
    if (!database) {
        return;
    }
    if (exception_count == std::uncaught_exceptions()) {
        database->exec("COMMIT");
        return;
    }
    // Called during stack unwinding. Rollback and don't throw
    try {
        database->exec("ROLLBACK");
    } catch (const std::exception &e) {
        std::cerr << "Transaction rollback error: " << e.what() << std::endl;
    }
}

void Transaction::commit() MAYTHROW
{
    if (!database) {
        throw DatabaseException("Can't commit on inactive transaction");
    }
    database->exec("COMMIT");
    database.reset();
}

void Transaction::rollback() MAYTHROW
{
    if (!database) {
        throw DatabaseException("Can't rollback on inactive transaction");
    }
    database->exec("ROLLBACK");
    database.reset();
}



SqliteDatabase::SqliteDatabase(sqlite3 *db) noexcept :
    db(db)
{

}

SqliteDatabase::~SqliteDatabase()
{
    sqlite3_close(db);
}

std::shared_ptr<SqliteDatabase> SqliteDatabase::open(const std::string &filename) MAYTHROW
{
    sqlite3 *db;
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        throw DatabaseException(db);
    }
    return std::make_shared<SqliteDatabase>(db);
}



void SqliteDatabase::exec(const char *sql) MAYTHROW
{
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw DatabaseException(db);
    }
    const auto res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (res != SQLITE_ROW && res != SQLITE_DONE) {
        throw DatabaseException(db);
    }
}

Query SqliteDatabase::create_query()
{
    return Query(shared_from_this());
}

Transaction SqliteDatabase::begin_transaction()
{
    return Transaction(shared_from_this());
}



DatabaseException::DatabaseException(sqlite3 *db) :
    message(db ? sqlite3_errmsg(db) : "Database isn't open")
{

}

DatabaseException::DatabaseException(const char *msg) :
    message(msg)
{

}

const char *DatabaseException::what() const noexcept
{
    return message.c_str();
}
