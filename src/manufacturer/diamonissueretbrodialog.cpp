#include "diamonissueretbrodialog.h"
#include "database/databaseutils.h"
#include "ui_diamonissueretbro.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

DiamonIssueRetBroDialog::DiamonIssueRetBroDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::DiamonIssueRetBroDialog) {
  ui->setupUi(this);

  ui->diamondRadioButton->setChecked(true);
  ui->sizeComboBox->setEnabled(true);

  connect(ui->diamondRadioButton, &QRadioButton::toggled, this,
          &DiamonIssueRetBroDialog::onRadioChanged);
  connect(ui->stoneRadioButton, &QRadioButton::toggled, this,
          &DiamonIssueRetBroDialog::onRadioChanged);
  connect(ui->otherRadioButton, &QRadioButton::toggled, this,
          &DiamonIssueRetBroDialog::onRadioChanged);
  connect(ui->typeComboBox, &QComboBox::currentTextChanged, this,
          &DiamonIssueRetBroDialog::onTypeChanged);
  connect(ui->addPushButton, &QPushButton::clicked, this,
          &DiamonIssueRetBroDialog::onSaveClicked);

  ui->historyTableWidget->setColumnCount(4);
  ui->historyTableWidget->setHorizontalHeaderLabels(
      {"Type", "Size", "Pcs", "Weight"});
  ui->historyTableWidget->horizontalHeader()->setStretchLastSection(true);
  ui->historyTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->historyTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

  loadTypeOptions(); // initial
  loadHistoryForCurrentContext();
}

DiamonIssueRetBroDialog::~DiamonIssueRetBroDialog() { delete ui; }

void DiamonIssueRetBroDialog::hideEvent(QHideEvent *event) {
  qApp->removeEventFilter(this);
  ui->stackedWidget->setCurrentIndex(0);
  // loadHistory();
  ui->sizeComboBox->clear();
  ui->typeComboBox->clear();
  ui->quantityLineEdit->clear();
  ui->weightLineEdit->clear();
  // emit menuHidden();
  QDialog::hideEvent(event);
}

void DiamonIssueRetBroDialog::setContext(int row, int col,
                                         const QString &jobNo) {
  currentRow = row;
  currentCol = col;
  currentJobNo = jobNo;

  // Determine mode based on column
  if (col == 1 || col == 2)
    currentMode = "issue";
  else if (col == 3 || col == 4)
    currentMode = "return";
  else if (col == 5 || col == 6)
    currentMode = "broken";
  else
    currentMode.clear();

  // Set radio button automatically
  if (row == 0)
    ui->diamondRadioButton->setChecked(true);
  else if (row == 1)
    ui->stoneRadioButton->setChecked(true);
  else
    ui->otherRadioButton->setChecked(true);

  // Load correct JSON data into table
  loadHistoryForCurrentContext();
}

void DiamonIssueRetBroDialog::onRadioChanged() {
  ui->typeComboBox->clear();
  ui->sizeComboBox->clear();

  if (ui->diamondRadioButton->isChecked()) {
    ui->sizeComboBox->setEnabled(true);
  } else if (ui->stoneRadioButton->isChecked()) {
    ui->sizeComboBox->setEnabled(true);
  } else { // Other
    ui->typeComboBox->setEditable(true);
    ui->sizeComboBox->setEnabled(false);
  }

  loadTypeOptions();
}

void DiamonIssueRetBroDialog::loadTypeOptions() {
  QStringList types;

  if (ui->diamondRadioButton->isChecked()) {
    types = DatabaseUtils::fetchShapes("diamond");
  } else if (ui->stoneRadioButton->isChecked()) {
    types = DatabaseUtils::fetchShapes("stone");
  } else {
    // Other: editable text
    ui->typeComboBox->setEditable(true);
  }
  // qDebug() << types;
  ui->typeComboBox->addItems(types);
}

void DiamonIssueRetBroDialog::onTypeChanged() {
  ui->sizeComboBox->clear();

  QString type = ui->typeComboBox->currentText();
  if (type.isEmpty())
    return;

  QStringList sizes;
  if (ui->diamondRadioButton->isChecked()) {
    sizes = DatabaseUtils::fetchSizes("diamond", type);
  } else if (ui->stoneRadioButton->isChecked()) {
    sizes = DatabaseUtils::fetchSizes("stone", type);
  } else {
    ui->sizeComboBox->setEnabled(false);
  }

  ui->sizeComboBox->addItems(sizes);
}

void DiamonIssueRetBroDialog::onSaveClicked() {
  QString type = ui->typeComboBox->currentText().trimmed();
  QString size = ui->sizeComboBox->currentText().trimmed();

  // Get input values
  QString qtyStr = ui->quantityLineEdit->text().trimmed();
  QString wtStr = ui->weightLineEdit->text().trimmed();

  // Validate quantity
  bool okQty;
  int qty = qtyStr.toInt(&okQty);
  if (!okQty || qty <= 0) {
    QMessageBox::warning(
        this, "Invalid Quantity",
        "Please enter a valid integer quantity greater than 0.");
    return;
  }

  // Validate weight
  bool okWt;
  double wt = wtStr.toDouble(&okWt);
  if (!okWt || wt <= 0.0) {
    QMessageBox::warning(this, "Invalid Weight",
                         "Please enter a valid weight greater than 0.");
    return;
  }

  // Round/format weight
  QString formattedWt = QString::number(wt, 'f', 3); // always 3 decimals

  // Determine category
  QString category;
  if (ui->diamondRadioButton->isChecked())
    category = "diamond";
  else if (ui->stoneRadioButton->isChecked())
    category = "stone";
  else
    category = "other";

  // Build JSON entry
  QJsonObject entry;
  entry["type"] = type;
  entry["size"] = size;
  entry["category"] = category;
  entry["pcs"] = qty;
  entry["wt"] = formattedWt;

  // Save JSON into database
  saveToDatabase(entry);

  // Emit signal to update main table
  QVariantMap vals;
  vals["issue_pcs"] = QString::number(qty);
  vals["issue_wt"] = formattedWt;

  emit valuesUpdated(currentRow, vals);
  emit menuHidden();
  close();
}

void DiamonIssueRetBroDialog::saveToDatabase(const QJsonObject &entry) {
  // QString colName;

  QString baseName;

  // 🧭 Decide base column using radio button, not row
  if (ui->diamondRadioButton->isChecked())
    baseName = "diamond";
  else if (ui->stoneRadioButton->isChecked())
    baseName = "stone";
  else if (ui->otherRadioButton->isChecked())
    baseName = "other";
  else
    return; // no valid selection

  // 🧭 Decide suffix based on mode (issue / return / broken)
  QString colName;
  if (currentMode == "issue")
    colName = baseName + "_issue";
  else if (currentMode == "return")
    colName = baseName + "_return";
  else if (currentMode == "broken")
    colName = baseName + "_broken"; // optional, if you add broken later
  else
    return;

  if (DatabaseUtils::addJobSheetHistoryEntry(currentJobNo, colName, entry)) {
    qDebug() << "✅ Updated" << colName;
  } else {
    qDebug() << "❌ Update failed for" << colName;
  }
}

// void DiamonIssueRetBroDialog::onCancelClicked()
// {
//     emit menuHidden();
//     close();
// }

void DiamonIssueRetBroDialog::on_pushButton_clicked() {
  ui->stackedWidget->setCurrentIndex(1);
}

void DiamonIssueRetBroDialog::loadHistoryForCurrentContext() {
  if (currentJobNo.isEmpty())
    return;

  // 🧭 Step 1: Determine base column from radio buttons
  QString baseName;
  if (ui->diamondRadioButton->isChecked())
    baseName = "diamond";
  else if (ui->stoneRadioButton->isChecked())
    baseName = "stone";
  else if (ui->otherRadioButton->isChecked())
    baseName = "other";
  else
    return;

  // 🧭 Step 2: Determine suffix based on mode
  QString colName;
  if (currentMode == "issue")
    colName = baseName + "_issue";
  else if (currentMode == "return")
    colName = baseName + "_return";
  else if (currentMode == "broken")
    colName = baseName + "_broken";
  else
    return;

  // 🧭 Step 3: Fetch from DB using DatabaseUtils
  QJsonArray arr = DatabaseUtils::fetchJobSheetHistory(currentJobNo, colName);

  // 🧭 Step 4: Clear and show history
  ui->historyTableWidget->setRowCount(0);
  // if (arr.isEmpty()) return; // Don't return if empty, just clear table is
  // fine

  int row = 0;
  for (const QJsonValue &val : arr) {
    if (!val.isObject())
      continue;
    QJsonObject obj = val.toObject();

    ui->historyTableWidget->insertRow(row);
    ui->historyTableWidget->setItem(
        row, 0, new QTableWidgetItem(obj["type"].toString()));
    ui->historyTableWidget->setItem(
        row, 1, new QTableWidgetItem(obj["size"].toString()));

    QString pcsStr = obj["pcs"].isString()
                         ? obj["pcs"].toString()
                         : QString::number(obj["pcs"].toInt());
    QString wtStr = obj["wt"].isString()
                        ? obj["wt"].toString()
                        : QString::number(obj["wt"].toDouble(), 'f', 3);

    ui->historyTableWidget->setItem(row, 2, new QTableWidgetItem(pcsStr));
    ui->historyTableWidget->setItem(row, 3, new QTableWidgetItem(wtStr));
    row++;
  }
}
