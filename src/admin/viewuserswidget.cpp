#include "viewuserswidget.h"
#include "ui_viewusers.h"

#include "database/UserRepository.h"
#include "common/SessionManager.h"
#include "admin/changepassworddialog.h"

#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
// #include <dlgs.h>

ViewUsersWidget::ViewUsersWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::ViewUsersWidget)
{
    ui->setupUi(this);

    // 🔐 Admin only
    if (!SessionManager::hasRole("admin")) {
        close();
        return;
    }

    setupTable();

    connect(ui->refreshButton, &QPushButton::clicked,
            this, &ViewUsersWidget::loadUsers);

    loadUsers();

    connect(ui->usersTableWidget, &QTableWidget::itemChanged,
            this, &ViewUsersWidget::saveEditedRow);

}

ViewUsersWidget::~ViewUsersWidget()
{
    delete ui;
}
void ViewUsersWidget::setupTable()
{
    QStringList headers = {
        "Username",
        "Full Name",
        "Roles",
        "Base Salary",
        "Worked Days",
        "Advance Paid",
        "Paid This Month",
        "Total Paid",
        "Pending",
        "Change Password"
    };

    ui->usersTableWidget->setColumnCount(headers.size());
    ui->usersTableWidget->setHorizontalHeaderLabels(headers);

    ui->usersTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->usersTableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->usersTableWidget->setSortingEnabled(true);

    ui->usersTableWidget->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    ui->usersTableWidget->verticalHeader()->setVisible(false);
}

void ViewUsersWidget::loadUsers()
{
    int month = ui->monthComboBox->currentIndex() + 1;
    int year = ui->yearComboBox->currentText().toInt();

    auto users = UserRepository::getUsersWithPayments(month, year);

    ui->usersTableWidget->setRowCount(users.size());

    double totalSalary = 0;
    double totalPaid = 0;
    double totalPending = 0;

    for (int row = 0; row < users.size(); ++row) {
        const auto &u = users[row];

        auto setItem = [&](int col, const QString &text, bool editable) {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setFlags(editable
                               ? item->flags() | Qt::ItemIsEditable
                               : item->flags() & ~Qt::ItemIsEditable);
            ui->usersTableWidget->setItem(row, col, item);
        };

        setItem(0, u.username, false);
        setItem(1, u.fullName, false);
        setItem(2, u.roles, false);
        setItem(3, QString::number(u.baseSalary, 'f', 2), false);
        setItem(4, QString::number(u.workedDays), true);
        setItem(5, QString::number(u.advancePaid, 'f', 2), true);
        setItem(6, QString::number(u.paidAmount, 'f', 2), true);
        setItem(7, QString::number(u.totalPaid, 'f', 2), false);
        setItem(8, QString::number(u.pending, 'f', 2), false);

        QPushButton *btn = new QPushButton("Change");
        btn->setProperty("userId", u.userId);

        connect(btn, &QPushButton::clicked, this, [=]() {
            ChangePasswordDialog dlg(u.userId, this);
            dlg.exec();
        });
        ui->usersTableWidget->setCellWidget(row, 9, btn);

        totalSalary += u.baseSalary;
        totalPaid += u.totalPaid;
        totalPending += u.pending;
    }

    ui->totalBaseSalaryLabel->setText(
        QString("Total Salary: ₹ %1").arg(totalSalary, 0, 'f', 2));

    ui->totalPaidLabel->setText(
        QString("Paid: ₹ %1").arg(totalPaid, 0, 'f', 2));

    ui->totalPendingLabel->setText(
        QString("Pending: ₹ %1").arg(totalPending, 0, 'f', 2));

}

void ViewUsersWidget::saveEditedRow(QTableWidgetItem *item)
{
    int row = item->row();

    // Only react when Worked Days or Advance Paid changes
    if (item->column() != 4 && item->column() != 5)
        return;

    int month = ui->monthComboBox->currentIndex() + 1;
    int year = ui->yearComboBox->currentText().toInt();

    QString username =
        ui->usersTableWidget->item(row, 0)->text();

    int employeeId =
        UserRepository::getEmployeeIdByUsername(username);

    if (employeeId == -1) {
        QMessageBox::critical(this, "Error",
                              "Employee not found.");
        return;
    }

    // -----------------------------
    // Fetch required values
    // -----------------------------
    int workedDays =
        ui->usersTableWidget->item(row, 4)->text().toInt();

    double advancePaid =
        ui->usersTableWidget->item(row, 5)->text().toDouble();

    double baseSalary =
        ui->usersTableWidget->item(row, 3)->text().toDouble();

    // -----------------------------
    // AUTO CALCULATION
    // -----------------------------
    double perDaySalary = baseSalary / 30.0;
    double paidThisMonth = perDaySalary * workedDays;

    // Update UI (Paid This Month column)
    ui->usersTableWidget->item(row, 6)
        ->setText(QString::number(paidThisMonth, 'f', 2));

    // -----------------------------
    // Save to DB
    // -----------------------------
    bool ok = UserRepository::updateMonthlyPayment(
        employeeId,
        month,
        year,
        workedDays,
        advancePaid,
        paidThisMonth
        );

    if (!ok) {
        QMessageBox::critical(this, "Error",
                              "Failed to save salary data.");
        return;
    }

    // Reload to update totals & pending
    // loadUsers();
}

