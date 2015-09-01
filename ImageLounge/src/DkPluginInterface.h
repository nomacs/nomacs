/*******************************************************************************************************
 DkPluginInterface.h
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

#include "DkImageContainer.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QStringList>
#include <QString>
#include <QImage>
#include <QGraphicsView>
#include <QToolBar>
#include <QFileInfo>
#include <QApplication>
#include <QMainWindow>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

class DkPluginViewPort;

class DkPluginInterface {

public:

	enum ifTypes {
		interface_basic = 0,
		interface_viewport,

		inteface_end,
	};

	virtual ~DkPluginInterface() {}

    virtual QString pluginID() const = 0;
    virtual QString pluginName() const = 0;
    virtual QString pluginDescription() const = 0;
    virtual QImage pluginDescriptionImage() const = 0;
    virtual QString pluginVersion() const = 0;

    virtual QStringList runID() const = 0;
    virtual QString pluginMenuName(const QString &runID = QString()) const = 0;
    virtual QString pluginStatusTip(const QString &runID = QString()) const = 0;
	virtual QList<QAction*> createActions(QWidget*) { return QList<QAction*>();};
	virtual QList<QAction*> pluginActions()	const { return QList<QAction*>();};
    virtual QSharedPointer<DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<DkImageContainer> imgC = QSharedPointer<DkImageContainer>()) const = 0;
	virtual int interfaceType() const {return interface_basic; };
	virtual bool closesOnImageChange() {return true;};
	QMainWindow* getMainWidnow() const {

		QWidgetList widgets = QApplication::topLevelWidgets();

		QMainWindow* win = 0;

		for (int idx = 0; idx < widgets.size(); idx++) {

			if (widgets.at(idx)->inherits("QMainWindow")) {
				win = qobject_cast<QMainWindow*>(widgets.at(idx));
				break;
			}
		}

		return win;
	}
};

class DkViewPortInterface : public DkPluginInterface {
	
public:

	virtual int interfaceType()  const {return interface_viewport;};

	virtual DkPluginViewPort* getViewPort() = 0;
	virtual void deleteViewPort() = 0;
};

class DkPluginViewPort : public QWidget {
	Q_OBJECT

public:
	DkPluginViewPort(QWidget* parent = 0, Qt::WindowFlags flags = 0) : QWidget(parent, flags) {
		
		// >DIR: I have removed the init here (it was called twice before due to derived classes) [16.10.2013 markus]
		//init();
	};

	void setWorldMatrix(QTransform* worldMatrix) {
		mWorldMatrix = worldMatrix;
	};

	void setImgMatrix(QTransform* imgMatrix) {
		mImgMatrix = imgMatrix;
	};

signals:
	void closePlugin(bool askForSaving = false, bool alreadySaving = false);
	void showToolbar(QToolBar* toolbar, bool show);
	void loadFile(QFileInfo file);
	void loadImage(QImage image);

protected:
	virtual void closeEvent(QCloseEvent *event) {
		emit closePlugin();
		QWidget::closeEvent(event);
	};

	virtual QPointF mapToImage(const QPointF& pos) const {
		
		if (!mWorldMatrix || !mImgMatrix)
			return pos;
		
		QPointF imgPos = mWorldMatrix->inverted().map(pos);
		imgPos = mImgMatrix->inverted().map(imgPos);

		return imgPos;
	};

	virtual QPointF mapToViewport(const QPointF& pos) const {

		if (!mWorldMatrix)
			return pos;

		return mWorldMatrix->inverted().map(pos);
	};

	virtual void init() {

		setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		//setStyleSheet("QGraphicsView{background-color: QColor(100,0,0,20); border: 1px solid #FFFFFF;}");
		//setMouseTracking(true);
	};

	QTransform* mWorldMatrix = 0;
	QTransform* mImgMatrix = 0;
};

};

// Change this version number if DkPluginInterface is changed!
Q_DECLARE_INTERFACE(nmc::DkPluginInterface, "com.nomacs.ImageLounge.DkPluginInterface/2.0")
Q_DECLARE_INTERFACE(nmc::DkViewPortInterface, "com.nomacs.ImageLounge.DkViewPortInterface/2.0")
