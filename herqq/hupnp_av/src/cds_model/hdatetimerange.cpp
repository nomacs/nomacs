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

#include "hdatetimerange.h"

#include <QtCore/QDateTime>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HDateTimeRange>(
        "Herqq::Upnp::Av::HDateTimeRange");

    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HDateTimeRangePrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HDateTimeRangePrivate)

public:

    QDateTime m_startTime, m_endTime;
    HDaylightSaving m_dlSaving;

    HDateTimeRangePrivate() :
        m_startTime(), m_endTime(), m_dlSaving(Unknown_DaylightSaving)
    {
    }
};

HDateTimeRange::HDateTimeRange() :
    h_ptr(new HDateTimeRangePrivate())
{
}

HDateTimeRange::HDateTimeRange(const QString& value, HDaylightSaving dlSaving) :
    h_ptr(new HDateTimeRangePrivate())
{
    int index = value.indexOf('/');
    if (index > 0)
    {
        QDateTime start = QDateTime::fromString(value.left(index), Qt::ISODate);
        if (start.isValid())
        {
            QDateTime end = QDateTime::fromString(value.mid(index+1), Qt::ISODate);
            if (end.isValid())
            {
                h_ptr->m_startTime = start;
                h_ptr->m_endTime = end;
                h_ptr->m_dlSaving = dlSaving;
            }
        }
    }
}

HDateTimeRange::HDateTimeRange(const HDateTimeRange& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HDateTimeRange& HDateTimeRange::operator=(const HDateTimeRange& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HDateTimeRange::~HDateTimeRange()
{
}

bool HDateTimeRange::isValid() const
{
    return h_ptr->m_startTime.isValid() && h_ptr->m_endTime.isValid();
}

QDateTime HDateTimeRange::startTime() const
{
    return h_ptr->m_startTime;
}

QDateTime HDateTimeRange::endTime() const
{
    return h_ptr->m_endTime;
}

HDaylightSaving HDateTimeRange::daylightSaving() const
{
    return h_ptr->m_dlSaving;
}

QString HDateTimeRange::toString() const
{
    if (!isValid())
    {
        return QString();
    }
    return QString("%1/%2").arg(
            startTime().toString(Qt::ISODate), endTime().toString(Qt::ISODate));
}

void HDateTimeRange::setStartTime(const QDateTime& value)
{
    h_ptr->m_startTime = value;
}

void HDateTimeRange::setEndTime(const QDateTime& value)
{
    h_ptr->m_endTime = value;
}

void HDateTimeRange::setDaylightSaving(HDaylightSaving dlSaving)
{
    h_ptr->m_dlSaving = dlSaving;
}

bool operator==(const HDateTimeRange& obj1, const HDateTimeRange& obj2)
{
    return obj1.toString() == obj2.toString() &&
           obj1.daylightSaving() == obj2.daylightSaving();
}

}
}
}

