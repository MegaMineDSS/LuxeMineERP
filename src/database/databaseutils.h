#ifndef DATABASEUTILS_H
#define DATABASEUTILS_H

#include "models/OrderData.h"

class DatabaseUtils
{
public:
    static bool createOrder(const OrderData &order);
    // static QList<OrderData> getOrdersBySeller(const QString &sellerId);


    DatabaseUtils();
};

#endif // DATABASEUTILS_H
