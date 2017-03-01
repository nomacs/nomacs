/*******************************************************************************************************
 DkPatchMatchingPlugin.h
 Created on:	14.07.2013

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
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QToolBar>
#include <QMainWindow>
#include <QColorDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QMouseEvent>

#include "DkPluginInterface.h"
#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkWidgets.h"
#include "DkBaseViewPort.h"
#include "DkImageStorage.h"
#include <memory>
#include "DkPolygon.h"
#include <QGraphicsView>
#include <QHBoxLayout>
#include "DkSyncedPolygon.h"
#include "DkPolyTimeline.h"

namespace nmp {

class DkPatchMatchingViewPort;
class DkPatchMatchingToolBar;
enum class SelectedTool;

class DkPatchMatchingPlugin : public QObject, nmc::DkViewPortInterface {
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkPatchMatchingPlugin/3.3" FILE "DkPatchMatchingPlugin.json")

public:
	DkPatchMatchingPlugin();
	virtual ~DkPatchMatchingPlugin() = default;

    QImage image() const override;

	QSharedPointer<nmc::DkImageContainer> runPlugin(const QString &runID = QString(), 
				QSharedPointer<nmc::DkImageContainer> image = QSharedPointer<nmc::DkImageContainer>()) const override;
	nmc::DkPluginViewPort* getViewPort() override;
	void deleteViewPort() override;
	bool closesOnImageChange() const override;

protected:
	DkPatchMatchingViewPort* mViewport;
};

class DkPatchMatchingViewPort : public nmc::DkPluginViewPort {
	Q_OBJECT

public:
	// ctor and sets up everything
	DkPatchMatchingViewPort(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	// saves settings
	~DkPatchMatchingViewPort();

	// checks on update if worldmatrix has changed and
	// emits corresponding signal when true
	void checkWorldMatrixChanged();
	QByteArray createCurrentJson();

	auto getNearestPolygon(QPointF point);

public slots:
	void setVisible(bool visible) override;
	void updateImageContainer(QSharedPointer<nmc::DkImageContainerT> imgC) override;
	void setPanning(bool checked);
	void discardChangesAndClose();
	
	
	// load/save json sidecar file
	QString getJsonFilePath() const;
	void loadFromFile();

	// stuff we can do with the toolbar
	void clonePolygon();
	void addPolygon();
	void removePolygon();
	void saveToFile();
	QJsonObject createJson(QSharedPointer<DkSyncedPolygon> poly);
	void changeCurrentPolygon(int idx);
	void updateInactive();
	QSharedPointer<DkSyncedPolygon> currentPolygon();
	void clear();

signals:
	void worldMatrixChanged(QTransform worldMatrix);
	void polygonAdded(/*some parameters are probably needed here*/);
	void reset(QSharedPointer<nmc::DkImageContainerT> imgC);

private:
	QSharedPointer<DkPolygonRenderer> firstPoly();
	QSharedPointer<DkPolygonRenderer> addClone(QSharedPointer<DkSyncedPolygon> poly);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent*event);
	void paintEvent(QPaintEvent *event);
	
	QColor getNextColor();
	void resetColorIndex();
	void loadSettings();
	void saveSettings() const;
	
	//initialization list
	bool panning;
	int mCurrentPolygon;

	QVector<QSharedPointer<DkSyncedPolygon>> mPolygonList;
	QSharedPointer<DkPatchMatchingToolBar> mtoolbar;
	QSharedPointer<QDockWidget> mDock;
	QSharedPointer<DkPolyTimeline> mTimeline;

	QCursor defaultCursor;
	
	

	// default constructors
	QTransform mWorldMatrixCache;
	QVector<QSharedPointer<DkPolygonRenderer>> mRenderer;
	QSharedPointer<nmc::DkImageContainerT> mImage;
	QByteArray mCurrentFile;
	int mColorIndex;
};

enum class SelectedTool {
	AddPoint,
	RemovePoint,
	Move, 
	Rotate
};

class DkPatchMatchingToolBar : public QToolBar {
	Q_OBJECT


public:

	enum {
		apply_icon = 0,
		cancel_icon,
		pan_icon,
		undo_icon,

		icons_end,
	};

	DkPatchMatchingToolBar(const QString & title, QWidget * parent = 0);
	virtual ~DkPatchMatchingToolBar() = default;

	// setter and getter for the step size (timeline)
	int getStepSize();
	void setStepSize(int size);

	// setter and gett for patch size (timeline)
	int getPatchSize();
	void setPatchSize(int size);

	int getCurrentPolygon();
	void addPolygon(QColor color, bool select = false);
	void clearPolygons();

public slots:
	virtual void setVisible(bool visible);
	void removePoly(int idx);
	void changeCurrentPoly(int newindex);
	void highlightedPoly(int);

signals:
	// emitted when the step size spinner is changed (timeline)
	void stepSizeChanged(int width);
	void patchSizeChanged(int width);
	void saveTriggered();
	void removePolyTriggered();
	void clonePolyTriggered();
	void addPolyTriggerd();
	void closeTriggerd();
	void currentPolyChanged(int newindex);
	void showTimelineTriggerd();

protected:
	void createLayout();
	
	QSpinBox* mPatchSizeSpinner;
	QSpinBox* mStepSizeSpinner;		// changes step size for timeline
	QComboBox* mPolygonCombobox;			// combobox for switching between different 
};


};
