#ifndef MODIFYCATALOG_H
#define MODIFYCATALOG_H

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTableWidgetItem>
#include "jewelrymenu.h"

namespace Ui {
class ModifyCatalog;
}

class ModifyCatalog : public QWidget
{
    Q_OBJECT

public:
    explicit ModifyCatalog(QWidget *parent = nullptr);
    ~ModifyCatalog();

signals:
    void designSelectedForModify(const QString &designNo);

private slots:
    void on_save_insert_clicked(); // reused form slot
    void on_addCatalog_cancel_button_clicked(); // reused form slot
    void on_brows_clicked();
    void calculateGoldWeights(QTableWidgetItem *item);
    void onJewelryItemSelected(const QString &item);

private:
    void setupLayout(); // Sets up the StackedWidget
    void setupGridPage(); // Programmatic Grid
    void setupFormPage(); // UI Form

    void loadCatalogGrid();
    void loadDesignForEdit(const QString &designNo);
    void setupGoldTable();
    void onModifyCatalogContextMenuRightClicked(const QPoint &pos);
    void deleteClickedAction(const QString &designNo);

    Ui::ModifyCatalog *ui;

    QStackedWidget *stackedWidget;
    QWidget *gridPage;
    QWidget *formPage;

    // Grid View Members
    QListView *modifyCatalogView;
    QLineEdit *searchBar;
    QStandardItemModel *modifyCatalogModel;
    QSortFilterProxyModel *filterModel;

    // Form Members
    JewelryMenu *jewelryMenu;
    QString selectedImageType;
    QString currentDesignNo; // Track being edited

    QAction *modifyDesignAct;
    QAction *deleteDesignAct;
};



#endif // MODIFYCATALOG_H
