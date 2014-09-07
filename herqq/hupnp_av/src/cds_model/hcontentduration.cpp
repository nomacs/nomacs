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

#include "hcontentduration.h"

#include <QtCore/QTime>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HContentDuration>(
        "Herqq::Upnp::Av::HContentDuration");

    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HContentDurationPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HContentDurationPrivate)
public:

    qint32 m_days;
    QTime m_time;

    HContentDurationPrivate() :
        m_days(0), m_time()
    {
    }
};

HContentDuration::HContentDuration() :
    h_ptr(new HContentDurationPrivate())
{
}

HContentDuration::HContentDuration(const QString& arg) :
    h_ptr(new HContentDurationPrivate())
{
    if (arg.size() >= 6 && arg[0] == 'P')
    {
        int days = 0;
        int iDays = arg.indexOf('D');
        if (iDays > 1)
        {
            bool ok = false;
            days = arg.mid(1, iDays-1).toInt(&ok);
            if (ok)
            {
                ++iDays;
            }
            else
            {
                iDays = -1;
            }
        }
        else
        {
            iDays = 1;
        }

        if (iDays > 0)
        {
            QTime tmp = QTime::fromString(arg.mid(iDays), Qt::ISODate);
            if (tmp.isValid())
            {
                h_ptr->m_time = tmp;
                h_ptr->m_days = days >= 0 ? days : -1;
            }
        }
    }
}

HContentDuration::HContentDuration(const HContentDuration& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HContentDuration& HContentDuration::operator =(const HContentDuration& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HContentDuration::~HContentDuration()
{
}

bool HContentDuration::isValid() const
{
    return h_ptr->m_time.isValid();
}

qint32 HContentDuration::days() const
{
    return h_ptr->m_days;
}

QTime HContentDuration::time() const
{
    return h_ptr->m_time;
}

QString HContentDuration::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    return QString("P%1%2").arg(
        days() > 1 ? QString::number(days()).append(QString('D')) : QString(),
        time().toString(Qt::ISODate));
}

void HContentDuration::setDays(qint32 arg)
{
    h_ptr->m_days = arg;
}

void HContentDuration::setTime(const QTime& arg)
{
    h_ptr->m_time = arg;
}

bool operator==(const HContentDuration& obj1, const HContentDuration& obj2)
{
    return obj1.toString() == obj2.toString();
}

}
}
}

