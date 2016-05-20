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
#include <QCombobox>
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
		mtoolbar(new DkPatchMatchingToolBar(tr("PatchMatching Toolbar)"),this), &QObject::deleteLater),
		mPolygon(QSharedPointer<DkSyncedPolygon>::create()),
		mDock(new QDockWidget(this), &QObject::deleteLater),
		mTimeline(new DkPolyTimeline(mPolygon, mDock.data()), &QObject::deleteLater),
		defaultCursor(Qt::CrossCursor), 
		mCurrentPolygon(-1)
	{

		setObjectName("DkPatchMatchingViewPort");
		setMouseTracking(true);
		setAttribute(Qt::WA_MouseTracking);

		setCursor(defaultCursor);

		loadSettings();

		// handler to clone the polygon
		connect(mtoolbar.data(), &DkPatchMatchingToolBar::clonePolyTriggered, this, &DkPatchMatchingViewPort::clonePolygon);
		connect(mtoolbar.data(), &DkPatchMatchingToolBar::saveTriggered, this, &DkPatchMatchingViewPort::saveToFile);
		connect(mtoolbar.data(), &DkPatchMatchingToolBar::addPolyTriggerd, this, &DkPatchMatchingViewPort::addPolygon);
		connect(mtoolbar.data(), &DkPatchMatchingToolBar::closeTriggerd, this, &DkPatchMatchingViewPort::discardChangesAndClose);
		connect(mtoolbar.data(), &DkPatchMatchingToolBar::currentPolyChanged, this, &DkPatchMatchingViewPort::changeCurrentPolygon);
		// timeline stuff
		mTimeline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		mTimeline->setStepSize(mtoolbar->getStepSize());
		connect(mtoolbar.data(), &DkPatchMatchingToolBar::stepSizeChanged, mTimeline.data(), &DkPolyTimeline::setStepSize);

		// add the neccesarry dock
		mDock->setStyleSheet("QDockWidget{background-color: #f00;}");
		mDock->setWidget(mTimeline.data());
		dynamic_cast<QMainWindow*>(qApp->activeWindow())->addDockWidget(Qt::BottomDockWidgetArea, mDock.data());
	}

	void DkPatchMatchingViewPort::updateImageContainer(QSharedPointer<nmc::DkImageContainerT> imgC)
	{
		mImage = imgC;
		loadFromFile();
	}

	void DkPatchMatchingViewPort::loadFromFile()
	{
		mCurrentPolygon = -1;
		mtoolbar->clearPolygons();
		mPolygonList.clear();

		QFile file{ getJsonFilePath() };
		if (!file.open(QIODevice::ReadOnly)) {
			qDebug() << "[PatchMatchingPlugin] No json file found for this image: " << getJsonFilePath();

			mtoolbar->addPolygon(true);
			//mPolygon->setInactive(true);
		}
		else {
			auto doc = QJsonDocument::fromJson(file.readAll());
			qDebug() << "[PatchMatchingPlugin] Json file found and successfully read.";

			auto list = doc.array();
			auto first = list.at(0).toObject();
			
			for (auto p : list) {
				mPolygonList << p.toObject();
				mtoolbar->addPolygon(first == p.toObject());
			}
		}
	}

	void DkPatchMatchingViewPort::load(QJsonObject& polygon)
	{
		// read synced polygon
		mPolygon->read(polygon["polygon"].toObject());
		
		// read clones
		auto array = polygon["clones"].toArray();
		for (auto obj : array) {
			auto poly = addClone();
			poly->read(obj.toObject());
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
		return QColor::fromHsvF(fmod(static_cast<double>(idx)*div, 1), 1, 1);
	}

	void DkPatchMatchingViewPort::loadSettings() {

		QSettings& settings = nmc::Settings::instance().getSettings();

		settings.beginGroup(objectName());
		mtoolbar->setStepSize(settings.value("StepSize", 50).toInt());
		settings.endGroup();
	}

	void DkPatchMatchingViewPort::clonePolygon()
	{
		auto poly = addClone();
		poly->translate(400, 0);

		update();

		emit polygonAdded();
	}

	void DkPatchMatchingViewPort::addPolygon()
	{
		// just to make sure check that an polygon is actually selected
		mtoolbar->addPolygon(true);

		qDebug() << "[PatchMatchingPlugin] add polygon triggerd";
	}

	void DkPatchMatchingViewPort::saveToFile()
	{
		saveCurrent(); // store current polygon

		QJsonArray root;
		for (auto obj : mPolygonList) {
			root << obj;
		}

		// write to file
		QFile saveFile(getJsonFilePath());

		if (!saveFile.open(QIODevice::WriteOnly)) {
			qWarning("Couldn't open saveCurrent file.");
			return;
		}

		QJsonDocument doc{ root };
		saveFile.write(doc.toJson());

		qDebug() << "[PatchMatchingPlugin] Saving file : Success!!!";

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
			addClone();
		}
		return mRenderer.first();
	}

	QSharedPointer<DkPolygonRenderer> DkPatchMatchingViewPort::addClone()
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
		if (checked) defaultCursor = Qt::OpenHandCursor;
		else defaultCursor = Qt::CrossCursor;
		setCursor(defaultCursor);
	}

	void DkPatchMatchingViewPort::saveCurrent() {

		QJsonObject polygon;

		// save the synced polygon
		QJsonObject syncedPoly;
		mPolygon->write(syncedPoly);
		polygon["polygon"] = syncedPoly;

		// save all clones
		QJsonArray array;
		for (auto p : mRenderer) {
			QJsonObject obj;
			p->write(obj);
			array.append(obj);
		}
		polygon["clones"] = array;

		if (mCurrentPolygon >= mPolygonList.size() ) {
			mPolygonList.resize(mCurrentPolygon +1);
		}
		mPolygonList[mCurrentPolygon] = polygon;
	}

	void DkPatchMatchingViewPort::changeCurrentPolygon(int idx)
	{
		if (mCurrentPolygon == idx) {
			return;
		}

		if (mCurrentPolygon >= 0) {
			saveCurrent();
		}

		mCurrentPolygon = idx;
		emit reset(mImage);

		if (mCurrentPolygon < mPolygonList.size()) {	
			load(mPolygonList[mCurrentPolygon]);		// load existing
		} 
	}

	//QSharedPointer<DkSyncedPolygon> DkPatchMatchingViewPort::currentPolygon()
	//{
	//	return mPolygonList[mCurrentPolygon];
	//}

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

		createLayout();

		if (nmc::Settings::param().display().toolbarGradient) {

			QColor hCol = nmc::Settings::param().display().highlightColor;
			hCol.setAlpha(80);

			setStyleSheet(
				QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); spacing: 3px; padding: 3px;}")
				+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
				+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + nmc::DkUtils::colorToString(hCol) + ";}")
			);
		}
		else {
			setStyleSheet("QToolBar{spacing: 3px; padding: 3px;}");
		}

		qDebug() << "[PAINT TOOLBAR] created...";
	}

	void DkPatchMatchingToolBar::createLayout() {

		// setup clone action
		mClonePolyAction = new QAction(tr("Clone"), this);
		mClonePolyAction->setShortcuts(QList<QKeySequence>{ QKeySequence(Qt::Key_Enter), QKeySequence(Qt::Key_Return) });
		connect(mClonePolyAction, &QAction::triggered, this, &DkPatchMatchingToolBar::clonePolyTriggered);

		// add actions to toolbar
		addAction(mClonePolyAction);
		addSeparator();

		// step size for timeline
		mStepSizeSpinner = new QSpinBox(this);
		mStepSizeSpinner->setObjectName("mStepSizeSpinner");
		mStepSizeSpinner->setSuffix("px");
		mStepSizeSpinner->setMinimum(10);
		mStepSizeSpinner->setMaximum(500);

		addWidget(new QLabel{ "Resolution:",this });
		addWidget(mStepSizeSpinner);

		connect(mStepSizeSpinner, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &DkPatchMatchingToolBar::stepSizeChanged);
		addSeparator();

		mPolygonCombobox = new QComboBox(this);
		addWidget(mPolygonCombobox);
		connect(mPolygonCombobox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
											, this, &DkPatchMatchingToolBar::currentPolyChanged);

		auto addPolygonWidget = new QAction{ tr("Add"), this };
		addAction(addPolygonWidget);
		connect(addPolygonWidget, &QAction::triggered, this, &DkPatchMatchingToolBar::addPolyTriggerd);

		addSeparator();

		QAction* saveAction = new QAction{ tr("Save"), this };
		connect(saveAction, &QAction::triggered, this, &DkPatchMatchingToolBar::saveTriggered);

		addAction(saveAction);

		QAction* close = new QAction{ tr("Close"), this };
		connect(close, &QAction::triggered, this, &DkPatchMatchingToolBar::closeTriggerd);

		addAction(close);
	}

	void DkPatchMatchingToolBar::setVisible(bool visible) {

		qDebug() << "[PAINT TOOLBAR] set visible: " << visible;

		QToolBar::setVisible(visible);
	}

	int DkPatchMatchingToolBar::getStepSize()
	{
		return mStepSizeSpinner->value();
	}

	void DkPatchMatchingToolBar::setStepSize(int size)
	{
		mStepSizeSpinner->setValue(size);
	}
	int DkPatchMatchingToolBar::getCurrentPolygon()
	{
		return mPolygonCombobox->currentIndex();
	}
	void DkPatchMatchingToolBar::addPolygon(bool select)
	{
		auto text = "Polygon " + QString::number(mPolygonCombobox->count() + 1);
		mPolygonCombobox->addItem(text);
		if (select) {
			mPolygonCombobox->setCurrentIndex(mPolygonCombobox->count() - 1);
		}
	}
	void DkPatchMatchingToolBar::clearPolygons()
	{
		mPolygonCombobox->clear();
	}
};
