#ifndef CASTINGLISTROW_H
#define CASTINGLISTROW_H

#include <QString>

struct CastingListRow
{
    int jobId = 0;
    QString deliveryDate;

    QString castingDate;
    QString vendorName;
    int pcs = 0;

    QString issueMetal;
    QString purity;
    double issueMetalWt = 0;
    int issueDiaPcs = 0;
    double issueDiaWt = 0;

    double receiveRunnerWt = 0;
    double receiveProductWt = 0;
    int receiveDiaPcs = 0;
    double receiveDiaWt = 0;

    double diaPrice = 0;

    QString status;   // PENDING / OPEN / CLOSED
};


#endif // CASTINGLISTROW_H
