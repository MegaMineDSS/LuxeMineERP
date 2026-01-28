#include "dashboardwidget.h"
#include "ui_dashboard.h"

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardWidget) {
  ui->setupUi(this);
}

DashboardWidget::~DashboardWidget() { delete ui; }
