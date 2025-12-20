#ifndef ORDERDATA_H
#define ORDERDATA_H

#include <QString>

struct OrderData
{
    // Primary key (optional for fetch)
    int id = -1;

    // Seller & Party
    QString sellerName;
    QString sellerId;
    QString partyId;
    QString partyName;

    // Order identity
    QString jobNo;
    QString orderNo;

    // Client hierarchy
    QString clientId;
    QString agencyId;
    QString shopId;
    QString retaillerId;
    QString starId;

    // Address
    QString address;
    QString city;
    QString state;
    QString country;

    // Dates
    QString orderDate;
    QString deliveryDate;

    // Product
    QString productName;
    int productPis = 0;
    double approxProductWt = 0.0;
    double approxDiamondWt = 0.0;

    // Metal
    double metalPrice = 0.0;
    QString metalName;
    QString metalPurity;
    QString metalColor;

    // Size & dimensions
    double sizeNo = 0.0;
    double sizeMM = 0.0;
    double length = 0.0;
    double width = 0.0;
    double height = 0.0;

    // Diamond
    QString diaPacific;
    QString diaPurity;
    QString diaColor;
    double diaPrice = 0.0;

    // Stone
    QString stPacific;
    QString stPurity;
    QString stColor;
    double stPrice = 0.0;

    // Design
    QString designNo1;
    QString designNo2;
    QString image1Path;
    QString image2Path;

    // Certification
    QString metalCertiName;
    QString metalCertiType;
    QString diaCertiName;
    QString diaCertiType;

    // Manufacturing options
    QString pesSaki;
    QString chainLock;
    QString polish;
    QString settingLebour;
    QString metalStemp;

    // Payment
    QString paymentMethod;
    double totalAmount = 0.0;
    double advance = 0.0;
    double remaining = 0.0;

    // Notes
    QString note;
    QString extraDetail;

    // State
    int isSaved = 0;
};

#endif // ORDERDATA_H
