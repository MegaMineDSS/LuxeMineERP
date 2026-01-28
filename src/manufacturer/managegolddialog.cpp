#include "managegolddialog.h"
#include "ui_managegold.h"

#include "database/databaseutils.h"
#include <QDateTime>
#include <QDir>
#include <QDoubleValidator>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSqlError>

ManageGoldDialog::ManageGoldDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ManageGoldDialog) {
  ui->setupUi(this);
  ui->stackedWidget->setCurrentIndex(0);
  // ui->typeComboBox->addItems({"gold", "solder"});

  setStyleSheet("QDialog { background-color: #84bbe8; }");

  QDoubleValidator *validator = new QDoubleValidator(0.0, 999999.999, 3, this);
  validator->setNotation(QDoubleValidator::StandardNotation);
  ui->weightLineEdit->setValidator(validator);

  connect(ui->weightLineEdit, &QLineEdit::editingFinished, this, [this]() {
    QString text = ui->weightLineEdit->text().trimmed();
    if (!text.isEmpty()) {
      double value = text.toDouble();
      ui->weightLineEdit->setText(QString::number(value, 'f', 3));
    }
  });

  loadHistory();
}

ManageGoldDialog::~ManageGoldDialog() { delete ui; }

void ManageGoldDialog::setMode(Mode mode) {
  currentMode = mode;

  if (mode == Filling)
    ui->pushButton->setText("Filling Gold");
  else if (mode == Returning)
    ui->pushButton->setText("Returning Gold");
  else if (mode == Dust)
    ui->pushButton->setText("Add Dust Weight");

  // ✅ Limit combo box for Dust
  if (mode == Dust) {
    ui->typeComboBox->clear();
    ui->typeComboBox->addItem("Dust");
  } else {
    ui->typeComboBox->clear();
    ui->typeComboBox->addItems({"gold", "solder"});
  }

  loadHistory();
}

bool ManageGoldDialog::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

    // If the click is outside the dialog geometry
    if (!this->geometry().contains(mouseEvent->globalPosition().toPoint())) {
      this->hide();
      return true; // consume the event
    }
  }
  return QDialog::eventFilter(watched, event);
}

void ManageGoldDialog::hideEvent(QHideEvent *event) {
  qApp->removeEventFilter(this);
  ui->stackedWidget->setCurrentIndex(0);
  loadHistory();
  emit menuHidden();
  QDialog::hideEvent(event);
}

void ManageGoldDialog::on_pushButton_clicked() {
  ui->stackedWidget->setCurrentIndex(1);
}

void ManageGoldDialog::on_issueAddPushButton_clicked() {
  QString weight = ui->weightLineEdit->text().trimmed();
  if (weight.isEmpty()) {
    QMessageBox::warning(this, "Empty Field", "Enter weight");
    return;
  }

  QString type = ui->typeComboBox->currentText().trimmed();
  QString dateTime =
      QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss");

  // Build JSON object
  QJsonObject newEntry;
  newEntry["type"] = type;
  newEntry["weight"] = weight;
  newEntry["date_time"] = dateTime;

  QString jobNo;
  if (parentWidget()) {
    QLineEdit *jobNoLineEdit =
        parentWidget()->findChild<QLineEdit *>("jobNoLineEdit");
    if (jobNoLineEdit)
      jobNo = jobNoLineEdit->text().trimmed();
  }

  if (jobNo.isEmpty()) {
    QMessageBox::warning(this, "Missing Data", "Job No not found.");
    return;
  }

  QString columnName;
  if (currentMode == Filling)
    columnName = "Filling_Issue";
  else if (currentMode == Returning)
    columnName = "Filling_Return";
  else
    columnName = "filling_dust"; // ✅ new column

  if (DatabaseUtils::addJobSheetHistoryEntry(jobNo, columnName, newEntry)) {
    QMessageBox::information(this, "Success", "Data saved successfully.");
    loadHistory();
  } else {
    QMessageBox::critical(this, "Update Error",
                          "Failed to save data. Check logs.");
  }
}

void ManageGoldDialog::loadHistory() {
  QString jobNo;
  if (parentWidget()) {
    QLineEdit *jobNoLineEdit =
        parentWidget()->findChild<QLineEdit *>("jobNoLineEdit");
    if (jobNoLineEdit)
      jobNo = jobNoLineEdit->text().trimmed();
  }

  QString columnName;
  if (currentMode == Filling)
    columnName = "Filling_Issue";
  else if (currentMode == Returning)
    columnName = "Filling_Return";
  else
    columnName = "filling_dust";

  double totalWeight = 0.0;

  ui->fillingIssueTableWidget->clear();
  ui->fillingIssueTableWidget->setRowCount(0);
  ui->fillingIssueTableWidget->setColumnCount(3);
  ui->fillingIssueTableWidget->setHorizontalHeaderLabels(
      {"Type", "Weight", "Date Time"});

  if (!jobNo.isEmpty()) {
    QJsonArray arr = DatabaseUtils::fetchJobSheetHistory(jobNo, columnName);

    // Update table
    ui->fillingIssueTableWidget->setRowCount(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
      QJsonObject obj = arr[i].toObject();
      ui->fillingIssueTableWidget->setItem(
          i, 0, new QTableWidgetItem(obj["type"].toString()));
      ui->fillingIssueTableWidget->setItem(
          i, 1, new QTableWidgetItem(obj["weight"].toString()));
      ui->fillingIssueTableWidget->setItem(
          i, 2, new QTableWidgetItem(obj["date_time"].toString()));
      totalWeight += obj["weight"].toString().toDouble();
    }
  }

  if (ui->fillingIssueTableWidget->rowCount() == 0) {
    ui->fillingIssueTableWidget->setRowCount(1);
    ui->fillingIssueTableWidget->setItem(
        0, 0, new QTableWidgetItem("No history found"));
  }

  emit totalWeightCalculated(totalWeight);
}
