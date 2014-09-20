/*******************************************************************************************************
 DkWidgets.cpp
 Created on:	17.05.2011
 
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

#include "DkWidgets.h"

#include "DkNoMacs.h"

namespace nmc {

DkWidget::DkWidget(QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags) {
	init();
}

void DkWidget::init() {

	setMouseTracking(true);

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
		DkSettings::display.bgColorWidget;
	
	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;
	opacityEffect = 0;

	// painter problems if the widget is a child of another that has the same graphicseffect
	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);
	setGraphicsEffect(opacityEffect);

	setVisible(false);
}

void DkWidget::show() {

	// here is a strange problem if you add a DkWidget to another DkWidget -> painters crash
	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true);
		animateOpacityUp();
	}
}

void DkWidget::hide() {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();

		// set display bit here too -> since the final call to setVisible takes a few seconds
		if (displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
			displaySettingsBits->setBit(DkSettings::app.currentAppMode, false);
		}
	}
}

void DkWidget::setVisible(bool visible) {
	
	if (blocked) {
		QWidget::setVisible(false);
		return;
	}

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	QWidget::setVisible(visible);
	emit visibleSignal(visible);	// if this gets slow -> put it into hide() or show()

	if (displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, visible);
	}
}

void DkWidget::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		showing = false;
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkWidget::animateOpacityDown() {

	if (!hiding)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		setVisible(false);	// finally hide the widget
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

// DkFolderScrollBar --------------------------------------------------------------------
DkFolderScrollBar::DkFolderScrollBar(QWidget* parent) : QScrollBar(Qt::Horizontal, parent) {
//#include <QStyle>
	minHandleWidth = 30;
	sliding = false;

	setMouseTracking(true);

	// apply style
	QVector<QColor> dummy;
	QVector<int> dummyIdx;
	update(dummy, dummyIdx);
 
	dummyWidget = new DkWidget(this);
	
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(emitFileSignal(int)));

	qRegisterMetaType<QVector<QColor> >("QVector<QColor>");
	qRegisterMetaType<QVector<int> >("QVector<int>");

	handle = new QLabel(this);
	handle->setMouseTracking(true);
	handle->setStyleSheet(QString("QLabel{border: 1px solid ")
		+ DkUtils::colorToString(DkSettings::display.highlightColor) + 
		QString("; background-color: ") + DkUtils::colorToString(DkSettings::display.bgColorWidget) + QString(";}"));
	updateFolder = false;
	updatesWaiting = 0;
	init();
}

DkFolderScrollBar::~DkFolderScrollBar() {

}

void DkFolderScrollBar::updateDir(QVector<QSharedPointer<DkImageContainerT> > images) {

	this->images = images;
	updateFolder = true;

	if (isVisible())
		updateColors();
}

void DkFolderScrollBar::updateColors() {

	if (!updateFolder)
		return;

	int maxThumbs = 800;

	indexes.clear();
	colors.clear();

	for (int idx = 0; idx < maxThumbs && idx < images.size(); idx++) {

		int fIdx = (images.size() > maxThumbs) ? qRound((float)idx/maxThumbs*(images.size()-1)) : idx;
		QSharedPointer<DkThumbNailT> thumb = images.at(fIdx)->getThumb();
		connect(thumb.data(), SIGNAL(colorUpdated()), this, SLOT(colorUpdated()));
		thumb->fetchColor();
		indexes.append(fIdx);
		colors.append(DkSettings::display.bgColorWidget);
	}

	handle->setFixedWidth((qRound(1.0f/(images.size()*this->width()+FLT_EPSILON)) < minHandleWidth) ? minHandleWidth : qRound(1.0f/(images.size()*this->width()+FLT_EPSILON)));
	setMaximum(images.size());

	updatesWaiting = 0;
	updateFolder = false;
}

void DkFolderScrollBar::colorUpdated() {

	// we keep a responsive gui by these means & loose the last 10 images
	if (updatesWaiting < 20) {
		updatesWaiting++;
		return;
	}
	updatesWaiting = 0;

	for (int idx = 0; idx < indexes.size(); idx++) {

		QSharedPointer<DkThumbNailT> thumb = images.at(indexes.at(idx))->getThumb();

		if (thumb->getMeanColor() != DkSettings::display.bgColorWidget)
			colors[idx] = thumb->getMeanColor();
	}

	update(colors, indexes);
}

void DkFolderScrollBar::updateFile(QSharedPointer<DkImageContainerT> imgC) {
	
	if (!sliding)
		cImg = imgC;

	if (isVisible() && cImg) {
		blockSignals(true);
		setValue(fileIdx(imgC));
		blockSignals(false);
	}	
}

void DkFolderScrollBar::update(const QVector<QColor>& colors, const QVector<int>& indexes) {

	float offset = 0;

	if (!images.empty()) {
		handle->setFixedWidth((qRound(1.0f/images.size()*this->width()) < minHandleWidth) ? minHandleWidth : qRound(1.0f/images.size()*this->width()));
		offset = (handle->width()*0.5f)/width();

		setValue(value());	// update position
	}

	QString gs = "qlineargradient(x1:0, y1:0, x2:1, y2:0 ";
	gs += ", stop: 0 " + DkUtils::colorToString(DkSettings::display.bgColorWidget);

	//int fileLimit = (colorLoader) ? colorLoader->maxFiles() : 100;
	//float maxFiles = (files.size() > fileLimit) ? fileLimit : files.size();

	for (int idx = 0; idx < colors.size(); idx++) {

		QColor cCol = colors[idx];
		//cCol.setAlphaF(0.7);
		gs += ", stop: " + QString::number((float)indexes[idx]/(images.size()-1)*(1.0f-2.5f*offset)+offset) + " " + 
			DkUtils::colorToString(cCol); 
	}

	gs += ", stop: 1 " + DkUtils::colorToString(DkSettings::display.bgColorWidget) + ");";

	setStyleSheet(QString("QScrollBar:horizontal { ") + 
		QString("border: none;") +
		QString("background: rgba(0,0,0,0);") +
		QString("margin: 0px 0px 0px 0px;") +
		QString("}") +
		// hide default handle
		QString("QScrollBar::handle:horizontal {") +
		QString("background-color: rgba(0,0,0,0); ") +
		QString("border: none;") + 
		QString("width: 0px; height: 0px;") +
		QString("}") +
		QString("QScrollBar::add-line:horizontal {") +
		QString("background: ") + gs +
		QString("border-bottom: 1px solid #000;") +
		QString("subcontrol-position: bottom;") +
		QString("subcontrol-origin: margin;") +
		QString("}") +
		// hide arrows
		QString("QScrollBar::sub-line:horizontal {") +
		QString("width: 0px;") +
		QString("height: 0px;") +
		QString("}") +
		QString("QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal{background: none}")	// remove 'ugly' Qt background pattern
		);

	qDebug() << "updating style...";
	
}

void DkFolderScrollBar::setValue(int i) {

	//if (i > maximum())
	//	i = maximum()-1;

	if (!images.empty() && cImg) {
		float handlePos = (float)fileIdx(cImg)/images.size();
		handlePos *= (handle->width() == minHandleWidth) ? this->width()-handle->width() : this->width();
		QRect r(qRound(handlePos), 0, handle->width(), height());
		handle->setGeometry(r);
	}

	QScrollBar::setValue(i);
}

int DkFolderScrollBar::fileIdx(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC)
		return -1;

	for (int idx = 0; idx < images.size(); idx++) {

		if (imgC->file().absoluteFilePath() == images.at(idx)->file().absoluteFilePath())
			return idx;
	}

	return -1;
}

void DkFolderScrollBar::emitFileSignal(int i) {

	if (!cImg)
		return;

	qDebug() << "value: " << i;
	int skipIdx = i-fileIdx(cImg);
	
	if (i >= 0 && i < images.size())
		cImg = images.at(i);
	emit changeFileSignal(skipIdx);
}

void DkFolderScrollBar::mouseMoveEvent(QMouseEvent *event) {

	int offset = (handle->width() == minHandleWidth) ? handle->width() : 0;
	int val = qRound((float)(event->pos().x()-handle->width()*0.5)/(width()-offset)*maximum());

	if (sliding && event->buttons() == Qt::LeftButton)
		setValue(val);
		
	if (val >= 0 && val < images.size())
		setToolTip(images.at(val)->file().fileName());

}

void DkFolderScrollBar::mousePressEvent(QMouseEvent *event) {

	sliding = handle->geometry().contains(event->pos());
}

void DkFolderScrollBar::mouseReleaseEvent(QMouseEvent *event) {

	int offset = (handle->width() == minHandleWidth) ? handle->width() : 0;
	setValue(qRound((float)(event->pos().x()-handle->width()*0.5)/(width()-offset)*maximum()));
	sliding = false;

	// do not propagate these events

}

void DkFolderScrollBar::resizeEvent(QResizeEvent *event) {

	update(colors, indexes);
	QScrollBar::resizeEvent(event);
}

// scrollbar - DkWidget functions
void DkFolderScrollBar::init() {

	setMouseTracking(true);

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
	DkSettings::display.bgColorWidget;

	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;
	opacityEffect = 0;

	// painter problems if the widget is a child of another that has the same graphicseffect
	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);
	setGraphicsEffect(opacityEffect);

	setVisible(false);
}

void DkFolderScrollBar::show() {

	// here is a strange problem if you add a DkWidget to another DkWidget -> painters crash
	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true);
		animateOpacityUp();
	}
}

void DkFolderScrollBar::hide() {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();

		// set display bit here too -> since the final call to setVisible takes a few seconds
		if (displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
			displaySettingsBits->setBit(DkSettings::app.currentAppMode, false);
		}
	}
}

void DkFolderScrollBar::setVisible(bool visible) {

	if (blocked) {
		QWidget::setVisible(false);
		return;
	}

	if (visible)
		updateColors();

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	QWidget::setVisible(visible);
	emit visibleSignal(visible);	// if this gets slow -> put it into hide() or show()

	if (displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, visible);
	}
}

void DkFolderScrollBar::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		showing = false;
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkFolderScrollBar::animateOpacityDown() {

	if (!hiding)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		setVisible(false);	// finally hide the widget
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}


// DkThumbsSaver --------------------------------------------------------------------
DkThumbsSaver::DkThumbsSaver(QWidget* parent) : DkWidget(parent) {
	stop = false;
	cLoadIdx = 0;
	numSaved = 0;
}

void DkThumbsSaver::processDir(QVector<QSharedPointer<DkImageContainerT> > images, bool forceSave) {

	if (images.empty())
		return;

	stop = false;
	cLoadIdx = 0;
	numSaved = 0;

	pd = new QProgressDialog(tr("\nCreating thumbnails...\n") + images.first()->file().absolutePath(), tr("Cancel"), 0, (int)images.size(), DkNoMacs::getDialogParent());
	pd->setWindowTitle(tr("Thumbnails"));

	//pd->setWindowModality(Qt::WindowModal);

	connect(this, SIGNAL(numFilesSignal(int)), pd, SLOT(setValue(int)));
	connect(pd, SIGNAL(canceled()), this, SLOT(stopProgress()));

	pd->show();

	this->forceSave = forceSave;
	this->images = images;
	loadNext();

}

void DkThumbsSaver::thumbLoaded(bool loaded) {

	numSaved++;
	emit numFilesSignal(numSaved);

	if (numSaved == images.size() || stop) {
		if (pd) {
			pd->close();
			pd->deleteLater();
			pd = 0;
		}
		stop = true;
	}
	else
		loadNext();
}

void DkThumbsSaver::loadNext() {
	
	if (stop)
		return;

	int missing = DkSettings::resources.maxThumbsLoading-DkSettings::resources.numThumbsLoading;
	int numLoading = cLoadIdx+missing;
	int force = (forceSave) ? DkThumbNail::force_save_thumb : DkThumbNail::save_thumb;

	qDebug() << "missing: " << missing << " num loading: " << numLoading;
	qDebug() << "loading bounds: " << cLoadIdx << " - " << numLoading;

	for (int idx = cLoadIdx; idx < images.size() && idx < numLoading; idx++) {
		cLoadIdx++;
		connect(images.at(idx)->getThumb().data(), SIGNAL(thumbLoadedSignal(bool)), this, SLOT(thumbLoaded(bool)));
		images.at(idx)->getThumb()->fetchThumb(force);
	}
}

void DkThumbsSaver::stopProgress() {

	stop = true;
}

// DkFileSystemModel --------------------------------------------------------------------
DkFileSystemModel::DkFileSystemModel(QObject* parent /* = 0 */) : QFileSystemModel(parent) {

	// some custom settings
	setRootPath(QDir::rootPath());
	setNameFilters(DkSettings::fileFilters);
	setReadOnly(false);
	//setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);

}

// DkSortFileProxyModel --------------------------------------------------------------------
DkSortFileProxyModel::DkSortFileProxyModel(QObject* parent /* = 0 */) : QSortFilterProxyModel(parent) {

}

bool DkSortFileProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {

	if (left.data().canConvert(QVariant::Url)) {

		QFileInfo lf = left.data().toString();
		QFileInfo rf = right.data().toString();

		// could not find a better way to tell files from dirs appart (isDir() is not what we expect)
		if (lf.suffix().isEmpty() && !rf.suffix().isEmpty())
			return true;
		else if (!lf.suffix().isEmpty() && rf.suffix().isEmpty())
			return false;		


		QString ls = (!lf.fileName().isEmpty()) ? lf.fileName() : lf.absoluteFilePath();	// otherwise e.g. C: is empty
		QString rs = (!rf.fileName().isEmpty()) ? rf.fileName() : rf.absoluteFilePath();

		QString ld = ls.section(QRegExp("[A-Z]:"), 1, -1, QString::SectionIncludeLeadingSep);
		
		// sort by drive letter if present
		if (!ld.isEmpty()) {
			ld.truncate(2);
			ls = ld;
		}

		QString rd = rs.section(QRegExp("[A-Z]:"), 1, -1, QString::SectionIncludeLeadingSep);

		// sort by drive letter if present
		if (!rd.isEmpty()) {
			rd.truncate(2);
			rs = rd;
		}

		return DkUtils::compLogicQString(ls, rs);
	}

	return QSortFilterProxyModel::lessThan(left, right);
}

// DkExplorer --------------------------------------------------------------------
DkExplorer::DkExplorer(const QString& title, QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : QDockWidget(title, parent, flags) {

	setObjectName("DkExplorer");
	createLayout();
	readSettings();

	connect(fileTree, SIGNAL(clicked(const QModelIndex&)), this, SLOT(fileClicked(const QModelIndex&)));
	//connect(fileTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
}

DkExplorer::~DkExplorer() {
	writeSettings();
}

void DkExplorer::createLayout() {

	fileModel = new DkFileSystemModel(this);
	
	sortModel = new DkSortFileProxyModel(this);
	sortModel->setSourceModel(fileModel);
	sortModel->setSortLocaleAware(true);

	fileTree = new QTreeView(this);
	fileTree->setSortingEnabled(true);
	fileTree->setModel(sortModel);
	fileTree->setDragEnabled(true);
	//fileTree->setContextMenuPolicy(Qt::CustomContextMenu);

	// by default descendingOrder is set
	fileTree->header()->setSortIndicator(0, Qt::AscendingOrder);

	setWidget(fileTree);
}

void DkExplorer::setCurrentPath(QFileInfo fileInfo) {

	// expand folders
	if (fileInfo.isDir())
		fileTree->expand(sortModel->mapFromSource(fileModel->index(fileInfo.absoluteFilePath())));

	fileTree->setCurrentIndex(sortModel->mapFromSource(fileModel->index(fileInfo.absoluteFilePath())));
}

void DkExplorer::fileClicked(const QModelIndex &index) const {

	QFileInfo cFile = fileModel->fileInfo(sortModel->mapToSource(index));

	qDebug() << "opening: " << cFile.absoluteFilePath();

	if (DkImageLoader::isValid(cFile))
		emit openFile(cFile);
	else if (cFile.isDir())
		emit openDir(cFile);
}

void DkExplorer::contextMenuEvent(QContextMenuEvent *event) {

	QMenu* cm = new QMenu();

	// enable editing
	QAction* editAction = new QAction(tr("Editable"), this);
	editAction->setCheckable(true);
	editAction->setChecked(!fileModel->isReadOnly());
	connect(editAction, SIGNAL(toggled(bool)), this, SLOT(setEditable(bool)));
	
	cm->addAction(editAction);
	cm->addSeparator();


	columnActions.clear();	// quick&dirty

	for (int idx = 0; idx < fileModel->columnCount(); idx++) {

		QAction* action = new QAction(fileModel->headerData(idx, Qt::Horizontal).toString(), this);
		action->setCheckable(true);
		action->setChecked(!fileTree->isColumnHidden(idx));
		action->setObjectName(QString::number(idx));

		connect(action, SIGNAL(toggled(bool)), this, SLOT(showColumn(bool)));
		columnActions.push_back(action);

		cm->addAction(action);
	}

	cm->exec(event->globalPos());	
}

void DkExplorer::showColumn(bool show) {

	bool ok = false;
	int idx = QObject::sender()->objectName().toInt(&ok);

	if (!ok)
		return;

	fileTree->setColumnHidden(idx, !show);
}

void DkExplorer::setEditable(bool editable) {
	fileModel->setReadOnly(!editable);	
}

void DkExplorer::closeEvent(QCloseEvent* event) {

	writeSettings();
}

void DkExplorer::writeSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());
	
	for (int idx = 0; idx < fileModel->columnCount(QModelIndex()); idx++) {
		QString headerVal = fileModel->headerData(idx, Qt::Horizontal).toString();
		settings.setValue(headerVal + "Size", fileTree->columnWidth(idx));
		settings.setValue(headerVal + "Hidden", fileTree->isColumnHidden(idx));
	}

	settings.setValue("ReadOnly", fileModel->isReadOnly());
	settings.endGroup();
	
}

void DkExplorer::readSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());

	for (int idx = 0; idx < fileModel->columnCount(QModelIndex()); idx++) {
		
		QString headerVal = fileModel->headerData(idx, Qt::Horizontal).toString();
		
		int colWidth = settings.value(headerVal + "Size", -1).toInt();
		if (colWidth != -1) 
			fileTree->setColumnWidth(idx, colWidth);

		bool showCol = idx != 0;	// by default, show the first column only
		fileTree->setColumnHidden(idx, settings.value(headerVal + "Hidden", showCol).toBool());
	}

	fileModel->setReadOnly(settings.value("ReadOnly", false).toBool());
	settings.endGroup();
}

// DkOverview --------------------------------------------------------------------
DkOverview::DkOverview(QWidget* parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	setObjectName("DkOverview");
	this->parent = parent;
	setMinimumSize(0, 0);
	setMaximumSize(200, 200);
}

void DkOverview::paintEvent(QPaintEvent *event) {

	if (img.isNull() || !imgMatrix || !worldMatrix)
		return;

	QPainter painter(this);

	int lm, tm, rm, bm;
	getContentsMargins(&lm, &tm, &rm, &bm);

	QSize viewSize = QSize(width()-lm-rm, height()-tm-bm);	// overview shall take 15% of the viewport....
	
	if (viewSize.width() > 2 && viewSize.height() > 2) {
	
		QRectF imgRect = QRectF(QPoint(), img.size());
		QRectF overviewRect = QRectF(QPoint(lm, tm), QSize(viewSize.width()-1, viewSize.height()-1));			// get the overview rect
		overviewRect = overviewRect.toRect();	// force round

		QRectF overviewImgRect(lm+1, tm+1, imgT.width(), imgT.height());
		overviewImgRect.moveCenter(overviewRect.center());

		QTransform overviewImgMatrix = getScaledImageMatrix();			// matrix that always resizes the image to the current viewport
		//QRectF overviewImgRect = overviewImgMatrix.mapRect(imgRect);
		//overviewImgRect.moveTop(overviewImgRect.top()+tm+1);
		//overviewImgRect.moveLeft(overviewImgRect.left()+lm+1);
		//overviewImgRect = overviewImgRect.toRect();	// force round
		//overviewImgRect.setWidth(overviewImgRect.width()-2);
		//overviewImgRect.setHeight(overviewImgRect.height()-2);

		//qDebug() << "overview image rect: " << overviewImgRect;
		//qDebug() << "overview img size: " << imgT.size();

		// now render the current view
		QRectF viewRect = viewPortRect;
		viewRect = worldMatrix->inverted().mapRect(viewRect);
		viewRect = imgMatrix->inverted().mapRect(viewRect);
		viewRect = overviewImgMatrix.mapRect(viewRect);
		viewRect.moveTopLeft(viewRect.topLeft()+QPointF(lm, tm));

		if(viewRect.topLeft().x() < overviewRect.topLeft().x()) viewRect.setTopLeft(QPointF(overviewRect.topLeft().x(), viewRect.topLeft().y()));
		if(viewRect.topLeft().y() < overviewRect.topLeft().y()) viewRect.setTopLeft(QPointF(viewRect.topLeft().x(), overviewRect.topLeft().y()));
		if(viewRect.bottomRight().x() > overviewRect.bottomRight().x()) viewRect.setBottomRight(QPointF(overviewRect.bottomRight().x(), viewRect.bottomRight().y()));
		if(viewRect.bottomRight().y() > overviewRect.bottomRight().y()) viewRect.setBottomRight(QPointF(viewRect.bottomRight().x(), overviewRect.bottomRight().y()));		

		//draw the image's location
		painter.setBrush(bgCol);
		painter.setPen(QColor(200, 200, 200));
		painter.drawRect(overviewRect);
		painter.setOpacity(0.8f);
		painter.drawImage(overviewImgRect, imgT, QRect(0, 0, imgT.width(), imgT.height()));

		QColor col = DkSettings::display.highlightColor;
		col.setAlpha(255);
		painter.setPen(col);
		col.setAlpha(50);
		painter.setBrush(col);
		painter.drawRect(viewRect);

	}
	painter.end();

	DkWidget::paintEvent(event);
}

void DkOverview::mousePressEvent(QMouseEvent *event) {
	
	enterPos = event->pos();
	posGrab = event->pos();
}

void DkOverview::mouseReleaseEvent(QMouseEvent *event) {

	QPointF dxy = enterPos-QPointF(event->pos());

	if (dxy.manhattanLength() < 4) {
		
		int lm, tm, rm, bm;
		getContentsMargins(&lm, &tm, &rm, &bm);
		
		// move to the current position
		QRectF viewRect = viewPortRect;
		viewRect = worldMatrix->inverted().mapRect(viewRect);
		viewRect = imgMatrix->inverted().mapRect(viewRect);
		viewRect = getScaledImageMatrix().mapRect(viewRect);
		QPointF currentViewPoint = viewRect.center();

		float panningSpeed = -(worldMatrix->m11()/(getScaledImageMatrix().m11()/imgMatrix->m11()));

		QPointF cPos = event->pos()-QPointF(lm, tm);
		QPointF dxy = (cPos - currentViewPoint)/worldMatrix->m11()*panningSpeed;
		emit moveViewSignal(dxy);

		if (event->modifiers() == DkSettings::global.altMod)
			emit sendTransformSignal();
	}

}

void DkOverview::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() != Qt::LeftButton)
		return;

	float panningSpeed = -(worldMatrix->m11()/(getScaledImageMatrix().m11()/imgMatrix->m11()));

	QPointF cPos = event->pos();
	QPointF dxy = (cPos - posGrab)/worldMatrix->m11()*panningSpeed;
	posGrab = cPos;
	emit moveViewSignal(dxy);

	if (event->modifiers() == DkSettings::global.altMod)
		emit sendTransformSignal();

}

void DkOverview::resizeEvent(QResizeEvent* event) {

	QSizeF newSize = event->size();
	newSize.setHeight(newSize.width() * viewPortRect.height()/viewPortRect.width());

	// in rare cases, the window won't be resized if width = maxWidth & height is < 1
	if (newSize.height() < 1)
		newSize.setWidth(0);
	
	resize(newSize.toSize());

	DkWidget::resizeEvent(event);
}

void DkOverview::resize(int w, int h) {

	resize(QSize(w, h));
}

void DkOverview::resize(const QSize& size) {

	DkWidget::resize(size);

	// update image
	resizeImg();
}

void DkOverview::resizeImg() {

	if (img.isNull())
		return;

	int lm, tm, rm, bm;
	getContentsMargins(&lm, &tm, &rm, &bm);

	QSize viewSize = QSize(width()-lm-rm, height()-tm-bm);	// overview shall take 15% of the viewport....
	QRectF overviewRect = QRectF(QPoint(lm, tm), QSize(viewSize.width()-2, viewSize.height()-2));			// get the overview rect
	overviewRect = overviewRect.toRect();	// force round

	QTransform overviewImgMatrix = getScaledImageMatrix();			// matrix that always resizes the image to the current viewport
	
	// is the overviewImgMatrix empty?
	if (overviewImgMatrix.isIdentity())
		return;
	
	if (overviewRect.width() <= 1|| overviewRect.height() <= 1)
		return;

	// fast downscaling
	imgT = img.scaled(overviewRect.size().width()*2, overviewRect.size().height()*2, Qt::KeepAspectRatio, Qt::FastTransformation);
	imgT = imgT.scaled(overviewRect.size().width(), overviewRect.size().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QTransform DkOverview::getScaledImageMatrix() {

	if (img.isNull())
		return QTransform();

	int lm, tm, rm, bm;
	getContentsMargins(&lm, &tm, &rm, &bm);

	QSize iSize = QSize(width()-lm-rm, height()-tm-bm);	// inner size

	if (iSize.width() < 2 || iSize.height() < 2)
		return QTransform();

	// the image resizes as we zoom
	QRectF imgRect = QRectF(QPoint(lm, tm), img.size());
	float ratioImg = imgRect.width()/imgRect.height();
	float ratioWin = (float)(iSize.width())/(float)(iSize.height());

	QTransform imgMatrix;
	float s;
	if (imgRect.width() == 0 || imgRect.height() == 0)
		s = 1.0f;
	else
		s = (ratioImg > ratioWin) ? (float)iSize.width()/imgRect.width() : (float)iSize.height()/imgRect.height();

	imgMatrix.scale(s, s);

	QRectF imgViewRect = imgMatrix.mapRect(imgRect);
	imgMatrix.translate((iSize.width()-imgViewRect.width())*0.5f/s, (iSize.height()-imgViewRect.height())*0.5f/s);

	return imgMatrix;
}


// DkLabel --------------------------------------------------------------------
DkLabel::DkLabel(QWidget* parent, const QString& text) : QLabel(text, parent) {

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
		DkSettings::display.bgColorWidget;

	setMouseTracking(true);
	this->parent = parent;
	this->text = text;
	init();
	hide();
}

void DkLabel::init() {

	time = -1;
	fixedWidth = -1;
	fontSize = 17;
	textCol = QColor(255, 255, 255);
	blocked = false;
	
	timer = new QTimer();
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()), this, SLOT(hide()));

	// default look and feel
	QFont font;
	font.setPixelSize(fontSize);
	QLabel::setFont(font);
	QLabel::setTextInteractionFlags(Qt::TextSelectableByMouse);
	
	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	updateStyleSheet();
}

void DkLabel::hide() {
	time = 0;
	QLabel::hide();
}

void DkLabel::setText(const QString msg, int time) {

	this->text = msg;
	this->time = time;

	if (!time || msg.isEmpty()) {
		hide();
		return;
	}

	setTextToLabel();
	show();

	if (time != -1)
		timer->start(time);

}

void DkLabel::showTimed(int time) {

	this->time = time;

	if (!time) {
		hide();
		return;
	}

	show();

	if (time != -1)
		timer->start(time);

}


QString DkLabel::getText() {
	return this->text;
}

void DkLabel::setFontSize(int fontSize) {

	this->fontSize = fontSize;

	QFont font;
	font.setPixelSize(fontSize);
	QLabel::setFont(font);
	QLabel::adjustSize();
}

void DkLabel::stop() {
	timer->stop();
	hide();
}

void DkLabel::updateStyleSheet() {
	QLabel::setStyleSheet("QLabel{color: " + textCol.name() + "; margin: " + 
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px " +
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px;}");
}

void DkLabel::paintEvent(QPaintEvent *event) {

	if (blocked || !time)	// guarantee that the label is hidden after the time is up
		return;

	QPainter painter(this);
	draw(&painter);
	painter.end();

	QLabel::paintEvent(event);
}

void DkLabel::draw(QPainter* painter) {

	drawBackground(painter);
}

void DkLabel::setFixedWidth(int fixedWidth) {

	this->fixedWidth = fixedWidth;
	setTextToLabel();
}

void DkLabel::setTextToLabel() {

	if (fixedWidth == -1) {
		QLabel::setText(text);
		QLabel::adjustSize();
	}
	else {
		setToolTip(text);
		QLabel::setText(fontMetrics().elidedText(text, Qt::ElideRight, fixedWidth-2*margin.x()));
		QLabel::resize(fixedWidth, height());
	}

}

DkLabelBg::DkLabelBg(QWidget* parent, const QString& text) : DkLabel(parent, text) {

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
		DkSettings::display.bgColorWidget;

	setAttribute(Qt::WA_TransparentForMouseEvents);	// labels should forward mouse events
	
	setObjectName("DkLabelBg");
	updateStyleSheet();

	margin = QPoint(7,2);
	setMargin(margin);
}

void DkLabelBg::updateStyleSheet() {

	QLabel::setStyleSheet("QLabel#DkLabelBg{color: " + textCol.name() + "; padding: " + 
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px " +
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px; " +
		"background-color: " + DkUtils::colorToString(bgCol) + ";}");	// background
}

// DkGradientLabel --------------------------------------------------------------------
DkGradientLabel::DkGradientLabel(QWidget* parent, const QString& text) : DkLabel(parent, text) {

	init();
	hide();
}

void DkGradientLabel::init() {

	DkLabel::init();
	gradient = QImage(":/nomacs/img/label-gradient.png");
	end = QImage(":/nomacs/img/label-end.png");
	
	QLabel::setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	QLabel::setStyleSheet("QLabel{color: " + textCol.name() + "; margin: 5px " + QString::number(end.width()) + "px 5px 10px}");
	
}

void DkGradientLabel::updateStyleSheet() {
	
	QLabel::setStyleSheet("QLabel{color: " + textCol.name() + "; margin: " + 
		QString::number(margin.y()) + "px " +
		QString::number(end.width()) + "px " +		// the fade-out
		QString::number(margin.y()) + "px " +
		QString::number(margin.x()) + "px;}");
}

void DkGradientLabel::drawBackground(QPainter* painter) {

	QRect textRect = QRect(QPoint(), size());
	textRect.setWidth(textRect.width()-end.width()-1);
	QRectF endRect = QRect(textRect.right()+1, 0, end.width(), geometry().height());
	painter->drawImage(textRect, gradient);
	painter->drawImage(endRect, end);
}

// DkFadeLabel --------------------------------------------------------------------
DkFadeLabel::DkFadeLabel(QWidget* parent, const QString& text) : DkLabel(parent, text) {
	init();
}

void DkFadeLabel::init() {

	bgCol = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
		DkSettings::display.bgColorWidget;

	showing = false;
	hiding = false;
	blocked = false;
	displaySettingsBits = 0;

	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);	// default disabled -> otherwise we get problems with children having the same effect
	setGraphicsEffect(opacityEffect);
	
	setVisible(false);
}

void DkFadeLabel::show() {

	if (!blocked && !showing) {
		hiding = false;
		showing = true;
		setVisible(true);
		animateOpacityUp();
	}
}

void DkFadeLabel::hide() {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();
	}
}

void DkFadeLabel::setVisible(bool visible) {

	if (blocked) {
		DkLabel::setVisible(false);
		return;
	}

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	emit visibleSignal(visible);
	DkLabel::setVisible(visible);

	if (displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		qDebug() << "setting visible to: " << visible;
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, visible);
	}

}

void DkFadeLabel::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		opacityEffect->setEnabled(false);
		showing = false;
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkFadeLabel::animateOpacityDown() {

	if (!hiding)
		return;
	
	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		opacityEffect->setEnabled(false);
		setVisible(false);	// finally hide the widget
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

// DkButton --------------------------------------------------------------------
DkButton::DkButton(QWidget* parent) : QPushButton(parent) {

}

DkButton::DkButton(const QString& text, QWidget* parent) : QPushButton(text, parent) {

}

DkButton::DkButton(const QIcon& icon, const QString& text, QWidget* parent) : QPushButton(icon, text, parent) {

	checkedIcon = icon;
	setText(text);

	init();
}

DkButton::DkButton(const QIcon& checkedIcon, const QIcon& uncheckedIcon, const QString& text, QWidget* parent) : QPushButton(checkedIcon, text, parent) {

	this->checkedIcon = checkedIcon;
	this->uncheckedIcon = uncheckedIcon;
	this->setCheckable(true);
	setText(text);
	
	init();
}

void DkButton::init() {

	setIcon(checkedIcon);

	if (!checkedIcon.availableSizes().empty())
		this->setMaximumSize(checkedIcon.availableSizes()[0]);	// crashes if the image is empty!!
	
	mouseOver = false;
	keepAspectRatio = true;
}

void DkButton::setFixedSize(QSize size) {
	mySize = size;
	this->setMaximumSize(size);
}

void DkButton::paintEvent(QPaintEvent *event) {

 	QPainter painter(this);
	QPoint offset;
	QSize s;
	float opacity = 1.0f;

	if (!isEnabled())
		opacity = 0.5f;
	else if(!mouseOver)
		opacity = 0.7f;

	painter.setOpacity(opacity);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);

	if (!mySize.isEmpty()) {
		
		offset = QPoint((float)(size().width()-mySize.width())*0.5f, (float)(size().height()-mySize.height())*0.5f);
		s = mySize;
	}
	else
		s = this->size();

	// scale to parent label
	QRect r = (keepAspectRatio) ? QRect(offset, checkedIcon.actualSize(s)) : QRect(offset, s);	// actual size preserves the aspect ratio
	QPixmap pm2draw;

	if (isChecked() || !isCheckable())
		pm2draw = checkedIcon.pixmap(s);
	else
		pm2draw = uncheckedIcon.pixmap(s);

	if (this->isDown()) {
		QPixmap effect = createSelectedEffect(&pm2draw);
		painter.drawPixmap(r, effect);
	}

	painter.drawPixmap(r, pm2draw);
	painter.end();
}

QPixmap DkButton::createSelectedEffect(QPixmap* pm) {

	if (!pm || pm->isNull())
		return QPixmap();

	return DkImage::colorizePixmap(*pm, DkSettings::display.highlightColor, 1.0f);
}

void DkButton::focusInEvent(QFocusEvent * event) {
	mouseOver = true;
}

void DkButton::focusOutEvent(QFocusEvent * event) {
	mouseOver = false;
}

void DkButton::enterEvent(QEvent *event) {
	mouseOver = true;
}

void DkButton::leaveEvent(QEvent *event) {
	mouseOver = false;
}

// star label --------------------------------------------------------------------
DkRatingLabel::DkRatingLabel(int rating, QWidget* parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	setObjectName("DkRatingLabel");
	this->rating = rating;
	init();

	int iconSize = 16;
	int lastStarRight = 0;
	int timeToDisplay = 3000;

	layout = new QBoxLayout(QBoxLayout::LeftToRight);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(3);
	layout->addStretch();
	
	for (int idx = 0; idx < stars.size(); idx++) {
		stars[idx]->setFixedSize(QSize(iconSize, iconSize));
		layout->addWidget(stars[idx]);
	}
	
	this->setLayout(layout);
}

void DkRatingLabel::init() {

	QPixmap starDark = QPixmap(":/nomacs/img/star-dark.png");
	QPixmap starWhite = QPixmap(":/nomacs/img/star-white.png");

	stars.resize(5);
	
	stars[rating_1] = new DkButton(starWhite, starDark, tr("one star"), this);
	stars[rating_1]->setCheckable(true);
	connect(stars[rating_1], SIGNAL(released()), this, SLOT(rating1()));

	stars[rating_2] = new DkButton(starWhite, starDark, tr("two stars"), this);
	stars[rating_2]->setCheckable(true);
	connect(stars[rating_2], SIGNAL(released()), this, SLOT(rating2()));

	stars[rating_3] = new DkButton(starWhite, starDark, tr("three star"), this);
	stars[rating_3]->setCheckable(true);
	connect(stars[rating_3], SIGNAL(released()), this, SLOT(rating3()));

	stars[rating_4] = new DkButton(starWhite, starDark, tr("four star"), this);
	stars[rating_4]->setCheckable(true);
	connect(stars[rating_4], SIGNAL(released()), this, SLOT(rating4()));

	stars[rating_5] = new DkButton(starWhite, starDark, tr("five star"), this);
	stars[rating_5]->setCheckable(true);
	connect(stars[rating_5], SIGNAL(released()), this, SLOT(rating5()));

}

DkRatingLabelBg::DkRatingLabelBg(int rating, QWidget* parent, Qt::WindowFlags flags) : DkRatingLabel(rating, parent, flags) {

	timeToDisplay = 4000;
	hideTimer = new QTimer(this);
	hideTimer->setInterval(timeToDisplay);
	hideTimer->setSingleShot(true);

	// we want a margin
	layout->setContentsMargins(10,4,10,4);
	layout->setSpacing(4);

	actions.resize(6);

	actions[rating_0] = new QAction(tr("no rating"), this);
	actions[rating_0]->setShortcut(Qt::Key_0);
	connect(actions[rating_0], SIGNAL(triggered()), this, SLOT(rating0()));

	actions[rating_1] = new QAction(tr("one star"), this);
	actions[rating_1]->setShortcut(Qt::Key_1);
	connect(actions[rating_1], SIGNAL(triggered()), this, SLOT(rating1()));

	actions[rating_2] = new QAction(tr("two stars"), this);
	actions[rating_2]->setShortcut(Qt::Key_2);
	connect(actions[rating_2], SIGNAL(triggered()), this, SLOT(rating2()));

	actions[rating_3] = new QAction(tr("three stars"), this);
	actions[rating_3]->setShortcut(Qt::Key_3);
	connect(actions[rating_3], SIGNAL(triggered()), this, SLOT(rating3()));

	actions[rating_4] = new QAction(tr("four stars"), this);
	actions[rating_4]->setShortcut(Qt::Key_4);
	connect(actions[rating_4], SIGNAL(triggered()), this, SLOT(rating4()));

	actions[rating_5] = new QAction(tr("five stars"), this);
	actions[rating_5]->setShortcut(Qt::Key_5);
	connect(actions[rating_5], SIGNAL(triggered()), this, SLOT(rating5()));

	stars[rating_1]->addAction(actions[rating_1]);
	stars[rating_2]->addAction(actions[rating_2]);
	stars[rating_3]->addAction(actions[rating_3]);
	stars[rating_4]->addAction(actions[rating_4]);
	stars[rating_5]->addAction(actions[rating_5]);
	
	connect(hideTimer, SIGNAL(timeout()), this, SLOT(hide()));

}

void DkRatingLabelBg::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	painter.fillRect(QRect(QPoint(), this->size()), bgCol);
	painter.end();

	DkRatingLabel::paintEvent(event);
}

// title info --------------------------------------------------------------------
DkFileInfoLabel::DkFileInfoLabel(QWidget* parent) : DkFadeLabel(parent) {

	setObjectName("DkFileInfoLabel");
	setStyleSheet("QLabel#DkFileInfoLabel{background-color: " + DkUtils::colorToString(bgCol) + ";} QLabel{color: white;}");
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

	minWidth = 110;
	this->parent = parent;
	title = new QLabel(this);
	title->setMouseTracking(true);
	title->setTextInteractionFlags(Qt::TextSelectableByMouse);
	date = new QLabel(this);
	date->setMouseTracking(true);
	date->setTextInteractionFlags(Qt::TextSelectableByMouse);
	rating = new DkRatingLabel(0, this);
	setMinimumWidth(minWidth);
	
	createLayout();
}

void DkFileInfoLabel::createLayout() {

	layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setSpacing(2);

	layout->addWidget(title);
	layout->addWidget(date);
	layout->addWidget(rating);
	//layout->addStretch();
}

void DkFileInfoLabel::setVisible(bool visible) {

	// nothing to display??
	if (!DkSettings::slideShow.display.testBit(DkSettings::display_file_name) &&
		!DkSettings::slideShow.display.testBit(DkSettings::display_creation_date) &&
		!DkSettings::slideShow.display.testBit(DkSettings::display_file_rating) && visible) {
			
			QMessageBox infoDialog(parent);
			infoDialog.setWindowTitle(tr("Info Box"));
			infoDialog.setText(tr("All information fields are currently hidden.\nDo you want to show them again?"));
			infoDialog.setIcon(QMessageBox::Information);
			infoDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			infoDialog.setDefaultButton(QMessageBox::Yes);
			infoDialog.show();
			int choice = infoDialog.exec();

			if (choice == QMessageBox::No) {
				DkFadeLabel::setVisible(false);
				return;
			}
			else {
				DkSettings::slideShow.display.setBit(DkSettings::display_file_name, true);
				DkSettings::slideShow.display.setBit(DkSettings::display_creation_date, true);
				DkSettings::slideShow.display.setBit(DkSettings::display_file_rating, true);
			}
	}

	DkFadeLabel::setVisible(visible);
	title->setVisible(DkSettings::slideShow.display.testBit(DkSettings::display_file_name));
	date->setVisible(DkSettings::slideShow.display.testBit(DkSettings::display_creation_date));
	rating->setVisible(DkSettings::slideShow.display.testBit(DkSettings::display_file_rating));

	int height = 32;
	if (title->isVisible())
		height += title->sizeHint().height();
	if (date->isVisible())
		height += date->sizeHint().height();
	if (rating->isVisible())
		height += rating->sizeHint().height();

	qDebug() << "my minimum height: " << height;
	setMinimumHeight(height);
	updateWidth();
}

void DkFileInfoLabel::setEdited(bool edited) {

	if (!isVisible() || !edited)
		return;

	QString cFileName = title->text() + "*";
	this->title->setText(cFileName);

}

DkRatingLabel* DkFileInfoLabel::getRatingLabel() {
	return rating;
}

void DkFileInfoLabel::updateInfo(const QFileInfo& file, const QString& attr, const QString& date, const int rating) {

	updateTitle(file, attr);
	updateDate(date);
	updateRating(rating);

	updateWidth();
}

void DkFileInfoLabel::updateTitle(const QFileInfo& file, const QString& attr) {
	
	this->file = file;
	updateDate();
	this->title->setText(file.fileName() + " " + attr);
	this->title->setAlignment(Qt::AlignRight);

	updateWidth();
}

void DkFileInfoLabel::updateDate(const QString& date) {

	QString dateConverted = DkUtils::convertDateString(date, file);

	this->date->setText(dateConverted);
	this->date->setAlignment(Qt::AlignRight);

	updateWidth();
}

void DkFileInfoLabel::updateRating(const int rating) {
	
	this->rating->setRating(rating);

}

void DkFileInfoLabel::updateWidth() {

	int width = 20;		// mar
	width += qMax(qMax(title->sizeHint().width(), date->sizeHint().width()), rating->sizeHint().width());
	
	if (width < minimumWidth())
		setMinimumWidth(width);
	
	setMaximumWidth(width);
}

// player --------------------------------------------------------------------
DkPlayer::DkPlayer(QWidget* parent) : DkWidget(parent) {

	init();
}

void DkPlayer::init() {
	
	setObjectName("DkPlayer");

	// slide show
	int timeToDisplayPlayer = 3000;
	timeToDisplay = DkSettings::slideShow.time*1000;
	playing = false;
	displayTimer = new QTimer(this);
	displayTimer->setInterval(timeToDisplay);
	displayTimer->setSingleShot(true);
	connect(displayTimer, SIGNAL(timeout()), this, SLOT(autoNext()));

	hideTimer = new QTimer(this);
	hideTimer->setInterval(timeToDisplayPlayer);
	hideTimer->setSingleShot(true);
	connect(hideTimer, SIGNAL(timeout()), this, SLOT(hide()));

	actions.resize(1);
	actions[play_action] = new QAction(tr("play"), this);
	actions[play_action]->setShortcut(Qt::Key_Space);
	connect(actions[play_action], SIGNAL(triggered()), this, SLOT(togglePlay()));

	QPixmap icon = QPixmap(":/nomacs/img/player-back.png");
	previousButton = new DkButton(icon, tr("previous"), this);
	previousButton->keepAspectRatio = false;
	connect(previousButton, SIGNAL(pressed()), this, SLOT(previous()));

	icon = QPixmap(":/nomacs/img/player-pause.png");
	QPixmap icon2 = QPixmap(":/nomacs/img/player-play.png");
	playButton = new DkButton(icon, icon2, tr("play"), this);
	playButton->keepAspectRatio = false;
	playButton->setChecked(false);
	playButton->addAction(actions[play_action]);
	connect(playButton, SIGNAL(toggled(bool)), this, SLOT(play(bool)));

	icon = QPixmap(":/nomacs/img/player-next.png");
	nextButton = new DkButton(icon, tr("next"), this);
	nextButton->keepAspectRatio = false;
	connect(nextButton, SIGNAL(pressed()), this, SLOT(next()));

	// now add to layout
	container = new QWidget(this);
	QHBoxLayout *layout = new QHBoxLayout(container);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(previousButton);
	layout->addWidget(playButton);
	layout->addWidget(nextButton);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMinimumSize(15, 5);
	setMaximumSize(315, 113);
}

void DkPlayer::resizeEvent(QResizeEvent *event) {

	if (event->oldSize() == event->size())
		return;

	// always preserve the player's aspect ratio
	QSizeF s = event->size();
	QSizeF ms = maximumSize();
	float aRatio = s.width()/s.height();
	float amRatio = ms.width()/ms.height();
	
	if (aRatio != amRatio && s.width() / amRatio <= s.height()) {
		s.setHeight(s.width() / amRatio);

		QRect r = QRect(QPoint(), s.toSize());
		r.moveBottom(event->size().height()-1);
		r.moveCenter(QPoint(qRound((float)event->size().width()/2.0f), r.center().y()));
		container->setGeometry(r);
	}
	else {
		s.setWidth(s.height() * amRatio);

		QRect r = QRect(QPoint(), s.toSize());
		r.moveBottom(event->size().height()-1);
		r.moveCenter(QPoint(qRound((float)event->size().width()/2.0f), r.center().y()));
		container->setGeometry(r);
	}

	QWidget::resizeEvent(event);
}

void DkPlayer::setTimeToDisplay(int ms) {

	timeToDisplay = ms;
	displayTimer->setInterval(ms);
}

void DkPlayer::show(int ms) {		
	
	if (ms > 0 && !hideTimer->isActive()) {
		hideTimer->setInterval(ms);
		hideTimer->start();
	}

	bool showPlayer = getCurrentDisplaySetting();

	DkWidget::show();

	// automatic showing, don't store it in the display bits
	if (ms > 0 && displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, showPlayer);
	}
}
 
// DkMetaDataInfo ------------------------------------------------------------------

////enums for tags divided in exif and iptc
//static enum exifT {
//	exif_width,
//	exif_length,
//	exif_orientation,
//	exif_make,
//	exif_model,
//	exif_rating,
//	exif_aperture,
//	exif_shutterspeed,
//	exif_flash,
//	exif_focallength,
//	exif_exposuredmode,
//	exif_exposuretime,
//	exif_usercomment,
//	exif_datetime,
//	exif_datetimeoriginal,
//	exif_description,
//
//	exif_end
//};
//
//static enum iptcT {
//	iptc_creator = exif_end,
//	iptc_creatortitle,
//	iptc_city,
//	iptc_country,
//	iptc_headline,
//	iptc_caption,
//	iptc_copyright,
//	iptc_keywords,
//
//	iptc_end
//};

//QString DkMetaDataInfo::sExifTags = QString("ImageWidth ImageLength Orientation Make Model Rating ApertureValue ShutterSpeedValue Flash FocalLength ") %
//	QString("ExposureMode ExposureTime UserComment DateTime DateTimeOriginal ImageDescription");
//QString DkMetaDataInfo::sExifDesc = QString("Image Width;Image Length;Orientation;Make;Model;Rating;Aperture Value;Shutter Speed Value;Flash;FocalLength;") %
//	QString("Exposure Mode;Exposure Time;User Comment;Date Time;Date Time Original;Image Description");
//QString DkMetaDataInfo::sIptcTags = QString("Iptc.Application2.Byline Iptc.Application2.BylineTitle Iptc.Application2.City Iptc.Application2.Country ") %
//	QString("Iptc.Application2.Headline Iptc.Application2.Caption Iptc.Application2.Copyright Iptc.Application2.Keywords");
//QString DkMetaDataInfo::sIptcDesc = QString("Creator;Creator Title;City;Country;Headline;Caption;Copyright;Keywords");

DkMetaDataInfo::DkMetaDataInfo(QWidget* parent) : DkWidget(parent) {
	
	setObjectName("DkMetaDataInfo");

	this->parent = parent;
	
	exifHeight = 120;
	minWidth = 900;
	fontSize = 12;
	textMargin = 10;
	numLines = 6;
	maxCols = 4;
	numLabels = 0;
	gradientWidth = 100;

	yMargin = 6;
	xMargin = 8;

	setMaximumSize(QWIDGETSIZE_MAX, exifHeight);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void DkMetaDataInfo::init() {

	mapIptcExif[DkSettings::camData_size] = 0;
	mapIptcExif[DkSettings::camData_orientation] = 0;
	mapIptcExif[DkSettings::camData_make] = 0;
	mapIptcExif[DkSettings::camData_model] = 0;
	mapIptcExif[DkSettings::camData_aperture] = 0;
	//mapIptcExif[DkSettings::camData_shutterspeed] = 0;
	mapIptcExif[DkSettings::camData_flash] = 0;
	mapIptcExif[DkSettings::camData_focallength] = 0;
	mapIptcExif[DkSettings::camData_exposuremode] = 0;
	mapIptcExif[DkSettings::camData_exposuretime] = 0;

	mapIptcExif[DkSettings::desc_rating] = 0;
	mapIptcExif[DkSettings::desc_usercomment] = 0;
	mapIptcExif[DkSettings::desc_date] = 0;
	mapIptcExif[DkSettings::desc_datetimeoriginal] = 0;
	mapIptcExif[DkSettings::desc_imagedescription] = 0;
	mapIptcExif[DkSettings::desc_creator] = 1;
	mapIptcExif[DkSettings::desc_creatortitle] = 1;
	mapIptcExif[DkSettings::desc_city] = 1;
	mapIptcExif[DkSettings::desc_country] = 1;
	mapIptcExif[DkSettings::desc_headline] = 1;
	mapIptcExif[DkSettings::desc_caption] = 1;
	mapIptcExif[DkSettings::desc_copyright] = 1;
	mapIptcExif[DkSettings::desc_keywords] = 1;

	mapIptcExif[DkSettings::desc_path] = 2;
	mapIptcExif[DkSettings::desc_filesize] = 2;

	worldMatrix = QTransform();

	setMouseTracking(true);
	//readTags();

	QColor tmpCol = bgCol;
	tmpCol.setAlpha(0);

	leftGradientRect = QRect(QPoint(), QSize(gradientWidth, exifHeight));
	leftGradient = QLinearGradient(leftGradientRect.topLeft(), leftGradientRect.topRight());
	leftGradient.setColorAt(0, tmpCol);
	leftGradient.setColorAt(1, bgCol);

	rightGradientRect = QRect(QPoint(size().width()-gradientWidth, 0), QSize(gradientWidth, exifHeight));
	rightGradient = QLinearGradient(rightGradientRect.topLeft(), rightGradientRect.topRight());
	rightGradient.setColorAt(0, bgCol);
	rightGradient.setColorAt(1, tmpCol);

}
void DkMetaDataInfo::setImageInfo(QSharedPointer<DkImageContainerT> imgC) {

	this->imgC = imgC;
	worldMatrix = QTransform();
	
	//DkTimer dt;
	if (isVisible()) {
		readTags();
		createLabels();
	}
}

void DkMetaDataInfo::readTags() {

	if (!imgC)
		return;

	try {
		if (mapIptcExif.empty())
			init();

		camDValues.clear();
		descValues.clear();


		//QString preExifI = "Exif.Image.";
		//QString preExifP = "Exif.Photo.";
		QString preIptc = "Iptc.Application2.";
		
		QFileInfo file = imgC->file();
#ifdef WITH_QUAZIP		
		if(imgC->isFromZip()) file = imgC->getZipData()->getZipFileInfo();
#endif
		QSharedPointer<DkMetaDataT> metaData = imgC->getMetaData();
		QStringList camSearchTags = DkMetaDataHelper::getInstance().getCamSearchTags();
		QStringList descSearchTags = DkMetaDataHelper::getInstance().getDescSearchTags();

		//if (metaData->isLoaded()) {

			for (int i=0; i<camSearchTags.size(); i++) {
				QString tmp, Value;

				if (mapIptcExif[i] == 0) {
					
					//tmp = preExifI + camDTags.at(i);
					tmp = camSearchTags.at(i);
					
					//special treatments
					// aperture
					if (i == DkSettings::camData_aperture) {
						Value = DkMetaDataHelper::getInstance().getApertureValue(metaData);
					}
					// focal length
					else if (i == DkSettings::camData_focallength) {
						Value = DkMetaDataHelper::getInstance().getFocalLength(metaData);
					}
					// exposure time
					else if (i == DkSettings::camData_exposuretime) {
						Value = DkMetaDataHelper::getInstance().getExposureTime(metaData);
					}
					else if (i == DkSettings::camData_size) {	
						Value = QString::number(imgC->image().width()) + " x " + QString::number(imgC->image().height());
					}
					else if (i == DkSettings::camData_exposuremode) {
						Value = DkMetaDataHelper::getInstance().getExposureMode(metaData);						
					} 
					else if (i == DkSettings::camData_flash) {
						Value = DkMetaDataHelper::getInstance().getFlashMode(metaData);
					}
					else {
						//qDebug() << "size" << imgSize.width() << imgSize.height();
						Value = metaData->getExifValue(tmp);
					}
				} else if (mapIptcExif[i] == 1) {
					tmp = preIptc + camSearchTags.at(i);
					Value = metaData->getIptcValue(tmp);
				}

				camDValues << Value;
			}
			//use getRating for Rating Value... otherwise the value is probably not correct: also Xmp.xmp.Rating, Xmp.MicrosoftPhoto.Rating is used
			QString rating;
			float tmp = metaData->getRating();
			if (tmp < 0) tmp=0;
			rating.setNum(tmp);
			descValues << rating;

		
			for (int i=1; i<descSearchTags.size(); i++) {
				QString tmp, Value;

				if (mapIptcExif[DkSettings::camData_end + i] == 0) {
					//tmp = preExifI + camDTags.at(i);
					tmp = descSearchTags.at(i);
					//qDebug() << tmp;
					Value = metaData->getExifValue(tmp);

					if (tmp.contains("Date"))
						Value = DkUtils::convertDateString(Value, file);

				} else if (mapIptcExif[DkSettings::camData_end + i] == 1) {
					tmp = preIptc + descSearchTags.at(i);
					Value = metaData->getIptcValue(tmp);
				} else if (mapIptcExif[DkSettings::camData_end + i] == 2) {
					//all other defined tags not in metadata
					tmp = descSearchTags.at(i);
					if (!tmp.compare("Path")) {
						Value = QString(file.absoluteFilePath());
					}
					else if (!tmp.compare("FileSize")) {
						Value = DkUtils::readableByte(file.size());
					} else
						Value = QString();
					
					//qDebug() << Value << "should be filepath";
				}
				descValues << Value;
			}
		//} else
		//	qDebug() << "Exif: file is not defined...";

	} catch (...) {
		qDebug() << "could not load Exif information";
	}
}

void DkMetaDataInfo::createLabels() {


	if (camDValues.empty() && descValues.empty()) {
		qDebug() << "no labels read (Exif)";
		return;
	}

	QStringList camDTags = DkMetaDataHelper::getInstance().getTranslatedCamTags();
	QStringList descTags = DkMetaDataHelper::getInstance().getTranslatedDescTags();

	if (camDValues.size() != camDTags.size() || descValues.size() != descTags.size()) {
		qDebug() << "error reading metadata: tag number is not equal value number";
		qDebug() << "cam value size: " << camDValues.size() << " cam tag size: " << camDTags.size();
		qDebug() << "desc value size: " << descValues.size() << " desc tag size: " << descTags.size();
		return;
	}

	for (int idx = 0; idx < pLabels.size(); idx++) {
		delete pLabels.at(idx);
	}
	
	for (int idx = 0; idx < pValues.size(); idx++) {
		delete pValues.at(idx);
	}

	pLabels.clear();
	pValues.clear();

	numLabels = 0;
	numLines = 6;
	for (int idx = 0; idx < DkSettings::metaData.metaDataBits.size(); idx++) {
		if (DkSettings::metaData.metaDataBits.testBit(idx))
			numLabels++;
	}

	// well that's a bit of a hack
	int cols = ((float)numLabels+numLines-1)/numLines > 2 ? ((float)numLabels+numLines-1)/numLines : 2;
	numLines = cvCeil((float)numLabels/cols);


	//pLabels.resize(camDTags.size() + descTags.size());
	//6 Lines...
	maxLenLabel.resize(cols);
	for (int i=0; i<cols; i++)
		maxLenLabel[i] = 0;

	numLabels=0;

	for(int i=0; i<camDTags.size(); i++) {
		//if bit set, create Label
		if (DkSettings::metaData.metaDataBits.testBit(i)) {
			DkLabel* pl = new DkLabel(this);//, camDTags.at(i));
			pl->setText(camDTags.at(i)+":",-1);
			pl->setFontSize(fontSize);
			pl->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			pl->setMouseTracking(true);
			DkLabel* pv = new DkLabel(this);//, camDValues.at(i));
			pv->setText(camDValues.at(i),-1);
			pv->setFontSize(fontSize);
			pv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
			pv->setMouseTracking(true);
			pLabels << pl;
			pValues << pv; 
			if (pl->geometry().width() > maxLenLabel[numLabels/numLines]) maxLenLabel[numLabels/numLines] = pl->geometry().width();
			numLabels++;
		}
	}

	for(int i=0; i<descTags.size(); i++) {
		//if bit set, create Label
		if (DkSettings::metaData.metaDataBits.testBit(DkSettings::camData_end + i)) {
			DkLabel* pl = new DkLabel(this);
			pl->setText(descTags.at(i)+":",-1);
			pl->setFontSize(fontSize);
			pl->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			pl->setMouseTracking(true);
			DkLabel* pv = new DkLabel(this);
			pv->setText(descValues.at(i),-1);
			pv->setFontSize(fontSize);
			pv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
			pv->setMouseTracking(true);
			pLabels << pl;
			pValues << pv;
			if (pl->geometry().width() > maxLenLabel[numLabels/numLines]) maxLenLabel[numLabels/numLines] = pl->geometry().width();
			numLabels++;
		}
	}


	//qDebug() << camDTags;
	//qDebug() << camDValues;

	layoutLabels();
}

void DkMetaDataInfo::layoutLabels() {

	if (pLabels.isEmpty())
		return;

	// #Labels / numLines = #Spalten
	numLines = 6;
	int cols = ((float)numLabels+numLines-1)/numLines > 2 ? ((float)numLabels+numLines-1)/numLines : 2;
	numLines = cvCeil((float)numLabels/cols);

	//qDebug() << "numCols: " << cols;

	if (cols > maxCols)
		qDebug() << "Labels are skipped...";

	//if (cols == 1) {
		exifHeight = (pLabels.at(0)->height() + yMargin)*numLines + yMargin;
	//} else exifHeight = 120;

	//widget size
	if (width() < minWidth)
		QWidget::setCursor(Qt::OpenHandCursor);
	else
		QWidget::unsetCursor();

	int width;
	//if (widthParent)
	//	width = widthParent > minWidth ? widthParent : minWidth;
	//else
	width = this->width() > minWidth ? this->width() : minWidth;

	//qDebug() << "width" << parent->width();

	//set Geometry if exif height is changed
	//setGeometry(0, parent->height()-exifHeight, parent->width(), exifHeight);

	//subtract label length
	for (int i=0; i<maxLenLabel.size(); i++) width -= (maxLenLabel[i] + xMargin);
	width-=xMargin;

	//rest length/#cols = column width for tags
	int widthValues = width/cols > 0 ? width/cols : 10;

	//subtract margin
	for (int i=0; i< cols; i++) widthValues -= xMargin;

	
	QPoint pos(xMargin + maxLenLabel[0], yMargin);
	int textHeight = fontSize;

	//set positions for labels
	for (int i=0; i<pLabels.size(); i++) {

		if (i%numLines == 0 && i>0) {
			pos = QPoint(pos.x() + maxLenLabel[i/numLines] + widthValues + xMargin*2, yMargin);

		}

		QPoint tmp = pos + QPoint(0, i*textHeight + i*yMargin);

		//pLabels.at(i)->move(pos + QPoint(0, (i%numLines)*textHeight + (i%numLines)*yMargin));
		QRect tmpRect = pLabels.at(i)->geometry();
		tmpRect.moveTopRight(QPoint(pos.x(), pos.y() + (i%numLines)*textHeight + (i%numLines)*yMargin));
		//qDebug() << "Pos X" << pos.x() << "PosY" << pos.y() + (i%numLines)*textHeight + (i%numLines)*yMargin;
		pLabels.at(i)->setGeometry(tmpRect);

		tmpRect = pValues.at(i)->geometry();
		tmpRect.moveTopLeft(pos + QPoint(xMargin, (i%numLines)*textHeight + (i%numLines)*yMargin));
		pValues.at(i)->setGeometry(tmpRect);
		pValues.at(i)->setFixedWidth(widthValues);

	}

	//update()

}

void DkMetaDataInfo::updateLabels() {

	if (mapIptcExif.empty())
		setImageInfo(imgC);

	createLabels();
}

void DkMetaDataInfo::setRating(int rating) {

	QString sRating;
	sRating.setNum(rating);

	for (int i=0; i<pLabels.size(); i++) {

		QString tmp = pLabels.at(i)->getText();
		if (!tmp.compare("Rating:")) {
			pValues.at(i)->setText(sRating, -1);
		}
	}
}

//void DkMetaDataInfo::setResolution(int xRes, int yRes) {
//
//	QString x,y;
//	x.setNum(xRes);
//	y.setNum(yRes);
//	x=x+"/1";
//	y=y+"/1";
//
//	metaData.setExifValue("Exif.Image.XResolution",x.toStdString());
//	metaData.setExifValue("Exif.Image.YResolution",y.toStdString());
//
//}


void DkMetaDataInfo::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	
	//painter.setWorldTransform(worldMatrix);
	//painter.setWorldMatrixEnabled(true);

	draw(&painter);
	painter.end();

	DkWidget::paintEvent(event);
}

void DkMetaDataInfo::resizeEvent(QResizeEvent *resizeW) {
	//qDebug() << "resizeW:" << resizeW->size().width();
	//qDebug() << parent->width();

	//setMinimumHeight(1);
	//setMaximumHeight(exifHeight);

	//resize(parent->width(), resizeW->size().height());
	
	int gw = qMin(gradientWidth, qRound(0.2f*resizeW->size().width()));

	rightGradientRect.setTopLeft(QPoint(resizeW->size().width()-gw, 0));
	rightGradientRect.setSize(QSize(gw, resizeW->size().height()));
	leftGradientRect.setSize(QSize(gw, resizeW->size().height()));

	rightGradient.setStart(rightGradientRect.topLeft());
	rightGradient.setFinalStop(rightGradientRect.topRight());
	leftGradient.setStart(leftGradientRect.topLeft());
	leftGradient.setFinalStop(leftGradientRect.topRight());

	if (resizeW->size().width() > minWidth) {
		worldMatrix = QTransform();
		layoutLabels();
		//qDebug() << "parent->width() > minWidth  d.h. layoutlabels";
	}

	if (resizeW->size().width() < minWidth && worldMatrix.dx() == 0) {
		layoutLabels();
	}

	if ((resizeW->size().width() < minWidth) && (worldMatrix.dx()+minWidth < resizeW->size().width())) {
		//layoutLabels();
		QTransform tmpMatrix = QTransform();
		int dX = (resizeW->size().width()-minWidth) - worldMatrix.dx();
		
		tmpMatrix.translate(dX, 0);
		worldMatrix.translate(dX, 0);

		for (int i=0; i< pLabels.size(); i++) {
			pLabels.at(i)->setGeometry(tmpMatrix.mapRect(pLabels.at(i)->geometry()));
			pValues.at(i)->setGeometry(tmpMatrix.mapRect(pValues.at(i)->geometry()));
		}
		//layoutLabels();
	}

	DkWidget::resizeEvent(resizeW);
}

void DkMetaDataInfo::draw(QPainter* painter) {

	//QImage* img;

	if (!painter)
		return;

	//labels are left outside of the widget -> set gradient
	if (width() < minWidth && worldMatrix.dx() < 0) {
		
		if (-worldMatrix.dx() < leftGradientRect.width())
			leftGradient.setFinalStop(-worldMatrix.dx(), 0);
		painter->fillRect(leftGradientRect, leftGradient);
	}
	else
		painter->fillRect(leftGradientRect, bgCol);

	//labels are right outside of the widget -> set gradient
	if (width() < minWidth && worldMatrix.dx()+minWidth > width()) {
		
		int rightOffset = worldMatrix.dx()+minWidth-width();
		if (rightOffset < rightGradientRect.width())
			rightGradient.setStart(rightGradientRect.left()+(rightGradientRect.width() - rightOffset), 0);
		painter->fillRect(rightGradientRect, rightGradient);
	}
	else
		painter->fillRect(rightGradientRect, bgCol);
	
	painter->fillRect(QRect(QPoint(leftGradientRect.right()+1,0), QSize(size().width()-leftGradientRect.width()-rightGradientRect.width(), size().height())), bgCol);
}

void DkMetaDataInfo::mouseMoveEvent(QMouseEvent *event) {


	if (lastMousePos.isNull()) {
		lastMousePos = event->pos();
		QWidget::mouseMoveEvent(event);
		return;
	}

	if (event->buttons() == Qt::LeftButton && width() < minWidth)
		QWidget::setCursor(Qt::ClosedHandCursor);
	if (event->buttons() != Qt::LeftButton && width() > minWidth)
		QWidget::unsetCursor();
	if (event->buttons() != Qt::LeftButton && width() < minWidth)
		QWidget::setCursor(Qt::OpenHandCursor);


	if (event->buttons() != Qt::LeftButton)
		lastMousePos = event->pos();

	int mouseDir = event->pos().x() - lastMousePos.x();


	if (width() < minWidth && event->buttons() == Qt::LeftButton) {
		
		currentDx = mouseDir;

		lastMousePos = event->pos();

		QTransform tmpMatrix = QTransform();
		tmpMatrix.translate(currentDx, 0);

		if (((worldMatrix.dx()+currentDx)+minWidth >= width() && currentDx < 0) ||
		    ((worldMatrix.dx()+currentDx) <= 0 && currentDx > 0)) {

			worldMatrix.translate(currentDx, 0);

			for (int i=0; i< pLabels.size(); i++) {
				pLabels.at(i)->setGeometry(tmpMatrix.mapRect(pLabels.at(i)->geometry()));
				pValues.at(i)->setGeometry(tmpMatrix.mapRect(pValues.at(i)->geometry()));
			}
		}

		update();

		return;
	}

	lastMousePos = event->pos();

	QWidget::mouseMoveEvent(event);
}


// DkTransformRectangle --------------------------------------------------------------------
DkTransformRect::DkTransformRect(int idx, DkRotatingRect* rect, QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f) {

	this->parentIdx = idx;
	this->size = QSize(12, 12);
	this->rect = rect;

	init();

	this->resize(size);
	setCursor(Qt::CrossCursor);
}

void DkTransformRect::init() {

}

void DkTransformRect::draw(QPainter *painter) {

	QPen penNoStroke;
	penNoStroke.setWidth(0);
	penNoStroke.setColor(QColor(0,0,0,0));
	
	QPen pen;
	//pen.setWidth(1);
	pen.setColor(QColor(255,255,0,100));

	QRectF visibleRect(QPointF(), QSizeF(5,5));
	QRectF whiteRect(QPointF(), QSize(7,7));
	visibleRect.moveCenter(geometry().center());
	whiteRect.moveCenter(geometry().center());

	// draw the control point
	painter->setWorldMatrixEnabled(false);
	painter->setPen(penNoStroke);
	painter->setBrush(QColor(0, 0, 0, 0));
	painter->drawRect(geometry());	// invisible rect for mouseevents...
	//painter->setPen(pen);
	painter->setBrush(QColor(255,255,255, 100));
	painter->drawRect(whiteRect);
	painter->setBrush(QColor(0,0,0));
	painter->drawRect(visibleRect);
	painter->setWorldMatrixEnabled(true);
}

void DkTransformRect::mousePressEvent(QMouseEvent *event) {
	
	if (event->buttons() == Qt::LeftButton) {
		posGrab = event->globalPos();
		initialPos = geometry().topLeft();

		emit updateDiagonal(parentIdx);
	}
	qDebug() << "mouse pressed control point";
	QWidget::mousePressEvent(event);
}

void DkTransformRect::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		
		QPointF pt = initialPos+event->globalPos()-posGrab;
		emit ctrlMovedSignal(parentIdx, pt, event->modifiers() == Qt::ShiftModifier, true);
		qDebug() << "accepted false...";
	}

	QWidget::mouseMoveEvent(event);
}

void DkTransformRect::mouseReleaseEvent(QMouseEvent *event) {

	QWidget::mouseReleaseEvent(event);
}

void DkTransformRect::enterEvent(QEvent *event) {

	if (rect)
		setCursor(rect->cpCursor(parentIdx));
}

// DkEditableRectangle --------------------------------------------------------------------
DkEditableRect::DkEditableRect(QRectF rect, QWidget* parent, Qt::WindowFlags f) : DkWidget(parent, f) {

	this->parent = parent;
	this->rect = rect;

	rotatingCursor = QCursor(QPixmap(":/nomacs/img/rotating-cursor.png"));
	
	setAttribute(Qt::WA_MouseTracking);
	paintMode = DkCropToolBar::no_guide;
	showInfo = false;

	pen = QPen(QColor(0, 0, 0, 255), 1);
	pen.setCosmetic(true);
	brush = (DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless :
		DkSettings::display.bgColorWidget;

	state = do_nothing;
	worldTform = 0;
	imgTform = 0;
	imgRect = 0;

	oldDiag = DkVector(-1.0f, -1.0f);
	
	for (int idx = 0; idx < 8; idx++) {
		ctrlPoints.push_back(new DkTransformRect(idx, &this->rect, this));
		ctrlPoints[idx]->hide();
		connect(ctrlPoints[idx], SIGNAL(ctrlMovedSignal(int, QPointF, bool, bool)), this, SLOT(updateCorner(int, QPointF, bool, bool)));
		connect(ctrlPoints[idx], SIGNAL(updateDiagonal(int)), this, SLOT(updateDiagonal(int)));
	}
	
	panning = false;
}

void DkEditableRect::reset() {

	rect = QRectF();
	//for (int idx = 0; idx < ctrlPoints.size(); idx++)
	//	ctrlPoints[idx]->reset();

}

QPointF DkEditableRect::map(const QPointF &pos) {

	QPointF posM = pos;
	if (worldTform) posM = worldTform->inverted().map(posM);
	if (imgTform)	posM = imgTform->inverted().map(posM);
	
	return posM;
}

QPointF DkEditableRect::clipToImage(const QPointF &pos) {
	
	if (!imgRect)
		return QPointF(pos);

	QRectF imgViewRect(*imgRect);
	if (worldTform) imgViewRect = worldTform->mapRect(imgViewRect);

	float x = pos.x();
	float y = pos.y();

	if (x < imgViewRect.left())
		x = imgViewRect.left();
	if (x > imgViewRect.right())
		x = imgViewRect.right();

	if (y < imgViewRect.top())
		y = imgViewRect.top();
	if (y > imgViewRect.bottom())
		y = imgViewRect.bottom();

	return QPointF(x,y);		// round
}

void DkEditableRect::updateDiagonal(int idx) {

	// we need to store the old diagonal in order to enable "keep aspect ratio"
	if (rect.isEmpty())
		oldDiag = DkVector(-1.0f, -1.0f);
	else
		oldDiag = rect.getDiagonal(idx);
}

void DkEditableRect::setFixedDiagonal(const DkVector& diag) {

	fixedDiag = diag;

	qDebug() << "after rotating: " << fixedDiag.getQPointF();

	// don't update in that case
	if (diag.x == 0 || diag.y == 0)
		return;
	else
		fixedDiag.rotate(-rect.getAngle());

	QPointF c = rect.getCenter();

	if (!rect.getPoly().isEmpty()) 
		rect.updateCorner(0, rect.getPoly().at(0), fixedDiag);

	rect.setCenter(c);
	update();
}

void DkEditableRect::setPanning(bool panning) {
	this->panning = panning;
	setCursor(Qt::OpenHandCursor);
	qDebug() << "panning set...";
}

void DkEditableRect::updateCorner(int idx, QPointF point, bool isShiftDown, bool changeState) {

	if (changeState)
		state = scaling;

	DkVector diag = (isShiftDown || fixedDiag.x != 0 && fixedDiag.y != 0) ? oldDiag : DkVector();

	rect.updateCorner(idx, map(point), diag);

	// edge control -> remove aspect ratio constraint
	if (idx >= 4 && idx < 8)
		emit aRatioSignal(QPointF(0,0));

	update();
}

void DkEditableRect::paintEvent(QPaintEvent *event) {

	// create path
	QPainterPath path;
	QRectF canvas = QRectF(geometry().x()-1, geometry().y()-1, geometry().width()+1, geometry().height()+1);
	path.addRect(canvas);
	
	QPolygonF p;
	if (!rect.isEmpty()) {
		// TODO: directly map the points (it's easier and not slower at all)
		p = rect.getClosedPoly();
		p = tTform.map(p);
		p = rTform.map(p); 
		p = tTform.inverted().map(p);
		if (imgTform) p = imgTform->map(p);
		if (worldTform) p = worldTform->map(p);
		path.addPolygon(p);
	}

	// now draw
	QPainter painter(this);

	painter.setPen(pen);
	painter.setBrush(brush);
	painter.drawPath(path);

	drawGuide(&painter, p, paintMode);
	
	//// debug
	//painter.drawPoint(rect.getCenter());

	// this changes the painter -> do it at the end
	if (!rect.isEmpty()) {
		
		for (int idx = 0; idx < ctrlPoints.size(); idx++) {
			
			QPointF cp;
			
			if (idx < 4) {
				QPointF c = p[idx];
				cp = c-ctrlPoints[idx]->getCenter();
			}
			// paint control points in the middle of the edge
			else if (idx >= 4) {
				QPointF s = ctrlPoints[idx]->getCenter();

				QPointF lp = p[idx % 4];
				QPointF rp = p[(idx+1) % 4];

				QVector2D lv = QVector2D(lp-s);
				QVector2D rv = QVector2D(rp-s);

				cp = (lv + 0.5*(rv - lv)).toPointF();
			}

			ctrlPoints[idx]->move(qRound(cp.x()+0.5f), qRound(cp.y()+0.5f));
			ctrlPoints[idx]->draw(&painter);
		}
	}
 
	painter.end();

	QWidget::paintEvent(event);
}

void DkEditableRect::drawGuide(QPainter* painter, const QPolygonF& p, int paintMode) {

	if (p.isEmpty() || paintMode == DkCropToolBar::no_guide)
		return;

	QColor col = painter->pen().color();
	col.setAlpha(150);
	QPen pen = painter->pen();
	QPen cPen = pen;
	cPen.setColor(col);
	painter->setPen(cPen);

	// vertical
	DkVector lp = p[1]-p[0];	// parallel to drawing
	DkVector l9 = p[3]-p[0];	// perpendicular to drawing

	int nLines = (paintMode == DkCropToolBar::rule_of_thirds) ? 3 : l9.norm()/20;
	DkVector offset = l9;
	offset.normalize();
	offset *= l9.norm()/nLines;

	DkVector offsetVec = offset;

	for (int idx = 0; idx < (nLines-1); idx++) {

		// step through & paint
		QLineF l = QLineF(DkVector(p[1]+offsetVec).getQPointF(), DkVector(p[0]+offsetVec).getQPointF());
		painter->drawLine(l);
		offsetVec += offset;
	}

	// horizontal
	lp = p[3]-p[0];	// parallel to drawing
	l9 = p[1]-p[0];	// perpendicular to drawing

	nLines = (paintMode == DkCropToolBar::rule_of_thirds) ? 3 : l9.norm()/20;
	offset = l9;
	offset.normalize();
	offset *= l9.norm()/nLines;

	offsetVec = offset;

	for (int idx = 0; idx < (nLines-1); idx++) {

		// step through & paint
		QLineF l = QLineF(DkVector(p[3]+offsetVec).getQPointF(), DkVector(p[0]+offsetVec).getQPointF());
		painter->drawLine(l);
		offsetVec += offset;
	}

	painter->setPen(pen);	// revert painter

}

// make events callable
void DkEditableRect::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == DkSettings::global.altMod || panning)) {
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	posGrab = map(QPointF(event->pos()));
	clickPos = QPointF(event->pos());

	if (rect.isEmpty()) {
		state = initializing;
		setAngle(0);
	}
	else if (rect.getPoly().containsPoint(posGrab, Qt::OddEvenFill)) {
		state = moving;
	}
	else {
		state = rotating;
	}

}

void DkEditableRect::mouseMoveEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod ||
		panning) {
		
		if (event->buttons() != Qt::LeftButton)
			setCursor(Qt::OpenHandCursor);
		else
			setCursor(Qt::ClosedHandCursor);

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
		return;
	}

	QPointF posM = map(QPointF(event->pos()));
	
	if (event->buttons() != Qt::LeftButton && !rect.isEmpty()) {
		// show rotating - moving
		if (rect.getPoly().containsPoint(map(event->pos()), Qt::OddEvenFill))
			setCursor(Qt::SizeAllCursor);
		else
			setCursor(rotatingCursor);
	}
	else if (rect.isEmpty())
		setCursor(Qt::CrossCursor);

	// additionally needed for showToolTip
	double angle = 0;

	if (state == initializing && event->buttons() == Qt::LeftButton) {

		QPointF clipPos = clipToImage(QPointF(event->pos()));

		if (!imgRect || !rect.isEmpty() || clipPos == QPointF(event->pos())) {
			
			if (rect.isEmpty()) {

				for (int idx = 0; idx < ctrlPoints.size(); idx++)
					ctrlPoints[idx]->show();

				QPointF p = map(clipToImage(clickPos));
				rect.setAllCorners(p);
			}
			
			DkVector diag;
			
			// when initializing shift should make the rect a square
			if (event->modifiers() == Qt::ShiftModifier)
				diag = DkVector(1.0f, 1.0f);
			else
				diag = fixedDiag;
			rect.updateCorner(2, map(clipPos), diag);
			update();
		}
 
	}
	else if (state == moving && event->buttons() == Qt::LeftButton) {
		
		QPointF dxy = posM-posGrab;
		rTform.translate(dxy.x(), dxy.y());
		posGrab = posM;
		update();
	}
	else if (state == rotating && event->buttons() == Qt::LeftButton) {

		DkVector c(rect.getCenter());
		DkVector xt(posGrab);
		DkVector xn(posM);

		// compute the direction vector;
		xt = c-xt;
		xn = c-xn;
		angle = xn.angle() - xt.angle();


		// just rotate in CV_PI*0.25 steps if shift is pressed
		if (event->modifiers() == Qt::ShiftModifier) {
			double angleRound = DkMath::normAngleRad(angle+rect.getAngle(), -CV_PI*0.125, CV_PI*0.125);
			angle -= angleRound;
		}
					
		setAngle(angle, false);
	}

	if (event->buttons() == Qt::LeftButton && state != moving) {

		QPolygonF p = rect.getPoly();

		double sAngle = (rect.getAngle()+angle)*DK_RAD2DEG;

		while (sAngle > 90)
			sAngle -= 180;
		while (sAngle < -90)
			sAngle += 180;

		sAngle = qRound(sAngle*100)/100.0f;
		int height = qRound(DkVector(p[1]-p[0]).norm());
		int width = qRound(DkVector(p[3]-p[0]).norm());

		if (showInfo) {
			QToolTip::showText(event->globalPos(),
				QString::number(width) + " x " +
				QString::number(height) + " px\n" +
				QString::number(sAngle) + "°",
				this);
		}

		emit statusInfoSignal(QString::number(width) + " x " + QString::number(height) + " px | " + QString::number(sAngle) + "°");
	}

	//QWidget::mouseMoveEvent(event);
	//qDebug() << "edit rect mouse move";
}

void DkEditableRect::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == DkSettings::global.altMod || panning)) {
		setCursor(Qt::OpenHandCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}

	state = do_nothing;

	applyTransform();
	//QWidget::mouseReleaseEvent(event);
}

void DkEditableRect::wheelEvent(QWheelEvent* event) {

	QWidget::wheelEvent(event);
	update();	// this is an extra update - however we get rendering errors otherwise?!
}

void DkEditableRect::applyTransform() {

	// apply transform
	QPolygonF p = rect.getPoly();
	p = tTform.map(p);
	p = rTform.map(p); 
	p = tTform.inverted().map(p);

	// Cropping tool fix start

	// Check the order or vertexes
	float signedArea = (p[1].x() - p[0].x()) * (p[2].y() - p[0].y()) - (p[1].y()- p[0].y()) * (p[2].x() - p[0].x());
	// If it's wrong, just change it
	if (signedArea > 0) {
		QPointF tmp = p[1];
		p[1] = p[3];
		p[3] = tmp;
	}
	// Cropping tool fix end

	rect.setPoly(p);

	rTform.reset();	
	tTform.reset();
	update();

}

void DkEditableRect::keyPressEvent(QKeyEvent *event) {

	if (event->key() == Qt::Key_Alt)
		setCursor(Qt::OpenHandCursor);

	QWidget::keyPressEvent(event);
}

void DkEditableRect::keyReleaseEvent(QKeyEvent *event) {

	//if (event->key() == Qt::Key_Escape)
	//	hide();
	//else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
	//	
	//	if (!rect.isEmpty())
	//		emit enterPressedSignal(rect);

	//	setVisible(false);
	//	setWindowOpacity(0);
	//}

	qDebug() << "key pressed rect";

	QWidget::keyPressEvent(event);
}

void DkEditableRect::setPaintHint(int paintMode /* = DkCropToolBar::no_guide */) {

	qDebug() << "painting mode: " << paintMode;
	this->paintMode = paintMode;
	update();
}

void DkEditableRect::setShadingHint(bool invert) {

	QColor col = brush.color();
	col = QColor(255-col.red(), 255-col.green(), 255-col.blue(), col.alpha());
	brush.setColor(col);

	col = pen.color();
	col = QColor(255-col.red(), 255-col.green(), 255-col.blue(), col.alpha());
	pen.setColor(col);

	update();
}

void DkEditableRect::setShowInfo(bool showInfo) {
	this->showInfo = showInfo;
}

void DkEditableRect::setAngle(double angle, bool apply) {

	DkVector c(rect.getCenter());

	if (!tTform.isTranslating())
		tTform.translate(-c.x, -c.y);
	
	rTform.reset();
	if (apply)
		rTform.rotateRadians(angle-rect.getAngle());
	else
		rTform.rotateRadians(angle);
	
	if (apply)
		applyTransform();
	else {
		emit angleSignal(rect.getAngle()+angle);
		update();
	}

}

void DkEditableRect::setVisible(bool visible) {

	if (!visible) {
		
		rect = DkRotatingRect();
		for (int idx = 0; idx < ctrlPoints.size(); idx++)
			ctrlPoints[idx]->hide();
	}
	else {
		//setFocus(Qt::ActiveWindowFocusReason);
		setCursor(Qt::CrossCursor);
	}

	DkWidget::setVisible(visible);
}

// DkEditableRect --------------------------------------------------------------------
DkCropWidget::DkCropWidget(QRectF rect /* = QRect */, QWidget* parent /* = 0*/, Qt::WindowFlags f /* = 0*/) : DkEditableRect(rect, parent, f) {

	cropToolbar = 0;
}

void DkCropWidget::createToolbar() {

	cropToolbar = new DkCropToolBar(tr("Crop Toolbar"), this);

	connect(cropToolbar, SIGNAL(cropSignal()), this, SLOT(crop()));
	connect(cropToolbar, SIGNAL(cancelSignal()), this, SIGNAL(cancelSignal()));
	connect(cropToolbar, SIGNAL(aspectRatio(const DkVector&)), this, SLOT(setFixedDiagonal(const DkVector&)));
	connect(cropToolbar, SIGNAL(angleSignal(double)), this, SLOT(setAngle(double)));
	connect(cropToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
	connect(cropToolbar, SIGNAL(paintHint(int)), this, SLOT(setPaintHint(int)));
	connect(cropToolbar, SIGNAL(shadingHint(bool)), this, SLOT(setShadingHint(bool)));
	connect(cropToolbar, SIGNAL(showInfo(bool)), this, SLOT(setShowInfo(bool)));
	connect(this, SIGNAL(angleSignal(double)), cropToolbar, SLOT(angleChanged(double)));
	connect(this, SIGNAL(aRatioSignal(const QPointF&)), cropToolbar, SLOT(setAspectRatio(const QPointF&)));

	cropToolbar->loadSettings();	// need to this manually after connecting the slots

}

void DkCropWidget::crop() {

	if (!cropToolbar)
		return;

	if (!rect.isEmpty())
		emit enterPressedSignal(rect, cropToolbar->getColor());

	setVisible(false);
	setWindowOpacity(0);
}

void DkCropWidget::setVisible(bool visible) {

	if (visible && !cropToolbar)
		createToolbar();

	emit showToolbar(cropToolbar, visible);
	DkEditableRect::setVisible(visible);
}


// DkAnimagionLabel --------------------------------------------------------------------
DkAnimationLabel::DkAnimationLabel(QString animationPath, QWidget* parent) : DkLabel(parent) {

	init(animationPath, QSize());
}

DkAnimationLabel::DkAnimationLabel(QString animationPath, QSize size, QWidget* parent) : DkLabel(parent) {

	init(animationPath, size);
}

DkAnimationLabel::~DkAnimationLabel() {

	animation->deleteLater();
}

void DkAnimationLabel::init(const QString& animationPath, const QSize& size) {
	
	animation = new QMovie(animationPath);
	margin = QSize(14, 14);

	QSize s = size;
	if(s.isEmpty()) {
		animation->jumpToNextFrame();
		s = animation->currentPixmap().size();
		animation->jumpToFrame(0);

		// padding
		s += margin;
	}

	setFixedSize(s);
	setMovie(animation);
	hide();

	setStyleSheet("QLabel {background-color: " + DkUtils::colorToString(bgCol) + "; border-radius: 10px;}");
}

void DkAnimationLabel::halfSize() {

	// allows for anti-aliased edges of gif animations
	if (animation) {
		QSize s = (size()-margin)*0.5f;
		animation->setScaledSize(s);
		setFixedSize(s+margin);
	}
}

void DkAnimationLabel::showTimed(int time) {
	
	if(!this->animation.isNull() &&
		(this->animation->state() == QMovie::NotRunning ||
		 this->animation->state() == QMovie::Paused)) {
		
			this->animation->start();
	}
	DkLabel::showTimed(time);
}


void DkAnimationLabel::hide() {
	
	if(!this->animation.isNull()) {
		if(this->animation->state() == QMovie::Running) {
			
			this->animation->stop();
		}
	}

	DkLabel::hide();
}

DkColorChooser::DkColorChooser(QColor defaultColor, QString text, QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags) {

	this->defaultColor = defaultColor;
	this->text = text;

	init();

}

void DkColorChooser::init() {

	accept = false;

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");
	colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(11,0,11,0);
	
	QLabel* colorLabel = new QLabel(text, this);
	colorButton = new QPushButton("", this);
	colorButton->setFlat(true);
	colorButton->setObjectName("colorButton");
	colorButton->setAutoDefault(false);
	
	QPushButton* resetButton = new QPushButton(tr("Reset"), this);
	resetButton->setObjectName("resetButton");
	resetButton->setAutoDefault(false);

	QWidget* colWidget = new QWidget(this);
	QHBoxLayout* hLayout = new QHBoxLayout(colWidget);
	hLayout->setContentsMargins(11,0,11,0);

	hLayout->addWidget(colorButton);
	hLayout->addWidget(resetButton);
	hLayout->addStretch();

	vLayout->addWidget(colorLabel);
	vLayout->addWidget(colWidget);

	setColor(defaultColor);
	QMetaObject::connectSlotsByName(this);
}

void DkColorChooser::setColor(QColor color) {

	colorDialog->setCurrentColor(color);
	colorButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(color) + "; border: 1px solid #888; min-height: 24px}");
}

QColor DkColorChooser::getColor() {
	return colorDialog->currentColor();
}

void DkColorChooser::on_resetButton_clicked() {
	setColor(defaultColor);
	emit resetClicked();
}

void DkColorChooser::on_colorButton_clicked() {
	colorDialog->show();
}

void DkColorChooser::on_colorDialog_accepted() {
	
	setColor(colorDialog->currentColor());
	accept = true;
	emit accepted();
}

// Image histogram  -------------------------------------------------------------------
DkHistogram::DkHistogram(QWidget *parent) : DkWidget(parent){
	
	setObjectName("DkHistogram");
	this->parent = parent;
	this->setMinimumWidth(260);
	this->setMinimumHeight(130);
	isPainted = false;
	maxValue = 20;
	scaleFactor = 1;
}

DkHistogram::~DkHistogram() {


}

/**
 * Paints the image histogram
 **/
void DkHistogram::paintEvent(QPaintEvent* event) {

	QPainter painter(this);
	painter.setPen(QColor(200, 200, 200));
	painter.fillRect(1, 1, width() - 3, height() - 2, bgCol);
	painter.drawRect(1, 1, width() - 3, height() - 2);

	if(isPainted && maxValue > 0){
		for(int i = 0; i < 256; i++){
			int rLineHeight = ((int) (hist[0][i] * (height() - 4) * scaleFactor / maxValue) < height() - 4) ? (int) (hist[0][i] * (height() - 4) * scaleFactor / maxValue) : height() - 4;
			int gLineHeight = ((int) (hist[1][i] * (height() - 4) * scaleFactor / maxValue) < height() - 4) ? (int) (hist[1][i] * (height() - 4) * scaleFactor / maxValue) : height() - 4;
			int bLineHeight = ((int) (hist[2][i] * (height() - 4) * scaleFactor / maxValue) < height() - 4) ? (int) (hist[2][i] * (height() - 4) * scaleFactor / maxValue) : height() - 4;
			int maxLineHeight = (rLineHeight > gLineHeight) ? ((rLineHeight > bLineHeight) ? rLineHeight : bLineHeight) :  ((gLineHeight > bLineHeight) ? gLineHeight : bLineHeight);

			int vCombined = qMin(qMin(rLineHeight, gLineHeight), bLineHeight);

			for(int j = 0; j <= maxLineHeight; j++) {

				if(j <= rLineHeight && j <= gLineHeight && j <= bLineHeight) {
				
					// make last pixel lighter -> enhances visual appearence
					int c = (j == vCombined && rLineHeight == gLineHeight && gLineHeight == bLineHeight) ? 200 : 100;
					painter.setPen(QColor(c,c,c));
				}
				else if(j <= rLineHeight && j <= gLineHeight) painter.setPen(Qt::yellow);
				else if(j <= rLineHeight && j <= bLineHeight) painter.setPen(Qt::magenta);
				else if(j <= gLineHeight && j <= bLineHeight) painter.setPen(Qt::cyan);
				else if(j <= rLineHeight) painter.setPen(Qt::red);
				else if(j <= gLineHeight) painter.setPen(Qt::green);
				else if(j <= bLineHeight) painter.setPen(Qt::blue);
				else 
					continue;

				painter.drawPoint(2 + i, height() - j - 2);
			}
		}
	}
}

/**
 * Goes through the image and counts pixels values. They are used to create the image histogram.
 * @param currently displayed image
 **/ 
void DkHistogram::drawHistogram(QImage imgQt) {

	if (!isVisible() || imgQt.isNull()) {
		setPainted(false);
		return;
	}

	DkTimer dt;

#ifdef WITH_OPENCV


	long histValues[3][256];

	for (int idx = 0; idx < 256; idx++) {
		histValues[0][idx] = 0;
		histValues[1][idx] = 0;
		histValues[2][idx] = 0;
	}
	

	// 8 bit images
	if (imgQt.depth() == 8) {

		qDebug() << "8 bit histogram -------------------";

		for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {

			const unsigned char* pixel = imgQt.constScanLine(rIdx);

			for (int cIdx = 0; cIdx < imgQt.width(); cIdx++, pixel++) {

				histValues[0][*pixel]++;
				histValues[1][*pixel]++;
				histValues[2][*pixel]++;
			}
		}
	}
	// 24 bit images
	else if (imgQt.depth() == 24) {
		
		qDebug() << "24 bit histogram -------------------";

		// TODO: not tested!!
		for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {

			const unsigned char* pixel = imgQt.constScanLine(rIdx);

			for (int cIdx = 0; cIdx < imgQt.width(); cIdx++) {

				// If I understood the api correctly, the first bits are 0 if we have 24bpp & < 8 bits per channel
				histValues[0][*pixel]++; pixel++;
				histValues[1][*pixel]++; pixel++;
				histValues[2][*pixel]++; pixel++;
			}
		}
	}
	// 32 bit images
	else if (imgQt.depth() == 32) {
		
		for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {
		
			const QRgb* pixel = (QRgb*)(imgQt.constScanLine(rIdx));
	
			for (int cIdx = 0; cIdx < imgQt.width(); cIdx++, pixel++) {

				histValues[0][qRed(*pixel)]++;
				histValues[1][qGreen(*pixel)]++;
				histValues[2][qBlue(*pixel)]++;
			}
		}
	}

	int maxHistValue = 0;

	for (int idx = 0; idx < 256; idx++) {
		
		if (histValues[0][idx] > maxHistValue)
			maxHistValue = histValues[0][idx];
		if (histValues[1][idx] > maxHistValue)
			maxHistValue = histValues[1][idx];
		if (histValues[2][idx] > maxHistValue)
			maxHistValue = histValues[2][idx];
	}

	//Mat imgMat = DkImage::qImage2Mat(imgQt);
	//
	//vector<Mat> imgChannels;
	//split(imgMat, imgChannels);

	//int noChannels = (imgChannels.size() < 3) ? 1 : 3;

	//// Set the number of bins
	//int histSize = 256;
	//// Set the ranges for B,G,R
	//float range[] = { 0, 256 } ;
	//const float* histRange = { range };

	//MatND hist;
	//// note: long == int if compiled with a 32bit compiler
	//long histValues[3][256];
	//long maxHistValue = 0;

	//for (int i = 0; i < noChannels; i++) {

	//	calcHist( &imgChannels[(noChannels - 1) - i], 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false); // careful! channels are rotated: B,G,R
	//	
	//	for (int j = 0; j < 256; j++) histValues[i][j] = hist.at<float>(j);
	//	hist.setTo(0);
	//}

	//if (noChannels == 1) {

	//	for (int i = 0; i < 256; i++) {
	//		histValues[2][i] = histValues[1][i] = histValues[0][i];
	//		
	//		if(histValues[0][i] > maxHistValue) maxHistValue = histValues[0][i];
	//	}
	//}
	//else {

	//	for (int i = 0; i < 256; i++) {
	//		long maxRGB = (histValues[0][i] > histValues[1][i]) ? 
	//			((histValues[0][i] > histValues[2][i]) ? histValues[0][i] : histValues[2][i]) :  
	//			((histValues[1][i] > histValues[2][i]) ? histValues[1][i] : histValues[2][i]);

	//		if(maxRGB > maxHistValue) maxHistValue = maxRGB;
	//	}
	//}
	//qDebug() << "computing the histogram took me: " << dt.getTotal();

	setMaxHistogramValue(maxHistValue);
	updateHistogramValues(histValues);
	setPainted(true);

#else

	setPainted(false);

#endif
	
	qDebug() << "drawing the histogram took me: " << dt.getTotal();

	update();
}

/**
 * Clears the histogram panel
 **/ 
void DkHistogram::clearHistogram() {

	setPainted(false);
	update();
}

void DkHistogram::setPainted(bool isPainted) {

	this->isPainted = isPainted;
}

void DkHistogram::setMaxHistogramValue(long maxValue) {

	if (maxValue == 0)
		setPainted(false);

	this->maxValue = maxValue;
}

/**
 * Updates histogram values.
 * @param values to be copied
 **/ 
void DkHistogram::updateHistogramValues(long histValues[][256]) {

	for(int i = 0; i < 256; i++) {
		this->hist[0][i] = histValues[0][i];
		this->hist[1][i] = histValues[1][i];
		this->hist[2][i] = histValues[2][i];
	}
}

/**
 * Mouse events for scaling the histogram - enlarge the histogram between the bottom axis and the cursor position
 **/ 
void DkHistogram::mousePressEvent(QMouseEvent *event) {

	// always propagate mouse events
	if (event->buttons() != Qt::LeftButton)
		DkWidget::mousePressEvent(event);
}

void DkHistogram::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		
		float cp = height() - event->pos().y();
		
		if (cp > 0) {
			scaleFactor = height() / cp;
			update();
		}
	}
	else
		DkWidget::mouseMoveEvent(event);

}

void DkHistogram::mouseReleaseEvent(QMouseEvent *event) {
	
	scaleFactor = 1;
	update();

	if (event->buttons() != Qt::LeftButton)
		DkWidget::mouseReleaseEvent(event);
}

// DkSlider --------------------------------------------------------------------
DkSlider::DkSlider(QString title, QWidget* parent) : QWidget(parent) {

	createLayout();
	
	// init
	titleLabel->setText(title);
	
	// defaults
	setMinimum(0);	
	setMaximum(100);
	setTickInterval(1);
	setValue(50);
}

void DkSlider::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);
	
	QWidget* dummy = new QWidget(this);
	QHBoxLayout* titleLayout = new QHBoxLayout(dummy);
	titleLayout->setContentsMargins(0,0,0,5);

	QWidget* dummyBounds = new QWidget(this);
	QHBoxLayout* boundsLayout = new QHBoxLayout(dummyBounds);
	boundsLayout->setContentsMargins(0,0,0,0);

	titleLabel = new QLabel(this);
	
	sliderBox = new QSpinBox(this);

	slider = new QSlider(this);
	slider->setOrientation(Qt::Horizontal);

	minValLabel = new QLabel(this);
	maxValLabel = new QLabel(this);
	
	titleLayout->addWidget(titleLabel);
	titleLayout->addStretch();
	titleLayout->addWidget(sliderBox);

	boundsLayout->addWidget(minValLabel);
	boundsLayout->addStretch();
	boundsLayout->addWidget(maxValLabel);

	layout->addWidget(dummy);
	layout->addWidget(slider);
	layout->addWidget(dummyBounds);

	// connect
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
	connect(sliderBox, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
}

// DkFileInfo --------------------------------------------------------------------
DkFileInfo::DkFileInfo() {
	fileExists = false;
	used = false;
}

DkFileInfo::DkFileInfo(const QFileInfo& fileInfo) {

	this->fileInfo = fileInfo;
	fileExists = false;
	used = false;
}

bool DkFileInfo::exists() const {
	return fileExists;
}

void DkFileInfo::setExists(bool fileExists) {
	this->fileExists = fileExists;
}

bool DkFileInfo::inUse() const {
	return used;
}

void DkFileInfo::setInUse(bool inUse) {
	used = inUse;
}

QFileInfo DkFileInfo::getFileInfo() const {
	return fileInfo;
}

// DkFileLabel --------------------------------------------------------------------
DkFolderLabel::DkFolderLabel(const DkFileInfo& fileInfo, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QLabel(parent, f) {

	if (fileInfo.getFileInfo().isDir())
		setText(fileInfo.getFileInfo().absoluteFilePath());
	else
		setText(fileInfo.getFileInfo().fileName());

	this->fileInfo = fileInfo;
	setObjectName("DkFileLabel");
}

void DkFolderLabel::mousePressEvent(QMouseEvent *ev) {

	emit loadFileSignal(fileInfo.getFileInfo());

	QLabel::mousePressEvent(ev);
}

// DkImageLabel --------------------------------------------------------------------
DkImageLabel::DkImageLabel(const QFileInfo& fileInfo, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QLabel(parent, f) {

	thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(fileInfo));
	connect(thumb.data(), SIGNAL(thumbLoadedSignal()), this, SIGNAL(labelLoaded()));
	connect(thumb.data(), SIGNAL(thumbLoadedSignal()), this, SLOT(thumbLoaded()));

	setFixedSize(DkSettings::display.thumbSize, DkSettings::display.thumbSize);
	setMouseTracking(true);

	setStatusTip(fileInfo.fileName());
	setToolTip(fileInfo.fileName());
	
	createLayout();
}

void DkImageLabel::createLayout() {

	imageLabel = new QLabel(this);
	imageLabel->setFixedSize(DkSettings::display.thumbSize, DkSettings::display.thumbSize);
	imageLabel->setScaledContents(true);
	imageLabel->setStyleSheet("QLabel{margin: 0 0 0 0; padding: 0 0 0 0; border: 1px solid " + DkUtils::colorToString(DkSettings::display.bgColorWidget) + ";}");

	QColor cA = DkSettings::display.highlightColor;
	cA.setAlpha(100);
	highLightLabel = new QLabel(this);
	highLightLabel->setFixedSize(DkSettings::display.thumbSize, DkSettings::display.thumbSize);
	highLightLabel->setStyleSheet("QLabel{background: " + DkUtils::colorToString(cA) + "; border: 1px solid " + DkUtils::colorToString(DkSettings::display.highlightColor) + ";}");
	highLightLabel->hide();

	removeFileButton = new QPushButton(QIcon(":/nomacs/img/close.png"), tr(""), this);
	removeFileButton->setFlat(true);
	removeFileButton->setStyleSheet("QPushButton{margin: 0 0 0 0; padding: 0 0 0 0; border: none;}");
	removeFileButton->move(width()-17, 0);
	removeFileButton->hide();

	connect(removeFileButton, SIGNAL(clicked()), this, SLOT(removeFileFromList()));
}

bool DkImageLabel::hasFile() const {

	return !thumb->getImage().isNull();
}

QSharedPointer<DkThumbNailT> DkImageLabel::getThumb() const {
	
	return thumb;
}

void DkImageLabel::thumbLoaded() {

	if (thumb->getImage().isNull()) {
		qDebug() << thumb->getFile().fileName() << " not loaded...";
		return;
	}
	//qDebug() << thumb->getFile().fileName() << " loaded...";
	
	QPixmap pm = QPixmap::fromImage(thumb->getImage());

	QRect r(QPoint(), pm.size());

	if (r.width() > r.height()) {
		r.setX(qFloor((r.width()-r.height())*0.5f));
		r.setWidth(r.height());
	}
	else {
		r.setY(qFloor((r.height()-r.width())*0.5f));
		r.setHeight(r.width());
	}
	pm = pm.copy(r);

	qDebug() << pm.size();

	imageLabel->setPixmap(pm);
}

void DkImageLabel::removeFileFromList() {

	imageLabel->hide();
	//highLightLabel->hide();
	removeFileButton->hide();
	highLightLabel->setStyleSheet("QLabel{background: " + DkUtils::colorToString(DkSettings::display.bgColorWidget) + "; border: 1px solid black;}");
	highLightLabel->show();

	for (int idx = 0; idx < DkSettings::global.recentFiles.size(); idx++) {

		if (thumb->getFile().absoluteFilePath() == DkSettings::global.recentFiles.at(idx))
			DkSettings::global.recentFiles.removeAt(idx);
	}
}

void DkImageLabel::enterEvent(QEvent *ev) {
	
	if (imageLabel->isVisible()) {
		highLightLabel->show();
		removeFileButton->show();
	}
	QLabel::enterEvent(ev);
}

void DkImageLabel::leaveEvent(QEvent *ev) {
	
	if (imageLabel->isVisible())
		highLightLabel->hide();
	removeFileButton->hide();
	QLabel::leaveEvent(ev);
}

void DkImageLabel::mousePressEvent(QMouseEvent *ev) {

	emit loadFileSignal(thumb->getFile());

	QLabel::mousePressEvent(ev);
}


// Recent Files Widget --------------------------------------------------------------------
DkRecentFilesWidget::DkRecentFilesWidget(QWidget* parent /* = 0 */) : DkWidget(parent) {

	setObjectName("DkRecentFilesWidget");

	rFileIdx = 0;

	createLayout();

	//connect(&fileWatcher, SIGNAL(finished()), this, SLOT(updateFiles()));
	connect(&folderWatcher, SIGNAL(finished()), this, SLOT(updateFolders()));
}

DkRecentFilesWidget::~DkRecentFilesWidget() {

	//fileWatcher.blockSignals(true);
	//fileWatcher.cancel();
	//fileWatcher.waitForFinished();

	folderWatcher.blockSignals(true);
	folderWatcher.cancel();
	folderWatcher.waitForFinished();
}

void DkRecentFilesWidget::createLayout() {

	filesWidget = new QWidget(this);
	filesLayout = new QGridLayout(filesWidget);

	folderWidget = new QWidget(this);
	folderLayout = new QVBoxLayout(folderWidget);
	
	filesTitle = new QLabel(tr("Recent Files"), this);
	filesTitle->setStyleSheet("QLabel{font-size: 20px;}");

	folderTitle = new QLabel(tr("Recent Folders"), this);
	folderTitle->setStyleSheet("QLabel{font-size: 20px;}");

	bgLabel = new QLabel(this);
	bgLabel->setObjectName("bgLabel");
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(bgLabel);
	QHBoxLayout* hLayout = new QHBoxLayout(bgLabel);

	hLayout->addWidget(filesWidget);
	hLayout->addWidget(folderWidget);
	hLayout->addStretch();

	setCustomStyle();
}

void DkRecentFilesWidget::setCustomStyle(bool imgLoadedStyle) {

	if (imgLoadedStyle) {
		setStyleSheet(QString("#bgLabel{background-color:") + DkUtils::colorToString(DkSettings::display.bgColorWidget) + ";}" +
			QString("QLabel{color: #FFFFFF; padding: 2 0 2 0; font-size: 13px;}") + 
			QString("#DkFileLabel:hover{color: " + DkUtils::colorToString(DkSettings::display.bgColorWidget)) + 
			QString("; background: qlineargradient(x1: 0.7, y1: 0, x2: 1, y2: 0, stop: 0 rgba(255,255,255,200), stop: 1 rgba(0,0,0,0));}"));

	}
	else {
		setStyleSheet(QString("#bgLabel{background-color: rgba(0,0,0,0);}" +
			QString("QLabel{padding: 2 0 2 0; font-size: 13px; color: ") + DkUtils::colorToString(DkSettings::display.bgColorWidget) + ";}" + 
			QString("#DkFileLabel:hover{color: #FFFFFF;") + 
			QString("; background: qlineargradient(x1: 0.7, y1: 0, x2: 1, y2: 0, stop: 0 ") + DkUtils::colorToString(DkSettings::display.bgColorWidget) + ", stop: 1 rgba(0,0,0,0));}"));
	}

}

void DkRecentFilesWidget::setVisible(bool visible) {
	
	if (visible) {
		updateFileList();
		qDebug() << "showing recent files...";
	}

	DkWidget::setVisible(visible);
}

void DkRecentFilesWidget::updateFileList() {

	delete folderLayout;
	delete filesLayout;
	rFileIdx = 0;
	numActiveLabels = 0;

	filesLayout = new QGridLayout(filesWidget);
	folderLayout = new QVBoxLayout(folderWidget);

	filesWidget->setFixedHeight(1080);
	folderWidget->setFixedHeight(1080);

	//filesLayout->setSpacing(10);
	folderLayout->setSpacing(0);

	for (int idx = 0; idx < fileLabels.size(); idx++) {
		delete fileLabels.at(idx);
	}

	for (int idx = 0; idx < folderLabels.size(); idx++) {
		delete folderLabels.at(idx);
	}

	filesTitle->hide();
	folderTitle->hide();

	//fileWatcher.cancel();
	//fileWatcher.waitForFinished();
	folderWatcher.cancel();
	folderWatcher.waitForFinished();

	fileLabels.clear();
	folderLabels.clear();
	recentFiles.clear();
	recentFolders.clear();

	for (int idx = 0; idx < DkSettings::global.recentFiles.size(); idx++)
		recentFiles.append(QFileInfo(DkSettings::global.recentFiles.at(idx)));
	for (int idx = 0; idx < DkSettings::global.recentFolders.size(); idx++)
		recentFolders.append(QFileInfo(DkSettings::global.recentFolders.at(idx)));

	//fileWatcher.setFuture(QtConcurrent::map(recentFiles, &nmc::DkRecentFilesWidget::mappedFileExists));
	folderWatcher.setFuture(QtConcurrent::map(recentFolders, &nmc::DkRecentFilesWidget::mappedFileExists));

	updateFiles();
}

void DkRecentFilesWidget::updateFiles() {

	int columns = 3;

	if (fileLabels.empty()) {
		filesTitle->show();
		filesLayout->setRowStretch(recentFiles.size()+2, 100);
		filesLayout->addWidget(filesTitle, 0, 0, 1, columns, Qt::AlignRight);
		//filesLayout->addItem(new QSpacerItem(30,10), 1, 0);
	}

	// show current
	if (rFileIdx < fileLabels.size() && fileLabels.at(rFileIdx)->hasFile()) {
		fileLabels.at(rFileIdx)->show();
		filesLayout->addWidget(fileLabels.at(rFileIdx), qFloor((float)numActiveLabels/columns)+2, numActiveLabels%columns);
		numActiveLabels++;
	}
	else if (rFileIdx < fileLabels.size()) {
		fileLabels.at(rFileIdx)->hide();

		// remove files which we can't load to keep the list clean...
		DkSettings::global.recentFiles.removeAll(fileLabels.at(rFileIdx)->getThumb()->getFile().absoluteFilePath());		// remove recent files which we could not load...
	}
	//else if (rFileIdx >= fileLabels.size()) {
	//	qDebug() << "index out of range";
	//}
	//else
	//	qDebug() << "could not load thumb..." << fileLabels.at(rFileIdx)->hasFile();

	if (!fileLabels.empty())
		rFileIdx++;

	// load next
	if ((rFileIdx/(float)columns*DkSettings::display.thumbSize < filesWidget->height()-200 || !(rFileIdx+1 % columns)) && rFileIdx < recentFiles.size()) {
		DkImageLabel* cLabel = new DkImageLabel(recentFiles.at(rFileIdx), this);
		cLabel->hide();
		cLabel->setStyleSheet(QString("QLabel{background-color: rgba(0,0,0,0), border: solid 1px black;}"));
		
		fileLabels.append(cLabel);
		connect(cLabel, SIGNAL(labelLoaded()), this, SLOT(updateFiles()));
		connect(cLabel, SIGNAL(loadFileSignal(QFileInfo)), this, SIGNAL(loadFileSignal(QFileInfo)));
		cLabel->getThumb()->fetchThumb(DkThumbNailT::force_exif_thumb);

	}

	update();


	//int cHeight = 0;

	//for (int idx = 0; idx < recentFiles.size(); idx++) {

	//	if (recentFiles.at(idx).inUse())
	//		continue;

	//	if (recentFiles.at(idx).exists()) {
	//		recentFiles[idx].setInUse(true);

	//		DkFolderLabel* fLabel = new DkFolderLabel(recentFiles.at(idx), this);
	//		connect(fLabel, SIGNAL(loadFileSignal(QFileInfo)), this, SIGNAL(loadFileSignal(QFileInfo)));
	//		fileLabels.append(fLabel);
	//		filesLayout->addWidget(fLabel);

	//		cHeight += fLabel->height();
	//		if (cHeight > folderWidget->height())
	//			break;

	//	}
	//}

	//filesLayout->addStretch();
}

void DkRecentFilesWidget::updateFolders() {

	folderTitle->show();
	folderLayout->addWidget(folderTitle);
	folderLayout->addSpacerItem(new QSpacerItem(10, 10));

	int cHeight = 0;

	for (int idx = 0; idx < recentFolders.size(); idx++) {

		if (recentFolders.at(idx).inUse())
			continue;

		if (recentFolders.at(idx).exists()) {
			recentFolders[idx].setInUse(true);

			DkFolderLabel* fLabel = new DkFolderLabel(recentFolders.at(idx), this);
			connect(fLabel, SIGNAL(loadFileSignal(QFileInfo)), this, SIGNAL(loadFileSignal(QFileInfo)));
			folderLayout->addWidget(fLabel);
			folderLabels.append(fLabel);

			cHeight += fLabel->height();
			if (cHeight > folderWidget->height())
				break;
		}
	}

	folderLayout->addStretch();
}

void DkRecentFilesWidget::mappedFileExists(DkFileInfo& fileInfo) {

	fileInfo.setExists(fileInfo.getFileInfo().exists());
}

}


