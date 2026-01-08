#include "addcatalog.h"
#include "ui_addcatalog.h"


#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QComboBox>
#include <QKeyEvent>
#include <QDialog>

#include "database/databaseutils.h"

AddCatalog::AddCatalog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddCatalog)
    , jewelryMenu(new JewelryMenu(this))
{
    ui->setupUi(this);

    setupGoldTable();
    ui->companyName_lineEdit->setText("SHREE LAXMINARAYAN EXPORT");

    connect(ui->jewelryButton, &QPushButton::clicked, this, [this]() {
        jewelryMenu->getMenu()->popup(ui->jewelryButton->mapToGlobal(QPoint(0, ui->jewelryButton->height())));
    });

    qDebug() << "Executed jewelryMenu->getMenu()->popup(ui->jewelryButton->mapToGlobal(QPoint(0, ui->jewelryButton->height())))" ;

    connect(jewelryMenu, &JewelryMenu::itemSelected, this, &AddCatalog::onJewelryItemSelected);
    qDebug() << "Executed &AddCatalog::onJewelryItemSelected" ;

}

AddCatalog::~AddCatalog()
{
    delete ui;
}

void AddCatalog::on_brows_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Select Image", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (filePath.isEmpty())
        return;

    ui->imagPath_lineEdit->setText(filePath);

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Image Error", "Failed to load the selected image.");
        return;
    }

    int labelWidth = ui->imageView_label_at_addImage->width();
    int labelHeight = ui->imageView_label_at_addImage->height();

    if (labelWidth > 0 && labelHeight > 0) {
        ui->imageView_label_at_addImage->setPixmap(
            pixmap.scaled(labelWidth, labelHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void AddCatalog::calculateGoldWeights(QTableWidgetItem *item)
{
    // Only recalc when 24kt weight is changed (row 0, column 1)
    if (!item || item->column() != 1 || item->row() != 0) return;

    bool ok;
    double weight24kt = item->text().toDouble(&ok);
    if (!ok || weight24kt <= 0) return;

    QList<int> karats = {24, 22, 20, 18, 14, 10};

    if (!ui->goldTable) return; // Safety check

    // Prevent recursive signals when updating table programmatically
    ui->goldTable->blockSignals(true);

    for (int i = 1; i < karats.size(); ++i) {
        // Check that item exists before accessing
        QTableWidgetItem *w = ui->goldTable->item(i, 1);
        if (!w) {
            w = new QTableWidgetItem();
            ui->goldTable->setItem(i, 1, w); // create if missing
        }

        double newWeight = (karats[i] / 24.0) * weight24kt;
        w->setText(QString::number(newWeight, 'f', 3));
    }

    ui->goldTable->blockSignals(false);
}

void AddCatalog::setupGoldTable()
{
    QList<int> karats = {24, 22, 20, 18, 14, 10};
    ui->goldTable->setRowCount(karats.size());
    ui->goldTable->setColumnCount(2); // ensure at least 2 columns

    for (int i = 0; i < karats.size(); ++i) {
        // Column 0: karat (read-only)
        QTableWidgetItem *karatItem = new QTableWidgetItem(QString::number(karats[i]) + "kt");
        karatItem->setFlags(karatItem->flags() & ~Qt::ItemIsEditable);
        ui->goldTable->setItem(i, 0, karatItem);

        // Column 1: editable weight
        ui->goldTable->setItem(i, 1, new QTableWidgetItem());
    }

    // Connect weight editing → triggers only when 24kt weight is modified
    connect(ui->goldTable, &QTableWidget::itemChanged,
            this, &AddCatalog::calculateGoldWeights);
}

void AddCatalog::onJewelryItemSelected(const QString &item)
{
    selectedImageType = item; // e.g., "Ring (Men's Party Wear)"
    ui->jewelryButton->setText(item); // Update the button text to show the selection
}


void AddCatalog::on_jewelryButton_clicked()
{

}


void AddCatalog::on_save_insert_clicked()
{
    QString imagePath = ui->imagPath_lineEdit->text();
    QString designNo = ui->designNO_lineEdit->text();
    QString companyName = ui->companyName_lineEdit->text();
    QString note = ui->note->toPlainText();

    // if (imagePath.isEmpty() || designNo.isEmpty() || selectedImageType.isEmpty() || companyName.isEmpty()) {
    //     QMessageBox::warning(this, "Input Error", "All fields must be filled!");
    //     return;
    // }

    if (designNo.isEmpty()){
        QMessageBox::warning(this, "Input Error", "Design Number must be filled!") ;
    }

    // Build diamond JSON
    QJsonArray diamondArray;
    for (int row = 0; row < ui->diaTable->rowCount(); ++row) {
        QJsonObject rowObject;
        if (auto *combo = qobject_cast<QComboBox*>(ui->diaTable->cellWidget(row, 0))) rowObject["type"] = combo->currentText();
        if (auto *combo = qobject_cast<QComboBox*>(ui->diaTable->cellWidget(row, 1))) rowObject["sizeMM"] = combo->currentText();
        if (auto *item = ui->diaTable->item(row, 2)) rowObject["quantity"] = item->text();
        diamondArray.append(rowObject);
    }

    // Build stone JSON
    QJsonArray stoneArray;
    for (int row = 0; row < ui->stoneTable->rowCount(); ++row) {
        QJsonObject rowObject;
        if (auto *combo = qobject_cast<QComboBox*>(ui->stoneTable->cellWidget(row, 0))) rowObject["type"] = combo->currentText();
        if (auto *combo = qobject_cast<QComboBox*>(ui->stoneTable->cellWidget(row, 1))) rowObject["sizeMM"] = combo->currentText();
        if (auto *item = ui->stoneTable->item(row, 2)) rowObject["quantity"] = item->text();
        stoneArray.append(rowObject);
    }

    // Build gold JSON
    QJsonArray goldArray;
    for (int row = 1; row < ui->goldTable->rowCount(); ++row) {
        QJsonObject rowObject;
        if (auto *item = ui->goldTable->item(row, 0)) rowObject["karat"] = item->text();
        if (auto *item = ui->goldTable->item(row, 1)) rowObject["weight(g)"] = item->text();
        goldArray.append(rowObject);
    }

    // Save image
    QString newImagePath = DatabaseUtils::saveImage(imagePath);
    if (newImagePath.isEmpty()) {
        QMessageBox::warning(this, "File Error", "Failed to save the image!");
        return;
    }

    // Insert DB record
    QString successReturn = DatabaseUtils::insertCatalogData(newImagePath, selectedImageType, designNo,
                                                             companyName, goldArray, diamondArray, stoneArray, note) ;
    if (successReturn == "error") {
        QMessageBox::critical(this, "Insert Error", "Failed to insert data into database!");
        return;
    }else if (successReturn == "insert") {
        QMessageBox::information(this, "Success", "Data inserted successfully!");
    } else if (successReturn == "modify") {
        QMessageBox::information(this, "Success", "Data updated successfully!");
        ui->designNO_lineEdit->setEnabled(true) ;
    } else {
        QMessageBox::warning(this, "Error", "Error");
    }


    // Clear fields safely
    ui->imagPath_lineEdit->clear();
    ui->designNO_lineEdit->clear();
    selectedImageType.clear();
    ui->jewelryButton->setText("select jewelry type");
    ui->imageView_label_at_addImage->clear();

    // Safely clear diamond + stone tables (delete widgets first)
    auto clearTable = [](QTableWidget *table) {
        for (int row = 0; row < table->rowCount(); ++row) {
            for (int col = 0; col < table->columnCount(); ++col) {
                QWidget *w = table->cellWidget(row, col);
                if (w) delete w;
            }
        }
        table->setRowCount(0);
    };
    clearTable(ui->diaTable);
    clearTable(ui->stoneTable);

    // Reset gold weights
    for (int row = 0; row < ui->goldTable->rowCount(); ++row) {
        if (auto *item = ui->goldTable->item(row, 1)) item->setText("");
    }

    ui->note->clear();
}


void AddCatalog::on_bulk_import_button_clicked()
{
    QString excelPath = QFileDialog::getOpenFileName(this, "Select Excel File","","Excel Files (*.xlsx)");
    if(excelPath.isEmpty())
        return ;

    if (DatabaseUtils::excelBulkInsertCatalog(excelPath)){
        QMessageBox::information(this, "Success", "Bulk import completed successfully") ;
    } else {
        QMessageBox::critical(this, "Error", "Bulk import failed!") ;
    }

}

void AddCatalog::on_demo_download_button_clicked()
{
    // 1 Get the path of the demo file
    QString appDir = QCoreApplication::applicationDirPath(); // path of .exe
    QString demoFilePath = appDir + "/excel/demo_catalog.xlsx";

    // 2️ Ask user save path
    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Save Demo Catalog",
        QDir::homePath() + "/demo_catalog.xlsx", // default file name
        "Excel Files (*.xlsx)"
        );

    if (savePath.isEmpty()) {
        return; // user cancelled
    }

    // 3️ Copy demo file to user's selected location
    if (QFile::copy(demoFilePath, savePath)) {
        QMessageBox::information(this, "Success", "Demo catalog downloaded successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Failed to download demo catalog. File may already exist or path is invalid.");
    }
}

void AddCatalog::addTableRow(QTableWidget *table, const QString &tableType)
{
    int newRow = table->rowCount();
    table->insertRow(newRow);

    // parented to table → no leaks if row is removed
    QComboBox *shapeCombo = new QComboBox(table);
    QComboBox *sizeCombo = new QComboBox(table);

    QStringList shapes = DatabaseUtils::fetchShapes(tableType);
    if (shapes.isEmpty()) {
        QMessageBox::critical(this, "Database Error", "Failed to fetch shapes for " + tableType);
        table->removeRow(newRow);
        return;
    }

    shapeCombo->addItems(shapes);

    auto populateSizeCombo = [this, sizeCombo, tableType](const QString &selectedShape) {
        sizeCombo->clear();
        QStringList sizes = DatabaseUtils::fetchSizes(tableType, selectedShape);
        if (sizes.isEmpty()) {
            QMessageBox::critical(this, "Database Error", "Failed to fetch sizes for " + selectedShape);
            return;
        }
        sizeCombo->addItems(sizes);
    };

    populateSizeCombo(shapes.first());
    connect(shapeCombo, &QComboBox::currentTextChanged, populateSizeCombo);

    table->setCellWidget(newRow, 0, shapeCombo);
    table->setCellWidget(newRow, 1, sizeCombo);
}

void AddCatalog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
		qDebug() << "Enter/Return key detected in AddCatalog";
        if (ui->diaTable->hasFocus()) {
            addTableRow(ui->diaTable, "diamond");
        } else if (ui->stoneTable->hasFocus()) {
            addTableRow(ui->stoneTable, "stone");
        }
    } else if (event->key() == Qt::Key_Delete) {
        QTableWidget *focusedTable =
            ui->diaTable->hasFocus() ? ui->diaTable :
                ui->stoneTable->hasFocus() ? ui->stoneTable : nullptr;

        if (focusedTable && focusedTable->currentRow() >= 0) {
            if (QMessageBox::question(this, "Confirm Deletion", "Delete this row?") == QMessageBox::Yes) {
                int row = focusedTable->currentRow();

                // Explicitly delete widgets in the row to prevent leaks
                for (int col = 0; col < focusedTable->columnCount(); ++col) {
                    QWidget *cellWidget = focusedTable->cellWidget(row, col);
                    if (cellWidget) {
                        delete cellWidget;
                    }
                }

                focusedTable->removeRow(row);
            }
        }
    } else {
        AddCatalog::keyPressEvent(event);
    }
}


