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

#include "hobject.h"
#include "hobject_p.h"
#include "hcontainer.h"
#include "hitem.h"

#include "../../common/hresource.h"
#include "../../common/hprotocolinfo.h"
#include "../model_mgmt/hcdsproperty.h"
#include "../model_mgmt/hcdsproperty_db.h"

#include <HUpnpCore/private/hlogger_p.h>
#include <HUpnpCore/private/hmisc_utils_p.h>

#include <QtCore/QMutex>
#include <QtCore/QXmlStreamWriter>

/*!
 * \defgroup hupnp_av_cds_objects CDS Object Model
 * \ingroup hupnp_av_cds
 *
 * \brief This page discusses the design and use of the HUPnPAv's CDS Object Model
 * functionality.
 */

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HObject::WriteStatus>(
        "Herqq::Upnp::Av::HObject::WriteStatus");

    return true;
}

static bool regMetaT = registerMetaTypes();

static unsigned int s_lastInt = 0;
static QMutex s_lastIntMutex;

namespace
{
inline unsigned int getNextId()
{
    unsigned int retVal;
    s_lastIntMutex.lock();
    retVal = ++s_lastInt;
    s_lastIntMutex.unlock();
    return retVal;
}
}

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HObjectEventInfoPrivate
 ******************************************************************************/
class HObjectEventInfoPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HObjectEventInfoPrivate)

public:

    QString m_variableName;
    QVariant m_oldValue;
    QVariant m_newValue;
    quint32 m_updateId;

    HObjectEventInfoPrivate() :
        m_variableName(), m_oldValue(), m_newValue(), m_updateId(0)
    {
    }
};

/*******************************************************************************
 * HObjectEventInfo
 ******************************************************************************/
HObjectEventInfo::HObjectEventInfo() :
    h_ptr(new HObjectEventInfoPrivate())
{
}

HObjectEventInfo::HObjectEventInfo(
    const QString& varName, const QVariant& oldValue, const QVariant& newValue,
    quint32 updateId) :
        h_ptr(new HObjectEventInfoPrivate())
{
    h_ptr->m_variableName = varName.trimmed();
    h_ptr->m_oldValue = oldValue;
    h_ptr->m_newValue = newValue;
    h_ptr->m_updateId = updateId;
}

HObjectEventInfo::HObjectEventInfo(const HObjectEventInfo& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HObjectEventInfo& HObjectEventInfo::operator=(const HObjectEventInfo& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HObjectEventInfo::~HObjectEventInfo()
{
}

bool HObjectEventInfo::isValid() const
{
    return !h_ptr->m_variableName.isEmpty();
}

QString HObjectEventInfo::variableName() const
{
    return h_ptr->m_variableName;
}

QVariant HObjectEventInfo::oldValue() const
{
    return h_ptr->m_oldValue;
}

QVariant HObjectEventInfo::newValue() const
{
    return h_ptr->m_newValue;
}

quint32 HObjectEventInfo::updateId() const
{
    return h_ptr->m_updateId;
}

void HObjectEventInfo::setVariableName(const QString& arg)
{
    h_ptr->m_variableName = arg.trimmed();
}

void HObjectEventInfo::setOldValue(const QVariant& arg)
{
    h_ptr->m_oldValue = arg;
}

void HObjectEventInfo::setNewValue(const QVariant& arg)
{
    h_ptr->m_newValue = arg;
}

void HObjectEventInfo::setUpdateId(quint32 arg)
{
    h_ptr->m_updateId = arg;
}

bool operator==(const HObjectEventInfo& obj1, const HObjectEventInfo& obj2)
{
    return obj1.variableName() == obj2.variableName() &&
           obj1.newValue() == obj1.newValue() &&
           obj1.oldValue() == obj2.oldValue() &&
           obj1.updateId() == obj2.updateId();
}

/*******************************************************************************
 * HObjectPrivate
 ******************************************************************************/
HObjectPrivate::HObjectPrivate(const QString& clazz, HObject::CdsType cdsType) :
    m_properties(),
    m_cdsType(cdsType),
    m_disabledProperties()
{
    Q_ASSERT(cdsType != HObject::UndefinedCdsType);
    Q_UNUSED(regMetaT)

    const HCdsProperties& inst = HCdsProperties::instance();
    insert(inst.get(HCdsProperties::dlite_id));
    insert(inst.get(HCdsProperties::dlite_parentId));
    insert(inst.get(HCdsProperties::dlite_restricted).name(), true);
    insert(inst.get(HCdsProperties::dlite_neverPlayable).name(), false);
    insert(inst.get(HCdsProperties::dc_title));
    insert(inst.get(HCdsProperties::dc_creator));
    insert(inst.get(HCdsProperties::upnp_class).name(), clazz);
    insert(inst.get(HCdsProperties::dlite_res));
    insert(inst.get(HCdsProperties::upnp_writeStatus));
    insert(inst.get(HCdsProperties::upnp_objectUpdateID).name(), 0U);
}

HObjectPrivate::~HObjectPrivate()
{
}

/*******************************************************************************
 * HObject
 ******************************************************************************/
HObject::HObject(const QString& clazz, CdsType cdsType) :
    h_ptr(new HObjectPrivate(clazz, cdsType))
{
    setTrackChangesOption(false);
}

HObject::HObject(HObjectPrivate& dd) :
    h_ptr(&dd)
{
    setTrackChangesOption(false);
}

HObject::~HObject()
{
    delete h_ptr;
}

HObject* HObject::clone() const
{
    return static_cast<HObject*>(HClonable::clone());
}

bool HObject::init(
    const QString& title, const QString& parentId, const QString& id)
{
    if (isInitialized())
    {
        return false;
    }

    setId(id.isEmpty() ? QString::number(getNextId()) : id);

    setParentId(parentId);
    setTitle(title);

    return true;
}

bool HObject::isInitialized() const
{
    return !id().isEmpty();
}

bool HObject::hasCdsProperty(const QString& property) const
{
    return h_ptr->m_properties.contains(property);
}

bool HObject::hasCdsProperty(HCdsProperties::Property property) const
{
    return h_ptr->m_properties.contains(HCdsProperties::instance().get(property).name());
}

bool HObject::isCdsPropertySet(const QString& property) const
{
    QVariant var = h_ptr->m_properties.value(property);
    return var.isValid() && !var.isNull();
}

bool HObject::isCdsPropertySet(HCdsProperties::Property property) const
{
    QVariant var = h_ptr->m_properties.value(HCdsProperties::instance().get(property).name());
    return var.isValid() && !var.isNull();
}

bool HObject::setCdsProperty(const QString& property, const QVariant& value)
{
    if (h_ptr->m_properties.contains(property))
    {
        QVariant oldValue = h_ptr->m_properties.value(property);
        h_ptr->m_properties.insert(property, value);
        const HCdsPropertyInfo& info = HCdsProperties::instance().get(property);
        if (info.isValid() &&
            info.type() != HCdsProperties::upnp_objectUpdateID &&
            info.type() != HCdsProperties::upnp_containerUpdateID &&
            info.type() != HCdsProperties::upnp_totalDeletedChildCount)
        {
            emit objectModified(this, HObjectEventInfo(property, oldValue, value));
        }
        return true;
    }
    return false;
}

bool HObject::setCdsProperty(HCdsProperties::Property property, const QVariant& value)
{
    const HCdsPropertyInfo& info = HCdsProperties::instance().get(property);
    if (h_ptr->m_properties.contains(info.name()))
    {
        QVariant oldValue = h_ptr->m_properties.value(info.name());
        h_ptr->m_properties.insert(info.name(), value);
        if (property != HCdsProperties::upnp_objectUpdateID &&
            property != HCdsProperties::upnp_containerUpdateID &&
            property != HCdsProperties::upnp_totalDeletedChildCount &&
            property != HCdsProperties::dlite_res)
        {
            emit objectModified(this, HObjectEventInfo(info.name(), oldValue, value));
        }
        return true;
    }
    return false;
}

bool HObject::getCdsProperty(const QString& property, QVariant* value) const
{
    Q_ASSERT(value);

    if (h_ptr->m_properties.contains(property))
    {
        *value = h_ptr->m_properties.value(property);
        return true;
    }

    return false;
}

bool HObject::getCdsProperty(HCdsProperties::Property property, QVariant* value) const
{
    Q_ASSERT(value);

    const HCdsPropertyInfo& info = HCdsProperties::instance().get(property);
    if (h_ptr->m_properties.contains(info.name()))
    {
        *value = h_ptr->m_properties.value(info.name());
        return true;
    }

    return false;
}

bool HObject::isCdsPropertyActive(const QString& property) const
{
    if (!hasCdsProperty(property)) { return false; }
    return !h_ptr->m_disabledProperties.contains(property);
}

bool HObject::isCdsPropertyActive(HCdsProperties::Property property) const
{
    if (!hasCdsProperty(property)) { return false; }
    const HCdsPropertyInfo& info = HCdsProperties::instance().get(property);
    return !h_ptr->m_disabledProperties.contains(info.name());
}

bool HObject::isValid() const
{
    return validate();
}

void HObject::setId(const QString& arg)
{
    if (isInitialized())
    {
        return;
    }
    setCdsProperty(HCdsProperties::dlite_id, arg);
}

void HObject::setParentId(const QString& arg)
{
    setCdsProperty(HCdsProperties::dlite_parentId, arg);
}

void HObject::setTrackChangesOption(bool arg)
{
    enableCdsProperty(HCdsProperties::instance().get(HCdsProperties::upnp_objectUpdateID).name(), arg);
    if (isContainer())
    {
        bool ok = enableCdsProperty(HCdsProperties::instance().get(HCdsProperties::upnp_containerUpdateID).name(), arg);
        Q_ASSERT(ok); Q_UNUSED(ok)
        enableCdsProperty(HCdsProperties::instance().get(HCdsProperties::upnp_totalDeletedChildCount).name(), arg);
        Q_ASSERT(ok);
    }
    else
    {
        HResources currentResources = resources(), newResources;
        foreach(HResource res, currentResources)
        {
            res.enableTrackChangesOption(arg);
            newResources.append(res);
        }
        setResources(newResources);
    }
}

void HObject::addResource(const HResource& res)
{
    HResources resList = resources();
    resList.append(res);
    setResources(resList);
}

bool HObject::enableCdsProperty(const QString& property, bool arg)
{
    if (!hasCdsProperty(property) ||
        !(HCdsPropertyDb::instance().property(property).info().propertyFlags() &
        HCdsPropertyInfo::Disableable))
    {
        return false;
    }

    if (arg)
    {
        if (!h_ptr->m_disabledProperties.contains(property))
        {
            return false;
        }
        else
        {
            h_ptr->m_disabledProperties.removeOne(property);
        }
    }
    else if (!h_ptr->m_disabledProperties.contains(property))
    {
        h_ptr->m_disabledProperties.append(property);
    }

    return true;
}

bool HObject::validate() const
{
    return !title().isEmpty() && !id().isEmpty() && !parentId().isEmpty();
}

bool HObject::serialize(
    const QString& propertyName, QVariant*, QXmlStreamReader* reader)
{
    Q_UNUSED(propertyName)
    Q_UNUSED(reader)
    return false;
}

bool HObject::serialize(
    const QString& propertyName, const QVariant& value,
    QXmlStreamWriter& writer) const
{
    Q_UNUSED(propertyName)
    Q_UNUSED(value)
    Q_UNUSED(writer)
    return false;
}

void HObject::doClone(HClonable* target) const
{
    HObject* obj = dynamic_cast<HObject*>(target);
    if (obj)
    {
        obj->h_ptr->m_cdsType = h_ptr->m_cdsType;
        obj->h_ptr->m_disabledProperties = h_ptr->m_disabledProperties;
        obj->h_ptr->m_properties = h_ptr->m_properties;
    }
}

QHash<QString, QVariant> HObject::cdsProperties() const
{
    return h_ptr->m_properties;
}

bool HObject::neverPlayable() const
{
    QVariant value;
    getCdsProperty(HCdsProperties::dlite_neverPlayable, &value);
    return value.toBool();
}

QString HObject::id() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::dlite_id, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.toString();
}

QString HObject::parentId() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::dlite_parentId, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.toString();
}

bool HObject::isRestricted() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::dlite_restricted, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.toBool();
}

quint32 HObject::objectUpdateId() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::upnp_objectUpdateID, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.toUInt();
}

QString HObject::title() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::dc_title, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.toString();
}

HObject::WriteStatus HObject::writeStatus() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::upnp_writeStatus, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.value<WriteStatus>();
}

QString HObject::creator() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::dc_creator, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.toString();
}

HResources HObject::resources() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::dlite_res, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return toList<HResource>(value.toList());
}

QString HObject::clazz() const
{
    QVariant value;
    bool b = getCdsProperty(HCdsProperties::upnp_class, &value);
    Q_ASSERT(b); Q_UNUSED(b)
    return value.toString();
}

HObject::CdsType HObject::type() const
{
    return h_ptr->m_cdsType;
}

bool HObject::isItem() const
{
    return isItem(type());
}

bool HObject::isContainer() const
{
    return isContainer(type());
}

bool HObject::isTrackChangesOptionEnabled() const
{
    return isCdsPropertyActive(HCdsProperties::instance().get(HCdsProperties::upnp_objectUpdateID).name());
}

void HObject::setNeverPlayable(bool arg)
{
    setCdsProperty(HCdsProperties::dlite_neverPlayable, arg);
}

void HObject::setRestricted(bool arg)
{
    bool b = setCdsProperty(HCdsProperties::dlite_restricted, arg);
    Q_ASSERT(b); Q_UNUSED(b)
}

void HObject::setTitle(const QString& arg)
{
    bool b = setCdsProperty(HCdsProperties::dc_title, arg);
    Q_ASSERT(b); Q_UNUSED(b)
}

void HObject::setCreator(const QString& arg)
{
    bool b = setCdsProperty(HCdsProperties::dc_creator, arg);
    Q_ASSERT(b); Q_UNUSED(b)
}

void HObject::setObjectUpdateId(quint32 arg)
{
    bool b = setCdsProperty(HCdsProperties::upnp_objectUpdateID, arg);
    Q_ASSERT(b); Q_UNUSED(b)
}

void HObject::setResources(const QList<HResource>& arg)
{
    bool b = setCdsProperty(HCdsProperties::dlite_res, toList(arg));
    Q_ASSERT(b); Q_UNUSED(b)
}

void HObject::setWriteStatus(WriteStatus arg)
{
    bool b = setCdsProperty(HCdsProperties::upnp_writeStatus, QVariant::fromValue(arg));
    Q_ASSERT(b); Q_UNUSED(b)
}

HContainer* HObject::asContainer()
{
    return isContainer() ? static_cast<HContainer*>(this) : 0;
}

const HContainer* HObject::asContainer() const
{
    return isContainer() ? static_cast<const HContainer*>(this) : 0;
}

HItem* HObject::asItem()
{
    return isItem() ? static_cast<HItem*>(this) : 0;
}

const HItem* HObject::asItem() const
{
    return isItem() ? static_cast<const HItem*>(this) : 0;
}

QString HObject::writeStatusToString(WriteStatus status)
{
    QString retVal = "Unknown";
    switch (status)
    {
    case UnknownWriteStatus:
        break;
    case WritableWriteStatus:
        retVal = "Writable";
        break;
    case ProtectedWriteStatus:
        retVal = "Protected";
        break;
    case NotWritableWriteStatus:
        retVal = "NotWritable";
        break;
    case MixedWriteStatus:
        retVal = "Mixed";
        break;
    default:
        break;
    }
    return retVal;
}

HObject::WriteStatus HObject::writeStatusFromString(const QString& status)
{
    WriteStatus retVal = UnknownWriteStatus;
    if (status.compare("writable", Qt::CaseInsensitive) == 0)
    {
        retVal = WritableWriteStatus;
    }
    else if (status.compare("protected", Qt::CaseInsensitive) == 0)
    {
        retVal = ProtectedWriteStatus;
    }
    else if (status.compare("mixed", Qt::CaseInsensitive) == 0)
    {
        retVal = MixedWriteStatus;
    }
    return retVal;
}

}
}
}
