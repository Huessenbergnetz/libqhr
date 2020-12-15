/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef QHR_ABSTRACTCONFIGURATION_H
#define QHR_ABSTRACTCONFIGURATION_H

#include <QObject>
#include "qhr_global.h"
#include "logging.h"

class QUrl;
class QJsonDocument;
class QJsonObject;

namespace QHR {

/*!
 * \brief Stores configuratoin for API requests.
 *
 * This class stores information used to perform API requests, especially the
 * url of the remote host and the username and pasword. Reimplement this class
 * to provide these information.
 *
 * All API classes have a configuration property that takes subclasses of AbstractConfiguration.
 * There is also QHR::setDefaultConfiguration() to set a global default configuration
 * that will be used if no local configuration is available.
 *
 * \headerfile "" <QHR/AbstractConfiguration>
 */
class QHR_LIBRARY AbstractConfiguration : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new %AbstractConfiguratoin object with the given \a parent.
     */
    explicit AbstractConfiguration(QObject *parent = nullptr);

    /*!
     * \brief Deconstructs the %AbstractConfiguration object.
     */
    ~AbstractConfiguration() override;

    /*!
     * \brief Returns the username.
     *
     * Reimplement this to return the username used for authentication.
     *
     * \sa setUsername()
     */
    virtual QString username() const = 0;

    /*!
     * \brief Set the username.
     *
     * Reimplement this to set the \a username used for authentication.
     * The default implementation does nothing.
     *
     * \sa username()
     */
    virtual void setUsername(const QString &username);

    /*!
     * \brief Returns the password.
     *
     * Reimplement this to return the password used for authentication.
     *
     * \sa setPassword()
     */
    virtual QString password() const = 0;

    /*!
     * \brief Sets the password.
     *
     * Reimplement this to set the \a password used for authentication.
     * The default implementation does nothing.
     *
     * \sa password()
     */
    virtual void setPassword(const QString &password);

    /*!
     * \brief Returns a user agent string.
     *
     * Reimplement this to return a custom user agent string that describes
     * your application. The default implementation returns "Libqhr $VERSION".
     */
    Q_INVOKABLE virtual QString userAgent() const;

private:
    Q_DISABLE_COPY(AbstractConfiguration)
};

}

#endif // QHR_ABSTRACTCONFIGURATION_H
