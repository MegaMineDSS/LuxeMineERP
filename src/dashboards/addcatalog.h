#ifndef ADDCATALOG_H
#define ADDCATALOG_H

#include "jewelrymenu.h"

#include <QWidget>
#include <QTableWidgetItem>


namespace Ui {
class AddCatalog;
}

class AddCatalog : public QWidget
{
    Q_OBJECT

public:
    explicit AddCatalog(QWidget *parent = nullptr);
    ~AddCatalog();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_brows_clicked();

    void onJewelryItemSelected(const QString &item) ;

    void on_jewelryButton_clicked();

    void on_save_insert_clicked();

    void on_bulk_import_button_clicked();

    void on_demo_download_button_clicked();

private:
    void setupGoldTable() ;
    void calculateGoldWeights(QTableWidgetItem *item);
    void addTableRow(QTableWidget *table, const QString &tableType);

    Ui::AddCatalog *ui;
    JewelryMenu *jewelryMenu ;
    QString selectedImageType;


};

#endif // ADDCATALOG_H
