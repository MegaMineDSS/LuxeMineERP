#include "stockwidget.h"
#include "database/databaseutils.h"
#include "ui_stock.h"
#include <QDate>
#include <QMessageBox>

StockWidget::StockWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::StockWidget) {
  ui->setupUi(this);

  ui->dateEdit->setDate(QDate::currentDate());

  connect(ui->spinPurity, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &StockWidget::calculateValues);
  connect(ui->spinWeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &StockWidget::calculateValues);
  connect(ui->spinPrice, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &StockWidget::calculateValues);
}

StockWidget::~StockWidget() { delete ui; }

void StockWidget::calculateValues() {
  double purity = ui->spinPurity->value();
  double weight = ui->spinWeight->value();
  double price = ui->spinPrice->value();

  // Assumption: Purity is percentage (e.g. 99.50)
  // 24K = Weight * (Purity / 100.0)
  // User might enter 0.995, but usually UI spinbox steps suggest % or direct
  // scale. If user enters 99.50, then /100. If 0.995, then *1. Let's assume %
  // for now as it's common in India/Gold trade (e.g. 91.6)

  double w24k = 0.0;
  if (purity > 1.0) {
    w24k = weight * (purity / 100.0);
  } else {
    w24k = weight * purity;
  }

  // Amount calculation based on 24K weight as per user request
  // Formula: Amount = 24K * Price
  double amount = w24k * price;

  ui->le24k->setText(QString::number(w24k, 'f', 3));
  ui->leAmount->setText(QString::number(amount, 'f', 2));
}

void StockWidget::setStockData(const StockData &data) {
  m_stockId = data.id;
  ui->dateEdit->setDate(QDate::fromString(data.date, "yyyy-MM-dd"));
  ui->comboMetal->setCurrentText(data.metal);
  ui->leDetail->setText(data.detail);
  ui->leNote->setText(data.note);
  ui->leVoucher->setText(data.voucherNo);
  ui->spinPurity->setValue(data.purity);
  ui->spinWeight->setValue(data.weight);
  ui->spinPrice->setValue(data.price);

  calculateValues(); // Refresh calcs

  setWindowTitle("Edit Stock");
}

void StockWidget::on_btnSave_clicked() {
  if (ui->leDetail->text().isEmpty()) {
    QMessageBox::warning(this, "Validation", "Detail is required.");
    return;
  }

  StockData data;
  data.date = ui->dateEdit->text();
  data.metal = ui->comboMetal->currentText();
  data.detail = ui->leDetail->text();
  data.note = ui->leNote->text();
  data.voucherNo = ui->leVoucher->text();
  data.purity = ui->spinPurity->value();
  data.weight = ui->spinWeight->value();
  data.weight24k = ui->le24k->text().toDouble();
  data.price = ui->spinPrice->value();
  data.amount = ui->leAmount->text().toDouble();

  bool success = false;
  if (m_stockId == -1) {
    success = DatabaseUtils::addStock(data);
  } else {
    data.id = m_stockId;
    success = DatabaseUtils::updateStock(data);
  }

  if (success) {
    QMessageBox::information(this, "Success", "Stock saved successfully.");
    close();
  } else {
    QMessageBox::critical(this, "Error", "Failed to save stock.");
  }
}

void StockWidget::on_btnCancel_clicked() { close(); }
