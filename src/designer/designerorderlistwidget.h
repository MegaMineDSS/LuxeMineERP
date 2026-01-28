#ifndef DESIGNERORDERLISTWIDGET_H
#define DESIGNERORDERLISTWIDGET_H

#include <QWidget>

namespace Ui {
class DesignerOrderListWidget;
}

class DesignerOrderListWidget : public QWidget {
  Q_OBJECT

public:
  explicit DesignerOrderListWidget(QWidget *parent = nullptr);
  ~DesignerOrderListWidget();

private:
  Ui::DesignerOrderListWidget *ui;
  void setupTable();
  void loadData();

private slots:
  void onOpenJobClicked();
  void onContextMenuRequested(const QPoint &pos);
  void openJobSheet();
};

#endif // DESIGNERORDERLISTWIDGET_H
