#include "DatabaseManager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QDir>
#include <QCryptographicHash>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize()
{
    if (QSqlDatabase::contains("LuxeMineConnection")) {
        m_db = QSqlDatabase::database("LuxeMineConnection");
        return true;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", "LuxeMineConnection");

    // Create data directory if not exists
    QDir dir(QDir::currentPath());
    if (!dir.exists("data")) {
        dir.mkdir("data");
    }

    // Database path
    QString dbPath = dir.filePath("data/luxemine.db");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Database open failed:" << m_db.lastError().text();
        return false;
    }

    return createTables();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    // -----------------------------
    // USERS (login only)
    // -----------------------------
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            is_active INTEGER DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )")) {
        qCritical() << "users table error:" << query.lastError();
        return false;
    }

    // -----------------------------
    // EMPLOYEES (HR / payroll)
    // -----------------------------
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS employees (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            full_name TEXT,
            account_no TEXT,
            base_salary REAL DEFAULT 0,
            working_hours_per_day REAL DEFAULT 0,
            FOREIGN KEY(user_id) REFERENCES users(id)
        );
    )")) {
        qCritical() << "employees table error:" << query.lastError();
        return false;
    }

    // -----------------------------
    // ROLES (master)
    // -----------------------------
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS roles (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL
        );
    )")) {
        qCritical() << "roles table error:" << query.lastError();
        return false;
    }

    // -----------------------------
    // USER ↔ ROLE (multi-role)
    // -----------------------------
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS user_roles (
            user_id INTEGER NOT NULL,
            role_id INTEGER NOT NULL,
            PRIMARY KEY (user_id, role_id),
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(role_id) REFERENCES roles(id)
        );
    )")) {
        qCritical() << "user_roles table error:" << query.lastError();
        return false;
    }

    // -----------------------------
    // SELLER PROFILE (seller-only)
    // -----------------------------
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS seller_profile (
            employee_id INTEGER PRIMARY KEY,
            selling_percentage REAL NOT NULL,
            total_sales REAL DEFAULT 0,
            amount_paid REAL DEFAULT 0,
            FOREIGN KEY(employee_id) REFERENCES employees(id)
        );
    )")) {
        qCritical() << "seller_profile table error:" << query.lastError();
        return false;
    }

    // -----------------------------
    // EMPLOYEE PAYMENTS (MONTHLY)
    // -----------------------------
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS employee_payments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            employee_id INTEGER NOT NULL,
            month INTEGER NOT NULL,
            year INTEGER NOT NULL,
            worked_days INTEGER DEFAULT 0,
            advance_paid REAL DEFAULT 0,
            paid_amount REAL DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            UNIQUE(employee_id, month, year),
            FOREIGN KEY(employee_id) REFERENCES employees(id)
        );
    )")) {
        qCritical() << "employee_payments table error:"
                    << query.lastError().text();
        return false;
    }

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS "order_book_detail" (
            "id"	INTEGER NOT NULL UNIQUE,
            "sellerName"	TEXT NOT NULL,
            "sellerId"	TEXT NOT NULL,
            "partyId"	TEXT NOT NULL,
            "partyName"	TEXT NOT NULL,
            "jobNo"	TEXT NOT NULL,
            "orderNo"	TEXT NOT NULL,
            "clientId"	TEXT,
            "agencyId"	TEXT,
            "shopId"	TEXT,
            "reteailleId"	TEXT,
            "starId"	TEXT,
            "address"	TEXT,
            "city"	TEXT,
            "state"	TEXT,
            "country"	TEXT,
            "orderDate"	TEXT NOT NULL,
            "deliveryDate"	TEXT NOT NULL,
            "productName"	TEXT,
            "productPis"	INTEGER,
            "approxProductWt"	REAL,
            "metalPrice"	REAL,
            "metalName"	TEXT,
            "metalPurity"	TEXT,
            "metalColor"	TEXT,
            "sizeNo"	REAL,
            "sizeMM"	REAL,
            "length"	REAL,
            "width"	REAL,
            "height"	REAL,
            "diaPacific"	TEXT,
            "diaPurity"	TEXT,
            "diaColor"	TEXT,
            "diaPrice"	REAL,
            "stPacific"	TEXT,
            "stPurity"	TEXT,
            "stColor"	TEXT,
            "stPrice"	REAL,
            "designNo1"	TEXT,
            "designNo2"	TEXT,
            "image1Path"	TEXT,
            "image2Path"	TEXT,
            "metalCertiName"	TEXT,
            "metalCertiType"	TEXT,
            "diaCertiName"	TEXT,
            "diaCertiType"	TEXT,
            "pesSaki"	TEXT,
            "chainLock"	TEXT,
            "polish"	TEXT,
            "settingLebour"	TEXT,
            "metalStemp"	TEXT,
            "paymentMethod"	TEXT,
            "totalAmount"	REAL,
            "advance"	REAL,
            "remaining"	REAL,
            "note"	TEXT,
            "extraDetail"	TEXT,
            "isSaved"	INTEGER DEFAULT 0,
            PRIMARY KEY("id" AUTOINCREMENT)
        );
    )")) {
        qCritical() << "seller_profile table error:" << query.lastError();
        return false;
    }

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS "order_status" (
            "jobNo" TEXT PRIMARY KEY,

            "Designer" TEXT NOT NULL DEFAULT 'Pending',
            "Manufacturer" TEXT NOT NULL DEFAULT 'Pending',
            "Accountant" TEXT NOT NULL DEFAULT 'Pending',

            "Order_Note" TEXT,
            "Design_Note" TEXT,
            "Quality_Note" TEXT
        );
    )")) {
        qCritical() << "seller_profile table error:" << query.lastError();
        return false;
    }


    // -----------------------------
    // SEED ROLES (SAFE)
    // -----------------------------
    QStringList defaultRoles = {
        "admin",
        "seller",
        "designer",
        "manufacturer",
        "accountant"
    };

    for (const QString &role : defaultRoles) {
        QSqlQuery check(m_db);
        check.prepare("SELECT COUNT(*) FROM roles WHERE name = ?");
        check.addBindValue(role);
        check.exec();
        check.next();

        if (check.value(0).toInt() == 0) {
            QSqlQuery insert(m_db);
            insert.prepare("INSERT INTO roles (name) VALUES (?)");
            insert.addBindValue(role);
            insert.exec();
        }
    }

    // -----------------------------
    // DEFAULT ADMIN (DEV ONLY)
    // -----------------------------
    QSqlQuery adminCheck(m_db);
    adminCheck.exec("SELECT COUNT(*) FROM users WHERE username='admin'");
    adminCheck.next();

    if (adminCheck.value(0).toInt() == 0) {
        QSqlQuery insertAdmin(m_db);
        insertAdmin.prepare(R"(
        INSERT INTO users (username, password_hash, is_active)
        VALUES ('admin', :pass, 1)
    )");

        insertAdmin.bindValue(
            ":pass",
            QString(
                QCryptographicHash::hash(
                    QByteArray("admin123"),
                    QCryptographicHash::Sha256
                    ).toHex()
                )
            );

        insertAdmin.exec();

        int adminUserId = insertAdmin.lastInsertId().toInt();

        // Assign admin role
        QSqlQuery roleQuery(m_db);
        roleQuery.exec("SELECT id FROM roles WHERE name='admin'");
        roleQuery.next();

        int adminRoleId = roleQuery.value(0).toInt();

        QSqlQuery map(m_db);
        map.prepare("INSERT INTO user_roles (user_id, role_id) VALUES (?, ?)");
        map.addBindValue(adminUserId);
        map.addBindValue(adminRoleId);
        map.exec();
    }


    return true;
}
