#include "DatabaseManager.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>

DatabaseManager &DatabaseManager::instance() {
  static DatabaseManager instance;
  return instance;
}

DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager() {
  if (m_db.isOpen()) {
    m_db.close();
  }
}

bool DatabaseManager::initialize() {
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

QSqlDatabase DatabaseManager::database() const { return m_db; }

bool DatabaseManager::createTables() {
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
    qCritical() << "employee_payments table error:" << query.lastError().text();
    return false;
  }

  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS order_book_detail (
            id INTEGER PRIMARY KEY AUTOINCREMENT,

            order_id INTEGER NOT NULL,
            job_id INTEGER NOT NULL,

            sellerName TEXT NOT NULL,
            sellerId TEXT NOT NULL,

            partyId TEXT NOT NULL,
            partyName TEXT NOT NULL,

            clientId TEXT,
            agencyId TEXT,
            shopId TEXT,
            reteillerId TEXT,

            starId TEXT,
            address TEXT,
            city TEXT,
            state TEXT,
            country TEXT,

            orderDate TEXT NOT NULL,
            deliveryDate TEXT NOT NULL,

            productName TEXT,
            productPis INTEGER,

            approxProductWt REAL,
            approxDiamondWt REAL,

            metalPrice REAL,
            metalName TEXT,
            metalPurity TEXT,
            metalColor TEXT,

            sizeNo REAL,
            sizeMM REAL,
            length REAL,
            width REAL,
            height REAL,

            diaPacific TEXT,
            diaPurity TEXT,
            diaColor TEXT,
            diaPrice REAL,

            stPacific TEXT,
            stPurity TEXT,
            stColor TEXT,
            stPrice REAL,

            designNo TEXT,
            image1Path TEXT,
            image2Path TEXT,

            metalCertiName TEXT,
            metalCertiType TEXT,
            diaCertiName TEXT,
            diaCertiType TEXT,

            pesSaki TEXT,
            chainLock TEXT,
            polish TEXT,
            settingLebour TEXT,
            metalStemp TEXT,

            paymentMethod TEXT,
            totalAmount REAL,
            advance REAL,
            remaining REAL,

            note TEXT,
            extraDetail TEXT,

            isSaved INTEGER DEFAULT 0,

            FOREIGN KEY (order_id) REFERENCES orders(order_id),
            FOREIGN KEY (job_id) REFERENCES jobs(job_id)
        );
    )")) {
    qCritical() << "seller_profile table error:" << query.lastError();
    return false;
  }

  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS order_status (
            job_id INTEGER PRIMARY KEY,

            Designer TEXT NOT NULL DEFAULT 'Pending',
            Manufacturer TEXT NOT NULL DEFAULT 'Pending',
            Accountant TEXT NOT NULL DEFAULT 'Pending',

            Order_Note TEXT,
            Design_Note TEXT,
            Quality_Note TEXT,

            FOREIGN KEY (job_id) REFERENCES jobs(job_id)
        );
    )")) {
    qCritical() << "seller_profile table error:" << query.lastError();
    return false;
  }
  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS jobs (
            job_id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )")) {
    qCritical() << "seller_profile table error:" << query.lastError();
    return false;
  }

  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS seller_order_counter (
            seller_id TEXT PRIMARY KEY,
            last_order_no INTEGER NOT NULL DEFAULT 0
        );
    )")) {
    qCritical() << "seller_profile table error:" << query.lastError();
    return false;
  }

  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS orders (
            order_id INTEGER PRIMARY KEY AUTOINCREMENT,
            job_id INTEGER NOT NULL,
            seller_id TEXT NOT NULL,
            seller_order_seq INTEGER NOT NULL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,

            UNIQUE (seller_id, seller_order_seq),
            FOREIGN KEY (job_id) REFERENCES jobs(job_id)
        );
    )")) {
    qCritical() << "seller_profile table error:" << query.lastError();
    return false;
  }

  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS casting_entry (
            id INTEGER PRIMARY KEY AUTOINCREMENT,

            job_id INTEGER NOT NULL,
            order_id INTEGER,

            casting_date TEXT NOT NULL,
            casting_name TEXT NOT NULL,
            pcs INTEGER,

            -- Issue section
            issue_metal_name TEXT,
            issue_metal_purity TEXT,
            issue_metal_wt REAL,
            issue_diamond_pcs INTEGER,
            issue_diamond_wt REAL,
            issue_diamond_category TEXT,
            issue_stone_pcs INTEGER,
            issue_stone_wt REAL,

            -- Receive section
            receive_runner_wt REAL,
            receive_product_wt REAL,
            receive_diamond_pcs INTEGER,
            receive_diamond_wt REAL,
            receive_stone_pcs INTEGER,
            receive_stone_wt REAL,

            -- Meta
            accountant_id INTEGER,
            status TEXT DEFAULT 'OPEN',
            dia_price REAL DEFAULT 0,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,

            FOREIGN KEY(job_id) REFERENCES jobs(id)
        );
    )")) {
    qCritical() << "seller_profile table error:" << query.lastError();
    return false;
  }

  // -----------------------------
  // MIGRATION: Ensure dia_price exists
  // -----------------------------
  // Check if column exists, if not add it.
  // SQLite doesn't support IF NOT EXISTS in ADD COLUMN, so we check pragma
  // (simple way) or just try blindly (ignoring specific error). Safest way
  // without complex pragma parsing: try to select it, if fails, add it.
  {
    QSqlQuery check(m_db);
    if (!check.exec("SELECT dia_price FROM casting_entry LIMIT 1")) {
      // Column likely missing
      QSqlQuery addCol(m_db);
      if (!addCol.exec("ALTER TABLE casting_entry ADD COLUMN dia_price REAL "
                       "DEFAULT 0")) {
        qWarning()
            << "Migration: Failed to add dia_price column (might verify why):"
            << addCol.lastError().text();
      } else {
        qInfo() << "Migration: Added dia_price column to casting_entry";
      }
    }
  }

  // Migration for new columns (Stone + Diamond Category)
  {
    QSqlQuery check(m_db);
    // Check if one of the new columns exists
    if (!check.exec(
            "SELECT issue_diamond_category FROM casting_entry LIMIT 1")) {
      QSqlQuery add(m_db);
      bool ok = true;
      ok &= add.exec(
          "ALTER TABLE casting_entry ADD COLUMN issue_diamond_category TEXT");
      ok &= add.exec("ALTER TABLE casting_entry ADD COLUMN issue_stone_pcs "
                     "INTEGER DEFAULT 0");
      ok &= add.exec(
          "ALTER TABLE casting_entry ADD COLUMN issue_stone_wt REAL DEFAULT 0");
      ok &= add.exec("ALTER TABLE casting_entry ADD COLUMN receive_stone_pcs "
                     "INTEGER DEFAULT 0");
      ok &= add.exec("ALTER TABLE casting_entry ADD COLUMN receive_stone_wt "
                     "REAL DEFAULT 0");

      if (!ok) {
        qWarning() << "Migration: Failed to add some new Stone/DiaCat columns: "
                   << add.lastError().text();
      } else {
        qInfo() << "Migration: Added Stone/DiaCat columns to casting_entry";
      }
    }
  }

  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS jobsheet_detail (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            job_no TEXT NOT NULL,

            filling_issue TEXT,
            filling_dust TEXT,
            filling_return TEXT,

            buffing_return TEXT,
            free_polish_return TEXT,
            setting_return TEXT,
            final_polish_return TEXT,

            diamond_issue TEXT,
            diamond_return TEXT,
            diamond_broken TEXT,

            stone_issue TEXT,
            stone_return TEXT,
            stone_broken TEXT,

            other_issue TEXT,
            other_return TEXT,
            other_broken TEXT,
            
            office_gold_receive TEXT,
            manufacturer_mfg_receive TEXT,

            FOREIGN KEY (job_no) REFERENCES jobs(job_no) ON DELETE CASCADE
        );
    )")) {
    qCritical() << "jobsheet_detail table error:" << query.lastError();
    return false;
  }

  // Migration: Add office_gold_receive, manufacturer_mfg_receive if not exists
  query.exec("ALTER TABLE jobsheet_detail ADD COLUMN office_gold_receive TEXT");
  query.exec(
      "ALTER TABLE jobsheet_detail ADD COLUMN manufacturer_mfg_receive TEXT");

  // -----------------------------
  // METAL PURCHASE (Accountant)
  // -----------------------------
  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS metal_purchase_entry (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            entry_date TEXT,
            bill_no TEXT,
            party_name TEXT,
            pic INTEGER DEFAULT 0,
            product_name TEXT,
            weight REAL DEFAULT 0,
            purity REAL DEFAULT 0,
            labour_amount REAL DEFAULT 0,
            total_gold REAL DEFAULT 0,
            pay_weight REAL DEFAULT 0,
            total_pay_amount REAL DEFAULT 0,
            costing_per_gm REAL DEFAULT 0,
            remark TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )")) {
    qCritical() << "metal_purchase_entry table error:" << query.lastError();
    return false;
  }

  // -----------------------------
  // STOCKS
  // -----------------------------
  if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS stocks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT,
            metal TEXT,
            detail TEXT,
            note TEXT,
            voucher_no TEXT,
            purity REAL,
            weight REAL,
            weight_24k REAL,
            price REAL,
            amount REAL
        );
    )")) {
    qCritical() << "stocks table error:" << query.lastError();
    return false;
  }

  // -----------------------------
  // SEED ROLES (SAFE)
  // -----------------------------
  QStringList defaultRoles = {"admin", "seller", "designer", "manufacturer",
                              "accountant"};

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
        ":pass", QString(QCryptographicHash::hash(QByteArray("admin123"),
                                                  QCryptographicHash::Sha256)
                             .toHex()));

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
