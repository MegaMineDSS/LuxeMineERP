#include "changepassworddialog.h"
#include "ui_changepassword.h"

#include "database/UserRepository.h"
#include <QMessageBox>

ChangePasswordDialog::ChangePasswordDialog(int userId, QWidget *parent)
    : QDialog(parent),
    m_userId(userId),
    ui(new Ui::ChangePasswordDialog)
{
    ui->setupUi(this);

    connect(ui->changeButton, &QPushButton::clicked,
            this, &ChangePasswordDialog::onChangeClicked);

    connect(ui->cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete ui;
}

void ChangePasswordDialog::onChangeClicked()
{
    QString pass1 = ui->newPasswordLineEdit->text();
    QString pass2 = ui->confirmPasswordLineEdit->text();

    if (pass1.isEmpty() || pass2.isEmpty()) {
        QMessageBox::warning(this, "Error",
                             "Password fields cannot be empty.");
        return;
    }

    if (pass1 != pass2) {
        QMessageBox::warning(this, "Error",
                             "Passwords do not match.");
        return;
    }

    bool ok = UserRepository::updateUserPassword(m_userId, pass1);

    if (!ok) {
        QMessageBox::critical(this, "Error",
                              "Failed to update password.");
        return;
    }

    QMessageBox::information(this, "Success",
                             "Password updated successfully.");
    accept();
}
