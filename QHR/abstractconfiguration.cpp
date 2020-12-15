/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "abstractconfiguration.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUrl>

using namespace QHR;

AbstractConfiguration::AbstractConfiguration(QObject *parent) : QObject(parent)
{

}

AbstractConfiguration::~AbstractConfiguration() = default;

void AbstractConfiguration::setUsername(const QString &username)
{
    Q_UNUSED(username)
}

void AbstractConfiguration::setPassword(const QString &password)
{
    Q_UNUSED(password);
}

QString AbstractConfiguration::userAgent() const
{
    return QStringLiteral("Libqhr %1").arg(QStringLiteral(QHR_VERSION));
}

#include "moc_abstractconfiguration.cpp"
