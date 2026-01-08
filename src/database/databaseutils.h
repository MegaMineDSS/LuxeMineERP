#ifndef DATABASEUTILS_H
#define DATABASEUTILS_H

#include "models/CastingData.h"
#include "models/CastingListRow.h" // Assuming CastingListRow struct is modified in its own header
#include "models/OrderData.h"
#include "models/JobSheetData.h"

#include <QTableWidget>
#include <QJsonArray>

#include <xlsxdocument.h>
#include <xlsxworksheet.h>

class DatabaseUtils {
public:
  static bool createOrder(const OrderData &o, int &outJobId, int &outSellerSeq);
  static QList<OrderData> getOrdersForSeller(int sellerId);

  static QList<OrderData> getAllOrders();

  static bool getOrderById(int orderId, OrderData &o);

  static bool updateOrder(int orderId, const OrderData &o);

  static QList<CastingListRow> getCastingList();

  static int getCastingIdByJob(int jobId);

  static bool insertCasting(const CastingData &c);

  static bool updateCasting(int castingId, const CastingData &c);

  static bool getCastingDataByJob(int jobId, CastingData &c);

  static bool updateCastingDiaPrice(int jobId, double price);

  static std::optional<JobSheetData> fetchJobSheetData(const QString &jobNo);

  static QPair<QString, QString> fetchDiamondAndStoneJson(const QString &designNo);

  static QString fetchImagePathForDesign(const QString &designNo);

  static bool updateDesignNoAndImagePath(const QString &jobNo, const QString &designNo, const QString &imagePath);

  static void fillStoneTable(QTableWidget *table, const QString &designNo);

  static QList<QVariantList> fetchJewelryMenuItems();
  static QList<QVariantList> fetchCatalogData();

  static QString insertCatalogData(const QString &imagePath, const QString &imageType, const QString &designNo,
                                   const QString &companyName, const QJsonArray &goldArray,
                                   const QJsonArray &diamondArray, const QJsonArray &stoneArray,
                                   const QString &note) ;

  static QString saveImage(const QString &imagePath) ;

  static bool excelBulkInsertCatalog(const QString &filePath);

  static QJsonArray generateGoldWeights(int inputKarat, double inputWeight) ;

  static QStringList fetchShapes(const QString &tableType);

  static QStringList fetchSizes(const QString &tableType, const QString &shape);

  static bool deleteDesign(QString &designNo) ;

  DatabaseUtils();
};

#endif // DATABASEUTILS_H
