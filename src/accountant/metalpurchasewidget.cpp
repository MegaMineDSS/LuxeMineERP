#include "metalpurchasewidget.h"
#include "metalpurchasedialog.h"
#include <QMessageBox>

MetalPurchaseWidget::MetalPurchaseWidget(QWidget *parent) : QWidget(parent) {
  setupUi();
  loadData();
}

void MetalPurchaseWidget::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Header / Controls
  QHBoxLayout *topLayout = new QHBoxLayout();
  btnAdd = new QPushButton("Add New Entry", this);
  btnAdd->setStyleSheet("background-color: #4CAF50; color: white; padding: 5px "
                        "15px; font-weight: bold;");

  topLayout->addStretch();
  topLayout->addWidget(btnAdd);
  mainLayout->addLayout(topLayout);

  // Table
  table = new QTableWidget(this);
  QStringList headers = {"Date",        "Bill No",           "Name Party",
                         "PIC",         "Product",           "Weight",
                         "Purity",      "Labour\nMattel",    "Total\nGold",
                         "Pay\nWeight", "Total Pay\nAmount", "Costing\nPer Gm",
                         "Remark"};

  table->setColumnCount(headers.size());
  table->setHorizontalHeaderLabels(headers);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  mainLayout->addWidget(table);

  connect(btnAdd, &QPushButton::clicked, this,
          &MetalPurchaseWidget::onAddEntryClicked);
}

void MetalPurchaseWidget::loadData() {
  table->setRowCount(0);
  QList<MetalPurchaseData> list = DatabaseUtils::getAllMetalPurchases();

  for (const auto &data : list) {
    int row = table->rowCount();
    table->insertRow(row);

    table->setItem(row, 0, new QTableWidgetItem(data.entryDate));
    table->setItem(row, 1, new QTableWidgetItem(data.billNo));
    table->setItem(row, 2, new QTableWidgetItem(data.partyName));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(data.pic)));
    table->setItem(row, 4, new QTableWidgetItem(data.productName));
    table->setItem(row, 5,
                   new QTableWidgetItem(QString::number(data.weight, 'f', 3)));
    table->setItem(row, 6,
                   new QTableWidgetItem(QString::number(data.purity, 'f', 2)));
    table->setItem(
        row, 7,
        new QTableWidgetItem(QString::number(data.labourAmount, 'f', 2)));
    table->setItem(
        row, 8, new QTableWidgetItem(QString::number(data.totalGold, 'f', 3)));
    table->setItem(
        row, 9, new QTableWidgetItem(QString::number(data.payWeight, 'f', 3)));
    table->setItem(
        row, 10,
        new QTableWidgetItem(QString::number(data.totalPayAmount, 'f', 2)));
    table->setItem(
        row, 11,
        new QTableWidgetItem(QString::number(data.costingPerGm, 'f', 2)));
    table->setItem(row, 12, new QTableWidgetItem(data.remark));
  }

  calculateTotals();
}

void MetalPurchaseWidget::calculateTotals() {
  int rowCount = table->rowCount();
  if (rowCount == 0)
    return;

  table->insertRow(rowCount);

  // Set "Total" label
  QTableWidgetItem *label = new QTableWidgetItem("Total");
  label->setTextAlignment(Qt::AlignCenter);
  QFont font = label->font();
  font.setBold(true);
  label->setFont(font);
  label->setFlags(label->flags() & ~Qt::ItemIsEditable);
  table->setItem(rowCount, 0, label);

  // Columns to sum: Weight(5), Labour(7), TotalGold(8), PayWeight(9),
  // TotalPayAmount(10)
  QList<int> sumCols = {5, 7, 8, 9, 10};

  for (int col : sumCols) {
    double sum = 0.0;

    for (int r = 0; r < rowCount; r++) {
      QTableWidgetItem *it = table->item(r, col);
      if (it) {
        sum += it->text().toDouble();
      }
    }

    QTableWidgetItem *totalItem =
        new QTableWidgetItem(QString::number(sum, 'f', 3));
    if (col == 7 || col == 10)
      totalItem->setText(
          QString::number(sum, 'f', 2)); // Amounts often 2 decimals

    totalItem->setTextAlignment(Qt::AlignCenter);
    totalItem->setFont(font);
    totalItem->setFlags(totalItem->flags() & ~Qt::ItemIsEditable);
    table->setItem(rowCount, col, totalItem);
  }

  // Empty cells
  int colCount = table->columnCount();
  for (int c = 0; c < colCount; c++) {
    if (!table->item(rowCount, c)) {
      QTableWidgetItem *empty = new QTableWidgetItem();
      empty->setFlags(empty->flags() & ~Qt::ItemIsEditable);
      table->setItem(rowCount, c, empty);
    }
  }
}

void MetalPurchaseWidget::onAddEntryClicked() {
  MetalPurchaseDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted) {
    loadData();
  }
}
