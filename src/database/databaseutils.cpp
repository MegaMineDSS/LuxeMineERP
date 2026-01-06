#include "DatabaseUtils.h"
#include "databasemanager.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>

bool DatabaseUtils::createOrder(const OrderData &o, int &outJobId,
                                int &outSellerSeq) {
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);

  if (!db.transaction()) {
    qCritical() << "Transaction start failed";
    return false;
  }

  // ---------- 1️⃣ jobs ----------
  if (!q.exec("INSERT INTO jobs DEFAULT VALUES")) {
    qCritical() << q.lastError() << "11111";
    db.rollback();
    return false;
  }
  int jobId = q.lastInsertId().toInt();

  // ---------- 2️⃣ seller_order_counter ----------
  // 1️⃣ Ensure seller counter row exists (SAFE, no race condition)
  q.prepare(R"(
    INSERT OR IGNORE INTO seller_order_counter (seller_id, last_order_no)
    VALUES (:sid, 0)
)");
  q.bindValue(":sid", o.sellerId);

  if (!q.exec()) {
    qCritical() << "Insert seller counter failed:" << q.lastError();
    db.rollback();
    return false;
  }

  // 2️⃣ Increment counter
  q.prepare(R"(
    UPDATE seller_order_counter
    SET last_order_no = last_order_no + 1
    WHERE seller_id = :sid
)");
  q.bindValue(":sid", o.sellerId);

  if (!q.exec() || q.numRowsAffected() == 0) {
    qCritical() << "Update seller counter failed:" << q.lastError();
    db.rollback();
    return false;
  }

  // 3️⃣ Fetch updated value
  q.prepare(R"(
        SELECT last_order_no
        FROM seller_order_counter
        WHERE seller_id = :sid
    )");
  q.bindValue(":sid", o.sellerId);

  if (!q.exec() || !q.next()) {
    qCritical() << "Select seller counter failed:" << q.lastError();
    db.rollback();
    return false;
  }

  int sellerSeq = q.value(0).toInt();

  // ---------- 3️⃣ orders ----------
  q.prepare(R"(
        INSERT INTO orders (job_id, seller_id, seller_order_seq)
        VALUES (:job_id, :seller_id, :seq)
    )");
  q.bindValue(":job_id", jobId);
  q.bindValue(":seller_id", o.sellerId);
  q.bindValue(":seq", sellerSeq);

  if (!q.exec()) {
    qCritical() << q.lastError();
    db.rollback();
    return false;
  }

  int orderId = q.lastInsertId().toInt();

  // ---------- 4️⃣ order_book_detail ----------
  q.prepare(R"(
        INSERT INTO order_book_detail (
            order_id, job_id,
            sellerName, sellerId,
            partyId, partyName,
            clientId, agencyId, shopId, reteillerId, starId,
            address, city, state, country,
            orderDate, deliveryDate,
            productName, productPis,
            approxProductWt, approxDiamondWt,
            metalPrice, metalName, metalPurity, metalColor,
            sizeNo, sizeMM, length, width, height,
            diaPacific, diaPurity, diaColor, diaPrice,
            stPacific, stPurity, stColor, stPrice,
            designNo, image1Path, image2Path,
            metalCertiName, metalCertiType,
            diaCertiName, diaCertiType,
            pesSaki, chainLock, polish, settingLebour, metalStemp,
            paymentMethod, totalAmount, advance, remaining,
            note, extraDetail, isSaved
        ) VALUES (
            :order_id, :job_id,
            :sellerName, :sellerId,
            :partyId, :partyName,
            :clientId, :agencyId, :shopId, :reteillerId, :starId,
            :address, :city, :state, :country,
            :orderDate, :deliveryDate,
            :productName, :productPis,
            :approxProductWt, :approxDiamondWt,
            :metalPrice, :metalName, :metalPurity, :metalColor,
            :sizeNo, :sizeMM, :length, :width, :height,
            :diaPacific, :diaPurity, :diaColor, :diaPrice,
            :stPacific, :stPurity, :stColor, :stPrice,
            :designNo, :image1Path, :image2Path,
            :metalCertiName, :metalCertiType,
            :diaCertiName, :diaCertiType,
            :pesSaki, :chainLock, :polish, :settingLebour, :metalStemp,
            :paymentMethod, :totalAmount, :advance, :remaining,
            :note, :extraDetail, 1
        )
    )");

  // 🔁 Bind everything EXACTLY as before

  // -------- IDs (NEW SYSTEM) --------
  q.bindValue(":order_id", orderId);
  q.bindValue(":job_id", jobId);

  // Seller & party
  q.bindValue(":sellerName", o.sellerName);
  q.bindValue(":sellerId", o.sellerId);
  q.bindValue(":partyId", o.partyId);
  q.bindValue(":partyName", o.partyName);

  // Client hierarchy
  q.bindValue(":clientId", o.clientId);
  q.bindValue(":agencyId", o.agencyId);
  q.bindValue(":shopId", o.shopId);
  q.bindValue(":retaillerId", o.retaillerId);
  q.bindValue(":starId", o.starId);

  // Address
  q.bindValue(":address", o.address);
  q.bindValue(":city", o.city);
  q.bindValue(":state", o.state);
  q.bindValue(":country", o.country);

  // Dates
  q.bindValue(":orderDate", o.orderDate);
  q.bindValue(":deliveryDate", o.deliveryDate);

  // Product
  q.bindValue(":productName", o.productName);
  q.bindValue(":productPis", o.productPis);
  q.bindValue(":approxProductWt", o.approxProductWt);
  q.bindValue(":approxDiamondWt", o.approxDiamondWt);

  // Metal
  q.bindValue(":metalPrice", o.metalPrice);
  q.bindValue(":metalName", o.metalName);
  q.bindValue(":metalPurity", o.metalPurity);
  q.bindValue(":metalColor", o.metalColor);

  // Size
  q.bindValue(":sizeNo", o.sizeNo);
  q.bindValue(":sizeMM", o.sizeMM);
  q.bindValue(":length", o.length);
  q.bindValue(":width", o.width);
  q.bindValue(":height", o.height);

  // Diamond
  q.bindValue(":diaPacific", o.diaPacific);
  q.bindValue(":diaPurity", o.diaPurity);
  q.bindValue(":diaColor", o.diaColor);
  q.bindValue(":diaPrice", o.diaPrice);

  // Stone
  q.bindValue(":stPacific", o.stPacific);
  q.bindValue(":stPurity", o.stPurity);
  q.bindValue(":stColor", o.stColor);
  q.bindValue(":stPrice", o.stPrice);

  // Design & images
  q.bindValue(":designNo", o.designNo);
  // q.bindValue(":designNo2", o.designNo2);
  q.bindValue(":image1Path", o.image1Path);
  q.bindValue(":image2Path", o.image2Path);

  // Certificates
  q.bindValue(":metalCertiName", o.metalCertiName);
  q.bindValue(":metalCertiType", o.metalCertiType);
  q.bindValue(":diaCertiName", o.diaCertiName);
  q.bindValue(":diaCertiType", o.diaCertiType);

  // Manufacturing
  q.bindValue(":pesSaki", o.pesSaki);
  q.bindValue(":chainLock", o.chainLock);
  q.bindValue(":polish", o.polish);
  q.bindValue(":settingLebour", o.settingLebour);
  q.bindValue(":metalStemp", o.metalStemp);

  // Payment
  q.bindValue(":paymentMethod", o.paymentMethod);
  q.bindValue(":totalAmount", o.totalAmount);
  q.bindValue(":advance", o.advance);
  q.bindValue(":remaining", o.remaining);

  // Notes
  q.bindValue(":note", o.note);
  q.bindValue(":extraDetail", o.extraDetail);

  q.bindValue(":isSaved", o.isSaved);

  if (!q.exec()) {
    qCritical() << "order_book_detail insert error:" << q.lastError();
    db.rollback();
    return false;
  }

  // ---------- 5️⃣ order_status ----------
  q.prepare("INSERT INTO order_status (job_id) VALUES (:job_id)");
  q.bindValue(":job_id", jobId);

  if (!q.exec()) {
    qCritical() << q.lastError();
    db.rollback();
    return false;
  }

  if (!db.commit()) {
    qCritical() << "Commit failed";
    return false;
  }

  outJobId = jobId;
  outSellerSeq = sellerSeq;
  return true;
}

QList<OrderData> DatabaseUtils::getOrdersForSeller(int sellerId) {
  QList<OrderData> list;
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);

  q.prepare(R"(
        SELECT
            o.order_id,
            o.job_id,
            o.seller_order_seq,
            od.partyName,
            od.sellerName,
            od.productPis,
            od.metalName,
            od.metalPurity,
            od.designNo,
            od.orderDate,
            od.deliveryDate,
            od.extraDetail
        FROM orders o
        JOIN order_book_detail od
            ON o.order_id = od.order_id
        WHERE o.seller_id = :sid
        ORDER BY o.order_id DESC
    )");

  q.bindValue(":sid", sellerId);

  if (!q.exec()) {
    qCritical() << "getOrdersForSeller failed:" << q.lastError();
    return list;
  }

  while (q.next()) {
    OrderData o;

    o.orderId = q.value("order_id").toInt();
    o.sellerOrderSeq = q.value("seller_order_seq").toInt();
    o.jobId = q.value("job_id").toInt();
    o.partyName = q.value("partyName").toString();
    o.sellerName = q.value("sellerName").toString();
    o.productPis = q.value("productPis").toInt();
    o.metalName = q.value("metalName").toString();
    o.metalPurity = q.value("metalPurity").toString();
    o.designNo = q.value("designNo").toString();
    o.orderDate = q.value("orderDate").toString();
    o.deliveryDate = q.value("deliveryDate").toString();
    o.extraDetail = q.value("extraDetail").toString();

    list.append(o);
  }

  return list;
}

QList<OrderData> DatabaseUtils::getAllOrders() {
  QList<OrderData> list;
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);

  q.prepare(R"(
        SELECT
            o.order_id,
            o.job_id,
            o.seller_order_seq,
            od.partyName,
            od.sellerName,
            od.orderDate,
            od.deliveryDate
        FROM orders o
        JOIN order_book_detail od
            ON o.order_id = od.order_id
        ORDER BY o.order_id DESC
    )");

  if (!q.exec()) {
    qCritical() << "getAllOrders failed:" << q.lastError();
    return list;
  }

  while (q.next()) {
    OrderData o;

    o.orderId = q.value("order_id").toInt();
    o.sellerOrderSeq = q.value("seller_order_seq").toInt();
    o.jobId = q.value("job_id").toInt();
    o.partyName = q.value("partyName").toString();
    o.sellerName = q.value("sellerName").toString();
    o.orderDate = q.value("orderDate").toString();
    o.deliveryDate = q.value("deliveryDate").toString();

    list.append(o);
  }

  return list;
}

bool DatabaseUtils::getOrderById(int orderId, OrderData &o) {
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);

  q.prepare(R"(
        SELECT
            o.order_id,
            o.job_id,
            o.seller_order_seq,
            od.sellerName, od.sellerId,
            od.partyId, od.partyName,
            od.clientId, od.agencyId, od.shopId, od.reteillerId, od.starId,
            od.address, od.city, od.state, od.country,
            od.orderDate, od.deliveryDate,
            od.productName, od.productPis,
            od.approxProductWt, od.approxDiamondWt,
            od.metalPrice, od.metalName, od.metalPurity, od.metalColor,
            od.sizeNo, od.sizeMM, od.length, od.width, od.height,
            od.diaPacific, od.diaPurity, od.diaColor, od.diaPrice,
            od.stPacific, od.stPurity, od.stColor, od.stPrice,
            od.designNo, od.image1Path, od.image2Path,
            od.metalCertiName, od.metalCertiType,
            od.diaCertiName, od.diaCertiType,
            od.pesSaki, od.chainLock, od.polish, od.settingLebour, od.metalStemp,
            od.paymentMethod, od.totalAmount, od.advance, od.remaining,
            od.note, od.extraDetail
        FROM orders o
        JOIN order_book_detail od
            ON o.order_id = od.order_id
        WHERE o.order_id = :id
    )");

  q.bindValue(":id", orderId);

  if (!q.exec() || !q.next()) {
    qCritical() << "Failed to fetch order:" << q.lastError();
    return false;
  }

  // Primary
  o.id = orderId;
  o.orderId = q.value("order_id").toInt();
  o.jobId = q.value("job_id").toInt();
  o.sellerOrderSeq = q.value("seller_order_seq").toInt();

  // Seller & Party
  o.sellerName = q.value("sellerName").toString();
  o.sellerId = q.value("sellerId").toInt();
  o.partyId = q.value("partyId").toString();
  o.partyName = q.value("partyName").toString();

  // Client hierarchy
  o.clientId = q.value("clientId").toString();
  o.agencyId = q.value("agencyId").toString();
  o.shopId = q.value("shopId").toString();
  o.retaillerId = q.value("reteillerId").toString();
  o.starId = q.value("starId").toString();

  // Address
  o.address = q.value("address").toString();
  o.city = q.value("city").toString();
  o.state = q.value("state").toString();
  o.country = q.value("country").toString();

  // Dates
  o.orderDate = q.value("orderDate").toString();
  o.deliveryDate = q.value("deliveryDate").toString();

  // Product
  o.productName = q.value("productName").toString();
  o.productPis = q.value("productPis").toInt();
  o.approxProductWt = q.value("approxProductWt").toDouble();
  o.approxDiamondWt = q.value("approxDiamondWt").toDouble();

  // Metal
  o.metalPrice = q.value("metalPrice").toDouble();
  o.metalName = q.value("metalName").toString();
  o.metalPurity = q.value("metalPurity").toString();
  o.metalColor = q.value("metalColor").toString();

  // Size & dimensions
  o.sizeNo = q.value("sizeNo").toDouble();
  o.sizeMM = q.value("sizeMM").toDouble();
  o.length = q.value("length").toDouble();
  o.width = q.value("width").toDouble();
  o.height = q.value("height").toDouble();

  // Diamond
  o.diaPacific = q.value("diaPacific").toString();
  o.diaPurity = q.value("diaPurity").toString();
  o.diaColor = q.value("diaColor").toString();
  o.diaPrice = q.value("diaPrice").toDouble();

  // Stone
  o.stPacific = q.value("stPacific").toString();
  o.stPurity = q.value("stPurity").toString();
  o.stColor = q.value("stColor").toString();
  o.stPrice = q.value("stPrice").toDouble();

  // Design
  o.designNo = q.value("designNo").toString();
  o.image1Path = q.value("image1Path").toString();
  o.image2Path = q.value("image2Path").toString();

  // Certification
  o.metalCertiName = q.value("metalCertiName").toString();
  o.metalCertiType = q.value("metalCertiType").toString();
  o.diaCertiName = q.value("diaCertiName").toString();
  o.diaCertiType = q.value("diaCertiType").toString();

  // Manufacturing options
  o.pesSaki = q.value("pesSaki").toString();
  o.chainLock = q.value("chainLock").toString();
  o.polish = q.value("polish").toString();
  o.settingLebour = q.value("settingLebour").toString();
  o.metalStemp = q.value("metalStemp").toString();

  // Payment
  o.paymentMethod = q.value("paymentMethod").toString();
  o.totalAmount = q.value("totalAmount").toDouble();
  o.advance = q.value("advance").toDouble();
  o.remaining = q.value("remaining").toDouble();

  // Notes
  o.note = q.value("note").toString();
  o.extraDetail = q.value("extraDetail").toString();

  o.isSaved = 1;
  return true;
}

bool DatabaseUtils::updateOrder(int orderId, const OrderData &o) {
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);

  q.prepare(R"(
        UPDATE order_book_detail SET

            sellerName = :sellerName,
            sellerId   = :sellerId,

            partyId   = :partyId,
            partyName = :partyName,

            clientId    = :clientId,
            agencyId    = :agencyId,
            shopId      = :shopId,
            reteillerId = :reteillerId,
            starId      = :starId,

            address = :address,
            city    = :city,
            state   = :state,
            country = :country,

            orderDate    = :orderDate,
            deliveryDate = :deliveryDate,

            productName     = :productName,
            productPis      = :productPis,
            approxProductWt = :approxProductWt,
            approxDiamondWt = :approxDiamondWt,

            metalPrice  = :metalPrice,
            metalName   = :metalName,
            metalPurity = :metalPurity,
            metalColor  = :metalColor,

            sizeNo = :sizeNo,
            sizeMM = :sizeMM,
            length = :length,
            width  = :width,
            height = :height,

            diaPacific = :diaPacific,
            diaPurity  = :diaPurity,
            diaColor   = :diaColor,
            diaPrice   = :diaPrice,

            stPacific = :stPacific,
            stPurity  = :stPurity,
            stColor   = :stColor,
            stPrice   = :stPrice,

            designNo   = :designNo,
            image1Path = :image1Path,
            image2Path = :image2Path,

            metalCertiName = :metalCertiName,
            metalCertiType = :metalCertiType,
            diaCertiName   = :diaCertiName,
            diaCertiType   = :diaCertiType,

            pesSaki       = :pesSaki,
            chainLock     = :chainLock,
            polish        = :polish,
            settingLebour = :settingLebour,
            metalStemp    = :metalStemp,

            paymentMethod = :paymentMethod,
            totalAmount   = :totalAmount,
            advance       = :advance,
            remaining     = :remaining,

            note        = :note,
            extraDetail = :extraDetail,
            isSaved     = :isSaved

        WHERE id = :order_id
    )");

  // -------- IDs (NEW SYSTEM) --------
  // q.bindValue(":order_id", orderId);
  // q.bindValue(":job_id", jobId);

  // Seller & party
  q.bindValue(":sellerName", o.sellerName);
  q.bindValue(":sellerId", o.sellerId);
  q.bindValue(":partyId", o.partyId);
  q.bindValue(":partyName", o.partyName);

  // Client hierarchy
  q.bindValue(":clientId", o.clientId);
  q.bindValue(":agencyId", o.agencyId);
  q.bindValue(":shopId", o.shopId);
  q.bindValue(":retaillerId", o.retaillerId);
  q.bindValue(":starId", o.starId);

  // Address
  q.bindValue(":address", o.address);
  q.bindValue(":city", o.city);
  q.bindValue(":state", o.state);
  q.bindValue(":country", o.country);

  // Dates
  q.bindValue(":orderDate", o.orderDate);
  q.bindValue(":deliveryDate", o.deliveryDate);

  // Product
  q.bindValue(":productName", o.productName);
  q.bindValue(":productPis", o.productPis);
  q.bindValue(":approxProductWt", o.approxProductWt);
  q.bindValue(":approxDiamondWt", o.approxDiamondWt);

  // Metal
  q.bindValue(":metalPrice", o.metalPrice);
  q.bindValue(":metalName", o.metalName);
  q.bindValue(":metalPurity", o.metalPurity);
  q.bindValue(":metalColor", o.metalColor);

  // Size
  q.bindValue(":sizeNo", o.sizeNo);
  q.bindValue(":sizeMM", o.sizeMM);
  q.bindValue(":length", o.length);
  q.bindValue(":width", o.width);
  q.bindValue(":height", o.height);

  // Diamond
  q.bindValue(":diaPacific", o.diaPacific);
  q.bindValue(":diaPurity", o.diaPurity);
  q.bindValue(":diaColor", o.diaColor);
  q.bindValue(":diaPrice", o.diaPrice);

  // Stone
  q.bindValue(":stPacific", o.stPacific);
  q.bindValue(":stPurity", o.stPurity);
  q.bindValue(":stColor", o.stColor);
  q.bindValue(":stPrice", o.stPrice);

  // Design & images
  q.bindValue(":designNo", o.designNo);
  // q.bindValue(":designNo2", o.designNo2);
  q.bindValue(":image1Path", o.image1Path);
  q.bindValue(":image2Path", o.image2Path);

  // Certificates
  q.bindValue(":metalCertiName", o.metalCertiName);
  q.bindValue(":metalCertiType", o.metalCertiType);
  q.bindValue(":diaCertiName", o.diaCertiName);
  q.bindValue(":diaCertiType", o.diaCertiType);

  // Manufacturing
  q.bindValue(":pesSaki", o.pesSaki);
  q.bindValue(":chainLock", o.chainLock);
  q.bindValue(":polish", o.polish);
  q.bindValue(":settingLebour", o.settingLebour);
  q.bindValue(":metalStemp", o.metalStemp);

  // Payment
  q.bindValue(":paymentMethod", o.paymentMethod);
  q.bindValue(":totalAmount", o.totalAmount);
  q.bindValue(":advance", o.advance);
  q.bindValue(":remaining", o.remaining);

  // Notes
  q.bindValue(":note", o.note);
  q.bindValue(":extraDetail", o.extraDetail);

  q.bindValue(":isSaved", o.isSaved);

  // WHERE clause
  q.bindValue(":order_id", orderId);

  if (!q.exec()) {
    qCritical() << "Update failed:" << q.lastError();
    return false;
  }

  return true;
}

QList<CastingListRow> DatabaseUtils::getCastingList() {
  QList<CastingListRow> list;

  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);

  q.prepare(R"(
        SELECT
            o.job_id,
            o.deliveryDate,

            c.casting_date,
            c.casting_name,
            c.pcs,

            c.issue_metal_name,
            c.issue_metal_purity,
            c.issue_metal_wt,
            c.issue_diamond_pcs,
            c.issue_diamond_wt,

            c.receive_runner_wt,
            c.receive_product_wt,
            c.receive_diamond_pcs,
            c.receive_diamond_wt,

            c.receive_diamond_pcs,
            c.receive_diamond_wt,

            c.status,
            c.dia_price
        FROM order_book_detail o
        LEFT JOIN casting_entry c ON c.job_id = o.job_id
        ORDER BY o.deliveryDate ASC
    )");

  if (!q.exec()) {
    qCritical() << "Failed to fetch casting list:" << q.lastError();
    return list;
  }

  while (q.next()) {
      CastingListRow r;

      r.jobId = q.value(0).toInt();
      r.deliveryDate = q.value(1).toString();

      r.castingDate = q.value(2).toString();
      r.vendorName = q.value(3).toString();
      r.pcs = q.value(4).toInt();

      r.issueMetal = q.value(5).toString();
      r.purity = q.value(6).toString();
      r.issueMetalWt = q.value(7).toDouble();
      r.issueDiaPcs = q.value(8).toInt();
      r.issueDiaWt = q.value(9).toDouble();

      r.receiveRunnerWt = q.value(10).toDouble();
      r.receiveProductWt = q.value(11).toDouble();
      r.receiveDiaPcs = q.value(12).toInt();
      r.receiveDiaWt = q.value(13).toDouble();

      r.status = q.value(16).toString();
      if (r.status.isEmpty())
          r.status = "PENDING"; // no casting yet

      r.diaPrice = q.value(17).toDouble();

      list.append(r);
  }


  return list;
}

int DatabaseUtils::getCastingIdByJob(int jobId) {
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);
  q.prepare("SELECT id FROM casting_entry WHERE job_id = :job");
  q.bindValue(":job", jobId);

  if (!q.exec())
    return 0;

  if (q.next())
    return q.value(0).toInt();

  return 0;
}

bool DatabaseUtils::insertCasting(const CastingData &c) {
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);

  if (!db.transaction())
    return false;

  q.prepare(R"(
        INSERT INTO casting_entry (
            job_id, casting_date, casting_name, pcs,
            issue_metal_name, issue_metal_purity, issue_metal_wt,
            issue_diamond_pcs, issue_diamond_wt,
            receive_runner_wt, receive_product_wt,
            receive_diamond_pcs, receive_diamond_wt,
            accountant_id, status
        ) VALUES (
            :job, :date, :name, :pcs,
            :metal, :purity, :metalWt,
            :diaPcs, :diaWt,
            :runner, :product,
            :recvDiaPcs, :recvDiaWt,
            :acc, :status
        )
    )");

  q.bindValue(":job", c.jobId);
  q.bindValue(":date", c.castingDate);
  q.bindValue(":name", c.castingName);
  q.bindValue(":pcs", c.pcs);

  q.bindValue(":metal", c.issueMetalName);
  q.bindValue(":purity", c.issueMetalPurity);
  q.bindValue(":metalWt", c.issueMetalWt);
  q.bindValue(":diaPcs", c.issueDiamondPcs);
  q.bindValue(":diaWt", c.issueDiamondWt);

  q.bindValue(":runner", c.receiveRunnerWt);
  q.bindValue(":product", c.receiveProductWt);
  q.bindValue(":recvDiaPcs", c.receiveDiamondPcs);
  q.bindValue(":recvDiaWt", c.receiveDiamondWt);

  q.bindValue(":acc", c.accountantId);
  q.bindValue(":status", c.status);

  if (!q.exec()) {
    db.rollback();
    qCritical() << q.lastError();
    return false;
  }

  return db.commit();
}

bool DatabaseUtils::updateCastingDiaPrice(int jobId, double price) {
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);
  q.prepare("UPDATE casting_entry SET dia_price = :p WHERE job_id = :id");
  q.bindValue(":p", price);
  q.bindValue(":id", jobId);

  if (!q.exec()) {
    qCritical() << "Failed to update dia_price:" << q.lastError();
    return false;
  }
  return true;
}

bool DatabaseUtils::updateCasting(int castingId, const CastingData &c) {
  QSqlDatabase db = DatabaseManager::instance().database();
  QSqlQuery q(db);
  q.prepare(R"(
        UPDATE casting_entry SET
            casting_date = :date,
            casting_name = :name,
            pcs = :pcs,

            issue_metal_name = :metal,
            issue_metal_purity = :purity,
            issue_metal_wt = :metalWt,
            issue_diamond_pcs = :diaPcs,
            issue_diamond_wt = :diaWt,

            receive_runner_wt = :runner,
            receive_product_wt = :product,
            receive_diamond_pcs = :recvDiaPcs,
            receive_diamond_wt = :recvDiaWt,

            status = :status
        WHERE id = :id
    )");

  q.bindValue(":id", castingId);
  q.bindValue(":date", c.castingDate);
  q.bindValue(":name", c.castingName);
  q.bindValue(":pcs", c.pcs);

  q.bindValue(":metal", c.issueMetalName);
  q.bindValue(":purity", c.issueMetalPurity);
  q.bindValue(":metalWt", c.issueMetalWt);
  q.bindValue(":diaPcs", c.issueDiamondPcs);
  q.bindValue(":diaWt", c.issueDiamondWt);

  q.bindValue(":runner", c.receiveRunnerWt);
  q.bindValue(":product", c.receiveProductWt);
  q.bindValue(":recvDiaPcs", c.receiveDiamondPcs);
  q.bindValue(":recvDiaWt", c.receiveDiamondWt);

  q.bindValue(":status", c.status);

  return q.exec();
}

bool DatabaseUtils::getCastingDataByJob(int jobId, CastingData &c)
{
    QSqlDatabase db = DatabaseManager::instance().database();
    QSqlQuery q(db);

    q.prepare(R"(
        SELECT
            id,
            casting_date,
            casting_name,
            pcs,
            issue_metal_name,
            issue_metal_purity,
            issue_metal_wt,
            issue_diamond_pcs,
            issue_diamond_wt,
            receive_runner_wt,
            receive_product_wt,
            receive_diamond_pcs,
            receive_diamond_wt,
            status
        FROM casting_entry
        WHERE job_id = :job
    )");

    q.bindValue(":job", jobId);

    if (!q.exec()) {
        qCritical() << "getCastingDataByJob query failed:" << q.lastError();
        return false;
    }

    if (!q.next())
        return false;

    c.id = q.value(0).toInt();
    c.jobId = jobId;
    c.castingDate = q.value(1).toString();
    c.castingName = q.value(2).toString();
    c.pcs = q.value(3).toInt();

    c.issueMetalName = q.value(4).toString();
    c.issueMetalPurity = q.value(5).toString();
    c.issueMetalWt = q.value(6).toDouble();
    c.issueDiamondPcs = q.value(7).toInt();
    c.issueDiamondWt = q.value(8).toDouble();

    c.receiveRunnerWt = q.value(9).toDouble();
    c.receiveProductWt = q.value(10).toDouble();
    c.receiveDiamondPcs = q.value(11).toInt();
    c.receiveDiamondWt = q.value(12).toDouble();

    c.status = q.value(13).toString();

    return true;
}

std::optional<JobSheetData> DatabaseUtils::fetchJobSheetData(const QString &jobNo)
{
    const QString connName = QStringLiteral("fetch_jobsheet_conn_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    std::optional<JobSheetData> result;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        // db.setDatabaseName(QCoreApplication::applicationDirPath() + "/database/mega_mine_orderbook.db");
        QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("database/mega_mine_orderbook.db");
        db.setDatabaseName(dbPath);

        if (!db.open()) {
            qWarning() << "[ERROR] Failed to open DB in fetchJobSheetData:" << db.lastError().text();
            return std::nullopt;
        }

        QSqlQuery query(db);
        query.prepare(R"(
            SELECT sellerId, partyId, jobNo, orderNo, clientId,
                   orderDate, deliveryDate, productPis, designNo1,
                   metalPurity, metalColor, sizeNo, sizeMM,
                   length, width, height, image1path
            FROM "OrderBook-Detail"
            WHERE jobNo = :jobNo
        )");
        query.bindValue(":jobNo", jobNo);

        if (query.exec() && query.next()) {
            JobSheetData data;
            data.sellerId    = query.value("sellerId").toString();
            data.partyId     = query.value("partyId").toString();
            data.jobNo       = query.value("jobNo").toString();
            data.orderNo     = query.value("orderNo").toString();
            data.clientId    = query.value("clientId").toString();
            data.orderDate   = query.value("orderDate").toString();
            data.deliveryDate= query.value("deliveryDate").toString();
            data.productPis  = query.value("productPis").toInt();
            data.designNo    = query.value("designNo1").toString();
            data.metalPurity = query.value("metalPurity").toString();
            data.metalColor  = query.value("metalColor").toString();
            data.sizeNo      = query.value("sizeNo").toDouble();
            data.sizeMM      = query.value("sizeMM").toDouble();
            data.length      = query.value("length").toDouble();
            data.width       = query.value("width").toDouble();
            data.height      = query.value("height").toDouble();
            data.imagePath   = query.value("image1path").toString();

            result = data;
        } else {
            qWarning() << "[WARNING] No data found for jobNo:" << jobNo
                       << " Error:" << query.lastError().text();
        }

        db.close();
    }

    QSqlDatabase::removeDatabase(connName);
    return result;
}

QPair<QString, QString> DatabaseUtils::fetchDiamondAndStoneJson(const QString &designNo)
{
    const QString connName = "fetch_diamond_stone_conn";
    QString diamondJson, stoneJson;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("database/mega_mine_image.db");
        db.setDatabaseName(dbPath);

        if (!db.open()) {
            qWarning() << "[ERROR] Failed to open DB in fetchDiamondAndStoneJson:" << db.lastError().text();
            QSqlDatabase::removeDatabase(connName);
            return {};
        }

        // 🔹 Fetch diamond + stone JSON from image_data
        QSqlQuery query(db);
        query.prepare("SELECT diamond, stone FROM image_data WHERE design_no = :designNo");
        query.bindValue(":designNo", designNo);

        if (query.exec() && query.next()) {
            diamondJson = query.value(0).toString();
            stoneJson   = query.value(1).toString();
        } else {
            qWarning() << "[WARNING] No diamond/stone JSON found for designNo:" << designNo;
        }

        // 🔹 Helper lambda to find single piece weight
        auto getWeight = [&](const QString &type, const QString &sizeMM, bool isDiamond) -> double {
            QSqlQuery q(db);
            if (isDiamond) {
                if (type.compare("Round", Qt::CaseInsensitive) == 0) {
                    q.prepare("SELECT weight FROM Round_diamond WHERE sizeMM = ?");
                    q.addBindValue(sizeMM.toDouble());
                } else {
                    q.prepare("SELECT weight FROM Fancy_diamond WHERE shape = ? AND sizeMM = ?");
                    q.addBindValue(type);
                    q.addBindValue(sizeMM);
                }
            } else {
                q.prepare("SELECT weight FROM Stones WHERE shape = ? AND sizeMM = ?");
                q.addBindValue(type);
                q.addBindValue(sizeMM);
            }
            if (q.exec() && q.next())
                return q.value(0).toDouble();
            return 0.0;
        };

        // 🔹 Add weight info into diamond JSON
        if (!diamondJson.isEmpty()) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(diamondJson.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError && doc.isArray()) {
                QJsonArray updated;
                for (auto v : doc.array()) {
                    if (!v.isObject()) continue;
                    QJsonObject o = v.toObject();
                    double wt = getWeight(o["type"].toString(), o["sizeMM"].toString(), true);
                    o["weight"] = wt;
                    updated.append(o);
                }
                diamondJson = QString::fromUtf8(QJsonDocument(updated).toJson(QJsonDocument::Compact));
            }
        }

        // 🔹 Add weight info into stone JSON
        if (!stoneJson.isEmpty()) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(stoneJson.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError && doc.isArray()) {
                QJsonArray updated;
                for (auto v : doc.array()) {
                    if (!v.isObject()) continue;
                    QJsonObject o = v.toObject();
                    double wt = getWeight(o["type"].toString(), o["sizeMM"].toString(), false);
                    o["weight"] = wt;
                    updated.append(o);
                }
                stoneJson = QString::fromUtf8(QJsonDocument(updated).toJson(QJsonDocument::Compact));
            }
        }

        db.close();
    }
    QSqlDatabase::removeDatabase(connName);

    return {diamondJson, stoneJson};
}

QString DatabaseUtils::fetchImagePathForDesign(const QString &designNo)
{
    const QString connName = QStringLiteral("fetch_img_path_conn_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    QString imagePath;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        // db.setDatabaseName("database/mega_mine_image.db");
        QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("database/mega_mine_image.db");
        db.setDatabaseName(dbPath);

        if (db.open()) {
            QSqlQuery query(db);
            query.prepare("SELECT image_path FROM image_data WHERE design_no = :designNo");
            query.bindValue(":designNo", designNo);

            if (query.exec() && query.next()) {
                imagePath = query.value("image_path").toString();
            } else {
                qWarning() << "[ERROR] No image found for designNo:" << designNo;
            }

            db.close();
        } else {
            qWarning() << "[ERROR] Failed to open DB in fetchImagePathForDesign:" << db.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(connName);
    return imagePath;
}

void DatabaseUtils::fillStoneTable(QTableWidget *table, const QString &designNo)
{
    auto [diamondJson, stoneJson] = DatabaseUtils::fetchDiamondAndStoneJson(designNo);

    table->setRowCount(0);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"Type", "Name", "Quantity", "Size (MM)"});

    auto parseAndAddRows = [&](const QString &jsonStr, const QString &typeLabel) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            qWarning() << "[WARNING] JSON parse error for" << typeLabel << ":" << parseError.errorString();
            return;
        }

        for (const QJsonValue &value : doc.array()) {
            if (!value.isObject()) continue;
            QJsonObject obj = value.toObject();

            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(typeLabel));
            table->setItem(row, 1, new QTableWidgetItem(obj.value("type").toString()));
            table->setItem(row, 2, new QTableWidgetItem(obj.value("quantity").toString()));
            table->setItem(row, 3, new QTableWidgetItem(obj.value("sizeMM").toString()));
        }
    };

    parseAndAddRows(diamondJson, "Diamond");
    parseAndAddRows(stoneJson, "Stone");

    table->resizeColumnsToContents();
}

bool DatabaseUtils::updateDesignNoAndImagePath(const QString &jobNo, const QString &designNo, const QString &imagePath)
{
    const QString connName = QStringLiteral("update_design_img_conn_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    bool success = false;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        // db.setDatabaseName("database/mega_mine_orderbook.db");
        QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("database/mega_mine_orderbook.db");
        db.setDatabaseName(dbPath);

        if (db.open()) {
            QSqlQuery query(db);
            query.prepare(R"(
                UPDATE "OrderBook-Detail"
                SET designNo1 = :designNo, image1path = :imagePath
                WHERE jobNo = :jobNo
            )");
            query.bindValue(":designNo", designNo);
            query.bindValue(":imagePath", imagePath);
            query.bindValue(":jobNo", jobNo);

            if (query.exec()) {
                qDebug() << "[+] Design number and image path updated for jobNo:" << jobNo;
                success = true;
            } else {
                qWarning() << "[ERROR] Failed to update OrderBook-Detail:" << query.lastError().text();
            }

            db.close();
        } else {
            qWarning() << "[ERROR] Failed to open DB in updateDesignNoAndImagePath:" << db.lastError().text();
        }
    }

    QSqlDatabase::removeDatabase(connName);
    return success;
}

