#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();

private:
    Ui::LoginWindow *ui;

    void openDashboardForRole(const QString &role);
};

#endif // LOGINWINDOW_H
