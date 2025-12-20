#ifndef USERPAYMENTVIEW_H
#define USERPAYMENTVIEW_H

#include <QString>

struct UserPaymentView
{
    int userId;
    int employeeId;

    QString username;
    QString fullName;
    QString roles;

    double baseSalary;

    int workedDays;
    double advancePaid;
    double paidAmount;

    double totalPaid;
    double pending;
};

#endif
