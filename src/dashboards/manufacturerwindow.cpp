#include "manufacturerwindow.h"
#include "ui_manufacturer.h"

#include "common/rolewindowfactory.h"
#include "common/sessionmanager.h"
#include "common/switchroledialog.h"
#include "manufacturer/jobslistwidget.h"


#include <QMdiSubWindow>

ManufacturerWindow::ManufacturerWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ManufacturerWindow) {
  ui->setupUi(this);

  connect(ui->actionSwitch_Role, &QAction::triggered, this,
          &ManufacturerWindow::changeRole);

  connect(ui->actionJobs_List, &QAction::triggered, this,
          &ManufacturerWindow::openJobsList);
}

ManufacturerWindow::~ManufacturerWindow() { delete ui; }

void ManufacturerWindow::openJobsList() {
  for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
    if (sub->widget()->objectName() == "JobsList") {
      sub->setFocus();
      return;
    }
  }

  auto *widget = new JobsListWidget;
  widget->setObjectName("JobsList");

  QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(widget);
  subWindow->setWindowTitle("Jobs List");
  subWindow->setAttribute(Qt::WA_DeleteOnClose);

  widget->show();
}

QMdiArea *ManufacturerWindow::mdiArea() const { return ui->mdiArea; }

void ManufacturerWindow::changeRole() {
  // 1️⃣ Open role selection dialog
  SwitchRoleDialog dlg(this);

  if (dlg.exec() != QDialog::Accepted)
    return;

  // 2️⃣ Get selected role
  QString newRole = dlg.selectedRole();

  // 3️⃣ Ignore if same role selected
  if (newRole == SessionManager::activeRole())
    return;

  // 4️⃣ Update session
  SessionManager::setActiveRole(newRole);

  // 5️⃣ Open new role window
  QWidget *nextWindow = RoleWindowFactory::create(newRole);
  if (!nextWindow)
    return;

  nextWindow->show();

  // 6️⃣ Close current window
  this->close();
}
