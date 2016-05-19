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
	~DkPatchMatchingPlugin();

	QString id() const override;
    QImage image() const override;

	QSharedPointer<nmc::DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<nmc::DkImageContainer> image = QSharedPointer<nmc::DkImageContainer>()) const override;
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

public slots:
	void setPanning(bool checked);
	void applyChangesAndClose();
	void discardChangesAndClose();
	virtual void setVisible(bool visible);
	
	void clonePolygon();
	void selectedToolChanged(SelectedTool tool);
	virtual void updateImageContainer(QSharedPointer<nmc::DkImageContainerT> imgC) override;
	QString getJsonFilePath() const;
signals:
	void worldMatrixChanged(QTransform worldMatrix);
	void polygonAdded(/*some parameters are probably needed here*/);
	void reset(QSharedPointer<nmc::DkImageContainerT> imgC);

private:
	QSharedPointer<DkPolygonRenderer> firstPoly();
	QSharedPointer<DkPolygonRenderer> addPoly();
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent*event);
	void paintEvent(QPaintEvent *event);
	
	static QColor getNextColor();
	void loadSettings();
	void saveSettings() const;

	// initialization list
	bool cancelTriggered;
	
	bool panning;
	QSharedPointer<DkPatchMatchingToolBar> mtoolbar;
	QSharedPointer<DkSyncedPolygon> mPolygon;
	QSharedPointer<QDockWidget> mDock;
	QSharedPointer<DkPolyTimeline> mTimeline;

	QCursor defaultCursor;
	
	// default constructors
	QTransform mWorldMatrixCache;
	QVector<QSharedPointer<DkPolygonRenderer>> mRenderer;
	QSharedPointer<nmc::DkImageContainerT> mImage;
	
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

	// OLD STUFF
	void setPenColor(const QColor& col);
	void setPenWidth(int width);

public slots:
	void modeChangeTriggered(QAction* action);

	// OLD STUFF, remove probably
	void on_applyAction_triggered();
	void on_cancelAction_triggered();
	void on_panAction_toggled(bool checked);
	void on_penColButton_clicked();
	void on_alphaBox_valueChanged(int val);
	void on_undoAction_triggered();
	virtual void setVisible(bool visible);

signals:
	// emitted when the step size spinner is changed (timeline)
	void stepSizeChanged(int width);

	// OLD STUFF, remove probably
	void selectedToolChanged(SelectedTool tool);
	void clonePolyTriggered();
	void applySignal();
	void cancelSignal();
	void colorSignal(QColor color);
	void paintHint(int paintMode);
	void shadingHint(bool invert);
	void panSignal(bool checked);
	void undoSignal();

protected:
	void createLayout();
	void createIcons();
	
	QSpinBox* mStepSizeSpinner;		// changes step size for timeline
	QAction* mClonePolyAction;		// adds new polygon clone (renderer)

	

	// OLD STUFF, remove probably
	QPushButton* penColButton;
	QColorDialog* colorDialog;
	
	QSpinBox* alphaBox;
	QColor penCol;
	int penAlpha;
	QAction* panAction;
	QAction* undoAction;

	QAction* mAddPointAction;
	QAction* mRemovePointAction;
	
	QAction* mRotateAction;
	QAction* mMoveAction;
	QActionGroup* mModeGroup;
	QVector<QIcon> icons;		// needed for colorizing
};


};
