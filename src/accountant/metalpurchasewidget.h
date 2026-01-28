#ifndef METALPURCHASEWIDGET_H
#define METALPURCHASEWIDGET_H

#include "database/databaseutils.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

class MetalPurchaseWidget : public QWidget {
  Q_OBJECT

public:
  explicit MetalPurchaseWidget(QWidget *parent = nullptr);
  void loadData();

private slots:
  void onAddEntryClicked();

private:
  QTableWidget *table;
  QPushButton *btnAdd;
  void calculateTotals();

  void setupUi();
};

#endif // METALPURCHASEWIDGET_H
