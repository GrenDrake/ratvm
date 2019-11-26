#include <ctime>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#include "gamedata.h"
#include <sqlite3.h>


static sqlite3* openDatabase();

std::string getBasePath() {
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    std::string path = homedir;
    if (path.back() != '/') path += '/';
    return path;
#else
    return "./";
#endif
}

sqlite3* openDatabase() {
    sqlite3 *db;
    int rc;

    std::string dbPath = getBasePath() + "quollvm.db";

    rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        std::stringstream ss;
        ss << "Can't open database: " << sqlite3_errmsg(db);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    const char *crateTableStatement = "CREATE TABLE files ( name, gameid, lastmod, content, PRIMARY KEY (name, gameid))";
    const char *sqlText = "SELECT count(name) FROM sqlite_master WHERE type='table' AND name='files'";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sqlText, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::stringstream ss;
        ss << "Failed to prepare statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        std::stringstream ss;
        ss << "Failed to step SQL statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    } else {
        int count = sqlite3_column_int(stmt, 0);
        if (count <= 0) {
            sqlite3_stmt *createTableStmt;
            rc = sqlite3_prepare_v2(db, crateTableStatement, -1, &createTableStmt, nullptr);
            if (rc != SQLITE_OK) {
                std::stringstream ss;
                ss << "Failed to prepare create table statement: ";
                ss << sqlite3_errmsg(db);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                throw GameError(ss.str());
            }
            rc = sqlite3_step(createTableStmt);
            sqlite3_finalize(createTableStmt);
            if (rc != SQLITE_DONE) {
                std::stringstream ss;
                ss << "Failed to execute create table statement: ";
                ss << sqlite3_errmsg(db);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                throw GameError(ss.str());
            }
        }
    }

    sqlite3_finalize(stmt);
    return db;
}



FileList GameData::getFileList(const std::string &gameId) {
    const char *getListSQL = "SELECT name, lastmod FROM files WHERE gameid=?";
    sqlite3_stmt *stmt = nullptr;
    FileList list;

    sqlite3 *db = openDatabase();
    if (!db) return list;

    int rc = sqlite3_prepare_v2(db, getListSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::stringstream ss;
        ss << "Failed to prepare list files statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }
    sqlite3_bind_text(stmt, 1, gameId.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        // no files, so we don't need to add any records
    } else if (rc == SQLITE_ROW) {
        do {
            const unsigned char *rawName = sqlite3_column_text(stmt, 0);
            int timestamp = sqlite3_column_int(stmt, 1);

            FileRecord record;
            for (int i = 0; rawName[i] != 0; ++i)
                record.name += static_cast<char>(rawName[i]);
            const time_t t = timestamp;
            record.date = ctime(&t);
            while (record.date.back() == '\n') {
                std::string::size_type pos = record.date.size() - 1;
                record.date.erase(pos);
            }
            list.push_back(record);

            rc = sqlite3_step(stmt);
        } while (rc == SQLITE_ROW);
        if (rc != SQLITE_DONE) {
            std::stringstream ss;
            ss << "Error while loading file list: ";
            ss << sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw GameError(ss.str());
        }
    } else {
        std::stringstream ss;
        ss << "Failed to execute list files statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return list;
}

Value GameData::getFile(const std::string &fileName, const std::string &gameId) {
    const char *sqlStmt = "SELECT content FROM files WHERE name=? AND gameid=?";
    sqlite3_stmt *stmt = nullptr;

    sqlite3 *db = openDatabase();
    if (!db) return noneValue;

    int rc = sqlite3_prepare_v2(db, sqlStmt, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::stringstream ss;
        ss << "Failed to prepare load file statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    sqlite3_bind_text(stmt, 1, fileName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, gameId.c_str(),   -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        // no such file
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return noneValue;
    } else if (rc != SQLITE_ROW) {
        std::stringstream ss;
        ss << "Failed to load file: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    const char *rawData = static_cast<const char*>(sqlite3_column_blob(stmt, 0));
    unsigned dataSize = sqlite3_column_bytes(stmt, 0);
    Value listId = makeNew(Value::List);
    ListDef &list = getList(listId.value);

    unsigned i = 0;
    while (i < dataSize) {
        unsigned raw = 0;
        raw |= rawData[i] & 0xFF;
        raw |= (rawData[i + 1] & 0xFF) << 8;
        raw |= (rawData[i + 2] & 0xFF) << 16;
        raw |= (rawData[i + 3] & 0xFF) << 24;

        Value v;
        v.type = Value::Integer;
        v.value = raw;
        list.items.push_back(v);
        i += 4;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return listId;
}

bool GameData::saveFile(const std::string &filename, const std::string &gameId, const ListDef *list) {
    if (list == nullptr) return false;
    const char *sqlStmt = "INSERT OR REPLACE INTO files (name, gameid, lastmod, content) VALUES (?, ?, ?, ?)";
    sqlite3_stmt *stmt = nullptr;

    sqlite3 *db = openDatabase();
    if (!db) return list;

    int rc = sqlite3_prepare_v2(db, sqlStmt, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::stringstream ss;
        ss << "Failed to prepare save file statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    time_t now = time(nullptr);
    sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, gameId.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 3, now);

    unsigned dataSize = list->items.size() * 4;
    char *data = static_cast<char*>(malloc(dataSize));
    unsigned i = 0;
    for (const Value &v : list->items) {
        if (v.type != Value::Integer) {
            free(data);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw GameError("List of data to save must contain only integers.");
            return false;
        }
        data[i]   = v.value & 0xFF;
        data[i+1] = (v.value >> 8) & 0xFF;
        data[i+2] = (v.value >> 16) & 0xFF;
        data[i+3] = (v.value >> 24) & 0xFF;
        i += 4;
    }

    sqlite3_bind_blob(stmt, 4, data, dataSize, free);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::stringstream ss;
        ss << "Failed to save file: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

bool GameData::deleteFile(const std::string &filename, const std::string &gameId) {
    const char *SQL = "DELETE FROM files WHERE gameid=? AND name=?";
    sqlite3_stmt *stmt = nullptr;

    sqlite3 *db = openDatabase();
    if (!db) return false;

    int rc = sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::stringstream ss;
        ss << "Failed to prepare delete file statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }
    sqlite3_bind_text(stmt, 1, gameId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, filename.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::stringstream ss;
        ss << "Failed to execute delete file statement: ";
        ss << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw GameError(ss.str());
    }

    int changes = sqlite3_total_changes(db);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return changes > 0 ? true : false;
}