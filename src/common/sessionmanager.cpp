#include "SessionManager.h"

static User g_user;
static QStringList g_roles;
static QString g_activeRole;

void SessionManager::setCurrentUser(const User &user)
{
    g_user = user;
}

User SessionManager::currentUser()
{
    return g_user;
}

void SessionManager::setRoles(const QStringList &roles)
{
    g_roles = roles;
}

QStringList SessionManager::roles()
{
    return g_roles;
}

void SessionManager::setActiveRole(const QString &role)
{
    g_activeRole = role;
}

QString SessionManager::activeRole()
{
    return g_activeRole;
}

bool SessionManager::hasRole(const QString &role)
{
    return g_roles.contains(role);
}

void SessionManager::clear()
{
    g_user = User{};
    g_roles.clear();
    g_activeRole.clear();
}
