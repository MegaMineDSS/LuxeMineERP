#include "manufacturerwindow.h"
#include "ui_manufacturer.h"

ManufacturerWindow::ManufacturerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ManufacturerWindow)
{
    ui->setupUi(this);
}

ManufacturerWindow::~ManufacturerWindow()
{
    delete ui;
}
