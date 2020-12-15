/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "getserversjob_p.h"
#include <QTimer>

using namespace QHR;

GetServersJobPrivate::GetServersJobPrivate(GetServersJob *q)
    : JobPrivate(q)
{
    namOperation = NetworkOperation::Get;
    expectedContentType = ExpectedContentType::JsonArray;
}

GetServersJobPrivate::~GetServersJobPrivate() = default;

QString GetServersJobPrivate::buildUrlPath() const
{
    return QStringLiteral("/server");
}

void GetServersJobPrivate::emitDescription()
{
    //% Job title
    //: "Getting servers"
    const QString _title = qtTrId("libqhr-job-desc-get-servers-title");

    Q_Q(GetServersJob);
    Q_EMIT q->description(q, _title);
}

void GetServersJobPrivate::extractError()
{
    Q_ASSERT(reply);
    Q_Q(GetServersJob);
    const int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpStatusCode == 404) {
        q->setError(NotFound);
        qCWarning(qhrCore) << "No servers found.";
    }
    JobPrivate::extractError();
}

GetServersJob::GetServersJob(QObject *parent)
    : Job(* new GetServersJobPrivate(this), parent)
{
    qCDebug(qhrCore) << "Creating new" << this;
}

GetServersJob::~GetServersJob() = default;

void GetServersJob::start()
{
    QTimer::singleShot(0, this, &GetServersJob::sendRequest);
}

QString GetServersJob::errorString() const
{
    if (error() == NotFound) {
        //% Error message if no servers have been found.
        //: "No servers found."
        return qtTrId("libqhr-error-get-servers-not-found");
    } else {
        return Job::errorString();
    }
}

#include "moc_getserversjob.cpp"
