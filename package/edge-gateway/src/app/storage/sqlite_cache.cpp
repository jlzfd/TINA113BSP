#include "storage/sqlite_cache.h"

#include "common/log.h"

#include <sqlite3.h>

namespace edge {

SQLiteCache::~SQLiteCache()
{
    close();
}

bool SQLiteCache::open(const std::string &path)
{
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        EG_ERROR("sqlite open %s failed: %s", path.c_str(), sqlite3_errmsg(db_));
        return false;
    }
    return createTables();
}

bool SQLiteCache::createTables()
{
    return exec("CREATE TABLE IF NOT EXISTS cache_records ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "topic TEXT NOT NULL,"
                "payload TEXT NOT NULL,"
                "uploaded INTEGER DEFAULT 0,"
                "created_at INTEGER DEFAULT (strftime('%s','now')));");
}

bool SQLiteCache::insertSensorData(const std::string &json)
{
    return insertRecord("edge/sensor", json);
}

bool SQLiteCache::insertAlarmEvent(const std::string &json)
{
    return insertRecord("edge/alarm", json);
}

bool SQLiteCache::queryPending(std::vector<CacheRecord> &records, int limit)
{
    records.clear();
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "SELECT id,topic,payload,uploaded FROM cache_records WHERE uploaded=0 ORDER BY id LIMIT ?;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, limit);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        CacheRecord r;
        r.id = sqlite3_column_int64(stmt, 0);
        r.topic = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        r.payload = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        r.uploaded = sqlite3_column_int(stmt, 3);
        records.push_back(r);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool SQLiteCache::markUploaded(int64_t id)
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "UPDATE cache_records SET uploaded=1 WHERE id=?;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int64(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

void SQLiteCache::close()
{
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool SQLiteCache::exec(const std::string &sql)
{
    char *err = nullptr;
    int ret = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
    if (ret != SQLITE_OK) {
        EG_ERROR("sqlite exec failed: %s", err ? err : "unknown");
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool SQLiteCache::insertRecord(const std::string &topic, const std::string &payload)
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "INSERT INTO cache_records(topic,payload) VALUES(?,?);", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, topic.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, payload.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

} // namespace edge
