#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct sqlite3;

namespace edge {

struct CacheRecord {
    int64_t id = 0;
    std::string topic;
    std::string payload;
    int uploaded = 0;
};

class SQLiteCache {
public:
    ~SQLiteCache();

    bool open(const std::string &path);
    bool createTables();
    bool insertSensorData(const std::string &json);
    bool insertAlarmEvent(const std::string &json);
    bool queryPending(std::vector<CacheRecord> &records, int limit);
    bool markUploaded(int64_t id);
    void close();

private:
    bool exec(const std::string &sql);
    bool insertRecord(const std::string &topic, const std::string &payload);

    sqlite3 *db_ = nullptr;
};

} // namespace edge
