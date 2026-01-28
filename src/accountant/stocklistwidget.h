#ifndef STOCKLISTWIDGET_H
#define STOCKLISTWIDGET_H

#include <QWidget>

namespace Ui {
class StockListWidget;
}

class StockListWidget : public QWidget {
  Q_OBJECT

public:
  explicit StockListWidget(QWidget *parent = nullptr);
  ~StockListWidget();

  void loadData();

private slots:
  void on_btnAddStock_clicked();
  void on_btnRefresh_clicked();
  void onCustomContextMenuRequested(const QPoint &pos);

private:
  Ui::StockListWidget *ui;
  void setupTable();
  void calculateTotals();
};

#endif // STOCKLISTWIDGET_H
