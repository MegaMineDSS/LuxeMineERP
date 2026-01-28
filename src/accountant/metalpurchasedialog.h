#ifndef METALPURCHASEDIALOG_H
#define METALPURCHASEDIALOG_H

#include "database/databaseutils.h"
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>


class MetalPurchaseDialog : public QDialog {
  Q_OBJECT

public:
  explicit MetalPurchaseDialog(QWidget *parent = nullptr);

private slots:
  void save();
  void calculateTotals();

private:
  QDateEdit *dateEdit;
  QLineEdit *txtBillNo;
  QLineEdit *txtPartyName;
  QSpinBox *spinPic;
  QLineEdit *txtProduct;
  QDoubleSpinBox *spinWeight;
  QDoubleSpinBox *spinPurity;
  QDoubleSpinBox *spinLabour;
  QDoubleSpinBox *spinTotalGold;
  QDoubleSpinBox *spinPayWeight;
  QDoubleSpinBox *spinTotalPayAmount;
  QDoubleSpinBox *spinCosting;
  QLineEdit *txtRemark;

  void setupUi();
};

#endif // METALPURCHASEDIALOG_H
