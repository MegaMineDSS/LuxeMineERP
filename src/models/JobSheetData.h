#ifndef JOBSHEETDATA_H
#define JOBSHEETDATA_H

#include <QString>

struct JobSheetData {
    QString sellerId;
    QString partyId;
    QString jobNo;
    QString orderNo;
    QString clientId;
    QString orderDate;
    QString deliveryDate;

    int productPis = 0;   // numeric
    QString designNo;

    QString metalPurity;
    QString metalColor;

    double sizeNo   = 0.0;
    double sizeMM   = 0.0;
    double length   = 0.0;
    double width    = 0.0;
    double height   = 0.0;

    QString imagePath;
};

#endif // JOBSHEETDATA_H
