/*******************************************************************************************************
 DkTestPlugin.h
 Created on:	14.04.2013

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#pragma once

#include <QObject>
#include <QtPlugin>
#include <QImage>
#include <QStringList>
#include <QString>
#include <QMessageBox>
#include <QAction>

#include "DkPluginInterface.h"


//#ifdef DK_DLL
//#define DllExport Q_DECL_EXPORT
//#else
//#define DllExport Q_DECL_IMPORT
//#endif

//#undef DK_DLL
#include "DkBaseViewport.h"





namespace nmc {
	
//class DkFirstClass : public DkBaseViewPort {
//	Q_OBJECT
//
//public:
//	DkFirstClass(QWidget *parent = 0, Qt::WFlags flags = 0);
//	QList<QAction*> getActions();
//
//public slots:
//	void on_josefAction_triggered();
//	void on_anaAction_triggered();
//	void on_paintAction_triggered();
//
//protected:
//	void mouseMoveEvent(QMouseEvent *event) {
//
//		qDebug() << "changing your code muhahaha...";
//
//		DkBaseViewPort::mouseMoveEvent(event);
//	};
//	
//	QList<QAction* > myActions;
//
//};

class DkPaintViewPort;

class DkTestPlugin : public QObject, DkViewPortInterface {
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)

public:
    
	DkTestPlugin();
	~DkTestPlugin();

	QString pluginID() const;
    QString pluginName() const;
    QString pluginDescription() const;
    QImage pluginDescriptionImage() const;
    QString pluginVersion() const;

    QStringList runID() const;
    QString pluginMenuName(const QString &runID = "") const;
    QString pluginStatusTip(const QString &runID) const;  
	QList<QAction*> pluginActions(QWidget* parent);
    QImage runPlugin(const QString &runID, const QImage &image) const;
	DkPluginViewPort* getViewPort();

protected:
	//DkFirstClass* myClass;
	DkPluginViewPort* viewport;
};

class DkPaintViewPort : public DkPluginViewPort {

public:
	DkPaintViewPort(QWidget* parent = 0);
	DkPaintViewPort(QGraphicsScene* scene, QWidget* parent = 0);

protected:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	virtual void init();

};


};