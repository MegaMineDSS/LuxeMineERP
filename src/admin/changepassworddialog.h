#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class ChangePasswordDialog;
}

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePasswordDialog(int userId, QWidget *parent = nullptr);
    ~ChangePasswordDialog();

private slots:
    void onChangeClicked();

private:
    int m_userId;
    Ui::ChangePasswordDialog *ui;
};

#endif
