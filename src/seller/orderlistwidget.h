#ifndef ORDERLISTWIDGET_H
#define ORDERLISTWIDGET_H

#include "OrderData.h"
#include <QTableWidget>
#include <QWidget>


namespace Ui {
class OrderListWidget;
}

class OrderListWidget : public QWidget {
  Q_OBJECT

public:
  explicit OrderListWidget(QWidget *parent = nullptr);
  ~OrderListWidget();

signals:
  void requestOpenOrder(int orderId);

private slots:
  void loadOrders();
  void onEditClicked(int orderId);
  void onDeleteClicked(int orderId);

private:
  Ui::OrderListWidget *ui;

  void setupTable();
  void addRow(int row, const OrderData &order);
  void calculateTotals();
};

#endif // ORDERLISTWIDGET_H
