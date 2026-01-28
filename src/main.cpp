#include <QApplication>
#include <QMessageBox>

#include "auth/LoginWindow.h"
#include "common/AppStyle.h"
#include "database/DatabaseManager.h"


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // -------------------------------------------------
  // Apply Global Style
  // -------------------------------------------------
  app.setStyleSheet(AppStyle::getDarkTheme());

  // -------------------------------------------------
  // Initialize Database
  // -------------------------------------------------
  if (!DatabaseManager::instance().initialize()) {
    QMessageBox::critical(
        nullptr, "Database Error",
        "Failed to initialize database.\nApplication will exit.");
    return -1;
  }

  // -------------------------------------------------
  // Show Login Window
  // -------------------------------------------------
  LoginWindow loginWindow;
  loginWindow.show();

  return app.exec();
}
