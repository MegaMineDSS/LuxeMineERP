#ifndef MANUFACTURERWINDOW_H
#define MANUFACTURERWINDOW_H

#include <QMainWindow>

namespace Ui {
class ManufacturerWindow;
}

class ManufacturerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ManufacturerWindow(QWidget *parent = nullptr);
    ~ManufacturerWindow();

private slots:
    void changeRole();

private:
    Ui::ManufacturerWindow *ui;

    void openJobsList();
};

#endif // MANUFACTURERWINDOW_H
