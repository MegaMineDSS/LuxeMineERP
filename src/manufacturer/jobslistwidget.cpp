#include "jobslistwidget.h"
#include "database/databaseutils.h"
#include "ui_jobslist.h"

#include <QDebug>
#include <QHeaderView>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPushButton>

#include "dashboards/manufacturerwindow.h"
#include "jobsheetwidget.h"

JobsListWidget::JobsListWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::JobsListWidget) {
  ui->setupUi(this);
  setupTable();
  loadData();
}

JobsListWidget::~JobsListWidget() { delete ui; }

void JobsListWidget::setupTable() {
  QStringList headers = {"Delivery\nDate",
                         "Design\nNo",
                         "Job\nNo",
                         "Pcs",
                         "Metal",
                         "Purity",
                         "Status",
                         "Mfg\nIssue\nDate",
                         "Issue\nWt",
                         "Material\nIssue\nWt",
                         "Issue\nDia\nPcs",
                         "Issue\nDia\nWt",
                         "Issue\nStone\nPcs",
                         "Issue\nStone\nWt",
                         "Issue\nDia\nCat",
                         "Receive\nDate",
                         "Gross\nWt",
                         "Receive\nDia\nPcs",
                         "Receive\nDia\nWt",
                         "Receive\nStone\nPcs",
                         "Receive\nStone\nWt",
                         "Office\nGold\nRec",
                         "Office\nReceive",
                         "Mfg\nReceive",
                         "Net\nWt",
                         "Purity",
                         "Gross\nLoss",
                         "Fine\nLoss",
                         "Percentage\n%",
                         "Dia\nLoss",
                         "Stone\nLoss",
                         "Remark",
                         "Action"};

  ui->tableWidget->setColumnCount(headers.size());
  ui->tableWidget->setHorizontalHeaderLabels(headers);

  // Adjust column widths if needed, or just stretch
  // ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  // Maybe stretch only "Remark" or "Design No"?
  // For now:
  // ui->tableWidget->horizontalHeader()->setStretchLastSection(true); // Action
  // column

  ui->tableWidget->setAlternatingRowColors(true);
  ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  // Enable editing triggers
  ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked |
                                   QAbstractItemView::EditKeyPressed);

  connect(ui->tableWidget, &QTableWidget::cellChanged, this,
          &JobsListWidget::onCellChanged);
}

void JobsListWidget::loadData() {
  ui->tableWidget->setRowCount(0);
  QList<JobListData> list = DatabaseUtils::getJobsList();

  // Block signals to prevent onCellChanged from firing during population
  const bool wasBlocked = ui->tableWidget->blockSignals(true);

  for (const JobListData &d : list) {
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    int col = 0;

    auto setItem = [&](const QString &text, bool editable = false,
                       int jobId = -1) {
      QTableWidgetItem *item = new QTableWidgetItem(text);
      item->setTextAlignment(Qt::AlignCenter);
      if (!editable) {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      } else {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if (jobId != -1) {
          item->setData(Qt::UserRole, jobId);
        }
      }
      ui->tableWidget->setItem(row, col++, item);
    };

    auto setNum = [&](double val, int prec = 3, bool editable = false,
                      int jobId = -1) {
      setItem(QString::number(val, 'f', prec), editable, jobId);
    };

    auto setInt = [&](int val) { setItem(QString::number(val)); };

    setItem(d.deliveryDate);
    setItem(d.designNo);
    setItem(d.jobNo);
    setInt(d.pcs);
    setItem(d.metal);
    setItem(d.purity);
    setItem(d.status);
    setItem(d.mfgIssueDate);
    setNum(d.issueWt);
    setNum(d.materialIssueWt);
    setInt(d.issueDiaPcs);
    setNum(d.issueDiaWt);
    setInt(d.issueStonePcs);
    setNum(d.issueStoneWt);
    setItem(d.issueDiaCategory);

    setItem(d.receiveDate);
    setNum(d.grossWt);
    setInt(d.receiveDiaPcs);
    setNum(d.receiveDiaWt);
    setInt(d.receiveStonePcs);
    setNum(d.receiveStoneWt);
    setNum(d.officeGoldReceive, 3, true, d.jobId);      // Col 21 Editable
    setNum(d.officeReceive, 3, true, d.jobId);          // Col 22 Editable
    setNum(d.manufacturerMfgReceive, 3, true, d.jobId); // Col 23 Editable
    setNum(d.netWt);
    setItem(d.purity);
    setNum(d.grossLoss);
    setNum(d.fineLoss);
    setItem(QString::number(d.percentage, 'f', 2) + "%");
    setNum(d.diaLoss);
    setNum(d.stoneLoss);
    setItem(d.remark);

    // Action
    QPushButton *btn = new QPushButton("Open");
    btn->setProperty("jobId", d.dbJobId);
    connect(btn, &QPushButton::clicked, this,
            &JobsListWidget::onOpenJobClicked);
    ui->tableWidget->setCellWidget(row, col++, btn);
  }

  ui->tableWidget->blockSignals(wasBlocked);
  calculateTotals();
}

void JobsListWidget::calculateTotals() {
  int rowCount = ui->tableWidget->rowCount();
  if (rowCount == 0)
    return;

  ui->tableWidget->insertRow(rowCount);

  // Set "Total" label
  QTableWidgetItem *label = new QTableWidgetItem("Total");
  label->setTextAlignment(Qt::AlignCenter);
  QFont font = label->font();
  font.setBold(true);
  label->setFont(font);
  label->setFlags(label->flags() & ~Qt::ItemIsEditable);
  ui->tableWidget->setItem(rowCount, 0, label);

  // Columns to sum
  QList<int> sumCols = {3,  8,  9,  10, 11, 12, 13, 16, 17, 18,
                        19, 20, 21, 22, 23, 24, 26, 27, 29, 30};

  for (int col : sumCols) {
    double sum = 0.0;
    bool isInt = true; // Assumption, check based on col index if needed or just
                       // sum as double

    // Check if integer column
    if (col == 3 || col == 10 || col == 12 || col == 17 || col == 19)
      isInt = true;
    else
      isInt = false;

    for (int r = 0; r < rowCount; r++) {
      QTableWidgetItem *it = ui->tableWidget->item(r, col);
      if (it) {
        sum += it->text().toDouble();
      }
    }

    QTableWidgetItem *totalItem = new QTableWidgetItem();
    if (isInt) {
      totalItem->setText(QString::number((int)sum));
    } else {
      totalItem->setText(QString::number(sum, 'f', 3));
    }

    totalItem->setTextAlignment(Qt::AlignCenter);
    totalItem->setFont(font);
    totalItem->setFlags(totalItem->flags() & ~Qt::ItemIsEditable);
    ui->tableWidget->setItem(rowCount, col, totalItem);
  }

  // Make other cells in total row read-only and bold/empty
  int colCount = ui->tableWidget->columnCount();
  for (int c = 0; c < colCount; c++) {
    if (!ui->tableWidget->item(rowCount, c)) {
      QTableWidgetItem *empty = new QTableWidgetItem();
      empty->setFlags(empty->flags() & ~Qt::ItemIsEditable);
      ui->tableWidget->setItem(rowCount, c, empty);
    }
  }
}

void JobsListWidget::onOpenJobClicked() {
  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (!btn)
    return;

  int jobId = btn->property("jobId").toInt();
  qDebug() << "Open job requested for ID:" << jobId;

  // Find ManufacturerWindow parent
  ManufacturerWindow *dashboard = nullptr;
  QWidget *p = this;
  while (p) {
    dashboard = qobject_cast<ManufacturerWindow *>(p);
    if (dashboard)
      break;
    p = p->parentWidget();
  }

  if (!dashboard) {
    qDebug() << "ManufacturerWindow parent not found!";
    return;
  }

  QMdiArea *mdiArea = dashboard->mdiArea();
  if (!mdiArea)
    return;

  // Check if already open? (Optional)

  JobSheetWidget *jobSheet = new JobSheetWidget;
  jobSheet->setUserRole("manufacturer");
  jobSheet->set_value(QString::number(jobId));

  QMdiSubWindow *sub = mdiArea->addSubWindow(jobSheet);
  sub->setWindowTitle("JobSheet - " + QString::number(jobId));
  sub->setAttribute(Qt::WA_DeleteOnClose);
  jobSheet->show();
}

void JobsListWidget::onCellChanged(int row, int col) {
  // Office Gold Rec is index 21
  if (col == 21) {
    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    if (!item)
      return;

    int jobId = item->data(Qt::UserRole).toInt();
    if (jobId <= 0)
      return; // Should not happen if set correctly

    bool ok = false;
    double val = item->text().toDouble(&ok);
    if (ok) {
      DatabaseUtils::updateOfficeGoldReceive(jobId, val);
      qDebug() << "Updated Office Gold Rec for Job" << jobId << "to" << val;
    }
  } else if (col == 22) {
    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    if (!item)
      return;

    int jobId = item->data(Qt::UserRole).toInt();
    if (jobId <= 0)
      return;

    bool ok = false;
    double val = item->text().toDouble(&ok);
    if (ok) {
      DatabaseUtils::updateOfficeReceive(jobId, val);
      qDebug() << "Updated Office Receive for Job" << jobId << "to" << val;
    }
  } else if (col == 23) {
    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    if (!item)
      return;

    int jobId = item->data(Qt::UserRole).toInt();
    if (jobId <= 0)
      return;

    bool ok = false;
    double val = item->text().toDouble(&ok);
    if (ok) {
      DatabaseUtils::updateManufacturerMfgReceive(jobId, val);
      qDebug() << "Updated Mfg Receive for Job" << jobId << "to" << val;
    }
  }
}
