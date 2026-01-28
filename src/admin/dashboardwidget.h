#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>

namespace Ui {
class DashboardWidget;
}

class DashboardWidget : public QWidget {
  Q_OBJECT

public:
  explicit DashboardWidget(QWidget *parent = nullptr);
  ~DashboardWidget();

private:
  Ui::DashboardWidget *ui;
};

#endif // DASHBOARDWIDGET_H
