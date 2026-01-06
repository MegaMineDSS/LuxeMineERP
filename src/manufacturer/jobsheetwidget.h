#ifndef JOBSHEETWIDGET_H
#define JOBSHEETWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

#include "managegolddialog.h"
#include "diamonissueretbrodialog.h"

namespace Ui {
class JobSheetWidget;
}

class JobSheetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JobSheetWidget(QWidget *parent = nullptr);
    ~JobSheetWidget();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onGoldDetailCellClicked(QTableWidgetItem *item); // New slot for cell click


private:
    Ui::JobSheetWidget *ui;

    QString userRole;
    int finalWidth = 0;
    int finalHeight = 0;
    double scaleFactor = 1.0;   // 🔹 scaling factor based on resolution


    QPixmap originalPixmap;


    ManageGoldDialog *newManageGold = nullptr;
    bool manageGold = false;

    DiamonIssueRetBroDialog *newDiamonIssueRetBro = nullptr;
    bool diamondMenuVisible = false;

    void addTableRow(QTableWidget *table);
    void set_value(const QString &jobNo);
    void loadImageForDesignNo();
    void saveDesignNoAndImagePath(const QString &designNo, const QString &imagePath);

    void set_value_designer();
    void set_value_manuf();

    void updateGoldTotalWeight();
    void handleCellSave(int row, int col);

    void updateDiamondTotals();
    void setupDiamondIssueClicks();
};

#endif // JOBSHEETWIDGET_H
