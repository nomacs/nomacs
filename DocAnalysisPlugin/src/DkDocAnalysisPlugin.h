/*******************************************************************************************************
 DkDocAnalysisPlugin.h
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
#include <QLabel>

#include "DkPluginInterface.h"
#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkBaseViewPort.h"
#include "DkImageStorage.h"
#include "DkDistanceMeasure.h"
#include "DkMagicCutWidgets.h"
#include "DkLineDetection.h"
//#include "DkDialog.h"
#include "DkSaveDialog.h"

//// Workaround
//#define PLUGIN_ID "5232a9d4459e431fb9b686365e693a30"
//#define PLUGIN_VERSION "1.0.0"

namespace nmc {

class DkDocAnalysisViewPort;
class DkDocAnalysisToolBar;

class DkDocAnalysisPlugin : public QObject, DkViewPortInterface {
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)

public:
    
	DkDocAnalysisPlugin();
	~DkDocAnalysisPlugin();

	QString pluginID() const;
    QString pluginName() const;
    QString pluginDescription() const;
    QImage pluginDescriptionImage() const;
    QString pluginVersion() const;

    QStringList runID() const;
    QString pluginMenuName(const QString &runID = QString()) const;
    QString pluginStatusTip(const QString &runID = QString()) const;
    QImage runPlugin(const QString &runID = QString(), const QImage &image = QImage()) const;
	DkPluginViewPort* getViewPort();
	void deleteViewPort();

protected:
	DkPluginViewPort* viewport;

	DkCompressDialog *jpgDialog;
	DkTifDialog *tifDialog;

signals:
	void magicCutSavedSignal(bool); /**< Signal for confirming if the magic cut could be saved or not **/
	
protected slots:
	void viewportDestroyed();

public slots:
	void saveMagicCut(QImage saveImage, int xCoord, int yCoord, int height, int width);
};

class DkDocAnalysisViewPort : public DkPluginViewPort {
	Q_OBJECT

public:
	DkDocAnalysisViewPort(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkDocAnalysisViewPort();
	
	bool editingActive();
	void stopEditing();
	bool editingDrawingActive();


	bool isCanceled();
	QImage getPaintedImage();
	
	void setMainWindow(QMainWindow* win);

	void getBrightness(const cv::Mat& frame, double& brightness);

signals:
	// distance measure functions
	void cancelDistanceMeasureRequest();
	void startDistanceMeasureRequest();

	// magic wand functions
	void cancelPickSeedpointRequest();
	void startPickSeedpointRequest();
	void cancelClearSingleRegionRequest();
	void startClearSingleRegionRequest();
	void saveMagicCutRequest(QImage saveImg, int xCoord, int yCoord, int height, int width);
	void enableSaveCutSignal(bool enable);

	// text line detection functions
	void enableShowTextLinesSignal(bool enable);
	void toggleBottomTextLinesButtonSignal(bool toggle);
	void toggleTopTextLinesButtonSignal(bool toggle);

public slots:
	// measure distance functions
	void pickDistancePoint(bool pick);
	void pickDistancePoint();
	
	//// magic wand selection functions
	void pickSeedpoint(bool pick);
	void pickSeedpoint();
	void setMagicCutTolerance(int tol);
	void pickResetRegionPoint(bool pick);
	void pickResetRegionPoint();
	void clearMagicCut();
	void openMagicCutDialog();
	//// animation of contours
	void updateAnimatedContours();
	void saveMagicCutPressed(QImage saveImg, int xCoord, int yCoord, int height, int width);
	void magicCutSaved(bool saved);

	//// line detection functions
	void openLineDetectionDialog();
	void showBottomTextLines(bool show);
	void showTopTextLines(bool show);
	void showBottomTextLines();
	void showTopTextLines();


	virtual void setVisible(bool visible);

protected:
	// >DIR: uncomment if function is added again [21.10.2014 markus]
	//virtual void draw(QPainter *painter);

	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	
	void paintEvent(QPaintEvent *event);
	virtual void init();

	bool cancelTriggered;
	bool isOutside;
	DkDocAnalysisToolBar* docAnalysisToolbar;
	QCursor defaultCursor;

	QMainWindow* win; // >DIR: we'll retrieve the current image container from here [21.10.2014 markus]

private:
	/**
	* The editing mode which is currently active. State is defined by user interaction:
	* mode_default: the default viewing mode
	* mode_pickSeedpoint: user picks seedpoints for the magic wand tool
	* mode_pickDistance: user picks points to measure the distance
	* \sa DkDistanceMeasure DkMagicCut
	**/
	enum editModes {
		mode_default,
		mode_pickSeedpoint,
		mode_pickDistance,
		mode_cancelSeedpoint
	};

	int editMode; /**< The current editing state that the program is in **/
	bool showBottomLines; /**< Flag for rendering to show or hide bottom text lines **/
	bool showTopLines; /**< Flag for rendering to show or hide top text lines **/
	double avgBrightness; /**< Saves the average brightness of the image usingthe HSV color model **/
	double brightnessThreshold; /**< brightness threshold for using white pen **/

	// distance measure section
	DkDistanceMeasure *distance; /**< Tool to measure distances between two points **/
	void drawDistanceLine(QPainter *painter);

	// magic wand selection variables
	DkMagicCut *magicCut; /**< Tool to make a magic cut from an image (magic wand) **/
	DkMagicCutDialog *magicCutDialog;
	void drawContours(QPainter *painter);
	// line detection variables
	DkLineDetection *lineDetection; /**< Tool for detecting text lines within an image **/
	DkLineDetectionDialog *lineDetectionDialog;

};


class DkDocAnalysisToolBar : public QToolBar {
	Q_OBJECT


public:

	/**< Enum to handle all icons for this toolbar **/
	enum {
		linedetection_icon = 0,
		showbottomlines_icon,
		showtoplines_icon,
		distance_icon,
		magic_icon,
		savecut_icon,
		clearselection_icon,
		clearsingleselection_icon,

		icons_end,
	};

	/**< Enum to handle all actions for this toolbar **/
	enum {
		linedetection_action = 0,
		showbottomlines_action,
		showtoplines_action,
		distance_action,
		magic_action,
		savecut_action,
		clearselection_action,
		clearsingleselection_action,

		actions_end,
	};


	DkDocAnalysisToolBar(const QString & title, QWidget * parent = 0);
	virtual ~DkDocAnalysisToolBar();

	void enableNoImageActions(bool enable);

public slots:
	
	virtual void setVisible(bool visible);

	// slots connected to the toolbar icons
	void on_linedetectionAction_triggered();
	void on_showbottomlinesAction_triggered();
	void on_showtoplinesAction_triggered();
	void on_distanceAction_toggled(bool checked);
	void on_magicAction_toggled(bool checked);
	void on_savecutAction_triggered();
	void on_clearselectionAction_triggered();
	void on_clearsingleselectionAction_toggled(bool checked);
	void on_toleranceBox_valueChanged(int val);

	// slots for signals coming from the view port
	void pickSeedpointCanceled();
	void pickSeedpointStarted();
	void clearSingleRegionCanceled();
	void clearSingleRegionStarted();
	void measureDistanceCanceled();
	void measureDistanceStarted();
	void enableButtonSaveCut(bool enable);
	void enableButtonShowTextLines(bool enable);
	void toggleBottomTextLinesButton(bool toggle);
	void toggleTopTextLinesButton(bool toggle);

signals:
	// signals to the viewport
	void pickSeedpointRequest(bool);  /**< Signal to either start or cancel the magic cut selection tool **/
	void clearSingleSelectionRequest(bool); /**<Signal to either start or cancel the clear a single region of the magic cut selections **/
	void clearSelectionSignal(); /**< Signal to declare that the current selection shall be resetted **/
	void toleranceChanged(int); /**< Signal to signal if the tolerance setting has been changed **/
	void measureDistanceRequest(bool); /**< Signal to either start or cancel the distance measure tool **/
	void openCutDialogSignal(); /**< Signal to open the save magic cut dialog **/
	void detectLinesSignal(); /**< Signal to start the line detection on the current image **/
	void showBottomTextLinesSignal(bool); /**< Signal to either show or hide the previously detected bottom text lines **/
	void showTopTextLinesSignal(bool); /**< Signal to either show or hide the previously detected top text lines **/

protected:
	void createLayout();
	void createIcons();

	QVector<QIcon> icons; /**< List of all icons **/
	QVector<QAction *> actions; /**< List of all actions **/
};


};
