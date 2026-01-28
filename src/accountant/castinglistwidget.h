#ifndef CASTINGLISTWIDGET_H
#define CASTINGLISTWIDGET_H

#include <QMdiArea>
#include <QTableWidget>
#include <QWidget>


namespace Ui {
class CastingListWidget;
}

class CastingListWidget : public QWidget {
  Q_OBJECT

public:
  explicit CastingListWidget(QWidget *parent = nullptr);
  ~CastingListWidget();

private slots:
  void onTableContextMenu(const QPoint &pos);
  void onItemChanged(QTableWidgetItem *item);

private:
  Ui::CastingListWidget *ui;

  void setupTable();
  void loadCastingList();
  void calculateTotals();

  void openCastingWidget(int jobId);
  QMdiArea *findMdiArea(QWidget *w);
};

#endif // CASTINGLISTWIDGET_H
