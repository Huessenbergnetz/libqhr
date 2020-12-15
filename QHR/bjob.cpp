/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "bjob_p.h"

#include <QEventLoop>
#include <QTimer>

using namespace QHR;

BJobPrivate::BJobPrivate()
{

}

BJobPrivate::~BJobPrivate()
{

}

void BJobPrivate::_k_speedTimeout()
{
    Q_Q(BJob);
    Q_EMIT q->speed(q, 0);
    speedTimer->stop();
}

BJob::BJob(QObject *parent)
    : QObject(parent), d_ptr(new BJobPrivate)
{
     d_ptr->q_ptr = this;
}

BJob::BJob(BJobPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

BJob::~BJob()
{
    if (!d_ptr->isFinished) {
        d_ptr->isFinished = true;
        Q_EMIT finished(this, QPrivateSignal());
    }

    delete d_ptr->speedTimer;
    delete d_ptr;
}

BJob::Capabilities BJob::capabilities() const
{
    return d_func()->capabilities;
}

bool BJob::isSuspended() const
{
    return d_func()->suspended;
}

void BJob::finishJob(bool emitResult)
{
    Q_D(BJob);
    Q_ASSERT(!d->isFinished);
    d->isFinished = true;

    if (d->eventLoop) {
        d->eventLoop->quit();
    }

    // If we are displaying a progress dialog, remove it first.
    Q_EMIT finished(this, QPrivateSignal());

    if (emitResult) {
        Q_EMIT result(this, QPrivateSignal());
    }

    if (isAutoDelete()) {
        deleteLater();
    }
}

bool BJob::kill(KillVerbosity verbosity)
{
    Q_D(BJob);
    if (d->isFinished) {
        return true;
    }

    if (doKill()) {
        // A subclass can (but should not) call emitResult() or kill()
        // from doKill() and thus set isFinished to true.
        if (!d->isFinished) {
            setError(KilledJobError);
            finishJob(verbosity != Quietly);
        }
        return true;
    } else {
        return false;
    }
}

bool BJob::suspend()
{
    Q_D(BJob);
    if (!d->suspended) {
        if (doSuspend()) {
            d->suspended = true;
            Q_EMIT suspended(this, QPrivateSignal());
            return true;
        }
    }

    return false;
}

bool BJob::resume()
{
    Q_D(BJob);
    if (d->suspended) {
        if (doResume()) {
            d->suspended = false;
            Q_EMIT resumed(this, QPrivateSignal());

            return true;
        }
    }

    return false;
}

bool BJob::doKill()
{
    return false;
}

bool BJob::doSuspend()
{
    return false;
}

bool BJob::doResume()
{
    return false;
}

void BJob::setCapabilities(BJob::Capabilities capabilities)
{
    Q_D(BJob);
    d->capabilities = capabilities;
}

bool BJob::exec()
{
    Q_D(BJob);
    // Usually this job would delete itself, via deleteLater() just after
    // emitting result() (unless configured otherwise). Since we use an event
    // loop below, that event loop will process the deletion event and we'll
    // have been deleted when exec() returns. This crashes, so temporarily
    // suspend autodeletion and manually do it afterwards.
    const bool wasAutoDelete = isAutoDelete();
    setAutoDelete(false);

    Q_ASSERT(!d->eventLoop);

    QEventLoop loop(this);
    d->eventLoop = &loop;

    start();
    if (!d->isFinished) {
        d->eventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    }
    d->eventLoop = nullptr;

    if (wasAutoDelete) {
        deleteLater();
    }
    return (d->error == NoError);
}

int BJob::error() const
{
    return d_func()->error;
}

QString BJob::errorText() const
{
    return d_func()->errorText;
}

QString BJob::errorString() const
{
    return d_func()->errorText;
}

qulonglong BJob::processedAmount(Unit unit) const
{
    return d_func()->processedAmount[unit];
}

qulonglong BJob::totalAmount(Unit unit) const
{
    return d_func()->totalAmount[unit];
}

unsigned long BJob::percent() const
{
    return d_func()->percentage;
}

bool BJob::isFinished() const
{
    return d_func()->isFinished;
}

void BJob::setError(int errorCode)
{
    Q_D(BJob);
    d->error = errorCode;
}

void BJob::setErrorText(const QString &errorText)
{
    Q_D(BJob);
    d->errorText = errorText;
}

void BJob::setProcessedAmount(Unit unit, qulonglong amount)
{
    Q_D(BJob);
    bool should_emit = (d->processedAmount[unit] != amount);

    d->processedAmount[unit] = amount;

    if (should_emit) {
        Q_EMIT processedAmount(this, unit, amount);
        if (unit == d->progressUnit) {
            Q_EMIT processedSize(this, amount);
            emitPercent(d->processedAmount[unit], d->totalAmount[unit]);
        }
    }
}

void BJob::setTotalAmount(Unit unit, qulonglong amount)
{
    Q_D(BJob);
    bool should_emit = (d->totalAmount[unit] != amount);

    d->totalAmount[unit] = amount;

    if (should_emit) {
        Q_EMIT totalAmount(this, unit, amount);
        if (unit == d->progressUnit) {
            Q_EMIT totalSize(this, amount);
            emitPercent(d->processedAmount[unit], d->totalAmount[unit]);
        }
    }
}

void BJob::setPercent(unsigned long percentage)
{
    Q_D(BJob);
    if (d->percentage != percentage) {
        d->percentage = percentage;
        Q_EMIT percent(this, percentage);
    }
}

void BJob::emitResult()
{
    if (!d_func()->isFinished) {
        finishJob(true);
    }
}

void BJob::emitPercent(qulonglong processedAmount, qulonglong totalAmount)
{
    Q_D(BJob);
    // calculate percents
    if (totalAmount) {
        unsigned long oldPercentage = d->percentage;
        d->percentage = 100.0 * processedAmount / totalAmount;
        if (d->percentage != oldPercentage) {
            Q_EMIT percent(this, d->percentage);
        }
    }
}

void BJob::emitSpeed(unsigned long value)
{
    Q_D(BJob);
    if (!d->speedTimer) {
        d->speedTimer = new QTimer(this);
        connect(d->speedTimer, SIGNAL(timeout()), SLOT(_k_speedTimeout()));
    }

    Q_EMIT speed(this, value);
    d->speedTimer->start(5000);   // 5 seconds interval should be enough
}


bool BJob::isAutoDelete() const
{
    Q_D(const BJob);
    return d->isAutoDelete;
}

void BJob::setAutoDelete(bool autodelete)
{
    Q_D(BJob);
    d->isAutoDelete = autodelete;
}


#include "moc_bjob.cpp"
