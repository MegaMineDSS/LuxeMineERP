#include "LoginWindow.h"
#include "ui_login.h"

#include "auth/AuthService.h"
#include "common/SessionManager.h"
#include "dashboards/AdminWindow.h"
#include "dashboards/SellerWindow.h"
#include "dashboards/accountantwindow.h"
#include "dashboards/designerwindow.h"
#include "dashboards/manufacturerwindow.h"


#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::LoginWindow) {
  ui->setupUi(this);

  // Connect login button
  connect(ui->loginButton, &QPushButton::clicked, this,
          &LoginWindow::onLoginClicked);

  // Connect Enter key
  connect(ui->usernameLineEdit, &QLineEdit::returnPressed, this,
          &LoginWindow::onLoginClicked);
  connect(ui->passwordLineEdit, &QLineEdit::returnPressed, this,
          &LoginWindow::onLoginClicked);

  // Connect show password checkbox
  connect(ui->showPasswordCheckBox, &QCheckBox::toggled, this,
          &LoginWindow::onShowPasswordToggled);
  onShowPasswordToggled(false);
}

LoginWindow::~LoginWindow() { delete ui; }

void LoginWindow::openDashboardForRole(const QString &role) {
  if (role == "admin") {
    auto *w = new AdminWindow();
    w->show();
  } else if (role == "seller") {
    auto *w = new SellerWindow();
    w->show();
  } else if (role == "manufacturer") {
    auto *w = new ManufacturerWindow();
    w->show();
  } else if (role == "designer") {
    auto *w = new DesignerWindow();
    w->show();
  } else if (role == "accountant") {
    auto *w = new AccountantWindow();
    w->show();
  } else {
    QMessageBox::critical(nullptr, "Error", "Unsupported role.");
    SessionManager::clear();
    this->show();
  }
}

void LoginWindow::onLoginClicked() {
  const QString username = ui->usernameLineEdit->text().trimmed();
  const QString password = ui->passwordLineEdit->text();

  User user = AuthService::login(username, password);

  if (!user.isValid()) {
    QMessageBox::critical(this, "Login Failed",
                          "Invalid username or password.");
    return;
  }
  // qDebug()<<user.id;
  // Store basic user
  SessionManager::setCurrentUser(user);

  // Fetch ALL roles
  QStringList roles = AuthService::getUserRoles(user.id);

  if (roles.isEmpty()) {
    QMessageBox::critical(this, "Access Denied",
                          "No roles assigned to this user.");
    SessionManager::clear();
    return;
  }

  SessionManager::setRoles(roles);

  this->hide();

  // 🔹 SINGLE ROLE → open directly
  if (roles.size() == 1) {
    SessionManager::setActiveRole(roles.first());
    openDashboardForRole(roles.first());
    return;
  }

  // TEMP: pick first role
  SessionManager::setActiveRole(roles.first());
  openDashboardForRole(roles.first());
}

void LoginWindow::onShowPasswordToggled(bool checked) {
  if (checked) {
    ui->passwordLineEdit->setEchoMode(QLineEdit::Normal);
  } else {
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
  }
}
