/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef QHR_BJOB_P_H
#define QHR_BJOB_P_H

#include "bjob.h"
#include <QMap>
#include <QEventLoopLocker>

class QTimer;
class QEventLoop;

namespace QHR {

class BJobPrivate
{
public:
    BJobPrivate();
    virtual ~BJobPrivate();

    BJob *q_ptr = nullptr;

    QString errorText;
    int error = BJob::NoError;
    BJob::Unit progressUnit = BJob::Bytes;
    QMap<BJob::Unit, qulonglong> processedAmount;
    QMap<BJob::Unit, qulonglong> totalAmount;
    unsigned long percentage = 0;
    QTimer *speedTimer = nullptr;
    QEventLoop *eventLoop = nullptr;
    // eventLoopLocker prevents QCoreApplication from exiting when the last
    // window is closed until the job has finished running
    QEventLoopLocker eventLoopLocker;
    BJob::Capabilities capabilities = BJob::NoCapabilities;
    bool suspended = false;
    bool isAutoDelete = true;

    void _k_speedTimeout();

    bool isFinished = false;

    Q_DECLARE_PUBLIC(BJob)
};

}

#endif // QHR_BJOB_P_H
