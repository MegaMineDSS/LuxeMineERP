#include "castingwidget.h"
#include "ui_casting.h"

#include "common/sessionmanager.h"
#include "database/databaseutils.h"
#include <QMdiSubWindow>
#include <QMessageBox>


CastingWidget::CastingWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CastingWidget)
{
    ui->setupUi(this);

    setupMetalComboBoxes();
}

CastingWidget::~CastingWidget() { delete ui; }

int CastingWidget::jobId() const { return m_jobId; }

void CastingWidget::setupMetalComboBoxes() {
    ui->metalNameComboBox->addItem("-");
    ui->metalNameComboBox->addItems({"Gold", "Silver"}); //, "Copper", "Palladium", "Titanium", "Tungsten", "Brass"});

    QMap<QString, QStringList> purityMap;
    purityMap["Gold"] = { "24K (99.9%)", "22K (91.6%)", "20K (83.3%)", "18K (75%)", "14K (58.5%)", "10K (41.7%)", "9K (37.5%)" };
    purityMap["Silver"] = { "Fine Silver (99.9%)", "Sterling Silver (92.5%)", "Coin Silver (90%)" };


    connect(ui->metalNameComboBox, &QComboBox::currentTextChanged, this, [=](const QString &metal) {
        ui->metalPurityComboBox->clear();
        if (purityMap.contains(metal)) {
            ui->metalPurityComboBox->addItem("-");
            ui->metalPurityComboBox->addItems(purityMap.value(metal));
        } else {
            ui->metalPurityComboBox->addItem("-");
        }
    });

    ui->metalNameComboBox->setCurrentIndex(0);
    ui->metalPurityComboBox->addItem("-");
}

void CastingWidget::setJobId(int jobId)
{
    m_jobId = jobId;
    resetForm();   //  IMPORTANT

    QString jobNo = "JOB" + QString("%1").arg(jobId, 7, 10, QChar('0'));
    ui->jobNoLabel->setText(jobNo);

    ui->issueDateEdit->setDate(QDate::currentDate());
    ui->issueDateEdit->setCalendarPopup(true);

    // CastingData c;
    CastingData c;
    if (DatabaseUtils::getCastingDataByJob(jobId, c)) {
        // Found! Populate UI
        ui->issueDateEdit->setDate(QDate::fromString(c.castingDate, "yyyy-MM-dd"));
        ui->nameLineEdit->setText(c.castingName);
        ui->pcsSpinBox->setValue(c.pcs);

        ui->metalNameComboBox->setCurrentText(c.issueMetalName);
        ui->metalPurityComboBox->setCurrentText(c.issueMetalPurity);
        ui->issueMetalWtDoubleSpinBox->setValue(c.issueMetalWt);
        ui->issueDiaPcsSpinBox->setValue(c.issueDiamondPcs);
        ui->issueDiaWtDoubleSpinBox->setValue(c.issueDiamondWt);

        ui->ranarWtDoubleSpinBox->setValue(c.receiveRunnerWt);
        ui->productWtDoubleSpinBox->setValue(c.receiveProductWt);
        ui->diaPcsSpinBox->setValue(c.receiveDiamondPcs);
        ui->diaWtDoubleSpinBox->setValue(c.receiveDiamondWt);
    } else {
        // Reset or load default if needed (e.g. from Order)
        // For now, keep as is (constructor defaults or empty)
        // Maybe clear certain fields if re-used for different job?
        // ui->nameLineEdit->clear();
        // ...
    }
}


void CastingWidget::resetForm()
{
    ui->nameLineEdit->clear();
    ui->pcsSpinBox->setValue(0);

    ui->metalNameComboBox->setCurrentIndex(-1);
    ui->metalPurityComboBox->setCurrentIndex(-1);

    ui->issueMetalWtDoubleSpinBox->setValue(0);
    ui->issueDiaPcsSpinBox->setValue(0);
    ui->issueDiaWtDoubleSpinBox->setValue(0);

    ui->ranarWtDoubleSpinBox->setValue(0);
    ui->productWtDoubleSpinBox->setValue(0);
    ui->diaPcsSpinBox->setValue(0);
    ui->diaWtDoubleSpinBox->setValue(0);
}

CastingData CastingWidget::collectCastingData() const {
  CastingData c;

  c.jobId = m_jobId;

  // ----- Detail -----
  c.castingDate = ui->issueDateEdit->date().toString("yyyy-MM-dd");
  c.castingName = ui->nameLineEdit->text().trimmed();
  c.pcs = ui->pcsSpinBox->value();

  // ----- Issue -----
  c.issueMetalName = ui->metalNameComboBox->currentText();
  c.issueMetalPurity = ui->metalPurityComboBox->currentText();
  c.issueMetalWt = ui->issueMetalWtDoubleSpinBox->value();
  c.issueDiamondPcs = ui->issueDiaPcsSpinBox->value();
  c.issueDiamondWt = ui->issueDiaWtDoubleSpinBox->value();

  // ----- Receive -----
  c.receiveRunnerWt = ui->ranarWtDoubleSpinBox->value();
  c.receiveProductWt = ui->productWtDoubleSpinBox->value();
  c.receiveDiamondPcs = ui->diaPcsSpinBox->value();
  c.receiveDiamondWt = ui->diaWtDoubleSpinBox->value();

  // ----- Meta -----
  c.accountantId = SessionManager::currentUser().id;
  c.status = "OPEN";

  return c;
}

void CastingWidget::on_savePushButton_clicked() {
  if (m_jobId <= 0) {
    QMessageBox::warning(this, "Error", "Invalid Job");
    return;
  }

  CastingData data = collectCastingData();

  int castingId = DatabaseUtils::getCastingIdByJob(m_jobId);

  bool ok = false;
  if (castingId > 0) {
    ok = DatabaseUtils::updateCasting(castingId, data);
  } else {
    ok = DatabaseUtils::insertCasting(data);
  }

  if (!ok) {
    QMessageBox::critical(this, "Error", "Failed to save casting");
    return;
  }

  QMessageBox::information(this, "Success", "Casting saved successfully");

  if (auto *sub = qobject_cast<QMdiSubWindow *>(parentWidget()))
    sub->close();
}
