#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <sstream>
#include "global.h"



typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
class SqliteDatabase;



class Query
{
    friend class SqliteDatabase;

public:
    ~Query();
    Query(const Query &) = delete;
    Query &operator=(const Query &) = delete;

    Query &operator<<(const char *value);
    Query &operator<<(const std::string &value);

    inline Query &operator<<(Query &(*pf)(Query &))
    {
        return pf(*this);
    }
    template<typename T>
    Query &operator<<(const T &value)
    {
        if (!ss.str().empty()) {
            ss << ' ';
        }
        ss << value;
        return *this;
    }

    Query &bind(const char *str, bool constant = false) MAYTHROW;
    Query &bind(const std::string &str, bool constant = false) MAYTHROW;
    Query &bind(int32_t value) MAYTHROW;
    Query &bind(uint32_t value) MAYTHROW;
    Query &bind(int64_t value) MAYTHROW;
    Query &bind(uint64_t value) MAYTHROW;
    Query &bind() MAYTHROW;

    bool step() MAYTHROW;
    static Query &step(Query &query) MAYTHROW;

    void add_array(size_t size) MAYTHROW;

    Query &reset() noexcept;
    Query &clear_bindings() noexcept;

    bool is_null() const noexcept;
    std::string get_string() MAYTHROW;
    int32_t get_int32() MAYTHROW;
    uint32_t get_uint32() MAYTHROW;
    int64_t get_int64() MAYTHROW;
    uint64_t get_uint64() MAYTHROW;

    std::shared_ptr<SqliteDatabase> get_database() const noexcept;

private:
    void prepare() MAYTHROW;
    Query(std::shared_ptr<SqliteDatabase> database);

    std::shared_ptr<SqliteDatabase> database;
    std::stringstream ss;
    sqlite3_stmt *stmt = nullptr;
    int bind_idx = 0;
    int col_idx = 0;
    int col_count = 0;
};



class Transaction
{
    friend class SqliteDatabase;

public:
    ~Transaction() MAYTHROW;
    Transaction(const Transaction &) = delete;
    Transaction &operator=(const Transaction &) = delete;

    void commit() MAYTHROW;
    void rollback() MAYTHROW;

private:
    Transaction(std::shared_ptr<SqliteDatabase> database) MAYTHROW;

    std::shared_ptr<SqliteDatabase> database;
    const int exception_count;
};



class SqliteDatabase : public std::enable_shared_from_this<SqliteDatabase>
{
    friend class Query;

public:
    SqliteDatabase(sqlite3 *db) noexcept;
    ~SqliteDatabase();

    static std::shared_ptr<SqliteDatabase> open(const std::string &filename) MAYTHROW;

    void exec(const char *sql) MAYTHROW;
    Query create_query();
    Transaction begin_transaction();

private:
    sqlite3 *db = nullptr;
};



class DatabaseException : public std::exception
{
public:
    DatabaseException(sqlite3 *db);
    DatabaseException(const char *msg);
    const char *what() const noexcept override;

private:
    std::string message;
};


#endif // SQLITEDATABASE_H
