#ifndef MANAGEGOLDDIALOG_H
#define MANAGEGOLDDIALOG_H

#include <QDialog>

namespace Ui {
class ManageGoldDialog;
}

class ManageGoldDialog : public QDialog
{
    Q_OBJECT

public:

    enum Mode {
        Filling,
        Returning,
        Dust
    };

    explicit ManageGoldDialog(QWidget *parent = nullptr);
    ~ManageGoldDialog();

    void setMode(Mode mode);
    Mode currentMode;  // new

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void menuHidden();
    void totalWeightCalculated(double weight);

private slots:
    void on_pushButton_clicked();
    void on_issueAddPushButton_clicked();

private:
    Ui::ManageGoldDialog *ui;

    void hideEvent(QHideEvent *event);
    void loadHistory();  // renamed generic version
};

#endif // MANAGEGOLDDIALOG_H
