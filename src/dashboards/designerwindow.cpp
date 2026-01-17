#include "designerwindow.h"
#include "ui_designer.h"

#include "common/switchroledialog.h"
#include "common/rolewindowfactory.h"
#include "common/sessionmanager.h"
#include "dashboards/addcatalog.h"
#include "dashboards/modifycatalog.h"

#include <QMdiSubWindow>

DesignerWindow::DesignerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DesignerWindow)
{
    ui->setupUi(this);

    setWindowTitle("Designer Dashboard") ;

    connect(ui->actionSwitch_Role, &QAction::triggered,
            this, &DesignerWindow::changeRole);
    connect(ui->actionAdd_Catalog, &QAction::triggered, this, &DesignerWindow::openAddCatalog) ;

    connect(ui->actionModify_Catalog, &QAction::triggered, this, &DesignerWindow::openModifyCatalog) ;

    // connect Add Catalog action if exists
    if (ui->menuRoles) {
        // actionAdd_Catalog likely added in UI; try to connect if present
        QAction *act = ui->menuRoles->findChild<QAction *>("actionAdd_Catalog");
        if (act) {
            connect(act, &QAction::triggered, this, &DesignerWindow::openAddCatalog);
        }
    }
}

DesignerWindow::~DesignerWindow()
{
    delete ui;
}

void DesignerWindow::changeRole(){
    // 1 Open role selection dialog
    SwitchRoleDialog dlg(this);

    if (dlg.exec() != QDialog::Accepted)
        return;

    // 2️ Get selected role
    QString newRole = dlg.selectedRole();

    // 3 Ignore if same role selected
    if (newRole == SessionManager::activeRole())
        return;

    // 4 Update session
    SessionManager::setActiveRole(newRole);
    // 5 Open new role window
    QWidget *nextWindow = RoleWindowFactory::create(newRole);
    if (!nextWindow)
        return;
    nextWindow->show();
    // 6 Close current window
    this->close();
}

void DesignerWindow::openAddCatalog()
{
    // open AddCatalog in MDI area
    for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
        if (sub->widget() && sub->widget()->objectName() == "AddCatalogWidget") {
            ui->mdiArea->setActiveSubWindow(sub);
            return;
        }
    }

    auto *widget = new AddCatalog;
    widget->setObjectName("AddCatalogWidget");

    QMdiSubWindow *sub = ui->mdiArea->addSubWindow(widget);
    sub->setAttribute(Qt::WA_DeleteOnClose);
    sub->setWindowTitle("Add Catalog");

    widget->showMaximized();
}

void DesignerWindow::openModifyCatalog() {
    for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
        if (sub->widget() && sub->widget()->objectName() == "ModifyCatalogWidget") {
            ui->mdiArea->setActiveSubWindow(sub);
            return;
        }
    }

    auto *widget = new ModifyCatalog;
    widget->setObjectName("ModifyCatalogWidget");

    QMdiSubWindow *sub = ui->mdiArea->addSubWindow(widget);
    sub->setAttribute(Qt::WA_DeleteOnClose);
    sub->setWindowTitle("Modify Catalog");

    widget->showMaximized();
}
