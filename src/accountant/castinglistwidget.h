#ifndef CASTINGLISTWIDGET_H
#define CASTINGLISTWIDGET_H

#include <QMdiArea>
#include <QWidget>
#include <QTableWidget>

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

  void openCastingWidget(int jobId);
  QMdiArea *findMdiArea(QWidget *w);
};

#endif // CASTINGLISTWIDGET_H
