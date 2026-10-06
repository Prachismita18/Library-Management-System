#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>

class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;
    bool isConnected;

public:
    DatabaseManager(const std::string& dbPath = "database/library.db");
    ~DatabaseManager();

    // Disable copy constructor and assignment operator (RAII / Resource management)
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool open();
    void close();
    bool initializeSchema(const std::string& schemaFilePath = "database/schema.sql");
    bool seedInitialData();

    sqlite3* getDbHandle() const;

    // Transaction Management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Utility execution
    bool executeRawSQL(const std::string& sql);
    int64_t getLastInsertedRowId() const;
};

#endif // DATABASE_MANAGER_HPP
