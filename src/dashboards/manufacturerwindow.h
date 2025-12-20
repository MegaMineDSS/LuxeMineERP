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

private:
    Ui::ManufacturerWindow *ui;
};

#endif // MANUFACTURERWINDOW_H
