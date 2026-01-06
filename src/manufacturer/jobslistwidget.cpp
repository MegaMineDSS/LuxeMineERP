#include "jobslistwidget.h"
#include "ui_jobslist.h"

JobsListWidget::JobsListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JobsListWidget)
{
    ui->setupUi(this);
}

JobsListWidget::~JobsListWidget()
{
    delete ui;
}
