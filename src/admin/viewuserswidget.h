#ifndef VIEWUSERSWIDGET_H
#define VIEWUSERSWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
class ViewUsersWidget;
}

class ViewUsersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewUsersWidget(QWidget *parent = nullptr);
    ~ViewUsersWidget();

private slots:
    void loadUsers();
    void saveEditedRow(QTableWidgetItem *item);


private:
    void setupTable();

private:
    Ui::ViewUsersWidget *ui;
};

#endif
