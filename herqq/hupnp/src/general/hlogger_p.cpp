/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hlogger_p.h"

#include <QtCore/QtDebug>
#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace Herqq
{

volatile int HLogger::s_logLevel = static_cast<qint32>(Critical);
volatile bool HLogger::s_nonStdWarningsEnabled = true;

HLogger::HLogger() :
    m_methodName(0), m_logPrefix(0)
{
}

HLogger::HLogger(const char* at, const char* methodName, const char* logPrefix) :
    m_methodName(methodName), m_logPrefix(logPrefix)
{
    if (traceLevel() == All)
    {
        QString stmt = (m_logPrefix ? QString(m_logPrefix) : QString()).append(
            QString("Entering %1 @ %2").arg(m_methodName, at));

        qDebug() << stmt;
    }
}

HLogger::~HLogger()
{
    if (traceLevel() == All)
    {
        QString stmt = (m_logPrefix ? QString(m_logPrefix) : QString()).append(
            QString("Exiting %1").arg(m_methodName));

        qDebug() << stmt;
    }
}

namespace
{
inline QString stmt(const char* prefix, const QString& text)
{
    return (prefix ? QString(prefix) : QString()).append(text);
}
}

void HLogger::logDebug(const QString& text)
{
    qDebug() << stmt(m_logPrefix, text);
}

void HLogger::logWarning(const QString& text)
{
    qWarning() << stmt(m_logPrefix, text);
}

void HLogger::logWarningNonStd(const QString& text)
{
    if (s_nonStdWarningsEnabled)
    {
        qWarning() << stmt(
            m_logPrefix, QString("**NON-STANDARD BEHAVIOR**: %1").arg(text));
    }
}

void HLogger::logInformation(const QString& text)
{
    qDebug() << stmt(m_logPrefix, text);
}

void HLogger::logFatal(const QString& text)
{
    qFatal("%s", stmt(m_logPrefix, text).toLocal8Bit().data());
}

void HLogger::logCritical(const QString& text)
{
    qCritical() << stmt(m_logPrefix, text);
}

void HLogger::logDebug_(const QString& text)
{
    if (traceLevel() >= Debug)
    {
        qDebug() << text;
    }
}

void HLogger::logWarning_(const QString& text)
{
    if (traceLevel() >= Warning)
    {
        qWarning() << text;
    }
}

void HLogger::logWarningNonStd_(const QString& text)
{
    if (traceLevel() && s_nonStdWarningsEnabled)
    {
        qWarning() << QString("**NON-STANDARD BEHAVIOR**: %1").arg(text);
    }
}

void HLogger::logInformation_(const QString& text)
{
    if (traceLevel() >= Information)
    {
        qDebug() << text;
    }
}

void HLogger::logCritical_(const QString& text)
{
    if (traceLevel() >= Critical)
    {
        qCritical() << text;
    }
}

void HLogger::logFatal_(const QString& text)
{
    if (traceLevel() >= Fatal)
    {
        qFatal("%s", text.toLocal8Bit().data());
    }
}

}
