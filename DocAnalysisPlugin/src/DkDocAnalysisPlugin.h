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

// Workaround
#define PLUGIN_ID "5232a9d4459e431fb9b686365e693a30"
#define PLUGIN_VERSION "1.0.0"

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

protected slots:
	void viewportDestroyed();
};

class DkDocAnalysisViewPort : public DkPluginViewPort {
	Q_OBJECT

public:
	DkDocAnalysisViewPort(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkDocAnalysisViewPort();
	
	QBrush getBrush() const;
	QPen getPen() const;
	bool isCanceled();
	QImage getPaintedImage();

public slots:
	void setBrush(const QBrush& brush);
	void setPen(const QPen& pen);
	void setPenWidth(int width);
	void setPenColor(QColor color);
	void setPanning(bool checked);
	void applyChangesAndClose();
	void discardChangesAndClose();
	virtual void setVisible(bool visible);

protected:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent*event);
	void paintEvent(QPaintEvent *event);
	virtual void init();

	QVector<QPainterPath> paths;
	QVector<QPen> pathsPen;

	bool cancelTriggered;
	bool isOutside;
	QBrush brush;
	QPen pen;
	QPointF lastPoint;
	bool panning;
	DkDocAnalysisToolBar* docAnalysisToolbar;
	QCursor defaultCursor;
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
	void on_toleranceBox_valueChanged(int val);

	// slots for signals coming from the view port
	void pickSeedpointCanceled();
	void pickSeedpointStarted();
	void measureDistanceCanceled();
	void measureDistanceStarted();
	void enableButtonSaveCut(bool enable);
	void enableButtonShowTextLines(bool enable);
	void toggleBottomTextLinesButton(bool toggle);
	void toggleTopTextLinesButton(bool toggle);

signals:
	// signals to the viewport
	void pickSeedpointRequest(bool);  /**< Signal to either start or cancel the magic cut selection tool **/
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
