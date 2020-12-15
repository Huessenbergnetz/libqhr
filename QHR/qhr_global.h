/*
 * SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef QHR_GLOBAL_H
#define QHR_GLOBAL_H

#include <QtCore/QtGlobal>

/*!
 * \namespace QHR
 * \brief The root namespace for libqhr.
 */

#if defined(qhr_EXPORTS)
#  define QHR_LIBRARY Q_DECL_EXPORT
#else
#  define QHR_LIBRARY Q_DECL_IMPORT
#endif

#endif // QHR_GLOBAL_H
