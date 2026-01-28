#ifndef STOCKWIDGET_H
#define STOCKWIDGET_H

#include "database/databaseutils.h"
#include <QWidget>


namespace Ui {
class StockWidget;
}

class StockWidget : public QWidget {
  Q_OBJECT

public:
  explicit StockWidget(QWidget *parent = nullptr);
  ~StockWidget();

  void setStockData(const StockData &data);

private slots:
  void calculateValues();
  void on_btnSave_clicked();
  void on_btnCancel_clicked();

private:
  Ui::StockWidget *ui;
  int m_stockId = -1;
};

#endif // STOCKWIDGET_H
