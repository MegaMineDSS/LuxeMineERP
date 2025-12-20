#include <QApplication>
#include <QMessageBox>

#include "auth/LoginWindow.h"
#include "database/DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // -------------------------------------------------
    // Initialize Database
    // -------------------------------------------------
    if (!DatabaseManager::instance().initialize()) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            "Failed to initialize database.\nApplication will exit."
            );
        return -1;
    }

    // -------------------------------------------------
    // Show Login Window
    // -------------------------------------------------
    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}
