#include "designerwindow.h"
#include "ui_designer.h"

DesignerWindow::DesignerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DesignerWindow)
{
    ui->setupUi(this);
}

DesignerWindow::~DesignerWindow()
{
    delete ui;
}
