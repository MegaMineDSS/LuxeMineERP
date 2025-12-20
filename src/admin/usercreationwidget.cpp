#include "usercreationwidget.h"
#include "ui_usercreation.h"

#include "auth/AuthService.h"
#include "common/SessionManager.h"

#include <QMessageBox>

UserCreationWidget::UserCreationWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::UserCreationWidget)
{
    ui->setupUi(this);

    // 🔐 Security check (defense in depth)
    if (!SessionManager::hasRole("admin")) {
        QMessageBox::critical(this, "Access Denied",
                              "Admin access required.");
        close();
        return;
    }

    // Seller group disabled by default (your UX decision 👍)
    ui->sellerGroupBox->setEnabled(false);

    // Role list selection change
    connect(ui->rolesListWidget, &QListWidget::itemSelectionChanged,
            this, &UserCreationWidget::onRolesSelectionChanged);

    // Create user button
    connect(ui->createUserButton, &QPushButton::clicked,
            this, &UserCreationWidget::onCreateUserClicked);
}

UserCreationWidget::~UserCreationWidget()
{
    delete ui;
}

void UserCreationWidget::onRolesSelectionChanged()
{
    ui->sellerGroupBox->setEnabled(isSellerSelected());
}

bool UserCreationWidget::isSellerSelected() const
{
    for (auto *item : ui->rolesListWidget->selectedItems()) {
        if (item->text() == "seller") {
            return true;
        }
    }
    return false;
}

void UserCreationWidget::onCreateUserClicked()
{
    // -------------------------
    // Login fields
    // -------------------------
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();
    bool isActive = ui->activeCheckBox->isChecked();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Username and password are required.");
        return;
    }

    // -------------------------
    // Employee fields
    // -------------------------
    QString fullName = ui->fullNameLineEdit->text().trimmed();
    QString accountNo = ui->accountNoLineEdit->text().trimmed();
    double baseSalary = ui->baseSalarySpinBox->value();
    double workingHours = ui->workingHoursSpinBox->value();

    if (fullName.isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Employee full name is required.");
        return;
    }

    // -------------------------
    // Roles (MULTI)
    // -------------------------
    QStringList roles;
    for (auto *item : ui->rolesListWidget->selectedItems())
        roles << item->text();

    if (roles.isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Select at least one role.");
        return;
    }

    // -------------------------
    // Seller-specific validation
    // -------------------------
    double sellingPercentage = 0.0;

    if (roles.contains("seller")) {
        sellingPercentage = ui->sellingPercentageSpinBox->value();

        if (sellingPercentage <= 0) {
            QMessageBox::warning(this, "Validation Error",
                                 "Selling percentage must be greater than 0.");
            return;
        }
    }

    // -------------------------
    // Create user via service
    // -------------------------
    bool success = AuthService::createUser(
        username,
        password,
        isActive,
        fullName,
        accountNo,
        baseSalary,
        workingHours,
        roles,
        sellingPercentage
        );

    if (!success) {
        QMessageBox::critical(this, "Error",
                              "Failed to create user.\n"
                              "Username may already exist.");
        return;
    }

    QMessageBox::information(this, "Success",
                             "User created successfully.");

    close(); // close MDI subwindow
}
