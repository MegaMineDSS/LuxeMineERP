#include "orderdialog.h"
#include "ui_order.h"

#include "database/DatabaseUtils.h"
#include "common/sessionmanager.h"

#include <QMessageBox>
#include <QDate>
#include <QStandardItemModel>

OrderDialog::OrderDialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::OrderDialog)
{
    ui->setupUi(this);

    prefillData();

}

OrderDialog::~OrderDialog()
{
    delete ui;
}

void OrderDialog::prefillData()
{
    // Seller info (from session)
    ui->sellerNameLineEdit->setText(
        SessionManager::currentUser().username
        );
    // ui->sellerIdLineEdit->setText(
    //     SessionManager::currentUser().userId
    //     );

    // Order date (today)
    ui->orderDateDateEdit->setDisplayFormat("yyyy-MM-dd");
    ui->orderDateDateEdit->setDate(QDate::currentDate());

    // Lock fields
    ui->sellerNameLineEdit->setReadOnly(true);
    ui->sellerIdLineEdit->setReadOnly(true);
    ui->orderDateDateEdit->setReadOnly(true);
}

bool OrderDialog::validateInput() const
{
    if (ui->jobNoLineEdit->text().isEmpty())
        return false;

    if (ui->partyNameLineEdit->text().isEmpty())
        return false;

    if (ui->deliveryDateDateEdit->date() < QDate::currentDate())
        return false;

    return true;
}

void OrderDialog::fillOrderData(OrderData &order)
{
    // --------------------
    // Identity
    // --------------------
    // order.id = dummyOrderId;

    // --------------------
    // Seller (from session – DO NOT trust UI for this)
    // --------------------
    // order.sellerId   = SessionManager::currentUser().userId;
    order.sellerName = SessionManager::currentUser().username;

    // --------------------
    // Party & Client Hierarchy
    // --------------------
    order.partyId   = ui->partyIdLineEdit->text().trimmed();
    order.partyName = ui->partyNameLineEdit->text().trimmed();

    order.clientId    = ui->clientIdLineEdit->text().trimmed();
    order.agencyId    = ui->agencyIdLineEdit->text().trimmed();
    order.shopId      = ui->shopIdLineEdit->text().trimmed();
    order.retaillerId = ui->reteailleIdLineEdit->text().trimmed();
    order.starId      = ui->starIdLineEdit->text().trimmed();

    // --------------------
    // Address
    // --------------------
    order.address = ui->addressLineEdit->text().trimmed();
    order.city    = ui->cityLineEdit->text().trimmed();
    order.state   = ui->stateLineEdit->text().trimmed();
    order.country = ui->countryLineEdit->text().trimmed();

    // --------------------
    // Order Identity
    // --------------------
    order.jobNo   = ui->jobNoLineEdit->text().trimmed();
    order.orderNo = ui->orderNoLineEdit->text().trimmed();

    // --------------------
    // Dates
    // --------------------
    order.orderDate    = QDate::currentDate().toString("yyyy-MM-dd");
    order.deliveryDate = ui->deliveryDateDateEdit->date().toString("yyyy-MM-dd");

    // --------------------
    // Product
    // --------------------
    order.productName     = ui->productNameLineEdit->text().trimmed();
    order.productPis      = ui->productPisSpinBox->value();
    order.approxProductWt = ui->approxWeightDoubleSpinBox->value();
    order.approxDiamondWt = ui->approxDiaWeightDoubleSpinBox->value();

    // --------------------
    // Metal
    // --------------------
    order.metalName   = ui->metalNameComboBox->currentText();
    order.metalPurity = ui->metalPurityComboBox->currentText();
    order.metalColor  = ui->metalColorComboBox->currentText();
    order.metalPrice  = ui->metalPriceDoubleSpinBox->value();

    // --------------------
    // Size & Dimensions
    // --------------------
    order.sizeNo = ui->sizeNoDoubleSpinBox->value();
    order.sizeMM = ui->sizeMMDoubleSpinBox->value();
    order.length = ui->lengthDoubleSpinBox->value();
    order.width  = ui->widthDoubleSpinBox->value();
    order.height = ui->heightDoubleSpinBox->value();

    // --------------------
    // Diamond
    // --------------------
    order.diaPacific = ui->diaPacificLineEdit->text().trimmed();
    order.diaPurity  = ui->diaPurityLineEdit->text().trimmed();
    order.diaColor   = ui->diaColorLineEdit->text().trimmed();
    order.diaPrice   = ui->diaPriceDoubleSpinBox->value();

    // --------------------
    // Stone
    // --------------------
    order.stPacific = ui->stPacificLineEdit->text().trimmed();
    order.stPurity  = ui->stPurityLineEdit->text().trimmed();
    order.stColor   = ui->stColorLineEdit->text().trimmed();
    order.stPrice   = ui->stPriceDoubleSpinBox->value();

    // --------------------
    // Design & Images
    // --------------------
    order.designNo1  = ui->designNoLineEdit->text().trimmed();
    // order.designNo2  = ui->designNoLineEdit2->text().trimmed();
    order.designNo2 = "123";
    QString imagePath1, imagePath2;
    order.image1Path = imagePath1;
    order.image2Path = imagePath2;

    // --------------------
    // Certification (Multi-select)
    // --------------------
    order.metalCertiName = ui->metalCertiNameComboBox->currentText();
    {
        QStringList list;
        auto model = qobject_cast<QStandardItemModel*>(ui->metalCertiTypeComboBox->model());
        for (int i = 0; i < model->rowCount(); ++i)
            if (model->item(i)->checkState() == Qt::Checked)
                list << model->item(i)->text();
        order.metalCertiType = list.join(", ");
    }

    order.diaCertiName = ui->diaCertiNameComboBox->currentText();
    {
        QStringList list;
        auto model = qobject_cast<QStandardItemModel*>(ui->diaCertiTypeComboBox->model());
        for (int i = 0; i < model->rowCount(); ++i)
            if (model->item(i)->checkState() == Qt::Checked)
                list << model->item(i)->text();
        order.diaCertiType = list.join(", ");
    }

    // --------------------
    // Manufacturing Options
    // --------------------
    order.pesSaki       = ui->pesSakiComboBox->currentText();
    order.chainLock     = ui->chainLockComboBox->currentText();
    order.polish        = ui->polishComboBox->currentText();
    order.settingLebour = ui->settingLabourComboBox->currentText();
    order.metalStemp    = ui->metalStempComboBox->currentText();

    // --------------------
    // Payment
    // --------------------
    order.paymentMethod = ui->payMethodComboBox->currentText();
    order.totalAmount   = ui->totalAmountDoubleSpinBox->value();
    order.advance       = ui->advanceDoubleSpinBox->value();
    order.remaining     = order.totalAmount - order.advance;

    // --------------------
    // Notes
    // --------------------
    order.note        = ui->notePlainTextEdit->toPlainText().trimmed();
    order.extraDetail = ui->extraDetailPlainTextEdit->toPlainText().trimmed();

    // --------------------
    // State
    // --------------------
    order.isSaved = 1;

}

void OrderDialog::on_savePushButton_clicked()
{
    if (!validateInput()) {
        QMessageBox::warning(this, "Validation",
                             "Please fill required fields correctly");
        return;
    }

    OrderData o;
    fillOrderData(o);

    if (!DatabaseUtils::createOrder(o)) {
        QMessageBox::critical(this, "Error",
                              "Failed to save order");
        return;
    }

    QMessageBox::information(this, "Success",
                             "Order created successfully");
    accept();
}
