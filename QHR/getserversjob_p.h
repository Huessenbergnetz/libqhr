/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef QHR_GETSERVERSJOB_P_H
#define QHR_GETSERVERSJOB_P_H

#include "getserversjob.h"
#include "job_p.h"

namespace QHR {

class GetServersJobPrivate : public JobPrivate
{
public:
    explicit GetServersJobPrivate(GetServersJob *q);
    ~GetServersJobPrivate() override;

    QString buildUrlPath() const override;

    void emitDescription() override;

    void extractError() override;

private:
    Q_DISABLE_COPY(GetServersJobPrivate)
    Q_DECLARE_PUBLIC(GetServersJob)
};

}

#endif // QHR_GETSERVERSJOB_P_H
