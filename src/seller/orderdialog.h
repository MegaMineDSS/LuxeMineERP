#ifndef ORDERDIALOG_H
#define ORDERDIALOG_H

#include <QDialog>
#include "models/OrderData.h"

namespace Ui {
class OrderDialog;
}

class OrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrderDialog(QWidget *parent = nullptr);
    ~OrderDialog();

private slots:
    void on_savePushButton_clicked();

private:
    Ui::OrderDialog *ui;

    void fillOrderData(OrderData &o);
    bool validateInput() const;
    void prefillData();
};

#endif // ORDERDIALOG_H
