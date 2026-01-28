#include "designerorderlistwidget.h"
#include "database/databaseutils.h"
#include "ui_designerorderlist.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPushButton>


#include "dashboards/designerwindow.h"
#include "manufacturer/jobsheetwidget.h"


DesignerOrderListWidget::DesignerOrderListWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::DesignerOrderListWidget) {
  ui->setupUi(this);
  setupTable();
  loadData();
}

DesignerOrderListWidget::~DesignerOrderListWidget() { delete ui; }

void DesignerOrderListWidget::setupTable() {
  QStringList headers = {"Delivery Date", "Design No", "Job No",
                         "Status",        "Remark",    "Action"};

  ui->tableWidget->setColumnCount(headers.size());
  ui->tableWidget->setHorizontalHeaderLabels(headers);
  ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
  ui->tableWidget->setAlternatingRowColors(true);
  ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Context Menu
  ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this,
          &DesignerOrderListWidget::onContextMenuRequested);
}

void DesignerOrderListWidget::loadData() {
  ui->tableWidget->setRowCount(0);
  QList<DesignerOrderData> list = DatabaseUtils::getDesignerOrders();

  for (const DesignerOrderData &d : list) {
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    int col = 0;

    auto setItem = [&](const QString &text) {
      QTableWidgetItem *item = new QTableWidgetItem(text);
      item->setTextAlignment(Qt::AlignCenter);
      ui->tableWidget->setItem(row, col++, item);
    };

    setItem(d.deliveryDate);
    setItem(d.designNo);
    setItem(d.jobNo);
    setItem(d.status);
    setItem(d.remark);

    // Action
    QPushButton *btn = new QPushButton("Open");
    btn->setProperty("jobId", d.dbJobId);
    connect(btn, &QPushButton::clicked, this,
            &DesignerOrderListWidget::onOpenJobClicked);
    ui->tableWidget->setCellWidget(row, col++, btn);
  }
}

void DesignerOrderListWidget::onOpenJobClicked() {
  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (!btn)
    return;

  int jobId = btn->property("jobId").toInt();
  qDebug() << "Opening Design Job:" << jobId;
}

void DesignerOrderListWidget::onContextMenuRequested(const QPoint &pos) {
  QTableWidgetItem *item = ui->tableWidget->itemAt(pos);
  if (!item)
    return;

  QMenu menu(this);
  QAction *openAction = menu.addAction("Open JobSheet");
  connect(openAction, &QAction::triggered, this,
          &DesignerOrderListWidget::openJobSheet);
  menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));
}

void DesignerOrderListWidget::openJobSheet() {
  int row = ui->tableWidget->currentRow();
  if (row < 0)
    return;

  QTableWidgetItem *jobItem = ui->tableWidget->item(row, 2);
  if (!jobItem)
    return;

  QString jobNo = jobItem->text();

  // 🔹 Find DesignerDashboard (parent in MDI)
  DesignerWindow *dashboard = nullptr;
  QWidget *p = this;
  while (p) {
    dashboard = qobject_cast<DesignerWindow *>(p);
    if (dashboard)
      break;
    p = p->parentWidget();
  }

  if (!dashboard) {
    qDebug() << "DesignerWindow not found as parent!";
    // Fallback or just return. If tested standalone, this might fail.
    return;
  }

  QMdiArea *mdiArea = dashboard->mdiArea();
  if (!mdiArea)
    return;

  // 🔹 Check if already open? (Optional enhancement, but nice)

  // 🔹 Create JobSheet
  JobSheetWidget *jobSheet = new JobSheetWidget();
  jobSheet->set_value(jobNo);
  jobSheet->setUserRole("designer");

  QMdiSubWindow *subWindow = mdiArea->addSubWindow(jobSheet);
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->setWindowTitle("Job Sheet - " + jobNo);
  subWindow->show();
}
