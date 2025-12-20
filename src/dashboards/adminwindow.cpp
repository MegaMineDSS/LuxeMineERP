#include "AdminWindow.h"
#include "ui_admin.h"

#include "common/SessionManager.h"
#include "admin/UserCreationWidget.h"
#include "admin/ViewUsersWidget.h"

#include <QMdiSubWindow>


AdminWindow::AdminWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::AdminWindow)
{
    ui->setupUi(this);

    // Security check
    if (!SessionManager::hasRole("admin")) {
        close();
        return;
    }

    setWindowTitle("Admin Dashboard");

    // 🔗 Menu actions
    connect(ui->actionCreate_Users, &QAction::triggered,
            this, &AdminWindow::openCreateUser);

    connect(ui->actionView_Users, &QAction::triggered,
            this, &AdminWindow::openViewUsers);
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::openCreateUser()
{
    // Prevent opening multiple same windows (optional but good UX)
    for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
        if (sub->widget()->objectName() == "UserCreationWidget") {
            sub->setFocus();
            return;
        }
    }

    auto *widget = new UserCreationWidget;
    widget->setObjectName("UserCreationWidget");

    QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(widget);
    subWindow->setWindowTitle("Create User");
    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    widget->show();
}


void AdminWindow::openViewUsers()
{
    // 🔁 Prevent opening duplicate View Users windows
    for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
        if (sub->widget()->objectName() == "ViewUsersWidget") {
            sub->setFocus();
            return;
        }
    }

    auto *widget = new ViewUsersWidget;
    widget->setObjectName("ViewUsersWidget");

    QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(widget);
    subWindow->setWindowTitle("View Users");
    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    widget->show();
}
