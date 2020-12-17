/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "job_p.h"
#include "abstractnamfactory.h"
#include <QReadWriteLock>
#include <QGlobalStatic>
#include <QNetworkAccessManager>
#include <QJsonParseError>
#include <QSslError>

#if defined(QT_DEBUG)
Q_LOGGING_CATEGORY(qhrCore, "qhr.core")
#else
Q_LOGGING_CATEGORY(qhrCore, "qhr.core", QtInfoMsg)
#endif

using namespace QHR;

class DefaultValues
{
public:
    mutable QReadWriteLock lock;

    AbstractConfiguration *configuration() const
    {
        return m_configuration;
    }

    void setConfiguration(AbstractConfiguration *config)
    {
        m_configuration = config;
    }

    AbstractNamFactory *namFactory() const
    {
        return m_namFactory;
    }

    void setNamFactory(AbstractNamFactory *factory)
    {
        m_namFactory = factory;
    }

private:
    AbstractConfiguration *m_configuration = nullptr;
    AbstractNamFactory *m_namFactory = nullptr;
};
Q_GLOBAL_STATIC(DefaultValues, defVals)

AbstractConfiguration *QHR::defaultConfiguration()
{
    const DefaultValues *defs = defVals();
    Q_ASSERT(defs);

    defs->lock.lockForRead();
    AbstractConfiguration *config = defs->configuration();
    defs->lock.unlock();

    return config;
}

void QHR::setDefaultConfiguration(AbstractConfiguration *configuration)
{
    DefaultValues *defs = defVals();
    Q_ASSERT(defs);
    QWriteLocker locker(&defs->lock);
    qCDebug(qhrCore) << "Setting defaultConfiguration to" << configuration;
    defs->setConfiguration(configuration);
}

AbstractNamFactory *QHR::networkAccessManagerFactory()
{
    const DefaultValues *defs = defVals();
    Q_ASSERT(defs);

    defs->lock.lockForRead();
    AbstractNamFactory *factory = defs->namFactory();
    defs->lock.unlock();

    return factory;
}

void QHR::setNetworkAccessManagerFactory(AbstractNamFactory *factory)
{
    DefaultValues *defs = defVals();
    Q_ASSERT(defs);
    QWriteLocker locker(&defs->lock);
    qCDebug(qhrCore) << "Setting networkAccessManagerFactory to" << factory;
    defs->setNamFactory(factory);
}

JobPrivate::JobPrivate(Job *parent)
    :q_ptr(parent)
{

}

JobPrivate::~JobPrivate() = default;

void JobPrivate::handleSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_Q(Job);
    q->setError(NetworkError);
    if (!errors.empty()) {
        q->setErrorText(errors.first().errorString());
    } else {
        //: Error message
        //% "Can not perform API request. An unknown SSL error has occured."
        q->setErrorText(qtTrId("libqhr-error-unknown-ssl"));
    }
    qCCritical(qhrCore) << "SSL error:" << errors.first().errorString();
    reply->abort();
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
void JobPrivate::requestTimedOut()
{
    Q_Q(Job);
    QNetworkReply *nr = reply;
    reply = nullptr;
    delete nr;

    q->setError(RequestTimedOut);
    q->setErrorText(QString::number(requestTimeout));
    q->emitResult();
}
#endif

void JobPrivate::requestFinished()
{
    Q_Q(Job);

    //: Job info message to display state information
    //% "Checking reply"
    Q_EMIT q->infoMessage(q, qtTrId("libqhr-info-msg-req-checking"));
    qCDebug(qhrCore) << "Request finished, checking reply.";
    qCDebug(qhrCore) << "HTTP status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    const QByteArray replyData = reply->readAll();

    qCDebug(qhrCore) << "Reply data:" << replyData;

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    if (Q_LIKELY(timeoutTimer && timeoutTimer->isActive())) {
        qCDebug(qhrCore) << "Stopping request timeout timer with" << timeoutTimer->remainingTime()/1000 << "seconds left.";
        timeoutTimer->stop();
    }
#endif

    if (Q_LIKELY(reply->error() == QNetworkReply::NoError)) {
        if (checkOutput(replyData)) {
            successCallback(replyData);
            Q_EMIT q->succeeded(jsonResult);
        } else {
            Q_EMIT q->failed(q->error(), q->errorString());
        }
    } else {
        extractError();
        Q_EMIT q->failed(q->error(), q->errorString());
    }

    reply->deleteLater();
    reply = nullptr;

    q->emitResult();
}

void JobPrivate::extractError()
{
    Q_ASSERT(reply);
    Q_Q(Job);
    if (q->error() == BJob::NoError) {
        qCCritical(qhrCore) << "Network error:" << reply->errorString();
        q->setError(NetworkError);
        q->setErrorText(reply->errorString());
    }
}

void JobPrivate::successCallback(const QByteArray &replyData)
{

}

void JobPrivate::emitError(int errorCode, const QString &errorText)
{
    Q_Q(Job);
    q->setError(errorCode);
    q->setErrorText(errorText);
    q->emitResult();
    Q_EMIT q->failed(errorCode, q->errorString());
}

QString JobPrivate::buildUrlPath() const
{
    return QString();
}

QUrlQuery JobPrivate::buildUrlQuery() const
{
    return QUrlQuery();
}

QMap<QByteArray, QByteArray> JobPrivate::buildRequestHeaders() const
{
    return QMap<QByteArray, QByteArray>();
}

std::pair<QByteArray, QByteArray> JobPrivate::buildPayload() const
{
    return std::make_pair(QByteArray(), QByteArray());
}

bool JobPrivate::checkInput()
{
    if (Q_UNLIKELY(requiresAuth && configuration->username().isEmpty())) {
        emitError(MissingUser);
        qCCritical(qhrCore) << "Can not send request: missing username.";
        return false;
    }

    if (Q_UNLIKELY(requiresAuth && configuration->password().isEmpty())) {
        emitError(MissingPassword);
        qCCritical(qhrCore) << "Can not send request: missing password.";
        return false;
    }

    return true;
}

bool JobPrivate::checkOutput(const QByteArray &data)
{
    Q_Q(Job);

    if (expectedContentType != ExpectedContentType::Empty && data.isEmpty()) {
        q->setError(EmptyReply);
        qCCritical(qhrCore) << "Invalid reply: content expected, but reply is empty.";
        return false;
    }

    if (expectedContentType == ExpectedContentType::JsonArray || expectedContentType == ExpectedContentType::JsonObject) {
        QJsonParseError jsonError;
        jsonResult = QJsonDocument::fromJson(data, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            q->setError(JsonParseError);
            q->setErrorText(jsonError.errorString());
            qCCritical(qhrCore) << "Invalid JSON data in reply at offset" << jsonError.offset << ":" << jsonError.errorString();
            return false;
        }
    }

    if ((expectedContentType == ExpectedContentType::JsonArray || expectedContentType == ExpectedContentType::JsonObject) && (jsonResult.isNull() || jsonResult.isEmpty())) {
        q->setError(EmptyJson);
        qCCritical(qhrCore) << "Invalid reply: content expected, but reply is empty.";
        return false;
    }

    if (expectedContentType == ExpectedContentType::JsonArray && !jsonResult.isArray()) {
        q->setError(WrongOutputType);
        qCCritical(qhrCore) << "Invaid reply: JSON array expected, but got something different.";
        return false;
    }

    if (expectedContentType == ExpectedContentType::JsonObject && !jsonResult.isObject()) {
        q->setError(WrongOutputType);
        qCCritical(qhrCore) << "Invaid reply: JSON object expected, but got something different.";
        return false;
    }

    return true;
}

void JobPrivate::emitDescription()
{

}

Job::Job(QObject *parent)
    : BJob(parent), bd_ptr(new JobPrivate(this))
{

}

Job::Job(JobPrivate &dd, QObject *parent)
    : BJob(parent), bd_ptr(&dd)
{

}

Job::~Job() = default;

void Job::sendRequest()
{
    Q_D(Job);

    d->emitDescription();

    //: Job info message to display state information
    //% "Setting up request"
    Q_EMIT infoMessage(this, qtTrId("libqhr-info-msg-req-setup"));
    qCDebug(qhrCore) << "Setting up network request.";

    if (!d->configuration) {
        d->configuration = QHR::defaultConfiguration();
        if (d->configuration) {
            qCDebug(qhrCore) << "Using default configuration" << d->configuration;
            Q_EMIT configurationChanged(d->configuration);
        } else {
            d->emitError(MissingConfig);
            qCCritical(qhrCore) << "Can not send request: missing configuration.";
            return;
        }
    }

    if (Q_UNLIKELY(!d->checkInput())) {
        return;
    }

    QUrl url;
    url.setScheme(QStringLiteral("https"));

    url.setHost(QStringLiteral("robot-ws.your-server.de"));
    url.setPath(d->buildUrlPath());
    url.setQuery(d->buildUrlQuery());

    if (Q_UNLIKELY(!url.isValid())) {
        d->emitError(InvalidRequestUrl, url.toString());
        return;
    }

    if (!d->nam) {

        auto namf = QHR::networkAccessManagerFactory();
        if (namf) {
            d->nam = namf->create(this);
        } else {
            d->nam = new QNetworkAccessManager(this);
            qCDebug(qhrCore) << "Using default created" << d->nam;
        }

        connect(d->nam, &QNetworkAccessManager::sslErrors, this, [d](QNetworkReply *reply, const QList<QSslError> &errors){
            d->handleSslErrors(reply, errors);
        });
    }

    QNetworkRequest nr(url);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    if (Q_LIKELY(d->requestTimeout > 0)) {
        nr.setTransferTimeout(static_cast<int>(d->requestTimeout) * 1000);
    }
#endif

    nr.setRawHeader(QByteArrayLiteral("User-Agent"), d->configuration->userAgent().toUtf8());

    switch (d->expectedContentType) {
    case ExpectedContentType::JsonObject:
    case ExpectedContentType::JsonArray:
        nr.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json"));
        break;
    case ExpectedContentType::Invalid:
        Q_ASSERT_X(false, "sending request", "invalid exepected content type");
        break;
    default:
        break;
    }

    const QMap<QByteArray, QByteArray> reqHeaders = d->buildRequestHeaders();
    if (!reqHeaders.empty()) {
        QMap<QByteArray, QByteArray>::const_iterator i = reqHeaders.constEnd();
        while (i != reqHeaders.constEnd()) {
            nr.setRawHeader(i.key(), i.value());
            ++i;
        }
    }

    const auto payload = d->buildPayload();

    if (!payload.second.isEmpty()) {
        nr.setRawHeader(QByteArrayLiteral("Content-Type"), payload.second);
    }

    if (d->requiresAuth) {
        const QString auth = d->configuration->username() + QLatin1Char(':') + d->configuration->password();
        const QByteArray authHeader = QByteArrayLiteral("Basic ") + auth.toUtf8().toBase64();
        nr.setRawHeader(QByteArrayLiteral("Authorization"), authHeader);
    }

    if (qhrCore().isDebugEnabled()) {
        QString opName;
        switch(d->namOperation) {
        case NetworkOperation::Head:
            opName = QStringLiteral("HEAD");
            break;
        case NetworkOperation::Post:
            opName = QStringLiteral("POST");
            break;
        case NetworkOperation::Put:
            opName = QStringLiteral("PUT");
            break;
        case NetworkOperation::Delete:
            opName = QStringLiteral("DELETE");
            break;
        case NetworkOperation::Get:
            opName = QStringLiteral("GET");
            break;
        default:
            Q_ASSERT_X(false, "sending request", "invalid network operation");
            break;
        }
        qCDebug(qhrCore) << "Start performing" << opName << "network operation.";
        qCDebug(qhrCore) << "API URL:" << url;
        const auto rhl = nr.rawHeaderList();
        for (const QByteArray &h : rhl) {
            if (h == QByteArrayLiteral("Authorization")) {
                qCDebug(qhrCore) << h << ":" << "**************";
            } else {
                qCDebug(qhrCore) << h << ":" << nr.rawHeader(h);
            }
        }
        if (!payload.first.isEmpty()) {
            qCDebug(qhrCore) << "Payload:" << payload.first;
        }
    }

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    if (Q_LIKELY(d->requestTimeout > 0)) {
        if (!d->timeoutTimer) {
            d->timeoutTimer = new QTimer(this);
            d->timeoutTimer->setSingleShot(true);
            d->timeoutTimer->setTimerType(Qt::VeryCoarseTimer);
            connect(d->timeoutTimer, &QTimer::timeout, this, [d](){
                d->requestTimedOut();
            });
        }
        d->timeoutTimer->start(static_cast<int>(d->requestTimeout) * 1000);
        qCDebug(qhrCore) << "Started request timeout timer with" << d->requestTimeout << "seconds.";
    }
#endif

    //: Job info message to display state information
    //% "Sending request"
    Q_EMIT infoMessage(this, qtTrId("libqhr-info-msg-req-send"));
    qCDebug(qhrCore) << "Sending network request.";

    switch(d->namOperation) {
    case NetworkOperation::Head:
        d->reply = d->nam->head(nr);
        break;
    case NetworkOperation::Post:
        d->reply = d->nam->post(nr, payload.first);
        break;
    case NetworkOperation::Put:
        d->reply = d->nam->put(nr, payload.first);
        break;
    case NetworkOperation::Delete:
        d->reply = d->nam->deleteResource(nr);
        break;
    case NetworkOperation::Get:
        d->reply = d->nam->get(nr);
        break;
    default:
        Q_ASSERT_X(false, "sending request", "invalid network operation");
        break;
    }

    connect(d->reply, &QNetworkReply::finished, this, [d](){
        d->requestFinished();
    });
}

AbstractConfiguration* Job::configuration() const
{
    Q_D(const Job);
    return d->configuration;
}

void Job::setConfiguration(AbstractConfiguration *configuration)
{
    Q_D(Job);
    if (configuration != d->configuration) {
        qCDebug(qhrCore) << "Changing configuration from" << d->configuration << "to" << configuration;
        d->configuration = configuration;
        Q_EMIT configurationChanged(d->configuration);
    }
}

QString Job::errorString() const
{
    switch (error()) {
    case MissingConfig:
        //: Error message
        //% "No configuration set."
        return qtTrId("libqhr-error-missing-config");
    case MissingUser:
        //: Error message
        //% "Missing username."
        return qtTrId("libqhr-error-missing-user");
    case MissingPassword:
        //: Error message
        //% "Missing user password."
        return qtTrId("libqhr-error-missing-password");
    case InvalidRequestUrl:
        //: Error message, %1 will be the invalid URL string.
        //% "The URL (%1) generated to perform the request is not valid, please check your input values."
        return qtTrId("libqhr-error-invalid-req-url").arg(errorText());
    case JsonParseError:
        //: Error message, %1 will be the JSON parser error string.
        //% "Failed to parse the received JSON data: %1"
        return qtTrId("libqhr-error-json-parser").arg(errorText());
    case WrongOutputType:
        //: Error message
        //% "Unexpected JSON type in received data."
        return qtTrId("libqhr-error-invalid-output-type");
    case EmptyJson:
    case EmptyReply:
        //: Error message
        //% "Unexpected empty reply data."
        return qtTrId("libqhr-error-empty-json");
    case NetworkError:
        return errorText();
    default:
        //: Error message
        //% "Sorry, but unfortunately an unknown error has occurred."
        return qtTrId("libqhr-error-unknown");
    }
}

QJsonDocument Job::result() const
{
    Q_D(const Job);
    return d->jsonResult;
}

#include "moc_job.cpp"
