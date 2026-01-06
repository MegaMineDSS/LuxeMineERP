#ifndef CASTINGWIDGET_H
#define CASTINGWIDGET_H

#include <QWidget>
#include "models/CastingData.h"

namespace Ui {
class CastingWidget;
}

class CastingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CastingWidget(QWidget *parent = nullptr);
    ~CastingWidget();

    void setJobId(int jobId);
    int jobId() const;


private slots:
    void on_savePushButton_clicked();

private:
    Ui::CastingWidget *ui;

    int m_jobId = 0;
    CastingData collectCastingData() const;

    void resetForm();
    void setupMetalComboBoxes();
};

#endif // CASTINGWIDGET_H
