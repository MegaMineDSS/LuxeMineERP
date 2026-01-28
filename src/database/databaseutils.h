#ifndef DATABASEUTILS_H
#define DATABASEUTILS_H

#include <QJsonArray>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QTableWidget>
#include <QVariant>

#include "models/CastingData.h"
#include "models/CastingListRow.h" // Assuming CastingListRow struct is modified in its own header
#include "models/JobSheetData.h"
#include "models/OrderData.h"

#include <xlsxdocument.h>
#include <xlsxworksheet.h>

struct JobListData {
  int jobId;
  QString deliveryDate;

  // Order / Job Info
  QString designNo;
  QString jobNo; // From jobs table (via ID or custom formatted)
  int pcs;
  QString metal;
  QString purity;

  // Casting Info (Issue)
  QString status;
  QString mfgIssueDate;   // casting_date
  double issueWt;         // issue_metal_wt
  double materialIssueWt; // ??? maybe same as issueWt or pure calculated? Using
                          // issueWt for now.
  int issueDiaPcs;
  double issueDiaWt;
  int issueStonePcs;
  double issueStoneWt;
  QString issueDiaCategory;

  // Casting Info (Receive)
  QString receiveDate; // created_at of receive? or maybe update casting_entry
                       // to have receive_date
  double grossWt;      // receive_product_wt
  int receiveDiaPcs;
  double receiveDiaWt;
  int receiveStonePcs;
  double receiveStoneWt;

  double officeGoldReceive; // receive_runner_wt
  double officeReceive;     // New column
  double manufacturerMfgReceive;
  double netWt; // Calculated
  // Purity repeated?

  // Loss Calculated
  double grossLoss;
  double fineLoss;
  double percentage;
  double diaLoss;
  double stoneLoss;
  double manufacturerMfgLoss;

  QString remark; // status or note?

  // For Actions
  int dbJobId;
};

// ... existing code ...

struct DesignerOrderData {
  QString deliveryDate;
  QString designNo;
  QString jobNo;
  QString status;
  QString remark;
  int dbJobId;
};

struct GoldTotals {
  double totalIssue = 0.0;
  double dustWeight = 0.0;
  QString returnJson;
  double totalReturn = 0.0;
  double buffingReturn = 0.0;
  double freePolishReturn = 0.0;
  double settingReturn = 0.0;
  double finalPolishReturn = 0.0;
};

struct StockData {
  int id = 0;
  QString date;
  QString metal;
  QString detail;
  QString note;
  QString voucherNo;
  double purity = 0.0;
  double weight = 0.0;
  double weight24k = 0.0;
  double price = 0.0;
  double amount = 0.0;
};

struct MetalPurchaseData {
  int id = 0;
  QString entryDate;
  QString billNo;
  QString partyName;
  int pic = 0;
  QString productName;
  double weight = 0.0;
  double purity = 0.0;
  double labourAmount = 0.0;
  double totalGold = 0.0;
  double payWeight = 0.0;
  double totalPayAmount = 0.0;
  double costingPerGm = 0.0;
  QString remark;
};

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

  static std::optional<JobSheetData> fetchJobSheetData(int jobId);

  static QPair<QString, QString>
  fetchDiamondAndStoneJson(const QString &designNo);

  // New declarations
  static QJsonArray fetchJobSheetHistory(const QString &jobNo,
                                         const QString &colName);
  static bool addJobSheetHistoryEntry(const QString &jobNo,
                                      const QString &colName,
                                      const QJsonObject &entry);
  static QList<JobListData> getJobsList();
  static QStringList fetchShapes(const QString &tableType);
  static QStringList fetchSizes(const QString &tableType, const QString &shape);

  static QString fetchImagePathForDesign(const QString &designNo);

  static bool updateDesignNoAndImagePath(int jobId, const QString &designNo,
                                         const QString &imagePath);

  static void fillStoneTable(QTableWidget *table, const QString &designNo);

  static QList<QVariantList> fetchJewelryMenuItems();
  static QList<QVariantList> fetchCatalogData();

  static QString
  insertCatalogData(const QString &imagePath, const QString &imageType,
                    const QString &designNo, const QString &companyName,
                    const QJsonArray &goldArray, const QJsonArray &diamondArray,
                    const QJsonArray &stoneArray, const QString &note);

  static QString saveImage(const QString &imagePath);

  static bool excelBulkInsertCatalog(const QString &filePath);

  static QJsonArray generateGoldWeights(int inputKarat, double inputWeight);

  static bool deleteDesign(QString &designNo);

  static bool saveGoldStageReturn(const QString &jobNo, const QString &column,
                                  double value);

  static bool updateOfficeGoldReceive(int jobId, double weight);
  static bool updateOfficeReceive(int jobId, double weight);
  static bool updateManufacturerMfgReceive(int jobId, double weight);

  // Missing declarations added below
  static QList<DesignerOrderData> getDesignerOrders();
  static QMap<QString, QPair<int, double>>
  fetchDiamondTotals(const QString &jobNo);
  static GoldTotals fetchGoldTotals(const QString &jobNo);

  static bool addStock(const StockData &data);
  static bool updateStock(const StockData &data);
  static QList<StockData> getAllStocks();

  static bool addMetalPurchase(const MetalPurchaseData &data);
  static QList<MetalPurchaseData> getAllMetalPurchases();

  // static bool deleteDesign(QString &designNo) ;

  DatabaseUtils();
};

#endif // DATABASEUTILS_H
