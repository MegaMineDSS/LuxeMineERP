#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>

class DatabaseManager
{
public:
    // Singleton access
    static DatabaseManager& instance();

    // Initialize database (open + create tables)
    bool initialize();

    // Get active database connection
    QSqlDatabase database() const;

private:
    DatabaseManager();
    ~DatabaseManager();

    // Disable copy
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();

private:
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
