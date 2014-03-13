/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hduration.h"

#include <QtCore/QStringList>
#include <QtCore/QDateTime>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HDurationPrivate
 ******************************************************************************/
class HDurationPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HDurationPrivate)

public:

    QString m_duration;
    qint32 m_hours, m_minutes, m_seconds;
    qreal m_fractions;
    bool m_positive;

    HDurationPrivate() :
        m_duration("00:00:00"),
        m_hours(0), m_minutes(0), m_seconds(0), m_fractions(0),
        m_positive(true)
    {
    }
};

/*******************************************************************************
 * HDuration
 ******************************************************************************/
HDuration::HDuration() :
    h_ptr(new HDurationPrivate())
{
}

namespace
{
qreal calculateFraction(const QString& arg, bool* ok)
{
    qreal retVal = 0;

    int index = arg.indexOf('.');
    int index2 = arg.indexOf('/');
    if (index > 0)
    {
        retVal = arg.mid(index+1, index2-(index+1)).toDouble(ok);
        if (*ok && retVal > 0)
        {
            if (index2 > 0)
            {
                double divisor = arg.mid(index2+1).toDouble(ok);
                if (*ok && divisor > retVal)
                {
                    retVal /= divisor;
                }
                else
                {
                    *ok = false;
                }
            }
        }
    }

    return retVal;
}
}

HDuration::HDuration(const QString& arg) :
    h_ptr(new HDurationPrivate())
{
    QString trimmed = arg.trimmed();
    QStringList tmp = trimmed.split(":");
    if (tmp.size() != 3)
    {
        h_ptr->m_duration = QString("00:00:00");
        return;
    }
    else if (tmp.at(0).startsWith("-"))
    {
        h_ptr->m_positive = false;
        tmp.first().remove(0, 1);
    }

    bool ok = false;
    qint32 hours = tmp.at(0).toInt(&ok);
    if (ok)
    {
        qint32 minutes = tmp.at(1).toInt(&ok);
        if (ok)
        {
            int index = tmp.at(2).indexOf('.');
            qint32 seconds = tmp.at(2).mid(0, index).toInt(&ok);
            if (ok)
            {
                qreal fractions = calculateFraction(tmp.at(2), &ok);
                if (ok)
                {
                    h_ptr->m_fractions = fractions;
                }

                h_ptr->m_seconds = seconds;
                h_ptr->m_duration = trimmed;
                h_ptr->m_hours = hours;
                h_ptr->m_minutes = minutes;
            }
        }
    }
}

HDuration::~HDuration()
{
}

HDuration::HDuration(const HDuration& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(this != &other);
}

HDuration& HDuration::operator=(const HDuration& other)
{
    Q_ASSERT(this != &other);
    h_ptr = other.h_ptr;
    return *this;
}

HDuration::HDuration(const QTime& time) :
    h_ptr(new HDurationPrivate())
{
    h_ptr->m_hours = time.hour();
    h_ptr->m_minutes = time.minute();
    h_ptr->m_seconds = time.second();
    h_ptr->m_duration = time.toString();
}

qint32 HDuration::hours() const
{
    return h_ptr->m_hours;
}

qint32 HDuration::minutes() const
{
    return h_ptr->m_minutes;
}

qint32 HDuration::seconds() const
{
    return h_ptr->m_seconds;
}

qreal HDuration::fractionsOfSecond() const
{
    return h_ptr->m_fractions;
}

bool HDuration::isPositive() const
{
    return h_ptr->m_positive;
}

bool HDuration::isZero() const
{
    return hours() == 0 && minutes() == 0 && seconds() == 0 &&
           fractionsOfSecond() == 0;
}

QString HDuration::toString() const
{
    return QString("%1%2").arg(!h_ptr->m_positive ? "-" : "", h_ptr->m_duration);
}

QTime HDuration::toTime() const
{
    return QTime(hours(), minutes(), seconds());
}

bool operator==(const HDuration& obj1, const HDuration& obj2)
{
    return obj1.h_ptr->m_hours == obj2.h_ptr->m_hours &&
           obj1.h_ptr->m_minutes == obj2.h_ptr->m_minutes &&
           obj1.h_ptr->m_seconds == obj2.h_ptr->m_seconds &&
           obj1.h_ptr->m_fractions == obj2.h_ptr->m_fractions &&
           obj1.h_ptr->m_positive == obj2.h_ptr->m_positive;
}

}
}
}
