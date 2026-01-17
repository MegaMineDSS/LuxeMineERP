#include "modifycatalog.h"
#include "ui_modifycatalog.h"
#include "database/databaseutils.h"

#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QComboBox>
#include <QDebug>
#include <databasemanager.h>

ModifyCatalog::ModifyCatalog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModifyCatalog)
    , stackedWidget(new QStackedWidget(this))
    , gridPage(new QWidget(this))
    , formPage(new QWidget(this))
    , modifyCatalogModel(new QStandardItemModel(this))
    , filterModel(new QSortFilterProxyModel(this))
    , jewelryMenu(new JewelryMenu(this))
{
    // 1. Setup Main Layout
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);

    // 2. Setup Pages
    setupGridPage();
    setupFormPage();

    stackedWidget->addWidget(gridPage);
    stackedWidget->addWidget(formPage);

    stackedWidget->setCurrentWidget(gridPage);

    // 3. Load Initial Data
    loadCatalogGrid();

    this->setStyleSheet(R"( QWidget{background-color:#F9FAFB;color:#2B2B2B;font-family:"Segoe UI","Arial";font-size:14px}QLineEdit{background-color:#FFFFFF;border:1px solid #C5C6C7;border-radius:0px;padding:4px 6px;selection-background-color:#4A90E2}QLineEdit:focus{border:1px solid #4A90E2;background-color:#FDFEFF}QPushButton{background-color:#E7E9EC;border:1px solid #C5C6C7;border-radius:0px;padding:6px 10px;font-weight:500}QPushButton:hover{background-color:#DDE4F2;border:1px solid #4A90E2}QPushButton:pressed{background-color:#C7D8F0;border:1px solid #4A90E2}QStackedWidget{background-color:#FFFFFF;border:1px solid #C5C6C7;border-radius:0px}QListView{background-color:#F9FAFB;border:1px solid #C5C6C7;border-radius:0px;color:#2B2B2B;outline:none;padding:6px}QListView::item{background-color:#FFFFFF;border:1px solid #D4D4D4;border-radius:0px;margin:8px;padding:8px 6px}QListView::item:hover{background-color:#EEF3FA;border:1px solid #9EB9E2;color:#1A1A1A}QListView::item:selected{background-color:#D9E8FC;border:1px solid #4A90E2;color:#000000;font-weight:500}QScrollBar:vertical,QScrollBar:horizontal{background:#F2F2F2;border:none;width:12px;height:12px}QScrollBar::handle:vertical,QScrollBar::handle:horizontal{background:#BDBDBD;border:1px solid #A5A5A5;border-radius:0px}QScrollBar::handle:vertical:hover,QScrollBar::handle:horizontal:hover{background:#9E9E9E}QScrollBar::add-line,QScrollBar::sub-line{background:none;border:none;width:0;height:0})");
}

ModifyCatalog::~ModifyCatalog()
{
    qDebug() << "[ModifyCatalog] Destructor called. Cleaning up resources.";
    if (modifyCatalogModel) {
        modifyCatalogModel->clear();
    }
    delete ui;
}

void ModifyCatalog::setupLayout()
{
    // No-op, handled in constructor
}

void ModifyCatalog::setupGridPage()
{
    qDebug() << "[ModifyCatalog] setupGridPage called";
    auto* layout = new QVBoxLayout(gridPage);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(6);

    searchBar = new QLineEdit(gridPage);
    searchBar->setPlaceholderText("Search by design number...");
    searchBar->setClearButtonEnabled(true);
    searchBar->setFixedHeight(32);

    modifyCatalogView = new QListView(gridPage);
    modifyCatalogView->setViewMode(QListView::IconMode);
    modifyCatalogView->setIconSize(QSize(120, 120));
    modifyCatalogView->setGridSize(QSize(160, 160));
    modifyCatalogView->setResizeMode(QListView::Adjust);
    modifyCatalogView->setUniformItemSizes(true); // Optimize performance
    modifyCatalogView->setSpacing(12);
    modifyCatalogView->setSelectionMode(QAbstractItemView::SingleSelection);
    modifyCatalogView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Disable Drag and Drop
    modifyCatalogView->setMovement(QListView::Static);
    modifyCatalogView->setDragEnabled(false);
    modifyCatalogView->setAcceptDrops(false);
    modifyCatalogView->setDropIndicatorShown(false);

    // <-- important: enable custom context menu signal
    modifyCatalogView->setContextMenuPolicy(Qt::CustomContextMenu);

    filterModel->setSourceModel(modifyCatalogModel);
    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filterModel->setDynamicSortFilter(true);
    filterModel->setFilterRole(Qt::UserRole + 2); // Filter by DesignNo
    modifyCatalogView->setModel(filterModel);

    layout->addWidget(searchBar);
    layout->addWidget(modifyCatalogView);

    // Connections
    connect(searchBar, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterModel->setFilterFixedString(text.trimmed());
    });

    // Robust double-click handler: validate proxy -> source mapping and item existence
    connect(modifyCatalogView, &QListView::doubleClicked, this, [this](const QModelIndex& proxyIndex) {
        if (!proxyIndex.isValid()) {
            qDebug() << "[ModifyCatalog] doubleClicked: invalid proxyIndex";
            return;
        }

        if (!filterModel) {
            qWarning() << "[ModifyCatalog] doubleClicked: filterModel is null";
            return;
        }

        QModelIndex srcIndex = filterModel->mapToSource(proxyIndex);
        if (!srcIndex.isValid()) {
            qDebug() << "[ModifyCatalog] doubleClicked: mapped source index invalid";
            return;
        }

        if (!modifyCatalogModel) {
            qWarning() << "[ModifyCatalog] doubleClicked: modifyCatalogModel is null";
            return;
        }

        QStandardItem* item = modifyCatalogModel->itemFromIndex(srcIndex);
        if (!item) {
            qDebug() << "[ModifyCatalog] doubleClicked: source item is null";
            return;
        }

        QVariant v = item->data(Qt::UserRole + 2);
        QString designNo = v.toString().trimmed();
        qDebug() << "[ModifyCatalog] Double clicked design (safe):" << designNo;

        if (!designNo.isEmpty()) {
            loadDesignForEdit(designNo);
        }
    });

    connect(modifyCatalogView, &QListView::customContextMenuRequested,
        this, &ModifyCatalog::onModifyCatalogContextMenuRightClicked);
}


void ModifyCatalog::setupFormPage()
{
    qDebug() << "[ModifyCatalog] setupFormPage called";
    // Inflate the form UI into formPage
    ui->setupUi(formPage);

    // Hide components not needed for Modify (e.g., Delete/Add buttons if they exist in UI)
    // The copied UI might have buttons from AddCatalog.
    // We expect "Save" (save_insert) and "Cancel".

    // Clean up UI state
    ui->designNO_lineEdit->setEnabled(false); // ID typically readonly in Edit mode

    setupGoldTable();

    // Connections
    connect(ui->save_insert, &QPushButton::clicked, this, &ModifyCatalog::on_save_insert_clicked);
    connect(ui->addCatalog_cancel_button, &QPushButton::clicked, this, &ModifyCatalog::on_addCatalog_cancel_button_clicked);
    connect(ui->brows, &QPushButton::clicked, this, &ModifyCatalog::on_brows_clicked);

    // Jewelry Menu
    connect(ui->jewelryButton, &QPushButton::clicked, this, [this]() {
        jewelryMenu->getMenu()->popup(ui->jewelryButton->mapToGlobal(QPoint(0, ui->jewelryButton->height())));
    });
    connect(jewelryMenu, &JewelryMenu::itemSelected, this, &ModifyCatalog::onJewelryItemSelected);
}

void ModifyCatalog::loadCatalogGrid()
{
    qDebug() << "[ModifyCatalog] loadCatalogGrid called";
    modifyCatalogModel->clear();
    QList<QVariantList> data = DatabaseUtils::fetchCatalogData();
    qDebug() << "[ModifyCatalog] Data received from DB:" << data.size();
    const QSize iconSize(160, 160);

    for (const QVariantList &row : data) {
        QString path = row[0].toString();
        QString designNo = row[1].toString();
        QString company = row[2].toString();

        // qDebug() << "Loading item:" << designNo << path;

        QString fullPath = QFile::exists(path) ? path : QDir(qApp->applicationDirPath()).filePath(path);
        if (!QFile::exists(fullPath)) fullPath = ":/icon/icons/no_image_1.png"; // Fallback

        QPixmap pix(fullPath);
        if (pix.isNull()) {
            // Fallback generated or color
            pix = QPixmap(iconSize);
            pix.fill(Qt::lightGray);
            qDebug() << "[ModifyCatalog] Failed to load image:" << fullPath;
        }

        auto *item = new QStandardItem(
            QIcon(pix.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
            QString("%1\n%2").arg(designNo, company));

        item->setEditable(false);
        item->setData(designNo, Qt::UserRole + 2);

        modifyCatalogModel->appendRow(item);
    }
    qDebug() << "[ModifyCatalog] Model row count:" << modifyCatalogModel->rowCount();
}

void ModifyCatalog::loadDesignForEdit(const QString & designNo)
{
    qDebug() << "[ModifyCatalog] loadDesignForEdit called for:" << designNo;
    currentDesignNo = designNo;

    if (!ui) {
        qWarning() << "[ModifyCatalog] ui is null!";
        return;
    }

    // Basic widget sanity checks
    if (!ui->imagPath_lineEdit || !ui->designNO_lineEdit || !ui->companyName_lineEdit ||
        !ui->note || !ui->jewelryButton || !ui->imageView_label_at_addImage ||
        !ui->goldTable || !ui->diaTable || !ui->stoneTable) {
        qWarning() << "[ModifyCatalog] One or more form widgets are null - aborting loadDesignForEdit";
        return;
    }

    // Fetch details
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isValid()) {
        qWarning() << "[ModifyCatalog] Database connection invalid";
        QMessageBox::warning(this, "Error", "Database not available.");
        return;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT image_path, image_type, company_name, note,
               gold_weight, diamond, stone
        FROM image_data
        WHERE design_no = :design_no AND "delete" = 0
    )");
    query.bindValue(":design_no", designNo);

    if (!query.exec()) {
        qWarning() << "[ModifyCatalog] query.exec() failed:" << query.lastError().text();
        QMessageBox::warning(this, "Error", "Could not fetch design details (query failed).");
        return;
    }

    if (!query.next()) {
        qWarning() << "[ModifyCatalog] query.next() returned false (no row)";
        QMessageBox::warning(this, "Error", "No design found or it has been deleted.");
        return;
    }

    qDebug() << "[ModifyCatalog] DB row fetched";

    // Populate UI fields safely
    QString imagePath = query.value("image_path").toString();
    QString fullPath = QFile::exists(imagePath) ? imagePath : QDir(qApp->applicationDirPath()).filePath(imagePath);

    ui->imagPath_lineEdit->setText(fullPath);
    ui->designNO_lineEdit->setText(designNo);
    ui->companyName_lineEdit->setText(query.value("company_name").toString());
    ui->note->setText(query.value("note").toString());

    selectedImageType = query.value("image_type").toString();
    ui->jewelryButton->setText(selectedImageType);

    qDebug() << "[ModifyCatalog] imagePath set to:" << fullPath << " type:" << selectedImageType;

    // Image Label - guard sizes
    QPixmap pix;
    if (!fullPath.isEmpty() && QFile::exists(fullPath)) {
        bool loaded = pix.load(fullPath);
        qDebug() << "[ModifyCatalog] pix.load result:" << loaded << "path:" << fullPath;
    }
    else {
        qDebug() << "[ModifyCatalog] image file not found, using fallback";
    }

    if (!pix.isNull()) {
        int w = ui->imageView_label_at_addImage->width();
        int h = ui->imageView_label_at_addImage->height();
        if (w > 0 && h > 0) {
            ui->imageView_label_at_addImage->setPixmap(pix.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            ui->imageView_label_at_addImage->setPixmap(pix);
        }
    }
    else {
        ui->imageView_label_at_addImage->clear();
    }

    // Prepare shape lists once and log
    QStringList diaShapes = DatabaseUtils::fetchShapes("diamond");
    QStringList stoneShapes = DatabaseUtils::fetchShapes("stone");
    qDebug() << "[ModifyCatalog] fetched shapes: diamond=" << diaShapes.size() << "stone=" << stoneShapes.size();

    // Tables population with defensive checks
    auto populateTable = [&](QTableWidget* table, const QString& jsonStr, const QString& type) {
        if (!table) {
            qWarning() << "[ModifyCatalog] populateTable: table is null for type" << type;
            return;
        }

        // Prevent UI updates and signals while we rebuild the table
        table->setUpdatesEnabled(false);
        table->blockSignals(true);

        // Remove existing cell widgets safely using deleteLater to avoid reentrancy issues
        for (int r = table->rowCount() - 1; r >= 0; --r) {
            for (int c = 0; c < table->columnCount(); ++c) {
                QWidget* w = table->cellWidget(r, c);
                if (w) {
                    table->removeCellWidget(r, c);
                    w->setParent(nullptr); // detach to be safe
                    w->deleteLater();
                }
            }
        }

        // Clear contents (removes QTableWidgetItems). We'll reset columns next.
        table->clearContents();
        table->setRowCount(0);

        // Ensure table has expected columns and headers to avoid setCellWidget/setItem crashes
        if (type == "gold") {
            table->setColumnCount(2);
            table->setHorizontalHeaderLabels({ "Karat", "Weight (g)" });
        }
        else {
            table->setColumnCount(3);
            table->setHorizontalHeaderLabels({ "Type/Shape", "Size (MM)", "Quantity" });
        }

        if (jsonStr.trimmed().isEmpty()) {
            qDebug() << "[ModifyCatalog] populateTable: empty JSON for type" << type;
            table->setUpdatesEnabled(true);
            table->blockSignals(false);
            return;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "[ModifyCatalog] JSON parse error for type" << type << ":" << parseError.errorString();
            table->setUpdatesEnabled(true);
            table->blockSignals(false);
            return;
        }
        if (!doc.isArray()) {
            qWarning() << "[ModifyCatalog] JSON is not an array for type" << type;
            table->setUpdatesEnabled(true);
            table->blockSignals(false);
            return;
        }

        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            if (!val.isObject()) {
                qWarning() << "[ModifyCatalog] skipping non-object JSON element in" << type;
                continue;
            }
            QJsonObject obj = val.toObject();

            int row = table->rowCount();
            table->insertRow(row);

            if (type == "gold") {
                table->setItem(row, 0, new QTableWidgetItem(obj.value("karat").toString()));
                table->setItem(row, 1, new QTableWidgetItem(obj.value("weight(g)").toString()));
            }
            else {
                // shape combo
                QComboBox* shapeBox = new QComboBox(table);
                if (type == "diamond")
                    shapeBox->addItems(diaShapes);
                else
                    shapeBox->addItems(stoneShapes);

                QString shapeVal = obj.value("type").toString();
                if (!shapeVal.isEmpty()) {
                    if (shapeBox->findText(shapeVal) == -1)
                        shapeBox->addItem(shapeVal);
                    shapeBox->setCurrentText(shapeVal);
                }

                // size combo - fetch sizes defensively
                QComboBox* sizeBox = new QComboBox(table);
                QStringList sizes = DatabaseUtils::fetchSizes(type, shapeVal);
                if (sizes.isEmpty()) {
                    QString sizeVal = obj.value("sizeMM").toString();
                    if (!sizeVal.isEmpty())
                        sizeBox->addItem(sizeVal);
                }
                else {
                    sizeBox->addItems(sizes);
                    QString sizeVal = obj.value("sizeMM").toString();
                    if (!sizeVal.isEmpty() && sizeBox->findText(sizeVal) == -1)
                        sizeBox->addItem(sizeVal);
                    sizeBox->setCurrentText(sizeVal);
                }

                // Parent widgets to the table by passing 'table' in constructor; safe to set as cell widgets now.
                table->setCellWidget(row, 0, shapeBox);
                table->setCellWidget(row, 1, sizeBox);
                table->setItem(row, 2, new QTableWidgetItem(obj.value("quantity").toString()));
            }
        }

        // Re-enable updates/signals
        table->setUpdatesEnabled(true);
        table->blockSignals(false);
        };

    // Defensive: ensure queries returned valid strings
    QString goldJson = query.value("gold_weight").toString();
    QString diaJson = query.value("diamond").toString();
    QString stoneJson = query.value("stone").toString();

    qDebug() << "[ModifyCatalog] gold length:" << goldJson.size()
        << "dia length:" << diaJson.size()
        << "stone length:" << stoneJson.size();

    populateTable(ui->goldTable, goldJson, "gold");
    populateTable(ui->diaTable, diaJson, "diamond");
    populateTable(ui->stoneTable, stoneJson, "stone");

    // Finally switch page
    stackedWidget->setCurrentWidget(formPage);
    qDebug() << "[ModifyCatalog] loadDesignForEdit completed for:" << designNo;
}

void ModifyCatalog::on_save_insert_clicked()
{
    // Gather Data
    QString imagePath = ui->imagPath_lineEdit->text();
    QString companyName = ui->companyName_lineEdit->text();
    QString note = ui->note->toPlainText();

    // JSON Builders (Simplified)
    auto buildJson = [](QTableWidget *t, bool isGold) -> QJsonArray {
        QJsonArray arr;
        for (int i=0; i<t->rowCount(); ++i) {
             QJsonObject obj;
             if (isGold) {
                 if (t->item(i,0)) obj["karat"] = t->item(i,0)->text();
                 if (t->item(i,1)) obj["weight(g)"] = t->item(i,1)->text();
                 if (obj["weight(g)"].toString().isEmpty()) continue; // skip empty rows
             } else {
                 auto *cb1 = qobject_cast<QComboBox*>(t->cellWidget(i,0));
                 auto *cb2 = qobject_cast<QComboBox*>(t->cellWidget(i,1));
                 auto *item = t->item(i,2);
                 if (cb1) obj["type"] = cb1->currentText();
                 if (cb2) obj["sizeMM"] = cb2->currentText();
                 if (item) obj["quantity"] = item->text();
             }
             arr.append(obj);
        }
        return arr;
    };

    QJsonArray goldArr = buildJson(ui->goldTable, true);
    QJsonArray diaArr = buildJson(ui->diaTable, false);
    QJsonArray stoneArr = buildJson(ui->stoneTable, false);

    QString newImagePath = DatabaseUtils::saveImage(imagePath);

    // Use insertCatalogData (logic in DBUtils handles modify if design exists)
    QString res = DatabaseUtils::insertCatalogData(
        newImagePath, selectedImageType, currentDesignNo, companyName,
        goldArr, diaArr, stoneArr, note
    );

    if (res == "modify" || res == "insert") {
        QMessageBox::information(this, "Success", "Catalog updated successfully.");
        // Go back to grid
        stackedWidget->setCurrentWidget(gridPage);
        loadCatalogGrid();
    } else {
        QMessageBox::warning(this, "Error", "Failed to update catalog.");
    }
}

void ModifyCatalog::on_addCatalog_cancel_button_clicked()
{
    stackedWidget->setCurrentWidget(gridPage);
}

void ModifyCatalog::on_brows_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Select Image", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!filePath.isEmpty()) {
        ui->imagPath_lineEdit->setText(filePath);
        QPixmap p(filePath);
        if (!p.isNull()) {
             ui->imageView_label_at_addImage->setPixmap(p.scaled(ui->imageView_label_at_addImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void ModifyCatalog::setupGoldTable()
{
    // Initialize headers/rows for gold table
    QList<int> karats = {24, 22, 20, 18, 14, 10};
    ui->goldTable->setRowCount(karats.size());
    ui->goldTable->setColumnCount(2);

    for (int i = 0; i < karats.size(); ++i) {
        auto *k = new QTableWidgetItem(QString::number(karats[i]) + "kt");
        k->setFlags(k->flags() & ~Qt::ItemIsEditable);
        ui->goldTable->setItem(i, 0, k);
        ui->goldTable->setItem(i, 1, new QTableWidgetItem());
    }

    connect(ui->goldTable, &QTableWidget::itemChanged, this, &ModifyCatalog::calculateGoldWeights);
}

void ModifyCatalog::calculateGoldWeights(QTableWidgetItem *item)
{
    if (!item || item->column() != 1 || item->row() != 0) return;
    bool ok;
    double w24 = item->text().toDouble(&ok);
    if (!ok) return;

    ui->goldTable->blockSignals(true);
    QList<int> karats = {24, 22, 20, 18, 14, 10};
    for (int i=1; i<karats.size(); ++i) {
        if (!ui->goldTable->item(i,1)) ui->goldTable->setItem(i,1,new QTableWidgetItem());
        ui->goldTable->item(i,1)->setText(QString::number((karats[i]/24.0)*w24, 'f', 3));
    }
    ui->goldTable->blockSignals(false);
}

void ModifyCatalog::onJewelryItemSelected(const QString &item)
{
    selectedImageType = item;
    ui->jewelryButton->setText(item);
}

void ModifyCatalog::onModifyCatalogContextMenuRightClicked(const QPoint& pos) {
    // pos is in view coordinates (viewport), indexAt expects viewport coordinates
    QModelIndex proxyIndex = modifyCatalogView->indexAt(pos);
    if (!proxyIndex.isValid()) {
        qDebug() << "[ModifyCatalog] context menu: no item at pos" << pos;
        return;
    }

    // Map proxy index to source index (important when using QSortFilterProxyModel)
    QModelIndex sourceIndex = filterModel->mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) {
        qDebug() << "[ModifyCatalog] context menu: failed to map to source index";
        return;
    }

    QStandardItem* item = modifyCatalogModel->itemFromIndex(sourceIndex);
    if (!item) {
        qDebug() << "[ModifyCatalog] context menu: source item is null";
        return;
    }

    QString designNo = item->data(Qt::UserRole + 2).toString().trimmed();
    if (designNo.isEmpty()) {
        qDebug() << "[ModifyCatalog] context menu: designNo empty for item";
        return;
    }

    QMenu modifyRightClickMenu;
    modifyDesignAct = modifyRightClickMenu.addAction("Modify");
    deleteDesignAct = modifyRightClickMenu.addAction("Delete");

    QPoint globalPos = modifyCatalogView->viewport()->mapToGlobal(pos);
    QAction* selectedAct = modifyRightClickMenu.exec(globalPos);

    if (!selectedAct) {
        return;
    }

    if (selectedAct == modifyDesignAct) {
        qDebug() << "[ModifyCatalog] Modify action selected for design:" << designNo;
        loadDesignForEdit(designNo);
    }
    else if (selectedAct == deleteDesignAct) {
        qDebug() << "[ModifyCatalog] Delete action selected for design:" << designNo;
        deleteClickedAction(designNo);
    }
}

void ModifyCatalog::deleteClickedAction(const QString& designNo)
{
    if (designNo.isEmpty()) {
        qWarning() << "[ModifyCatalog] deleteClickedAction called with empty designNo";
        return;
    }

    // Ask user for confirmation
    auto reply = QMessageBox::question(this,
        "Confirm Delete",
        QString("Delete design '%1' ? This action is permanent.").arg(designNo),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;

    // DatabaseUtils::deleteDesign has a weird return convention in the current code:
    // it returns 0 on success and 1 on failure (instead of true/false).
    // Treat `false` (0) as success.
    QString dn = designNo; // DatabaseUtils::deleteDesign expects a non-const ref
    bool dbResult = DatabaseUtils::deleteDesign(dn);

    if (dbResult == false) {
        // Remove the corresponding row from the source model (modifyCatalogModel).
        // The view uses filterModel as its model (proxy). Find source row(s) by matching
        // the stored designNo (Qt::UserRole + 2) and remove from the source model.
        bool removed = false;
        for (int r = 0; r < modifyCatalogModel->rowCount(); ++r) {
            QStandardItem* it = modifyCatalogModel->item(r);
            if (!it) continue;
            QString itemDesign = it->data(Qt::UserRole + 2).toString();
            if (itemDesign == designNo) {
                modifyCatalogModel->removeRow(r);
                removed = true;
                break;
            }
        }

        QMessageBox::information(this, "Deleted", QString("Design '%1' deleted.").arg(designNo));

        if (!removed) {
            // If we didn't find it in the source model (unlikely), refresh the grid.
            loadCatalogGrid();
        }
    }
    else {
        QMessageBox::warning(this, "Delete Failed", QString("Failed to delete design '%1'.").arg(designNo));
    }
}
