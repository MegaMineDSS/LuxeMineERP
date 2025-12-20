#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "models/User.h"
#include <QString>
#include <QStringList>

class AuthService
{
public:
    // Login
    static User login(const QString& username,
                      const QString& password);

    // Fetch roles for role selector
    static QStringList getUserRoles(int userId);

    // Create user (ADMIN ONLY)
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
};

#endif // AUTHSERVICE_H
