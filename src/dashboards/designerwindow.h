#ifndef DESIGNERWINDOW_H
#define DESIGNERWINDOW_H

#include <QMainWindow>

namespace Ui {
class DesignerWindow;
}

class DesignerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DesignerWindow(QWidget *parent = nullptr);
    ~DesignerWindow();

private slots:
    void changeRole();
    void openAddCatalog();
    void openModifyCatalog() ;

private:
    Ui::DesignerWindow *ui;
};

#endif // DESIGNERWINDOW_H
