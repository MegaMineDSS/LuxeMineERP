#ifndef USER_H
#define USER_H

#include <QString>

struct User
{
    int id = -1;
    QString username;
    QString role;
    bool isActive = false;

    bool isValid() const {
        return id > 0 && isActive;
    }
};

#endif // USER_H
