#include "AuthService.h"

#include "database/UserRepository.h"
#include "common/SessionManager.h"

User AuthService::login(const QString& username,
                        const QString& password)
{
    if (username.isEmpty() || password.isEmpty()) {
        return User{};
    }

    return UserRepository::authenticate(username, password);
}

QStringList AuthService::getUserRoles(int userId)
{
    return UserRepository::getUserRoles(userId);
}

bool AuthService::createUser(
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
    // 🔐 Only admin can create users
    if (!SessionManager::hasRole("admin")) {
        return false;
    }

    return UserRepository::createUser(
        username,
        password,
        isActive,
        fullName,
        accountNo,
        baseSalary,
        workingHours,
        roles,
        sellingPercentage
        );
}
