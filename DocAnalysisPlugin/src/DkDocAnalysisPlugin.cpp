/*******************************************************************************************************
 DkDocAnalysisPlugin.cpp
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

#include "DkDocAnalysisPlugin.h"

namespace nmc {

/*-----------------------------------DkDocAnalysisPlugin ---------------------------------------------*/

DkSettings::Display& DkSettings::display = DkSettings::getDisplaySettings();
DkSettings::Global& DkSettings::global = DkSettings::getGlobalSettings();

/**
*	Constructor
**/
DkDocAnalysisPlugin::DkDocAnalysisPlugin() {

	viewport = 0;
}

/**
*	Destructor
**/
DkDocAnalysisPlugin::~DkDocAnalysisPlugin() {

	qDebug() << "[PAINT PLUGIN] deleted...";

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/**
* Returns unique ID for the generated dll
**/
QString DkDocAnalysisPlugin::pluginID() const {
	return PLUGIN_ID;
};


/**
* Returns plugin name
**/
QString DkDocAnalysisPlugin::pluginName() const {

   return "Document Analysis";
};

/**
* Returns long description
**/
QString DkDocAnalysisPlugin::pluginDescription() const {

   return "<b>Created by:</b> Daniel Fischl<br><b>Modified:</b> Oct 2014<br><b>Description:</b> Tools to analyze images of documents, like text line detection, region cutting, distance measurement.";
};

/**
* Returns descriptive image
**/
QImage DkDocAnalysisPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPluginDocAnalysis/img/description.png");
};

/**
* Returns plugin version
**/
QString DkDocAnalysisPlugin::pluginVersion() const {

   return PLUGIN_VERSION;
};

/**
* Returns unique IDs for every plugin in this dll
* plugin can have more the one functionality that are triggered in the menu
* runID differes from pluginID
* viewport plugins can have only one runID and one functionality bound to it 
**/
QStringList DkDocAnalysisPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "b04b54c4b92b43aa86eb14e65accf3bc";
};

/**
* Returns plugin name for every run ID
* @param run ID
**/
QString DkDocAnalysisPlugin::pluginMenuName(const QString &runID) const {

	if (runID=="b04b54c4b92b43aa86eb14e65accf3bc") return "Document Analysis";
	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkDocAnalysisPlugin::pluginStatusTip(const QString &runID) const {

	if (runID=="b04b54c4b92b43aa86eb14e65accf3bc") return "Tools for Document Analysis";
	return "Wrong GUID!";
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QImage DkDocAnalysisPlugin::runPlugin(const QString &runID, const QImage &image) const {
	
	//for a viewport plugin runID and image are null
	if (viewport) {

		DkDocAnalysisViewPort* paintViewport = dynamic_cast<DkDocAnalysisViewPort*>(viewport);

		QImage retImg = QImage();
		if (!paintViewport->isCanceled()) retImg = paintViewport->getPaintedImage();

		viewport->setVisible(false);
	
		return retImg;
	}
	
	return image;
};

/**
* returns paintViewPort
**/
DkPluginViewPort* DkDocAnalysisPlugin::getViewPort() {

	if (!viewport) {
		viewport = new DkDocAnalysisViewPort();
		//connect(viewport, SIGNAL(destroyed()), this, SLOT(viewportDestroyed()));
	}
	return viewport;
}

/**
* sets the viewport pointer to NULL after the viewport is destroyed
**/
void DkDocAnalysisPlugin::viewportDestroyed() {

	viewport = 0;
}

void DkDocAnalysisPlugin::deleteViewPort() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/* macro for exporting plugin */
Q_EXPORT_PLUGIN2("com.nomacs.ImageLounge.DkDocAnalysisPlugin/1.0", DkDocAnalysisPlugin)


/*-----------------------------------DkDocAnalysisViewPort ---------------------------------------------*/

DkDocAnalysisViewPort::DkDocAnalysisViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {

	init();
}

DkDocAnalysisViewPort::~DkDocAnalysisViewPort() {
	qDebug() << "[PAINT VIEWPORT] deleted...";
	
	// acitive deletion since the MainWindow takes ownership...
	// if we have issues with this, we could disconnect all signals between viewport and toolbar too
	// however, then we have lot's of toolbars in memory if the user opens the plugin again and again
	if (docAnalysisToolbar) {
		delete docAnalysisToolbar;
		docAnalysisToolbar = 0;
	}
}

void DkDocAnalysisViewPort::init() {
	
	panning = false;
	cancelTriggered = false;
	isOutside = false;
	defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
	pen = QColor(0,0,0);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setWidth(1);
	
	docAnalysisToolbar = new DkDocAnalysisToolBar(tr("Paint Toolbar"), this);

	connect(docAnalysisToolbar, SIGNAL(colorSignal(QColor)), this, SLOT(setPenColor(QColor)));
	connect(docAnalysisToolbar, SIGNAL(widthSignal(int)), this, SLOT(setPenWidth(int)));
	connect(docAnalysisToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
	connect(docAnalysisToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
	connect(docAnalysisToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));
	
	DkPluginViewPort::init();
}

void DkDocAnalysisViewPort::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == DkSettings::global.altMod || panning)) {
		setCursor(Qt::ClosedHandCursor);
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	if (event->buttons() == Qt::LeftButton) {
		if(parent()) {

			DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
			if(viewport) {
		
				if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->posF()))) {
					
					isOutside = false;
					paths.append(QPainterPath());
					paths.last().moveTo(mapToImage(event->posF()));
					paths.last().lineTo(mapToImage(event->posF())+QPointF(0.1,0));
					pathsPen.append(pen);
					update();
				}
				else isOutside = true;
			}
		}
	}

	// no propagation
}

void DkDocAnalysisViewPort::mouseMoveEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
		return;
	}

	if (event->buttons() == Qt::LeftButton) {
		if(parent()) {
			DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());

			if(viewport) {
		
				if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->posF()))) {
					if (isOutside) {
						paths.append(QPainterPath());
						paths.last().moveTo(mapToImage(event->posF()));
						pathsPen.append(pen);
					}
					else {
						QPointF point = mapToImage(event->posF());
						paths.last().lineTo(point);
						update();
					}
					isOutside = false;
				}
				else isOutside = true;
			}
		}
	}

	//QWidget::mouseMoveEvent(event);	// do not propagate mouse event
}

void DkDocAnalysisViewPort::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod || panning) {
		setCursor(defaultCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}
}

void DkDocAnalysisViewPort::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	
	if (worldMatrix)
		painter.setWorldTransform((*imgMatrix) * (*worldMatrix));	// >DIR: using both matrices allows for correct resizing [16.10.2013 markus]

	for (int idx = 0; idx < paths.size(); idx++) {

		painter.setPen(pathsPen.at(idx));
		painter.drawPath(paths.at(idx));
	}

	painter.end();

	DkPluginViewPort::paintEvent(event);
}

QImage DkDocAnalysisViewPort::getPaintedImage() {

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		if (viewport) {

			if (!paths.isEmpty()) {   // if nothing is drawn there is no need to change the image

				QImage img = viewport->getImage();

				QPainter painter(&img);

				if (worldMatrix)
					painter.setWorldTransform(*worldMatrix);

				for (int idx = 0; idx < paths.size(); idx++) {
					painter.setPen(pathsPen.at(idx));
					painter.drawPath(paths.at(idx));
				}
				painter.end();

				return img;
			}
		}
	}
	
	return QImage();
}

void DkDocAnalysisViewPort::setBrush(const QBrush& brush) {
	this->brush = brush;
}

void DkDocAnalysisViewPort::setPen(const QPen& pen) {
	this->pen = pen;
}

void DkDocAnalysisViewPort::setPenWidth(int width) {

	this->pen.setWidth(width);
}

void DkDocAnalysisViewPort::setPenColor(QColor color) {

	this->pen.setColor(color);
}

void DkDocAnalysisViewPort::setPanning(bool checked) {

	this->panning = checked;
	if(checked) defaultCursor = Qt::OpenHandCursor;
	else defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
}

void DkDocAnalysisViewPort::applyChangesAndClose() {

	cancelTriggered = false;
	emit closePlugin();
}

void DkDocAnalysisViewPort::discardChangesAndClose() {

	cancelTriggered = true;
	emit closePlugin();
}

QBrush DkDocAnalysisViewPort::getBrush() const {
	return brush;
}

QPen DkDocAnalysisViewPort::getPen() const {
	return pen;
}

bool DkDocAnalysisViewPort::isCanceled() {
	return cancelTriggered;
}

void DkDocAnalysisViewPort::setVisible(bool visible) {
	
	if (docAnalysisToolbar)
		emit showToolbar(docAnalysisToolbar, visible);

	DkPluginViewPort::setVisible(visible);
}

/*-----------------------------------DkDocAnalysisToolBar ---------------------------------------------*/
DkDocAnalysisToolBar::DkDocAnalysisToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

	createIcons();
	createLayout();
	QMetaObject::connectSlotsByName(this);

	if (DkSettings::display.smallIcons)
		setIconSize(QSize(16, 16));
	else
		setIconSize(QSize(32, 32));

	if (DkSettings::display.toolbarGradient) {

		QColor hCol = DkSettings::display.highlightColor;
		hCol.setAlpha(80);

		setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); spacing: 3px; padding: 3px;}")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}
	else
		setStyleSheet("QToolBar{spacing: 3px; padding: 3px;}");

	qDebug() << "[DOCANALYSIS TOOLBAR] created...";
}

DkDocAnalysisToolBar::~DkDocAnalysisToolBar() {

	qDebug() << "[DOCANALYSIS TOOLBAR] deleted...";
}

/**
* Enables/Disables All the actions which require a loaded image
**/
void DkDocAnalysisToolBar::enableNoImageActions(bool enable) {

	actions[linedetection_action]->setEnabled(enable);
	actions[distance_action]->setEnabled(enable);
	actions[magic_action]->setEnabled(enable);
}

void DkDocAnalysisToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[linedetection_icon] = QIcon(":/nomacsPluginDocAnalysis/img/detect_lines.png");
	icons[showbottomlines_icon] = QIcon(":/nomacsPluginDocAnalysis/img/lower_lines.png");
	icons[showtoplines_icon] = QIcon(":/nomacsPluginDocAnalysis/img/upper_lines.png");
	icons[distance_icon] = QIcon(":/nomacsPluginDocAnalysis/img/distance.png");
	icons[magic_icon] = QIcon(":/nomacsPluginDocAnalysis/img/magic_wand.png");
	icons[savecut_icon] = QIcon(":/nomacsPluginDocAnalysis/img/save_cut.png");
	icons[clearselection_icon] = QIcon(":/nomacsPluginDocAnalysis/img/reset_cut.png");


	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::display.iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		}
	}
}

void DkDocAnalysisToolBar::createLayout() {

	/*QList<QKeySequence> enterSc;
	enterSc.append(QKeySequence(Qt::Key_Enter));
	enterSc.append(QKeySequence(Qt::Key_Return));*/
	actions.resize(actions_end);

	QAction* linedetectionAction = new QAction(icons[linedetection_icon], tr("Detect text lines"), this);
	linedetectionAction->setShortcut(Qt::SHIFT + Qt::Key_D);
	linedetectionAction->setStatusTip(tr("Opens dialog to configure and start the detection of text lines for the current image"));
	linedetectionAction->setCheckable(false);
	linedetectionAction->setChecked(false);
	linedetectionAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	linedetectionAction->setObjectName("linedetectionAction");
	actions[linedetection_action] = linedetectionAction;

	QAction* showbottomlinesAction = new QAction(icons[showbottomlines_icon], tr("Show detected bottom text lines"), this);
	showbottomlinesAction->setShortcut(Qt::SHIFT + Qt::Key_L);
	showbottomlinesAction->setStatusTip(tr("Displays the previously calculated lower text lines for the image"));
	showbottomlinesAction->setCheckable(true);
	showbottomlinesAction->setChecked(false);
	showbottomlinesAction->setEnabled(false);
	showbottomlinesAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	showbottomlinesAction->setObjectName("showbottomlinesAction");
	actions[showbottomlines_action] = showbottomlinesAction;

	QAction* showtoplinesAction = new QAction(icons[showtoplines_icon], tr("Show detected top text lines"), this);
	showtoplinesAction->setShortcut(Qt::SHIFT + Qt::Key_U);
	showtoplinesAction->setStatusTip(tr("Displays the previously calculated upper text lines for the image"));
	showtoplinesAction->setCheckable(true);
	showtoplinesAction->setChecked(false);
	showtoplinesAction->setEnabled(false);
	showtoplinesAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	showtoplinesAction->setObjectName("showtoplinesAction");
	actions[showtoplines_action] = showtoplinesAction;

	QAction* distanceAction = new QAction(icons[distance_icon], tr("Measure distance"), this);
	distanceAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
	distanceAction->setStatusTip(tr("Measure the distance between 2 points (in cm)"));
	distanceAction->setCheckable(true);
	distanceAction->setChecked(false);
	distanceAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	distanceAction->setObjectName("distanceAction");
	actions[distance_action] = distanceAction;

	QAction* magicAction = new QAction(icons[magic_icon], tr("Select region"), this);
	magicAction->setShortcut(Qt::SHIFT + Qt::Key_S);
	magicAction->setStatusTip(tr("Select regions with similar color"));
	magicAction->setCheckable(true);
	magicAction->setChecked(false);
	magicAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	magicAction->setObjectName("magicAction");
	actions[magic_action] = magicAction;

	QAction* savecutAction = new QAction(icons[savecut_icon], tr("Save the selected region"), this);
	savecutAction->setStatusTip(tr("Open the Save Dialog for saving the currently selected region"));
	savecutAction->setCheckable(false);
	savecutAction->setChecked(false);
	savecutAction->setEnabled(false);
	savecutAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	savecutAction->setObjectName("savecutAction");
	actions[savecut_action] = savecutAction;

	QAction* clearselectionAction = new QAction(icons[clearselection_icon], tr("Clear selection"), this);
	clearselectionAction->setShortcut(Qt::SHIFT + Qt::Key_C);
	clearselectionAction->setStatusTip(tr("Clear the current selection"));
	clearselectionAction->setCheckable(false);
	clearselectionAction->setChecked(false);
	clearselectionAction->setEnabled(false);
	clearselectionAction->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable
	clearselectionAction->setObjectName("clearselectionAction");
	actions[clearselection_action] = clearselectionAction;

	QLabel *lbl_tolerance = new QLabel(tr("Tolerance:"));
	lbl_tolerance->setStatusTip(tr("Set the maximum color difference tolerance for flood-filling"));
	lbl_tolerance->setWhatsThis(tr("alwaysenabled")); // set flag to always make this icon clickable

	QSpinBox *toleranceBox = new QSpinBox();
	toleranceBox->setStatusTip(tr("Set the maximum color difference tolerance for flood-filling"));
	toleranceBox->setRange(0, 255);
	toleranceBox->setValue(40);
	toleranceBox->setWhatsThis(tr("alwaysenabled"));
	toleranceBox->setObjectName("toleranceBox");

	// actually add things to interface
	addAction(linedetectionAction);
	addAction(showbottomlinesAction);
	addAction(showtoplinesAction);
	addSeparator();
	addAction(distanceAction);
	addSeparator();
	addAction(magicAction);
	addAction(savecutAction);
	addAction(clearselectionAction);
	addWidget(lbl_tolerance);
	addWidget(toleranceBox);
}

/**
* Make the DocAnalyis toolbar visible
**/
void DkDocAnalysisToolBar::setVisible(bool visible) {

	qDebug() << "[DOCANALYSIS TOOLBAR] set visible: " << visible;

	QToolBar::setVisible(visible);
}

/**
* Called when the detect lines tool icon is clicked.
* Emits signal (a request) to open the configuration dialog for detecting text lines.
* \sa detectLinesSignal() DkViewPortContrast::openLineDetectionDialog() DkLineDetection
**/
void DkDocAnalysisToolBar::on_linedetectionAction_triggered() {
	emit detectLinesSignal();
}

/**
* Called when the show bottom text lines tool icon is pressed.
* Willd display the previously detected bottom text lines if available.
* Emits signal (a request) to display/hide the lines.
* \sa showBottomTextLinesSignal(bool) DkViewPortContrast::showBottomTextLines(bool show) DkLineDetection
**/
void DkDocAnalysisToolBar::on_showbottomlinesAction_triggered() {
	emit showBottomTextLinesSignal(actions[showbottomlines_action]->isChecked());
}

/**
* Called when the show top text lines tool icon is pressed.
* Willd display the previously detected top text lines if available.
* Emits signal (a request) to display/hide the lines.
* \sa showTopTextLinesSignal(bool) DkViewPortContrast::showBottomTextLines(bool show) DkLineDetection
**/
void DkDocAnalysisToolBar::on_showtoplinesAction_triggered() {
	emit showTopTextLinesSignal(actions[showtoplines_action]->isChecked());
}

/**
* Called when the measure distance tool icon is clicked.
* Emits signal (a request) to start/end this tool.
* \sa measureDistanceRequest(bool) DkViewPortContrast::pickDistancePoint(bool pick) DkDistanceMeasure
**/
void DkDocAnalysisToolBar::on_distanceAction_toggled(bool checked) {
	emit measureDistanceRequest(actions[distance_action]->isChecked());
}

/**
* Called when the magic cut tool icon is clicked.
* Emits signal (a request) to start/end this tool.
* \sa pickSeedpointRequest(bool) DkViewPortContrast::pickSeedpoint(bool pick) DkMagicCut
**/
void DkDocAnalysisToolBar::on_magicAction_toggled(bool checked) {
	emit pickSeedpointRequest(actions[magic_action]->isChecked());
}

/**
* Called when the currently selected magic cut areas shall be saved (click on the icon)
* \sa openCutDialogSignal() DkViewPortContrast::openMagicCutDialog() DkMagicCutDialog
**/
void DkDocAnalysisToolBar::on_savecutAction_triggered() {
	emit openCutDialogSignal();
}

/**
* Called when the clear magic cut selection tool icon is clicked.
* Emits signal (a request) to clear all selected magic cut regions.
* \sa clearSelectionSignal() DkViewPortContrast::clearMagicCut() DkMagicCut
**/
void DkDocAnalysisToolBar::on_clearselectionAction_triggered() {
	emit clearSelectionSignal();
}

/**
* Called when the tolerance value has changed within the toolbar.
* @param value The new tolerance value
* \sa toeranceChanged(int) DkViewPortContrast::setMagicCutTolerance(int) DkMagicCut::magicwand(QPoint)
**/
void DkDocAnalysisToolBar::on_toleranceBox_valueChanged(int value) {
	emit toleranceChanged(value);
}


/**
* Slot - called when the user canceles during picking a seedpoint in the magic cut tool.
* Untoggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::pickSeedpointCanceled() {
	actions[magic_action]->setChecked(false);
}

/**
* Slot - called when the user starts the picking seedpoints for magic cut tool (e.g. via shortcut).
* Toggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::pickSeedpointStarted() {
	actions[magic_action]->setChecked(true);
}

/**
* Slot - called when the user canceles during picking a distance measure point.
* Untoggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::measureDistanceCanceled() {
	actions[distance_action]->setChecked(false);
}

/**
* Slot - called when the user requests to measure the distance (e.g. via shortcut).
* Toggles the corresponding tool icon.
**/
void DkDocAnalysisToolBar::measureDistanceStarted() {
	actions[distance_action]->setChecked(true);
}

/**
* Slot - called when the icon should be enabled or disabled (e.g. nothing selected, no image, ...)
**/
void DkDocAnalysisToolBar::enableButtonSaveCut(bool enable) {
	actions[savecut_action]->setEnabled(enable);
	// also enable the clear selection, since something has been selected
	actions[clearselection_action]->setEnabled(enable);
}

/**
* Slot - called when the icon should be enabled or disabled (e.g. no image, no text lines detected...)
**/
void DkDocAnalysisToolBar::enableButtonShowTextLines(bool enable) {
	actions[showbottomlines_action]->setEnabled(enable);
	actions[showtoplines_action]->setEnabled(enable);
}

/**
* Slot - called when after calculation of text lines the bottom text lines are automatically
* displayed to also toggle the corresponding icon
**/
void DkDocAnalysisToolBar::toggleBottomTextLinesButton(bool toggle) {
	actions[showbottomlines_action]->setChecked(toggle);
}

/**
* Slot - called when after calculation of text lines the top text lines are automatically
* enabled, in case they have been enabled in a previous text line calculation
**/
void DkDocAnalysisToolBar::toggleTopTextLinesButton(bool toggle) {
	actions[showtoplines_action]->setChecked(toggle);
}


};
