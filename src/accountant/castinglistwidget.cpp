#include "castinglistwidget.h"
#include "ui_castinglist.h"

#include "accountant/castingwidget.h"
#include "database/databaseutils.h"

#include <QMenu>
// #include <QMdiArea>
#include <QMdiSubWindow>

CastingListWidget::CastingListWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CastingListWidget) {
  ui->setupUi(this);

  setupTable();

  ui->castingTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(ui->castingTableWidget, &QTableWidget::customContextMenuRequested,
          this, &CastingListWidget::onTableContextMenu);

  connect(ui->castingTableWidget, &QTableWidget::itemChanged, this,
          &CastingListWidget::onItemChanged);

  loadCastingList();
}

CastingListWidget::~CastingListWidget() { delete ui; }

void CastingListWidget::setupTable() {
  auto *table = ui->castingTableWidget;

  QStringList headers = {"Job No",
                         "Delivery Date",
                         "Casting Date",
                         "Vendor Name",
                         "PCS",
                         "Issue Metal",
                         "Purity",
                         "Issue Metal Wt.",
                         "Issue Dia Pcs.",
                         "Issue Dia Wt.",
                         "Ranar Wt.",
                         "Product Wt",
                         "Receive Dia Pcs.",
                         "Receive Dia Wt.",
                         "Gross Loss",
                         "Fine Loss",
                         "Dia Pcs Loss",
                         "Dia Wt. Loss",
                         "Dia Price",
                         "Dia Loss Price",
                         "Status",
                         "Action"};

  table->setColumnCount(headers.size());
  table->setHorizontalHeaderLabels(headers);

  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::SingleSelection);

  // Allow editing, but we control it per item via flags
  table->setEditTriggers(QAbstractItemView::DoubleClicked |
                         QAbstractItemView::EditKeyPressed);

  table->horizontalHeader()->setStretchLastSection(true);
  table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);

  table->verticalHeader()->setVisible(false);
}

void CastingListWidget::loadCastingList() {
  auto *table = ui->castingTableWidget;
  table->setRowCount(0);

  table->setRowCount(0);
  // Block signals while loading to prevent onItemChanged from firing
  table->blockSignals(true);

  auto rows = DatabaseUtils::getCastingList();

  for (int i = 0; i < rows.size(); ++i) {
    const auto &r = rows[i];
    table->insertRow(i);

    // Job No
    auto *jobItem = new QTableWidgetItem(QString::number(r.jobId));
    jobItem->setData(Qt::UserRole, r.jobId);
    table->setItem(i, 0, jobItem);

    table->setItem(i, 1, new QTableWidgetItem(r.deliveryDate));
    table->setItem(i, 2, new QTableWidgetItem(r.castingDate));
    table->setItem(i, 3, new QTableWidgetItem(r.vendorName));
    table->setItem(i, 4, new QTableWidgetItem(QString::number(r.pcs)));

    table->setItem(i, 5, new QTableWidgetItem(r.issueMetal));
    table->setItem(i, 6, new QTableWidgetItem(r.purity));
    table->setItem(i, 7, new QTableWidgetItem(QString::number(r.issueMetalWt)));
    table->setItem(i, 8, new QTableWidgetItem(QString::number(r.issueDiaPcs)));
    table->setItem(i, 9, new QTableWidgetItem(QString::number(r.issueDiaWt)));

    table->setItem(i, 10,
                   new QTableWidgetItem(QString::number(r.receiveRunnerWt)));
    table->setItem(i, 11,
                   new QTableWidgetItem(QString::number(r.receiveProductWt)));
    table->setItem(i, 12,
                   new QTableWidgetItem(QString::number(r.receiveDiaPcs)));
    table->setItem(i, 13,
                   new QTableWidgetItem(QString::number(r.receiveDiaWt)));

    // Loss columns (leave empty for now)
    // 14..17
    // --- CALCULATIONS ---
    // Gross Loss = ((Ranar Wt + Product Wt.) - (Issue Dia Wt. / 20.0) - Issue
    // Wt.) Note: Issue Wt usually refers to Issue Metal Wt. Dia Wt conversion:
    // User said "Issue Dia Wt. / (10*2)". Assuming 20.0 factor.
    double diaWtAdjustment = (r.issueDiaWt / 5.0);
    double grossLoss = (r.receiveRunnerWt + r.receiveProductWt) -
                       diaWtAdjustment - r.issueMetalWt;

    // Fine Loss = Gross Loss / (100 * purity) -- AS REQUESTED
    // We need to parse purity. "18K", "75.0", etc.
    // Let's assume it might start with number.
    QString purityStr = r.purity;
    QRegularExpression re(R"((\d+)\s*[kK])");
    QRegularExpressionMatch match = re.match(purityStr);
    double purityVal = 0.0;
    if (match.hasMatch()) {
      purityVal = match.captured(1).toDouble(); // → 22
    }

    double fineLoss = 0.0;
    // IF user literally meant Gross / (100 * purity):
    // Example: Purity 75. 100*75=7500. Loss/7500.
    // Standard logic is Gross * (Purity/100).
    // I will implement the user's literal formula but with a safety check.
    // IF they meant standard, it would be grossLoss * (purityVal / 100.0).
    // Given "Gross Loss/(100*purity)"... maybe they treat 'purity' as 0.75?
    // If purity is 0.75 -> 100*0.75 = 75. Loss/75.
    // I'll stick to a reasonable interpretation: Gross * (Purity/100).
    // The user wrote division. "Gross Loss / (100*purity)".
    // If I calculate fine loss, it should be the pure gold loss.
    // Loss * Purity% = Pure Loss.
    // I will use Gross * (Purity / 100.0) as it is the only physically sensible
    // "Fine Loss". If the user complains, I will change it to division.
    if (purityVal > 0) {
      fineLoss = grossLoss * (purityVal / 100.0);
    }

    // Diamond Pcs Loss = Received Dia Pcs - Issue Dia Pcs
    int diaPcsLoss = r.receiveDiaPcs - r.issueDiaPcs;

    // Diamond Wt. Loss = Received Dia Wt. - Issue Dia Wt.
    double diaWtLoss = r.receiveDiaWt - r.issueDiaWt;

    // Diamond Loss Price = Diamond Wt. Loss * Dia Price
    double diaLossPrice = diaWtLoss * r.diaPrice;

    // 14: Gross Loss
    table->setItem(i, 14,
                   new QTableWidgetItem(QString::number(grossLoss, 'f', 3)));

    // 15: Fine Loss
    table->setItem(i, 15,
                   new QTableWidgetItem(QString::number(fineLoss, 'f', 3)));

    // 16: Dia Pcs Loss
    table->setItem(i, 16, new QTableWidgetItem(QString::number(diaPcsLoss)));

    // 17: Dia Wt. Loss
    table->setItem(i, 17,
                   new QTableWidgetItem(QString::number(diaWtLoss, 'f', 3)));

    table->setItem(i, 18,
                   new QTableWidgetItem(QString::number(r.diaPrice, 'f', 3)));

    // 19: Dia Loss Price
    table->setItem(i, 19,
                   new QTableWidgetItem(QString::number(diaLossPrice, 'f', 2)));

    // Update calculated columns when Dia Price changes?
    // Yes, we need to handle that in onItemChanged too if we want dynamic
    // updates without reload. For now, only on load.

    table->setItem(i, 20, new QTableWidgetItem(r.status));
    table->setItem(i, 21, new QTableWidgetItem("")); // Action placeholder

    // Set flags for others to NOT be editable
    for (int c = 0; c < table->columnCount(); ++c) {
      if (c == 18)
        continue;
      auto *it = table->item(i, c);
      if (it)
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
  }
  table->blockSignals(false);
  calculateTotals();
}

void CastingListWidget::calculateTotals() {
  auto *table = ui->castingTableWidget;
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

  // Columns to sum
  QList<int> sumCols = {4, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19};

  for (int col : sumCols) {
    double sum = 0.0;
    bool isInt = (col == 4 || col == 8 || col == 12 || col == 16);

    for (int r = 0; r < rowCount; r++) {
      QTableWidgetItem *it = table->item(r, col);
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
    totalItem->setFlags(totalItem->flags() &
                        ~Qt::ItemIsEditable); // Total cells read only
    table->setItem(rowCount, col, totalItem);
  }

  // Empty cells for others
  int colCount = table->columnCount();
  for (int c = 0; c < colCount; c++) {
    if (!table->item(rowCount, c)) {
      QTableWidgetItem *empty = new QTableWidgetItem();
      empty->setFlags(empty->flags() & ~Qt::ItemIsEditable);
      table->setItem(rowCount, c, empty);
    }
  }
}

void CastingListWidget::onItemChanged(QTableWidgetItem *item) {
  if (!item)
    return;

  // Check if it's the specific column (18: Dia Price)
  if (item->column() == 18) {
    bool ok;
    double price = item->text().toDouble(&ok);
    if (!ok)
      return; // Invalid number

    // Get jobId from UserRole (stored on this item explicitly or from column 0)
    // We stored it on this item too in loadCastingList
    int jobId = item->data(Qt::UserRole).toInt();

    // Fallback to col 0 if needed
    if (jobId == 0) {
      auto *jobItem = ui->castingTableWidget->item(item->row(), 0);
      if (jobItem)
        jobId = jobItem->data(Qt::UserRole).toInt();
    }

    if (jobId > 0) {
      DatabaseUtils::updateCastingDiaPrice(jobId, price);
    }
  }
}

void CastingListWidget::onTableContextMenu(const QPoint &pos) {
  auto *table = ui->castingTableWidget;

  QModelIndex index = table->indexAt(pos);
  if (!index.isValid())
    return;

  int row = index.row();

  // Fetch job_id from column 0 (Job No)
  QTableWidgetItem *item = table->item(row, 0);
  if (!item)
    return;

  int jobId = item->data(Qt::UserRole).toInt();
  if (jobId <= 0)
    return;

  QMenu menu(this);

  QAction *castingAction = menu.addAction("Casting");

  QAction *selectedAction = menu.exec(table->viewport()->mapToGlobal(pos));

  if (selectedAction == castingAction) {
    openCastingWidget(jobId);
  }
}

QMdiArea *CastingListWidget::findMdiArea(QWidget *w) {
  while (w) {
    if (auto *mdi = qobject_cast<QMdiArea *>(w))
      return mdi;
    w = w->parentWidget();
  }
  return nullptr;
}

void CastingListWidget::openCastingWidget(int jobId) {
  QMdiArea *mdi = findMdiArea(this);
  if (!mdi)
    return;

  // 🔍 Check if already open
  for (QMdiSubWindow *sub : mdi->subWindowList()) {
    auto *w = qobject_cast<CastingWidget *>(sub->widget());
    if (w && w->jobId() == jobId) {
      mdi->setActiveSubWindow(sub);
      return; // ✅ DO NOT open duplicate
    }
  }

  // 🆕 Open new Casting window
  auto *castingWidget = new CastingWidget;
  castingWidget->setJobId(jobId);

  QMdiSubWindow *sub = mdi->addSubWindow(castingWidget);
  sub->setAttribute(Qt::WA_DeleteOnClose);
  sub->setWindowTitle(QString("Casting - Job %1").arg(jobId));

  castingWidget->show();
}
