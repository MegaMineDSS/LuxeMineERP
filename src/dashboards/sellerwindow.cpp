#include "sellerwindow.h"
#include "ui_seller.h"

#include <QMdiSubWindow>
#include "seller/orderdialog.h"

SellerWindow::SellerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SellerWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew_Order, &QAction::triggered,
            this, &SellerWindow::openCreateOrder);

}

SellerWindow::~SellerWindow()
{
    delete ui;
}

void SellerWindow::openCreateOrder()
{
    // Prevent opening multiple same windows (optional but good UX)
    for (QMdiSubWindow *sub : ui->mdiArea->subWindowList()) {
        if (sub->widget()->objectName() == "OrderCreationDialog") {
            sub->setFocus();
            return;
        }
    }

    auto *widget = new OrderDialog;
    widget->setObjectName("OrderCreationDialog");

    QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(widget);
    subWindow->setWindowTitle("Create Order");
    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    widget->show();
}
