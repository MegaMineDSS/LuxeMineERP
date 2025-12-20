#include "DatabaseUtils.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>

bool DatabaseUtils::createOrder(const OrderData &o)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    if (!db.transaction()) {
        qCritical() << "Failed to start DB transaction";
        return false;
    }

    // -------- order_book_detail --------
    query.prepare(R"(
        INSERT INTO order_book_detail (
            sellerName, sellerId,
            partyId, partyName,
            jobNo, orderNo,
            clientId, agencyId, shopId, reteailleId, starId,
            address, city, state, country,
            orderDate, deliveryDate,
            productName, productPis, approxProductWt,
            metalPrice, metalName, metalPurity, metalColor,
            sizeNo, sizeMM, length, width, height,
            diaPacific, diaPurity, diaColor, diaPrice,
            stPacific, stPurity, stColor, stPrice,
            designNo1, designNo2,
            image1Path, image2Path,
            metalCertiName, metalCertiType,
            diaCertiName, diaCertiType,
            pesSaki, chainLock, polish, settingLebour, metalStemp,
            paymentMethod,
            totalAmount, advance, remaining,
            note, extraDetail,
            isSaved
        )
        VALUES (
            :sellerName, :sellerId,
            :partyId, :partyName,
            :jobNo, :orderNo,
            :clientId, :agencyId, :shopId, :retaillerId, :starId,
            :address, :city, :state, :country,
            :orderDate, :deliveryDate,
            :productName, :productPis, :approxProductWt,
            :metalPrice, :metalName, :metalPurity, :metalColor,
            :sizeNo, :sizeMM, :length, :width, :height,
            :diaPacific, :diaPurity, :diaColor, :diaPrice,
            :stPacific, :stPurity, :stColor, :stPrice,
            :designNo1, :designNo2,
            :image1Path, :image2Path,
            :metalCertiName, :metalCertiType,
            :diaCertiName, :diaCertiType,
            :pesSaki, :chainLock, :polish, :settingLebour, :metalStemp,
            :paymentMethod,
            :totalAmount, :advance, :remaining,
            :note, :extraDetail,
            :isSaved
        )
    )");

    // Seller & party
    query.bindValue(":sellerName", o.sellerName);
    query.bindValue(":sellerId", o.sellerId);
    query.bindValue(":partyId", o.partyId);
    query.bindValue(":partyName", o.partyName);

    // Order identity
    query.bindValue(":jobNo", o.jobNo);
    query.bindValue(":orderNo", o.orderNo);

    // Client hierarchy
    query.bindValue(":clientId", o.clientId);
    query.bindValue(":agencyId", o.agencyId);
    query.bindValue(":shopId", o.shopId);
    query.bindValue(":retaillerId", o.retaillerId);
    query.bindValue(":starId", o.starId);

    // Address
    query.bindValue(":address", o.address);
    query.bindValue(":city", o.city);
    query.bindValue(":state", o.state);
    query.bindValue(":country", o.country);

    // Dates
    query.bindValue(":orderDate", o.orderDate);
    query.bindValue(":deliveryDate", o.deliveryDate);

    // Product
    query.bindValue(":productName", o.productName);
    query.bindValue(":productPis", o.productPis);
    query.bindValue(":approxProductWt", o.approxProductWt);

    // Metal
    query.bindValue(":metalPrice", o.metalPrice);
    query.bindValue(":metalName", o.metalName);
    query.bindValue(":metalPurity", o.metalPurity);
    query.bindValue(":metalColor", o.metalColor);

    // Size
    query.bindValue(":sizeNo", o.sizeNo);
    query.bindValue(":sizeMM", o.sizeMM);
    query.bindValue(":length", o.length);
    query.bindValue(":width", o.width);
    query.bindValue(":height", o.height);

    // Diamond
    query.bindValue(":diaPacific", o.diaPacific);
    query.bindValue(":diaPurity", o.diaPurity);
    query.bindValue(":diaColor", o.diaColor);
    query.bindValue(":diaPrice", o.diaPrice);

    // Stone
    query.bindValue(":stPacific", o.stPacific);
    query.bindValue(":stPurity", o.stPurity);
    query.bindValue(":stColor", o.stColor);
    query.bindValue(":stPrice", o.stPrice);

    // Design & images
    query.bindValue(":designNo1", o.designNo1);
    query.bindValue(":designNo2", o.designNo2);
    query.bindValue(":image1Path", o.image1Path);
    query.bindValue(":image2Path", o.image2Path);

    // Certificates
    query.bindValue(":metalCertiName", o.metalCertiName);
    query.bindValue(":metalCertiType", o.metalCertiType);
    query.bindValue(":diaCertiName", o.diaCertiName);
    query.bindValue(":diaCertiType", o.diaCertiType);

    // Manufacturing
    query.bindValue(":pesSaki", o.pesSaki);
    query.bindValue(":chainLock", o.chainLock);
    query.bindValue(":polish", o.polish);
    query.bindValue(":settingLebour", o.settingLebour);
    query.bindValue(":metalStemp", o.metalStemp);

    // Payment
    query.bindValue(":paymentMethod", o.paymentMethod);
    query.bindValue(":totalAmount", o.totalAmount);
    query.bindValue(":advance", o.advance);
    query.bindValue(":remaining", o.remaining);

    // Notes
    query.bindValue(":note", o.note);
    query.bindValue(":extraDetail", o.extraDetail);

    query.bindValue(":isSaved", o.isSaved);

    if (!query.exec()) {
        qCritical() << "order_book_detail insert error:" << query.lastError();
        db.rollback();
        return false;
    }

    // -------- order_status --------
    query.prepare(R"(
        INSERT INTO order_status (jobNo)
        VALUES (:jobNo)
    )");
    query.bindValue(":jobNo", o.jobNo);

    if (!query.exec()) {
        qCritical() << "order_status insert error:" << query.lastError();
        db.rollback();
        return false;
    }

    return db.commit();
}
