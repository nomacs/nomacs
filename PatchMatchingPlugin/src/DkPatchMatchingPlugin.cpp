/*******************************************************************************************************
 DkPatchMatchingPlugin.cpp
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

#include "DkPatchMatchingPlugin.h"

#include <QDebug>
#include <QMouseEvent>
#include <QActionGroup>
#include "DkPolyTimeline.h"
#include <QjsonDocument>
#include <Qjsonarray>
namespace nmp {

/*-----------------------------------DkPatchMatchingPlugin ---------------------------------------------*/

/**
*	Constructor
**/
DkPatchMatchingPlugin::DkPatchMatchingPlugin() 
	:mViewport(nullptr)
{
}

/**
*	Destructor
**/
DkPatchMatchingPlugin::~DkPatchMatchingPlugin() {

	qDebug() << "[PAINT PLUGIN] deleted...";
}

/**
* Returns unique ID for the generated dll
**/
QString DkPatchMatchingPlugin::id() const {
	return PLUGIN_ID;
};


/**
* Returns descriptive image
**/
QImage DkPatchMatchingPlugin::image() const {

   return QImage(":/nomacsPluginPaint/img/description.png");
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QSharedPointer<nmc::DkImageContainer> DkPatchMatchingPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> image) const {
	
	qDebug() << "Run PatchMatchinPlugin";

	return image;
};

/**
* returns paintViewPort
**/
nmc::DkPluginViewPort* DkPatchMatchingPlugin::getViewPort() {

	qDebug() << "Get viewport";

	if (!mViewport) {
		mViewport = new DkPatchMatchingViewPort;
	}
	return mViewport;
}

void DkPatchMatchingPlugin::deleteViewPort() {

	if (mViewport) {
		mViewport->deleteLater();
		mViewport = nullptr;
	}
}

bool DkPatchMatchingPlugin::closesOnImageChange() const
{
	return false;
}

	/*-----------------------------------DkPatchMatchingViewPort ---------------------------------------------*/

DkPatchMatchingViewPort::DkPatchMatchingViewPort(QWidget* parent, Qt::WindowFlags flags) 
	: DkPluginViewPort(parent, flags), 
		cancelTriggered(false),
		panning(false),
		mtoolbar(new DkPatchMatchingToolBar(tr("Paint Toolbar"), this), &QObject::deleteLater),
		mPolygon(QSharedPointer<DkSyncedPolygon>::create()),
		mDock(new QDockWidget(this), &QObject::deleteLater),
		mTimeline(new DkPolyTimeline(mPolygon, mDock.data()), &QObject::deleteLater),
		defaultCursor(Qt::CrossCursor)
{

	setObjectName("DkPatchMatchingViewPort");
	setMouseTracking(true);
	setAttribute(Qt::WA_MouseTracking);
	
	setCursor(defaultCursor);

	// 
	// OLD Toolbar stuff
	// 
	connect(mtoolbar.data(), SIGNAL(colorSignal(QColor)), this, SLOT(setPenColor(QColor)));
	connect(mtoolbar.data(), SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
	connect(mtoolbar.data(), SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
	connect(mtoolbar.data(), SIGNAL(undoSignal()), this, SLOT(undoLastPaint()));
	connect(mtoolbar.data(), SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));

	loadSettings();

	// 
	// ~~END OLD Toolbar stuff
	// 

	// handler to clone the polygon
	connect(mtoolbar.data(), &DkPatchMatchingToolBar::clonePolyTriggered, this, &DkPatchMatchingViewPort::clonePolygon);
	connect(mtoolbar.data(), &DkPatchMatchingToolBar::selectedToolChanged, this, &DkPatchMatchingViewPort::selectedToolChanged);
	

	// add timeline stuff, should probably move this to the plugin part
	// to separate it better from the view
	mTimeline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	mTimeline->setStepSize(mtoolbar->getStepSize());
	connect(mtoolbar.data(), &DkPatchMatchingToolBar::stepSizeChanged, mTimeline.data(), &DkPolyTimeline::setStepSize);

	//QLabel* l = new QLabel(this);
	//l->setStyleSheet("background-color: #00f;");

	mDock->setStyleSheet("QDockWidget{background-color: #f00;}");
	mDock->setWidget(mTimeline.data());
	
	dynamic_cast<QMainWindow*>(qApp->activeWindow())->addDockWidget(Qt::BottomDockWidgetArea, mDock.data());
}

void DkPatchMatchingViewPort::updateImageContainer(QSharedPointer<nmc::DkImageContainerT> imgC)
{
	mImage = imgC;

	// just emit reset to clear everything
	emit reset(mImage);

	QFile file{ getJsonFilePath() };
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "[PatchMatchingPlugin] No json file found for this image: " << getJsonFilePath();
	}
	else {
		auto doc = QJsonDocument::fromJson(file.readAll());
		auto root = doc.object();
		mPolygon->read(root["polygon"].toObject());
		qDebug() << "[PatchMatchingPlugin] Json file found and successfully read.";
		
		auto array = root["clones"].toArray();
		for (auto obj : array) {
			auto poly = addPoly();
			poly->read(obj.toObject());
		}
	}
}

QString DkPatchMatchingViewPort::getJsonFilePath() const
{
	if (!mImage) {
		return "";
	}
	return mImage->filePath() + ".patches.json";
}

DkPatchMatchingViewPort::~DkPatchMatchingViewPort() {
	qDebug() << "[VIEWPORT] deleted...";
	
	saveSettings();
}


void DkPatchMatchingViewPort::saveSettings() const {

	QSettings& settings = nmc::Settings::instance().getSettings();

	settings.beginGroup(objectName());
	settings.setValue("StepSize", mtoolbar->getStepSize());
	settings.endGroup();
}

QColor DkPatchMatchingViewPort::getIndexedColor(int idx)
{
	const auto div = 60 / 360.;
	return QColor::fromHsvF(fmod(static_cast<double>(idx)*div,1),1,1);
}

void DkPatchMatchingViewPort::loadSettings() {

	QSettings& settings = nmc:: Settings::instance().getSettings();

	settings.beginGroup(objectName());
	mtoolbar->setStepSize(settings.value("StepSize", 50).toInt());
	settings.endGroup();
}

void DkPatchMatchingViewPort::clonePolygon()
{
	auto poly = addPoly();
	poly->translate(400, 0);

	update();

	emit polygonAdded();
}

void DkPatchMatchingViewPort::selectedToolChanged(SelectedTool tool)
{
	qDebug() << "Selected tool changed to :" << static_cast<int>(tool);
}


void DkPatchMatchingViewPort::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == nmc::Settings::param().global().altMod || panning)) {
		setCursor(Qt::ClosedHandCursor);
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	if (event->buttons() == Qt::LeftButton && parent()) {
		QPointF point = event->pos(); //
		
		firstPoly()->addPointMouseCoords(point);
	}
}

void DkPatchMatchingViewPort::mouseMoveEvent(QMouseEvent *event) {
	// panning -> redirect to viewport
	if (event->modifiers() == nmc::Settings::param().global().altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
	}
}

void DkPatchMatchingViewPort::mouseReleaseEvent(QMouseEvent *event) {
	// panning -> redirect to viewport
	if (event->modifiers() == nmc::Settings::param().global().altMod || panning) {
		setCursor(defaultCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
	}
}

void DkPatchMatchingViewPort::paintEvent(QPaintEvent *event) {
	checkWorldMatrixChanged();
	DkPluginViewPort::paintEvent(event);
}

void DkPatchMatchingViewPort::checkWorldMatrixChanged()
{
	QTransform mat;
	if (mImgMatrix) {
		mat = *mImgMatrix;
	}
	if (mWorldMatrix) {
		mat = mat*(*mWorldMatrix);
	}
	if (mat != mWorldMatrixCache) {
		mWorldMatrixCache = mat;
		emit worldMatrixChanged(mWorldMatrixCache);
	}
}

QSharedPointer<DkPolygonRenderer> DkPatchMatchingViewPort::firstPoly()
{
	if (mRenderer.empty()) {
		addPoly();
	}
	return mRenderer.first();
}

QSharedPointer<DkPolygonRenderer> DkPatchMatchingViewPort::addPoly()
{
	auto render = QSharedPointer<DkPolygonRenderer>::create(this, mPolygon.data(), mWorldMatrixCache);
	
	render->setColor(getIndexedColor(mRenderer.size()));
	render->setImageRect(mImage->image().rect());
	
	connect(this, &DkPatchMatchingViewPort::worldMatrixChanged, render.data(), &DkPolygonRenderer::setWorldMatrix);

	mRenderer.append(render);

	// add a new timeline for this renderer
	auto transform = mTimeline->addPolygon(render->getColor());
	mTimeline->setImage(mImage);
	
	//// connections for timeline
	connect(render.data(), &DkPolygonRenderer::transformChanged, 
		transform.data(), &std::remove_pointer<decltype(transform.data())>::type::set);

	mTimeline->refresh();

	// this is our cleanup slot
	connect(render.data(), &DkPolygonRenderer::removed, this,
	
		[this, render, transform]() {
			// disconnect the polygon and clear it (delete all QWidgets which have viewport as parent)
			render.data()->disconnect();
			render->clear();

			mTimeline->removeTransform(transform);

			// remove the polygon from the renderer list
			mRenderer.removeAll(render);

			// if no render exists anymore also clean up the synced polygon
			if (mRenderer.empty()) {
				mPolygon->clear();
			}

	});

	// remove renderer if the whole viewport is reset
	connect(this, &DkPatchMatchingViewPort::reset, render.data(), &DkPolygonRenderer::removed);

	return render;
}


void DkPatchMatchingViewPort::setPanning(bool checked) {

	this->panning = checked;
	if(checked) defaultCursor = Qt::OpenHandCursor;
	else defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
}

void DkPatchMatchingViewPort::applyChangesAndClose() {
	
	QJsonObject root;

	QJsonObject syncedPoly;
	mPolygon->write(syncedPoly);
	root["polygon"] = syncedPoly;

	QJsonArray array;
	for (auto p : mRenderer) {
		QJsonObject obj;
		p->write(obj);
		array.append(obj);
	}
	root["clones"] = array;

	QFile saveFile(getJsonFilePath());
	
	if (!saveFile.open(QIODevice::WriteOnly)) {
		qWarning("Couldn't open save file.");
		return;
	}
	
	QJsonDocument doc{ root };
	saveFile.write(doc.toJson());

	qDebug() << "[PatchMatchingPlugin] Saving file : Success!!!";
}

void DkPatchMatchingViewPort::discardChangesAndClose() {

	cancelTriggered = true;
	emit DkPluginViewPort::closePlugin();
}

void DkPatchMatchingViewPort::setVisible(bool visible) {

	if (mtoolbar)
		emit DkPluginViewPort::showToolbar(mtoolbar.data(), visible);

	DkPluginViewPort::setVisible(visible);
}
	/*-----------------------------------DkPatchMatchingToolBar ---------------------------------------------*/
DkPatchMatchingToolBar::DkPatchMatchingToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

	setObjectName("paintToolBar");
	createIcons();
	createLayout();
	QMetaObject::connectSlotsByName(this);

	setIconSize(QSize(nmc::Settings::param().display().iconSize, nmc::Settings::param().display().iconSize));

	if (nmc::Settings::param().display().toolbarGradient) {

		QColor hCol = nmc::Settings::param().display().highlightColor;
		hCol.setAlpha(80);

		setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); spacing: 3px; padding: 3px;}")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + nmc::DkUtils::colorToString(hCol) + ";}")
			);
	}
	else
		setStyleSheet("QToolBar{spacing: 3px; padding: 3px;}");

	qDebug() << "[PAINT TOOLBAR] created...";
}

void DkPatchMatchingToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[apply_icon] = QIcon(":/nomacsPluginPaint/img/apply.png");
	icons[cancel_icon] = QIcon(":/nomacsPluginPaint/img/cancel.png");
	icons[pan_icon] = 	QIcon(":/nomacsPluginPaint/img/pan.png");
	icons[pan_icon].addPixmap(QPixmap(":/nomacsPluginPaint/img/pan_checked.png"), QIcon::Normal, QIcon::On);
	icons[undo_icon] = 	QIcon(":/nomacsPluginPaint/img/undo.png");

	if (!nmc::Settings::param().display().defaultIconColor || nmc::Settings::param().app().privateMode) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(nmc::DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), nmc::Settings::param().display().iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(nmc::DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), nmc::Settings::param().display().iconColor), QIcon::Normal, QIcon::Off);
		}
	}
}

void DkPatchMatchingToolBar::createLayout() {

	
	// Setup group which handles mode changes
	mModeGroup = new QActionGroup(this);
	mModeGroup->setExclusive(true);		
	
	auto create = [this](const char* name) {	//convenience, add actions to group
		QAction* action = new QAction(tr(name), this);
		action->setCheckable(true);
		action->setChecked(false);
		mModeGroup->addAction(action);
		return action;
	};

	mAddPointAction = create("Add");
	mRemovePointAction = create("Remove");
	mMoveAction = create("Move");
	mRotateAction = create("Roate");
	mAddPointAction->setChecked(true);

	// connect with single handler
	connect(mModeGroup, &QActionGroup::triggered, this, &DkPatchMatchingToolBar::modeChangeTriggered);

	// setup clone action
	mClonePolyAction = new QAction(tr("Clone"), this);
	connect(mClonePolyAction, &QAction::triggered, this, &DkPatchMatchingToolBar::clonePolyTriggered);


	// add actions to toolbar
	//addActions(mModeGroup->actions());
	//addSeparator();
	addAction(mClonePolyAction);
	addSeparator();

	// step size for timeline
	mStepSizeSpinner = new QSpinBox(this);
	mStepSizeSpinner->setObjectName("mStepSizeSpinner");
	mStepSizeSpinner->setSuffix("px");
	mStepSizeSpinner->setMinimum(10);
	mStepSizeSpinner->setMaximum(500);

	addWidget(mStepSizeSpinner);

	connect(mStepSizeSpinner, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), 
						this, &DkPatchMatchingToolBar::stepSizeChanged);
	addSeparator();

	//
	// OLD STUFF -> We should change this perhaps if we don't need it anymore
	//
	QList<QKeySequence> enterSc;
	enterSc.append(QKeySequence(Qt::Key_Enter));
	enterSc.append(QKeySequence(Qt::Key_Return));

	QAction* applyAction = new QAction(icons[apply_icon], tr("Apply (ENTER)"), this);
	applyAction->setShortcuts(enterSc);
	applyAction->setObjectName("applyAction");

	QAction* cancelAction = new QAction(icons[cancel_icon], tr("Cancel (ESC)"), this);
	cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
	cancelAction->setObjectName("cancelAction");


	panAction = new QAction(icons[pan_icon], tr("Pan"), this);
	panAction->setShortcut(QKeySequence(Qt::Key_P));
	panAction->setObjectName("panAction");
	panAction->setCheckable(true);
	panAction->setChecked(false);

	// pen colorc
	penCol = QColor(0,0,0);
	penColButton = new QPushButton(this);
	penColButton->setObjectName("penColButton");
	penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
	penColButton->setToolTip(tr("Background Color"));
	penColButton->setStatusTip(penColButton->toolTip());

	// undo Button
	undoAction = new QAction(icons[undo_icon], tr("Undo (CTRL+Z)"), this);
	undoAction->setShortcut(QKeySequence::Undo);
	undoAction->setObjectName("undoAction");

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");

	// pen alpha
	alphaBox = new QSpinBox(this);
	alphaBox->setObjectName("alphaBox");
	alphaBox->setSuffix("%");
	alphaBox->setMinimum(0);
	alphaBox->setMaximum(100);

	addAction(applyAction);
	addAction(cancelAction);
	addSeparator();
	addAction(panAction);
	addAction(undoAction);

	addWidget(penColButton);
	addWidget(alphaBox);
}

void DkPatchMatchingToolBar::modeChangeTriggered(QAction* action)
{
	if (action == mAddPointAction) emit selectedToolChanged(SelectedTool::AddPoint);
	if (action == mRemovePointAction) emit selectedToolChanged(SelectedTool::RemovePoint);
	if (action == mMoveAction) emit selectedToolChanged(SelectedTool::Move);
	if (action == mRotateAction) emit selectedToolChanged(SelectedTool::Rotate);
}


void DkPatchMatchingToolBar::setVisible(bool visible) {

	//if (!visible)
	//	emit colorSignal(QColor(0,0,0));
	if (visible) {
		//emit colorSignal(penCol);
		//widthBox->setValue(10);
		//alphaBox->setValue(100);
		panAction->setChecked(false);
	}

	qDebug() << "[PAINT TOOLBAR] set visible: " << visible;

	QToolBar::setVisible(visible);
}

void DkPatchMatchingToolBar::setPenColor(const QColor& col) {

	penCol = col;
	penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
	penAlpha = col.alpha();
	alphaBox->setValue(col.alphaF()*100);
}

void DkPatchMatchingToolBar::setPenWidth(int width) {

	mStepSizeSpinner->setValue(width);
}

int DkPatchMatchingToolBar::getStepSize()
{
	return mStepSizeSpinner->value();
}

void DkPatchMatchingToolBar::setStepSize(int size)
{
	mStepSizeSpinner->setValue(size);
}

void DkPatchMatchingToolBar::on_undoAction_triggered() {
	emit undoSignal();
}

void DkPatchMatchingToolBar::on_applyAction_triggered() {
	emit applySignal();
}

void DkPatchMatchingToolBar::on_cancelAction_triggered() {
	emit cancelSignal();
}

void DkPatchMatchingToolBar::on_panAction_toggled(bool checked) {

	emit panSignal(checked);
}

void DkPatchMatchingToolBar::on_alphaBox_valueChanged(int val) {

	penAlpha = val;
	QColor penColWA = penCol;
	penColWA.setAlphaF(penAlpha/100.0);
	emit colorSignal(penColWA);
}

void DkPatchMatchingToolBar::on_penColButton_clicked() {

	QColor tmpCol = penCol;
	
	colorDialog->setCurrentColor(tmpCol);
	int ok = colorDialog->exec();

	if (ok == QDialog::Accepted) {
		penCol = colorDialog->currentColor();
		penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
		
		QColor penColWA = penCol;
		penColWA.setAlphaF(penAlpha/100.0);
		emit colorSignal(penColWA);
	}

}

};
