#ifndef DESIGNERWINDOW_H
#define DESIGNERWINDOW_H

#include <QMainWindow>
#include <QMdiArea>

namespace Ui {
class DesignerWindow;
}

class DesignerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DesignerWindow(QWidget *parent = nullptr);
    ~DesignerWindow();

    QMdiArea* mdiArea() const;

private slots:
    void changeRole();
    void openAddCatalog();
    void openModifyCatalog() ;

    void openOrderList();

private:
    Ui::DesignerWindow *ui;
};

#endif // DESIGNERWINDOW_H
