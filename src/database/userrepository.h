#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "models/User.h"
#include "models/UserPaymentView.h"
#include <QString>
#include <QStringList>


class UserRepository
{
public:
    static User authenticate(const QString& username,
                             const QString& password);

    // 🔹 NEW: fetch roles for a user
    static QStringList getUserRoles(int userId);

    // 🔹 NEW: create user (FULL ERP)
    static bool createUser(
        const QString &username,
        const QString &password,
        bool isActive,
        const QString &fullName,
        const QString &accountNo,
        double baseSalary,
        double workingHours,
        const QStringList &roles,
        double sellingPercentage
        );

    static QList<UserPaymentView> getUsersWithPayments(int month, int year);

    static bool updateMonthlyPayment(
        int employeeId,
        int month,
        int year,
        int workedDays,
        double advancePaid,
        double paidAmount
        );

    static int getEmployeeIdByUsername(const QString &username);

    static bool updateUserPassword(int userId,
                                   const QString &newPassword);




};

#endif
