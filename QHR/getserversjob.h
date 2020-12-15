/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef QHR_GETSERVERSJOB_H
#define QHR_GETSERVERSJOB_H

#include <QObject>
#include "qhr_global.h"
#include "job.h"

namespace QHR {

class GetServersJobPrivate;

/*!
 * \brief Gets a list of servers.
 *
 * After setting the mandatory properties, call start() to perform the request.
 *
 * \par Mandatory properties
 * \li Job::configuratoin
 *
 * \par API method
 * GET
 *
 * \par API route
 * /servers
 *
 * \par API docs
 * https://robot.your-server.de/doc/webservice/de.html#get-server
 *
 * \headerfile "" <QHR/GetServersJob>
 */
class GetServersJob : public Job
{
    Q_OBJECT
public:
    /*!
     * \brief Creates a new %GetServersJob object with the given \a parent.
     */
    explicit GetServersJob(QObject *parent = nullptr);

    /*!
     * \brief Destroys the %GetServersJob object.
     */
    ~GetServersJob();

    /*!
     * \brief Starts the job asynchronously.
     */
    void start() override;

    /*!
     * \brief Returns a human readable and translated error string.
     *
     * If BJob::error() returns not \c 0, an error has occured and the human readable
     * description can be returned by this function.
     */
    QString errorString() const override;

private:
    Q_DECLARE_PRIVATE_D(bd_ptr, GetServersJob)
    Q_DISABLE_COPY(GetServersJob)
};

}

#endif // QHR_GETSERVERSJOB_H
