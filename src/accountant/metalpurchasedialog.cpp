#include "metalpurchasedialog.h"
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

MetalPurchaseDialog::MetalPurchaseDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("New Metal Purchase Entry");
  resize(400, 600);
  setupUi();
}

void MetalPurchaseDialog::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QFormLayout *form = new QFormLayout();

  dateEdit = new QDateEdit(QDate::currentDate());
  dateEdit->setCalendarPopup(true);
  dateEdit->setDisplayFormat("yyyy-MM-dd");

  txtBillNo = new QLineEdit();
  txtPartyName = new QLineEdit();

  spinPic = new QSpinBox();
  spinPic->setRange(0, 10000);

  txtProduct = new QLineEdit();

  spinWeight = new QDoubleSpinBox();
  spinWeight->setRange(0, 100000);
  spinWeight->setDecimals(3);

  spinPurity = new QDoubleSpinBox();
  spinPurity->setRange(0, 100);
  spinPurity->setDecimals(2);
  spinPurity->setValue(99.50);

  spinLabour = new QDoubleSpinBox();
  spinLabour->setRange(0, 1000000);
  spinLabour->setDecimals(2);

  spinTotalGold = new QDoubleSpinBox();
  spinTotalGold->setRange(0, 100000);
  spinTotalGold->setDecimals(3);
  spinTotalGold->setReadOnly(false); // Editable but maybe calc later

  spinPayWeight = new QDoubleSpinBox();
  spinPayWeight->setRange(0, 100000);
  spinPayWeight->setDecimals(3);

  spinTotalPayAmount = new QDoubleSpinBox();
  spinTotalPayAmount->setRange(0, 100000000);
  spinTotalPayAmount->setDecimals(2);

  spinCosting = new QDoubleSpinBox();
  spinCosting->setRange(0, 1000000);
  spinCosting->setDecimals(2);

  txtRemark = new QLineEdit();

  form->addRow("Date:", dateEdit);
  form->addRow("Bill No:", txtBillNo);
  form->addRow("Party Name:", txtPartyName);
  form->addRow("PIC:", spinPic);
  form->addRow("Product:", txtProduct);
  form->addRow("Weight:", spinWeight);
  form->addRow("Purity (%):", spinPurity);
  form->addRow("Labour Amount:", spinLabour);
  form->addRow("Total Gold:", spinTotalGold);
  form->addRow("Pay Weight:", spinPayWeight);
  form->addRow("Total Pay Amount:", spinTotalPayAmount);
  form->addRow("Costing Per Gm:", spinCosting);
  form->addRow("Remark:", txtRemark);

  mainLayout->addLayout(form);

  // Buttons
  QDialogButtonBox *btnBox =
      new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
  connect(btnBox, &QDialogButtonBox::accepted, this,
          &MetalPurchaseDialog::save);
  connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(btnBox);

  // Basic calculation connection example (Optional)
  connect(spinLabour, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MetalPurchaseDialog::calculateTotals);
  connect(spinPayWeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MetalPurchaseDialog::calculateTotals);
  connect(spinTotalPayAmount,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &MetalPurchaseDialog::calculateTotals);
}

void MetalPurchaseDialog::calculateTotals() {
  double weight = spinWeight->value();
  double purity = spinPurity->value();
  double labour = spinLabour->value();
  double payWeight = spinPayWeight->value();
  double totalPayAmount = spinTotalPayAmount->value();

  // Formula 1: Total Gold
  // User Formula: =SUM(K3)-(F3/100*G3)+(F3/100*H3)
  // Helper: PayWeight - (Weight/100 * Purity) + (Weight/100 * Labour)
  double term1 = (weight / 100.0) * purity;
  double term2 = (weight / 100.0) * labour;
  double totalGold = payWeight - term1 + term2;
  spinTotalGold->setValue(totalGold);

  // Formula 2: Costing Per Gm
  // User Formula: =SUM(M3/F3*1) => TotalPayAmount / Weight
  if (weight != 0.0) {
    double costing = totalPayAmount / weight;
    spinCosting->setValue(costing);
  } else {
    spinCosting->setValue(0.0);
  }
}

void MetalPurchaseDialog::save() {
  if (txtPartyName->text().isEmpty()) {
    QMessageBox::warning(this, "Validation", "Party Name is required.");
    return;
  }

  MetalPurchaseData d;
  d.entryDate = dateEdit->text();
  d.billNo = txtBillNo->text();
  d.partyName = txtPartyName->text();
  d.pic = spinPic->value();
  d.productName = txtProduct->text();
  d.weight = spinWeight->value();
  d.purity = spinPurity->value();
  d.labourAmount = spinLabour->value();
  d.totalGold = spinTotalGold->value();
  d.payWeight = spinPayWeight->value();
  d.totalPayAmount = spinTotalPayAmount->value();
  d.costingPerGm = spinCosting->value();
  d.remark = txtRemark->text();

  if (DatabaseUtils::addMetalPurchase(d)) {
    QMessageBox::information(this, "Success", "Entry saved successfully.");
    accept();
  } else {
    QMessageBox::critical(this, "Error", "Failed to save entry to database.");
  }
}
