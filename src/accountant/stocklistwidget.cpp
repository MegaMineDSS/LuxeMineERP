#include "stocklistwidget.h"
#include "database/databaseutils.h"
#include "stockwidget.h"
#include "ui_stocklist.h"
#include <QMenu>
#include <QMessageBox>

StockListWidget::StockListWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::StockListWidget) {
  ui->setupUi(this);
  setupTable();
  loadData();
}

StockListWidget::~StockListWidget() { delete ui; }

void StockListWidget::setupTable() {
  QStringList headers = {"Date",   "Metal",  "Detail", "Note",  "Voucher No",
                         "Purity", "Weight", "24K",    "Price", "Amount"};
  ui->tableWidget->setColumnCount(headers.size());
  ui->tableWidget->setHorizontalHeaderLabels(headers);
  ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
  ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
  ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this,
          &StockListWidget::onCustomContextMenuRequested);
}

void StockListWidget::loadData() {
  ui->tableWidget->setRowCount(0);
  QList<StockData> list = DatabaseUtils::getAllStocks();

  for (const auto &s : list) {
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    ui->tableWidget->insertRow(row);

    QTableWidgetItem *itemDate = new QTableWidgetItem(s.date);
    itemDate->setData(Qt::UserRole, s.id); // Store ID
    itemDate->setData(Qt::UserRole + 1, s.purity);
    itemDate->setData(Qt::UserRole + 2, s.weight);
    itemDate->setData(Qt::UserRole + 3, s.price);
    // Could store whole struct pointer but checking ID + reload from UI is
    // easier or just passing what we have. Actually we need to pass all data to
    // setStockData. Simplified: Store ID, and when editing, iterate list to
    // find data matching ID? Or just store important fields in hidden columns?
    // Better: Helper function to get StockData from row, or just store ID and
    // use getStockById (need to implement?) Or simpler: We have the list in
    // loadData scope but not member variable. Let's store necessary data fields
    // in UserRole of different columns or just rely on text for strings and
    // UserRole for doubles.

    ui->tableWidget->setItem(row, 0, itemDate);
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(s.metal));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(s.detail));
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(s.note));
    ui->tableWidget->setItem(row, 4, new QTableWidgetItem(s.voucherNo));
    ui->tableWidget->setItem(
        row, 5, new QTableWidgetItem(QString::number(s.purity, 'f', 3)));
    ui->tableWidget->setItem(
        row, 6, new QTableWidgetItem(QString::number(s.weight, 'f', 3)));
    ui->tableWidget->setItem(
        row, 7, new QTableWidgetItem(QString::number(s.weight24k, 'f', 3)));
    ui->tableWidget->setItem(
        row, 8, new QTableWidgetItem(QString::number(s.price, 'f', 2)));
    ui->tableWidget->setItem(
        row, 9, new QTableWidgetItem(QString::number(s.amount, 'f', 2)));
  }

  calculateTotals();
}

void StockListWidget::calculateTotals() {
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

  // Columns to sum: Weight(6), 24K(7), Amount(9)
  QList<int> sumCols = {6, 7, 9};

  for (int col : sumCols) {
    double sum = 0.0;

    for (int r = 0; r < rowCount; r++) {
      QTableWidgetItem *it = ui->tableWidget->item(r, col);
      if (it) {
        sum += it->text().toDouble();
      }
    }

    QTableWidgetItem *totalItem =
        new QTableWidgetItem(QString::number(sum, 'f', 3));
    if (col == 9)
      totalItem->setText(
          QString::number(sum, 'f', 2)); // Amount usually 2 decimals

    totalItem->setTextAlignment(Qt::AlignCenter);
    totalItem->setFont(font);
    totalItem->setFlags(totalItem->flags() & ~Qt::ItemIsEditable);
    ui->tableWidget->setItem(rowCount, col, totalItem);
  }

  // Empty cells
  int colCount = ui->tableWidget->columnCount();
  for (int c = 0; c < colCount; c++) {
    if (!ui->tableWidget->item(rowCount, c)) {
      QTableWidgetItem *empty = new QTableWidgetItem();
      empty->setFlags(empty->flags() & ~Qt::ItemIsEditable);
      ui->tableWidget->setItem(rowCount, c, empty);
    }
  }
}

void StockListWidget::on_btnAddStock_clicked() {
  StockWidget *w = new StockWidget;
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show();
  // In a real MDI app, ideally verify integration context, but separate window
  // is fine for popup Or connect signal to reload
  connect(w, &QWidget::destroyed, this, &StockListWidget::loadData);
}

void StockListWidget::on_btnRefresh_clicked() { loadData(); }

void StockListWidget::onCustomContextMenuRequested(const QPoint &pos) {
  QTableWidgetItem *item = ui->tableWidget->itemAt(pos);
  if (!item)
    return;

  QMenu menu(this);
  QAction *actEdit = menu.addAction("Edit Stock");
  connect(actEdit, &QAction::triggered, [=]() {
    int row = item->row();
    int id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();

    StockData data;
    data.id = id;
    data.date = ui->tableWidget->item(row, 0)->text();
    data.metal = ui->tableWidget->item(row, 1)->text();
    data.detail = ui->tableWidget->item(row, 2)->text();
    data.note = ui->tableWidget->item(row, 3)->text();
    data.voucherNo = ui->tableWidget->item(row, 4)->text();

    // Parsing numbers from text might be error prone due to formatting.
    // Best to store originals or re-fetch.
    // Let's implement getStockById for cleaner approach?
    // OR just parse carefully. We formatted with 'f', 3 or 2.

    data.purity = ui->tableWidget->item(row, 5)->text().toDouble();
    data.weight = ui->tableWidget->item(row, 6)->text().toDouble();

    // 24k is calc
    data.weight24k = ui->tableWidget->item(row, 7)->text().toDouble();

    data.price = ui->tableWidget->item(row, 8)->text().toDouble();
    data.amount = ui->tableWidget->item(row, 9)->text().toDouble();

    StockWidget *w = new StockWidget;
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setStockData(data);
    w->show();
    connect(w, &QWidget::destroyed, this, &StockListWidget::loadData);
  });

  menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));
}
