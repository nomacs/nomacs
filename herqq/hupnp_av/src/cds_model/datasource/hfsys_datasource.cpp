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

#include "hfsys_datasource.h"
#include "hfsys_datasource_p.h"
#include "hrootdir.h"

#include "../cds_objects/hitem.h"
#include "../cds_objects/hstoragefolder.h"
#include "../model_mgmt/hcdsobjectdata_p.h"
#include "../model_mgmt/hcds_fsys_reader_p.h"

#include <HUpnpCore/private/hlogger_p.h>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HFileSystemDataSourcePrivate
 *******************************************************************************/
HFileSystemDataSourcePrivate::HFileSystemDataSourcePrivate() :
    HAbstractCdsDataSourcePrivate(),
        m_itemPaths(), m_fsysReader()
{
    m_configuration.reset(new HFileSystemDataSourceConfiguration());
}

HFileSystemDataSourcePrivate::HFileSystemDataSourcePrivate(
    const HFileSystemDataSourceConfiguration& conf) :
        HAbstractCdsDataSourcePrivate(conf), m_itemPaths(), m_fsysReader()
{
}

HFileSystemDataSourcePrivate::~HFileSystemDataSourcePrivate()
{
}

bool HFileSystemDataSourcePrivate::add(
    HCdsObjectData* item, HFileSystemDataSource::AddFlag addFlag)
{
    Q_ASSERT(item);

    HObject* obj = item->object();
    if (add(obj, addFlag))
    {
        m_itemPaths.insert(obj->id(), item->dataPath());
        item->takeObject();
        return true;
    }

    return false;
}

bool HFileSystemDataSourcePrivate::add(
    const QList<HCdsObjectData*> items, HFileSystemDataSource::AddFlag addFlag)
{
    foreach(HCdsObjectData* item, items)
    {
        if (!add(item, addFlag))
        {
            return false;
        }
    }
    return true;
}

/*******************************************************************************
 * HFileSystemDataSource
 *******************************************************************************/
HFileSystemDataSource::HFileSystemDataSource(QObject* parent) :
    HAbstractCdsDataSource(*new HFileSystemDataSourcePrivate(), parent)
{
}

HFileSystemDataSource::HFileSystemDataSource(
    const HFileSystemDataSourceConfiguration& conf, QObject* parent) :
        HAbstractCdsDataSource(*new HFileSystemDataSourcePrivate(conf), parent)
{
}

HFileSystemDataSource::HFileSystemDataSource(
    HFileSystemDataSourcePrivate& dd, QObject* parent) :
        HAbstractCdsDataSource(dd, parent)
{
}

HFileSystemDataSource::~HFileSystemDataSource()
{
}

bool HFileSystemDataSource::doInit()
{
    H_D(HFileSystemDataSource);

    HStorageFolder* rootContainer = new HStorageFolder("Contents", "-1", "0");
    HCdsObjectData root(rootContainer);
    h->add(&root);

    h->m_fsysReader.reset(new HCdsFileSystemReader());

    const HFileSystemDataSourceConfiguration* conf = configuration();
    HRootDirs rootDirs = conf->rootDirs();
    foreach(const HRootDir& rootDir, rootDirs)
    {
        QList<HCdsObjectData*> items;
        if (h->m_fsysReader->scan(rootDir, "0", &items))
        {
            if (!h->add(items))
            {
                qDeleteAll(items);
                return false;
            }
        }
        qDeleteAll(items);
    }

    return true;
}

const HFileSystemDataSourceConfiguration*
    HFileSystemDataSource::configuration() const
{
    return static_cast<const HFileSystemDataSourceConfiguration*>(
        HAbstractCdsDataSource::configuration());
}

bool HFileSystemDataSource::supportsLoading() const
{
    return true;
}

bool HFileSystemDataSource::isLoadable(const QString& itemId) const
{
    return !getPath(itemId).isEmpty();
}

QIODevice* HFileSystemDataSource::loadItemData(const QString& itemId)
{
    if (!isInitialized())
    {
        return 0;
    }

    HLOG(H_AT, H_FUN);
    HLOG_INFO(QString("Attempting to load item [%1]").arg(itemId));

    QString path = getPath(itemId);
    if (path.isEmpty())
    {
        QString err =
            QString("The specified objectId [%1] does not correspond to any "
            "item that can be loaded").arg(itemId);

        HLOG_WARN(err);
        return 0;
    }

    QFile* file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly))
    {
        QString err = QString("Could not open file [%1] for reading").arg(path);
        HLOG_WARN(err);
        delete file;
        return 0;
    }

    return file;
}

void HFileSystemDataSource::clear()
{
    if (!isInitialized())
    {
        return;
    }

    H_D(HFileSystemDataSource);
    HAbstractCdsDataSource::clear();

    h->configuration()->clear();
    h->m_itemPaths.clear();

    HStorageFolder* rootContainer = new HStorageFolder("Contents", "-1", "0");
    HCdsObjectData root(rootContainer);
    h->add(&root);
}

bool HFileSystemDataSource::add(HContainer* cdsContainer, AddFlag addFlag)
{
    if (!isInitialized())
    {
        return false;
    }

    H_D(HFileSystemDataSource);
    HCdsObjectData odata(reinterpret_cast<HObject*>(cdsContainer));
    if (!h->add(&odata, addFlag))
    {
        odata.takeObject();
        return false;
    }
    return true;
}

HItem* HFileSystemDataSource::add(
    const QString& path, const QString& parentId, AddFlag addFlag)
{
    if (!isInitialized())
    {
        return 0;
    }

    HItem* newItem = HCdsFileSystemReader::createItem(path, parentId);
    if (newItem)
    {
        H_D(HFileSystemDataSource);
        HCdsObjectData odata(reinterpret_cast<HObject*>(newItem), path);
        if (!h->add(&odata, addFlag))
        {
            return 0;
        }
    }

    return newItem;
}

bool HFileSystemDataSource::add(HItem* item, const QString& path, AddFlag addFlag)
{
    if (!isInitialized())
    {
        return false;
    }

    H_D(HFileSystemDataSource);
    HCdsObjectData odata(reinterpret_cast<HObject*>(item), path);
    if (!h->add(&odata, addFlag))
    {
        odata.takeObject();
        return false;
    }

    if (!item->hasContentFormat())
    {
        QString mimeType = HCdsFileSystemReader::deduceMimeType(path);
        if (!mimeType.isEmpty())
        {
            item->setContentFormat(mimeType);
        }
    }

    return true;
}

qint32 HFileSystemDataSource::add(const HRootDir& rootDir, AddFlag addFlag)
{
    if (!isInitialized())
    {
        return -1;
    }

    H_D(HFileSystemDataSource);
    if (!h->configuration()->addRootDir(rootDir))
    {
        return -1;
    }

    QList<HCdsObjectData*> items;
    if (h->m_fsysReader->scan(rootDir, "0", &items))
    {
        if (!h->add(items, addFlag))
        {
            qDeleteAll(items);
            h->configuration()->removeRootDir(rootDir);
            return -1;
        }
    }
    qDeleteAll(items);

    return items.size();
}

QString HFileSystemDataSource::getPath(const QString& objectId) const
{
    const H_D(HFileSystemDataSource);
    return h->m_itemPaths.value(objectId);
}

}
}
}
