#include "UserRepository.h"
#include "DatabaseManager.h"
#include "models/UserPaymentView.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>

static QString hashPassword(const QString& password)
{
    return QString(
        QCryptographicHash::hash(
            password.toUtf8(),
            QCryptographicHash::Sha256
            ).toHex()
        );
}

User UserRepository::authenticate(const QString& username,
                                  const QString& password)
{
    User user;

    QSqlQuery query(DatabaseManager::instance().database());

    query.prepare(R"(
        SELECT id, username, is_active
        FROM users
        WHERE username = :username
          AND password_hash = :password
    )");

    query.bindValue(":username", username);
    query.bindValue(":password", hashPassword(password));

    if (!query.exec()) {
        qCritical() << "Auth query failed:" << query.lastError().text();
        return user;
    }

    if (query.next()) {
        user.id = query.value("id").toInt();
        user.username = query.value("username").toString();
        user.isActive = query.value("is_active").toInt() == 1;
    }

    return user;
}

QStringList UserRepository::getUserRoles(int userId)
{
    QStringList roles;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT r.name
        FROM roles r
        JOIN user_roles ur ON ur.role_id = r.id
        WHERE ur.user_id = ?
    )");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            roles << query.value(0).toString();
        }
    }

    return roles;
}

bool UserRepository::createUser(
    const QString &username,
    const QString &password,
    bool isActive,
    const QString &fullName,
    const QString &accountNo,
    double baseSalary,
    double workingHours,
    const QStringList &roles,
    double sellingPercentage
    )
{
    QSqlDatabase db = DatabaseManager::instance().database();

    if (!db.transaction()) {
        qCritical() << "Failed to start DB transaction";
        return false;
    }

    // -----------------------------
    // 1️⃣ Insert into users
    // -----------------------------
    QSqlQuery userQuery(db);
    userQuery.prepare(R"(
        INSERT INTO users (username, password_hash, is_active)
        VALUES (:username, :password, :active)
    )");

    userQuery.bindValue(":username", username);
    userQuery.bindValue(":password", hashPassword(password));
    userQuery.bindValue(":active", isActive ? 1 : 0);

    if (!userQuery.exec()) {
        qCritical() << "Insert users failed:" << userQuery.lastError().text();
        db.rollback();
        return false;
    }

    int userId = userQuery.lastInsertId().toInt();

    // -----------------------------
    // 2️⃣ Insert into employees
    // -----------------------------
    QSqlQuery empQuery(db);
    empQuery.prepare(R"(
        INSERT INTO employees
        (user_id, full_name, account_no, base_salary, working_hours_per_day)
        VALUES (:user_id, :name, :account, :salary, :hours)
    )");

    empQuery.bindValue(":user_id", userId);
    empQuery.bindValue(":name", fullName);
    empQuery.bindValue(":account", accountNo);
    empQuery.bindValue(":salary", baseSalary);
    empQuery.bindValue(":hours", workingHours);

    if (!empQuery.exec()) {
        qCritical() << "Insert employees failed:" << empQuery.lastError().text();
        db.rollback();
        return false;
    }

    int employeeId = empQuery.lastInsertId().toInt();

    // -----------------------------
    // 3️⃣ Assign roles
    // -----------------------------
    for (const QString &roleName : roles) {

        QSqlQuery roleQuery(db);
        roleQuery.prepare("SELECT id FROM roles WHERE name = ?");
        roleQuery.addBindValue(roleName);

        if (!roleQuery.exec() || !roleQuery.next()) {
            qCritical() << "Role not found:" << roleName;
            db.rollback();
            return false;
        }

        int roleId = roleQuery.value(0).toInt();

        QSqlQuery mapQuery(db);
        mapQuery.prepare(R"(
            INSERT INTO user_roles (user_id, role_id)
            VALUES (?, ?)
        )");

        mapQuery.addBindValue(userId);
        mapQuery.addBindValue(roleId);

        if (!mapQuery.exec()) {
            qCritical() << "Insert user_roles failed:"
                        << mapQuery.lastError().text();
            db.rollback();
            return false;
        }
    }

    // -----------------------------
    // 4️⃣ Seller profile (if seller)
    // -----------------------------
    if (roles.contains("seller")) {
        QSqlQuery sellerQuery(db);
        sellerQuery.prepare(R"(
            INSERT INTO seller_profile (employee_id, selling_percentage)
            VALUES (:emp, :percent)
        )");

        sellerQuery.bindValue(":emp", employeeId);
        sellerQuery.bindValue(":percent", sellingPercentage);

        if (!sellerQuery.exec()) {
            qCritical() << "Insert seller_profile failed:"
                        << sellerQuery.lastError().text();
            db.rollback();
            return false;
        }
    }

    // -----------------------------
    // ✅ Commit transaction
    // -----------------------------
    if (!db.commit()) {
        qCritical() << "DB commit failed";
        db.rollback();
        return false;
    }

    return true;
}


QList<UserPaymentView> UserRepository::getUsersWithPayments(int month, int year)
{
    QList<UserPaymentView> list;

    QSqlQuery query(DatabaseManager::instance().database());

    query.prepare(R"(
        SELECT
            u.id AS user_id,
            e.id AS employee_id,
            u.username,
            e.full_name,
            e.base_salary,

            IFNULL(p.worked_days, 0) AS worked_days,
            IFNULL(p.advance_paid, 0) AS advance_paid,
            IFNULL(p.paid_amount, 0) AS paid_amount,

            GROUP_CONCAT(r.name, ', ') AS roles
        FROM users u
        JOIN employees e ON e.user_id = u.id
        LEFT JOIN user_roles ur ON ur.user_id = u.id
        LEFT JOIN roles r ON r.id = ur.role_id
        LEFT JOIN employee_payments p
            ON p.employee_id = e.id
           AND p.month = :month
           AND p.year = :year
        GROUP BY u.id
        ORDER BY e.full_name
    )");

    query.bindValue(":month", month);
    query.bindValue(":year", year);

    if (!query.exec()) {
        qCritical() << "View users query failed:"
                    << query.lastError().text();
        return list;
    }

    while (query.next()) {
        UserPaymentView row;

        row.userId = query.value("user_id").toInt();
        row.employeeId = query.value("employee_id").toInt();
        row.username = query.value("username").toString();
        row.fullName = query.value("full_name").toString();
        row.roles = query.value("roles").toString();
        row.baseSalary = query.value("base_salary").toDouble();

        row.workedDays = query.value("worked_days").toInt();
        row.advancePaid = query.value("advance_paid").toDouble();
        row.paidAmount = query.value("paid_amount").toDouble();

        row.totalPaid = row.advancePaid + row.paidAmount;
        row.pending = row.baseSalary - row.totalPaid;

        list.append(row);
    }

    return list;
}

bool UserRepository::updateMonthlyPayment(
    int employeeId,
    int month,
    int year,
    int workedDays,
    double advancePaid,
    double paidAmount
    )
{
    QSqlDatabase db = DatabaseManager::instance().database();

    if (!db.transaction()) {
        qCritical() << "Failed to start payment transaction";
        return false;
    }
    qDebug()<<"here" << employeeId  ;
    // Check if record exists
    QSqlQuery check(db);
    check.prepare(R"(
        SELECT id FROM employee_payments
        WHERE employee_id = ? AND month = ? AND year = ?
    )");
    check.addBindValue(employeeId);
    check.addBindValue(month);
    check.addBindValue(year);

    if (!check.exec()) {
        qCritical() << "Payment check failed:" << check.lastError().text();
        db.rollback();
        return false;
    }

    bool exists = check.next();

    QSqlQuery query(db);

    if (exists) {
        // -----------------------------
        // UPDATE existing row
        // -----------------------------
        query.prepare(R"(
            UPDATE employee_payments
            SET worked_days = :days,
                advance_paid = :advance,
                paid_amount = :paid
            WHERE employee_id = :emp
              AND month = :month
              AND year = :year
        )");
    } else {
        // -----------------------------
        // INSERT new row
        // -----------------------------
        query.prepare(R"(
            INSERT INTO employee_payments
            (employee_id, month, year, worked_days, advance_paid, paid_amount)
            VALUES (:emp, :month, :year, :days, :advance, :paid)
        )");
    }

    query.bindValue(":emp", employeeId);
    query.bindValue(":month", month);
    query.bindValue(":year", year);
    query.bindValue(":days", workedDays);
    query.bindValue(":advance", advancePaid);
    query.bindValue(":paid", paidAmount);

    if (!query.exec()) {
        qCritical() << "Payment save failed:" << query.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        qCritical() << "Payment commit failed";
        db.rollback();
        return false;
    }

    return true;
}

int UserRepository::getEmployeeIdByUsername(const QString &username)
{
    QSqlQuery query(DatabaseManager::instance().database());

    query.prepare(R"(
        SELECT e.id
        FROM employees e
        JOIN users u ON u.id = e.user_id
        WHERE u.username = ?
    )");

    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        qCritical() << "Failed to get employeeId for user:" << username
                    << query.lastError().text();
        return -1;
    }

    return query.value(0).toInt();
}

bool UserRepository::updateUserPassword(int userId,
                                        const QString &newPassword)
{
    QSqlQuery query(DatabaseManager::instance().database());

    query.prepare(R"(
        UPDATE users
        SET password_hash = ?
        WHERE id = ?
    )");

    query.addBindValue(hashPassword(newPassword));
    query.addBindValue(userId);

    if (!query.exec()) {
        qCritical() << "Password update failed:"
                    << query.lastError().text();
        return false;
    }

    return true;
}

