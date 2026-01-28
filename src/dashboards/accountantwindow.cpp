#include "accountantwindow.h"
#include "ui_accountant.h"

#include "accountant/castinglistwidget.h"
#include "accountant/metalpurchasewidget.h"
#include "accountant/stocklistwidget.h"
#include "common/rolewindowfactory.h"
#include "common/sessionmanager.h"
#include "common/switchroledialog.h"

#include <QMdiSubWindow>

AccountantWindow::AccountantWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AccountantWindow) {
  ui->setupUi(this);

  connect(ui->actionSwitch_Role, &QAction::triggered, this,
          &AccountantWindow::changeRole);

  connect(ui->actionOrders_List, &QAction::triggered, this,
          &AccountantWindow::openOrderList);

  QAction *actStock = new QAction("Stock Register", this);
  menuBar()->addAction(actStock);
  connect(actStock, &QAction::triggered, this,
          &AccountantWindow::openStockList);

  QMenu *menuPurchase = menuBar()->addMenu("Purchase");
  QAction *actMetalPurchase = menuPurchase->addAction("Metal Purchase");
  connect(actMetalPurchase, &QAction::triggered, this,
          &AccountantWindow::openMetalPurchase);
}

AccountantWindow::~AccountantWindow() { delete ui; }

void AccountantWindow::changeRole() {
  // 1 Open role selection dialog
  SwitchRoleDialog dlg(this);

  if (dlg.exec() != QDialog::Accepted)
    return;

  // 2 Get selected role
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

void AccountantWindow::openOrderList() {
  for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
    if (sub->widget()->objectName() == "AccountantOrderList") {
      sub->setFocus();
      return;
    }
  }

  auto *widget = new CastingListWidget;
  widget->setObjectName("AccountantOrderList");

  QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(widget);
  subWindow->setWindowTitle("Order List");
  subWindow->setAttribute(Qt::WA_DeleteOnClose);

  widget->show();
}

void AccountantWindow::openStockList() {
  for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
    if (sub->widget()->objectName() == "StockListWidget") {
      sub->setFocus();
      return;
    }
  }

  auto *widget = new StockListWidget;
  widget->setObjectName("StockListWidget");

  QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(widget);
  subWindow->setWindowTitle("Stock Register");
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->showMaximized();
}

void AccountantWindow::openMetalPurchase() {
  for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
    if (sub->widget()->objectName() == "MetalPurchaseWidget") {
      sub->setFocus();
      return;
    }
  }

  auto *widget = new MetalPurchaseWidget;
  widget->setObjectName("MetalPurchaseWidget");

  QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(widget);
  subWindow->setWindowTitle("Metal Purchase");
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->showMaximized();
}
