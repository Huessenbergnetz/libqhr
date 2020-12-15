/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef QHR_ABSTRACTNAMFACTORY_H
#define QHR_ABSTRACTNAMFACTORY_H

#include <QObject>
#include "qhr_global.h"

class QNetworkAccessManager;

namespace QHR {

class QHR_LIBRARY AbstractNamFactory
{
public:
    virtual ~AbstractNamFactory();

    virtual QNetworkAccessManager *create(QObject *parent) = 0;
};

}

#endif // QHR_ABSTRACTNAMFACTORY_H
