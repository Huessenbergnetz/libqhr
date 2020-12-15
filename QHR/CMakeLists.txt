# SPDX-FileCopyrightText: (C) 2020 Matthias Fehring / www.huessenbergnetz.de
# SPDX-License-Identifier: LGPL-3.0-or-later

set(qhr_HEADERS
    qhr_global.h
    job.h
    logging.h
    abstractconfiguration.h
    AbstractConfiguration
    abstractnamfactory.h
    AbstractConfiguration
    getserversjob.h
    GetServersJob
)

set(qhr_SRCS
    job.cpp
    job_p.h
    abstractconfiguration.cpp
    abstractnamfactory.cpp
    getserversjob.cpp
    getserversjob_p.h
)

if (NOT WITH_KDE)
    list(APPEND qhr_HEADERS bjob.h)
    list(APPEND qhr_SRCS bjob.cpp bjob_p.h)
endif (NOT WITH_KDE)

add_library(qhr
    ${qhr_HEADERS}
    ${qhr_SRCS}
)

add_library(QHRQt${QTVERMAJ}::Core ALIAS qhr)

if(CMAKE_VERSION GREATER_EQUAL "3.16.0")
    target_precompile_headers(qhr
        PRIVATE
            <QString>
            <QStringList>
            <QTimer>
            <QJsonDocument>
            <QJsonObject>
            <QJsonArray>
            <QJsonValue>
    )
endif(CMAKE_VERSION GREATER_EQUAL "3.16.0")

target_compile_features(qhr PUBLIC cxx_std_14)

target_link_libraries(qhr
    PUBLIC
        Qt5::Core
    PRIVATE
        Qt5::Network
)

target_compile_definitions(qhr
    PRIVATE
        QT_NO_KEYWORDS
        QT_NO_CAST_TO_ASCII
        QT_NO_CAST_FROM_ASCII
        QT_STRICT_ITERATORS
        QT_NO_URL_CAST_FROM_STRING
        QT_NO_CAST_FROM_BYTEARRAY
        QT_USE_QSTRINGBUILDER
        QT_NO_SIGNALS_SLOTS_KEYWORDS
        QT_USE_FAST_OPERATOR_PLUS
        QT_DISABLE_DEPRECATED_BEFORE=0x050500
        QHR_VERSION="${PROJECT_VERSION}"
)

if (WITH_KDE)
    message(STATUS "KDE support enabled")
    target_compile_definitions(qhr
        PUBLIC
            QHR_WITH_KDE
    )
    target_link_libraries(qhr
        PUBLIC
            KF5::CoreAddons
    )
endif (WITH_KDE)

if(ENABLE_MAINTAINER_FLAGS)
    target_compile_definitions(qhr
        PRIVATE
            QT_DEPRECATED_WARNINGS
    )
    target_compile_options(qhr
        PRIVATE
            -Wall
            -Wcast-align
            -Wno-uninitialized
            -Wempty-body
            -Wformat-security
            -Wformat
            -Winit-self
    )
endif(ENABLE_MAINTAINER_FLAGS)

target_include_directories(qhr PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qhr-qt${QTVERMAJ}>
)

set_target_properties(qhr PROPERTIES
    EXPORT_NAME Core
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME qhrqt${QTVERMAJ}
    PUBLIC_HEADER "${qhr_HEADERS}"
)

install(TARGETS qhr
    EXPORT QHRTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libs
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qhr-qt${QTVERMAJ}/QHR COMPONENT devel
)
