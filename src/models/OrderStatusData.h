#ifndef ORDERSTATUSDATA_H
#define ORDERSTATUSDATA_H

#include <QString>

struct OrderStatusData
{
    QString jobNo;

    QString designer = "Pending";
    QString manufacturer = "Pending";
    QString accountant = "Pending";

    QString orderNote;
    QString designNote;
    QString qualityNote;
};

#endif // ORDERSTATUSDATA_H
