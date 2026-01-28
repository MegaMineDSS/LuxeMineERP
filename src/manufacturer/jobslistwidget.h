#ifndef JOBSLISTWIDGET_H
#define JOBSLISTWIDGET_H

#include <QWidget>

namespace Ui {
class JobsListWidget;
}

class JobsListWidget : public QWidget {
  Q_OBJECT

public:
  explicit JobsListWidget(QWidget *parent = nullptr);
  ~JobsListWidget();

private:
  Ui::JobsListWidget *ui;
  void setupTable();
  void loadData();
  void calculateTotals();

private slots:
  void onOpenJobClicked();
  void onCellChanged(int row, int col);
};

#endif // JOBSLISTWIDGET_H
