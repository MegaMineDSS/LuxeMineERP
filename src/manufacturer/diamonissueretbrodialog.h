#ifndef DIAMONISSUERETBRODIALOG_H
#define DIAMONISSUERETBRODIALOG_H

#include <QDialog>

namespace Ui {
class DiamonIssueRetBroDialog;
}

class DiamonIssueRetBroDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiamonIssueRetBroDialog(QWidget *parent = nullptr);
    ~DiamonIssueRetBroDialog();

    void setContext(int row, int col, const QString &jobNo); // from JobSheet

signals:
    void menuHidden();
    void valuesUpdated(int row, const QVariantMap &vals);


private slots:
    void onRadioChanged();
    void onTypeChanged();
    void onSaveClicked();
    // void onCancelClicked();

    void on_pushButton_clicked();

private:
    Ui::DiamonIssueRetBroDialog *ui;
    void hideEvent(QHideEvent *event);

    int currentRow;
    int currentCol;
    QString currentJobNo;
    QString currentMode;  // "issue" or "return"


    void loadTypeOptions();
    void loadSizeOptions(const QString &type);
    void saveToDatabase(const QJsonObject &entry);
    void loadHistoryForCurrentContext();
};

#endif // DIAMONISSUERETBRODIALOG_H
