#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "models/User.h"
#include <QStringList>

class SessionManager
{
public:
    static void setCurrentUser(const User &user);
    static User currentUser();

    static void setRoles(const QStringList &roles);
    static QStringList roles();

    static void setActiveRole(const QString &role);
    static QString activeRole();

    static bool hasRole(const QString &role);
    static void clear();
};

#endif
