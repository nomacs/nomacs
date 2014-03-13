/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpAvSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP A/V (HUPnPAv) library.
 *
 *  HUpnpAvSimpleTestApp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HUpnpAvSimpleTestApp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HUpnpAvSimpleTestApp. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTROLPOINT_NAVIGATORITEM_H_
#define CONTROLPOINT_NAVIGATORITEM_H_

#include <HUpnpAv/HConnection>
#include <HUpnpCore/HClientDevice>

#include <QtCore/QString>
#include <QtCore/QPointer>

template<typename T>
class QList;

class QVariant;
class RendererItem;
class ConnectionItem;
class CdsContainerItem;
class ContentDirectoryItem;

//
//
//
class ControlPointNavigatorItemVisitor
{
public:

    virtual void visit(RendererItem*) = 0;
    virtual void visit(ConnectionItem*) = 0;
    virtual void visit(CdsContainerItem*) = 0;
    virtual void visit(ContentDirectoryItem*) = 0;
};

//
//
//
class ControlPointNavigatorItem
{
protected:

    QList<ControlPointNavigatorItem*> m_childItems;
    ControlPointNavigatorItem*        m_parentItem;

public:

    explicit ControlPointNavigatorItem(ControlPointNavigatorItem* parent = 0);
    virtual ~ControlPointNavigatorItem();

    virtual QVariant data(int column) const = 0;

    void appendChild(ControlPointNavigatorItem* child);
    void removeChild(qint32 row);
    ControlPointNavigatorItem* child (int row) const;

    int childCount () const;
    int columnCount() const;

    int row        () const;
    ControlPointNavigatorItem* parent();

    int rowCount   () const;

    virtual void accept(ControlPointNavigatorItemVisitor*) = 0;
};

//
//
//
class RootItem :
    public ControlPointNavigatorItem
{
private:

public:

    explicit RootItem(ControlPointNavigatorItem* parent = 0);
    virtual ~RootItem();

    virtual QVariant data (int column) const;

    virtual void accept(ControlPointNavigatorItemVisitor*);
};

//
//
//
class ContainerItem :
    public ControlPointNavigatorItem
{
private:

    QString m_name;

public:

    explicit ContainerItem(
        const QString& name, ControlPointNavigatorItem* parent = 0);

    virtual ~ContainerItem();

    virtual QVariant data (int column) const;

    virtual void accept(ControlPointNavigatorItemVisitor*);
};

//
//
//
class RendererItem :
    public ControlPointNavigatorItem
{
private:

    Herqq::Upnp::Av::HMediaRendererAdapter* m_renderer;

public:

    explicit RendererItem(
        Herqq::Upnp::Av::HMediaRendererAdapter* renderer,
        ControlPointNavigatorItem* parent = 0);

    virtual ~RendererItem();

    virtual QVariant data (int column) const;

    inline Herqq::Upnp::Av::HMediaRendererAdapter* renderer() const
    {
        return m_renderer;
    }

    virtual void accept(ControlPointNavigatorItemVisitor*);
};

//
//
//
class ConnectionItem :
    public ControlPointNavigatorItem
{
private:

    QPointer<Herqq::Upnp::Av::HConnection> m_connection;

public:

    explicit ConnectionItem(
        Herqq::Upnp::Av::HConnection* connection,
        ControlPointNavigatorItem* parent = 0);

    virtual ~ConnectionItem();

    virtual QVariant data (int column) const;

    inline Herqq::Upnp::Av::HConnection* connection() const
    {
        return m_connection;
    }

    virtual void accept(ControlPointNavigatorItemVisitor*);
};

//
//
//
class ContentDirectoryItem :
    public ControlPointNavigatorItem
{
private:

    Herqq::Upnp::Av::HMediaBrowser* m_browser;

public:

    explicit ContentDirectoryItem(
        Herqq::Upnp::Av::HMediaBrowser*,
        ControlPointNavigatorItem* parent = 0);

    virtual ~ContentDirectoryItem();

    virtual QVariant data (int column) const;

    inline Herqq::Upnp::Av::HMediaBrowser* browser() const { return m_browser; }

    virtual void accept(ControlPointNavigatorItemVisitor*);
};

//
//
//
class CdsContainerItem :
    public ControlPointNavigatorItem
{
private:

    Herqq::Upnp::Av::HContainer* m_container;
    Herqq::Upnp::Av::HCdsDataSource* m_dataSource;

public:

    explicit CdsContainerItem(
        Herqq::Upnp::Av::HContainer* rootContainer,
        Herqq::Upnp::Av::HCdsDataSource*,
        ControlPointNavigatorItem* parent = 0);

    virtual ~CdsContainerItem();

    virtual QVariant data (int column) const;

    inline Herqq::Upnp::Av::HContainer* container() const
    {
        return m_container;
    }

    inline Herqq::Upnp::Av::HCdsDataSource* dataSource() const
    {
        return m_dataSource;
    }

    virtual void accept(ControlPointNavigatorItemVisitor*);
};

#endif /* CONTROLPOINT_NAVIGATORITEM_H_ */
