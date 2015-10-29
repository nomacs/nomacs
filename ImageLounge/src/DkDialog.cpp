/*******************************************************************************************************
 DkDialog.cpp
 Created on:	21.04.2011
 
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

#include "DkDialog.h"

#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkBaseViewPort.h"
#include "DkTimer.h"
#include "DkWidgets.h"
#include "DkThumbs.h"
#include "DkUtils.h"
#include "DkActionManager.h"

#if defined(WIN32) && !defined(SOCK_STREAM)
#include <winsock2.h>	// needed since libraw 0.16
#endif

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QSlider>
#include <QColorDialog>
#include <QPushButton>
#include <QBoxLayout>
#include <QCheckBox>
#include <QFileInfo>
#include <QTableView>
#include <QCompleter>
#include <QMainWindow>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QItemEditorFactory>
#include <QHeaderView>
#include <QTreeView>
#include <QMimeData>
#include <QStringListModel>
#include <QListView>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QProgressDialog>
#include <QTextEdit>
#include <QApplication>
#include <QInputDialog>
#include <QFileDialog>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QToolBar>
#include <QFormLayout>
#include <QProgressBar>
#include <QFuture>
#include <QtConcurrentRun>
#include <QMouseEvent>
#include <QAction>
#include <QMessageBox>
#include <QToolButton>
#include <QComboBox>
#include <QTimer>
#include <qmath.h>
#include <QDesktopServices>
#include <QSplashScreen>

#if QT_VERSION >= 0x050000
#include <QKeySequenceEdit>
#endif

// quazip
#ifdef WITH_QUAZIP
#include <quazip/JlCompress.h>
#endif

#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkSplashScreen --------------------------------------------------------------------
DkSplashScreen::DkSplashScreen(QWidget* /*parent*/, Qt::WindowFlags flags) : QDialog(0, flags) {

	QPixmap img(":/nomacs/img/splash-screen.png");
	setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	setMouseTracking(true);
    
#ifdef Q_WS_MAC
    setObjectName("DkSplashScreenMac");
#else
	setObjectName("DkSplashScreen");
	setAttribute(Qt::WA_TranslucentBackground);
#endif

	imgLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	imgLabel->setObjectName("DkSplashInfoLabel");
	imgLabel->setMouseTracking(true);
	imgLabel->setScaledContents(true);
	imgLabel->setPixmap(img);
	imgLabel->setFixedSize(600, 474);
	imgLabel->show();

	setFixedSize(imgLabel->size());

	//// create exit shortcuts
	//QShortcut* escExit = new QShortcut(Qt::Key_Escape, this);
	//QObject::connect(escExit, SIGNAL(activated()), this, SLOT(close()));

	//QPushButton* exitButton = new QPushButton(tr("Close"));
	//exitButton->setFlat(true);

	exitButton = new QPushButton(tr("CLOSE"), this);
	exitButton->setObjectName("cancelButtonSplash");
	exitButton->setFlat(true);
	exitButton->setIcon(QIcon(DkImage::colorizePixmap(QPixmap(":/nomacs/img/cancel2.png"), QColor(0,0,0,200), 1.0f)));
	exitButton->setToolTip(tr("Close (ESC)"));
	exitButton->setShortcut(QKeySequence(Qt::Key_Escape));
	exitButton->move(10, 435);
	exitButton->hide();
	connect(exitButton, SIGNAL(clicked()), this, SLOT(close()))	;

	// set the text
	text = 
		QString("Flo was here und w&uuml;nscht<br>" 
		"Stefan fiel Spa&szlig; w&auml;hrend<br>" 
		"Markus rockt... <br><br>" 

		"<a href=\"http://www.nomacs.org\">www.nomacs.org</a><br>"
		"<a href=\"mailto:developers@nomacs.org\">developers@nomacs.org</a><br><br>" 

		"This program is licensed under GNU General Public License v3<br>"
		"&#169; Markus Diem, Stefan Fiel and Florian Kleber, 2011-2015<br><br>"

		"Press [ESC] to exit");

	textLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	textLabel->setObjectName("DkSplashInfoLabel");
	textLabel->setMouseTracking(true);
	textLabel->setScaledContents(true);
	textLabel->setTextFormat(Qt::RichText);
	textLabel->setText(text);
	textLabel->move(131, 280);
	textLabel->setOpenExternalLinks(true);
	
	//textLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	
	QLabel* versionLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	versionLabel->setObjectName("DkSplashInfoLabel");
	versionLabel->setTextFormat(Qt::RichText);

	QString platform = "";
#ifdef _WIN64
	platform = " [x64] ";
#elif defined _WIN32
	platform = " [x86] ";
#endif

	QString qtVersion = "Qt " + QString::fromUtf8(qVersion());

	if (!DkSettings::isPortable())
		qDebug() << "nomacs is not portable: " << DkSettings::getSettingsFile().absoluteFilePath();

	versionLabel->setText("Version: " + QApplication::applicationVersion() + platform + "<br>" +
#ifdef WITH_LIBRAW
		"RAW support: Yes<br>"
#else
		"RAW support: No<br>"
#endif  		
		+ qtVersion + "<br>"
		+ (DkSettings::isPortable() ? tr("Portable") : "")
		);

	versionLabel->move(360, 280);
	versionLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

	showTimer = new QTimer(this);
	showTimer->setInterval(5000);
	showTimer->setSingleShot(true);
	connect(showTimer, SIGNAL(timeout()), exitButton, SLOT(hide()));
	
	qDebug() << "splash screen created...";
}

void DkSplashScreen::mousePressEvent(QMouseEvent* event) {

	setCursor(Qt::ClosedHandCursor);
	mouseGrab = event->globalPos();
	QDialog::mousePressEvent(event);
}

void DkSplashScreen::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		move(pos()-(mouseGrab-event->globalPos()));
		mouseGrab = event->globalPos();
		qDebug() << "moving...";
	}
	else
		setCursor(Qt::OpenHandCursor);
	showClose();
	QDialog::mouseMoveEvent(event);
}

void DkSplashScreen::mouseReleaseEvent(QMouseEvent *event) {

	setCursor(Qt::OpenHandCursor);
	showClose();
	QDialog::mouseReleaseEvent(event);
}

void DkSplashScreen::showClose() {

	exitButton->show();
	showTimer->start();
}

// file validator --------------------------------------------------------------------
DkFileValidator::DkFileValidator(const QString& lastFile, QObject * parent) : QValidator(parent) {

	mLastFile = lastFile;
}

void DkFileValidator::fixup(QString& input) const {

	if(!QFileInfo(input).exists())
		input = mLastFile;
}

QValidator::State DkFileValidator::validate(QString& input, int&) const {

	if (QFileInfo(input).exists())
		return QValidator::Acceptable;
	else
		return QValidator::Intermediate;
}

// train dialog --------------------------------------------------------------------
DkTrainDialog::DkTrainDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	setWindowTitle(tr("Add New Image Format"));
	createLayout();
	setFixedSize(340, 400);		// due to the baseViewport we need fixed sized dialogs : (
	setAcceptDrops(true);
}

void DkTrainDialog::createLayout() {

	// first row
	QLabel* newImageLabel = new QLabel(tr("Load New Image Format"), this);
	mPathEdit = new QLineEdit(this);
	mPathEdit->setValidator(&mFileValidator);
	mPathEdit->setObjectName("DkWarningEdit");
	connect(mPathEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
	connect(mPathEdit, SIGNAL(editingFinished()), this, SLOT(loadFile()));

	QPushButton* openButton = new QPushButton(tr("&Browse"), this);
	connect(openButton, SIGNAL(pressed()), this, SLOT(openFile()));

	mFeedbackLabel = new QLabel("", this);
	mFeedbackLabel->setObjectName("DkDecentInfo");

	// shows the image if it could be loaded
	mViewport = new DkBaseViewPort(this);
	mViewport->setForceFastRendering(true);
	mViewport->setPanControl(QPointF(0.0f, 0.0f));

	// mButtons
	mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	mButtons->button(QDialogButtonBox::Ok)->setText(tr("&Add"));
	mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
	mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

	QWidget* trainWidget = new QWidget(this);
	QGridLayout* gdLayout = new QGridLayout(trainWidget);
	gdLayout->addWidget(newImageLabel, 0, 0);
	gdLayout->addWidget(mPathEdit, 1, 0);
	gdLayout->addWidget(openButton, 1, 1);
	gdLayout->addWidget(mFeedbackLabel, 2, 0, 1, 2);
	gdLayout->addWidget(mViewport, 3, 0, 1, 2);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(trainWidget);
	layout->addWidget(mButtons);
}

void DkTrainDialog::textChanged(const QString& text) {
	
	if (QFileInfo(text).exists())
		mPathEdit->setProperty("warning", false);
	else
		mPathEdit->setProperty("warning", false);

	mPathEdit->style()->unpolish(mPathEdit);
	mPathEdit->style()->polish(mPathEdit);
	mPathEdit->update();
}

void DkTrainDialog::openFile() {

	// load system default open dialog
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"),
		mFile, tr("All Files (*.*)"));

	if (QFileInfo(filePath).exists()) {
		mPathEdit->setText(filePath);
		loadFile(filePath);
	}
}

void DkTrainDialog::userFeedback(const QString& msg, bool error) {

	if (!error)
		mFeedbackLabel->setProperty("warning", false);
	else
		mFeedbackLabel->setProperty("warning", true);

	mFeedbackLabel->setText(msg);
	mFeedbackLabel->style()->unpolish(mFeedbackLabel);
	mFeedbackLabel->style()->polish(mFeedbackLabel);
	mFeedbackLabel->update();
}

void DkTrainDialog::loadFile(const QString& filePath) {

	QString lFilePath = filePath;

	if (filePath.isEmpty() && !mPathEdit->text().isEmpty())
		lFilePath = mPathEdit->text();
	else if (filePath.isEmpty())
		return;

	QFileInfo fileInfo(lFilePath);
	if (!fileInfo.exists() || mAcceptedFile == lFilePath)
		return;	// error message?!

	// update validator
	mFileValidator.setLastFile(lFilePath);

	DkBasicLoader basicLoader;
	basicLoader.setTraining(true);

	// TODO: archives cannot be trained currently
	bool imgLoaded = basicLoader.loadGeneral(filePath, true);

	if (!imgLoaded) {
		mViewport->setImage(QImage());	// remove the image
		mAcceptedFile = "";
		userFeedback(tr("Sorry, currently we don't support: *.%1 files").arg(fileInfo.suffix()), true);
		return;
	}

	if (DkSettings::app.fileFilters.join(" ").contains(fileInfo.suffix(), Qt::CaseInsensitive)) {
		userFeedback(tr("*.%1 is already supported.").arg(fileInfo.suffix()), false);
		imgLoaded = false;
	}
	else
		userFeedback(tr("*.%1 is supported.").arg(fileInfo.suffix()), false);

	mViewport->setImage(basicLoader.image());
	mAcceptedFile = lFilePath;

	// try loading the file
	// if loaded !
	mButtons->button(QDialogButtonBox::Ok)->setEnabled(imgLoaded);
}

void DkTrainDialog::accept() {

	QFileInfo acceptedFileInfo(mAcceptedFile);

	// add the extension to user filters
	if (!DkSettings::app.fileFilters.join(" ").contains(acceptedFileInfo.suffix(), Qt::CaseInsensitive)) {

		QString name = QInputDialog::getText(this, "Format Name", tr("Please name the new format:"), QLineEdit::Normal, "Your File Format");
		QString tag = name + " (*." + acceptedFileInfo.suffix() + ")";

		// load user filters
		QSettings& settings = Settings::instance().getSettings();
		QStringList userFilters = settings.value("ResourceSettings/userFilters", QStringList()).toStringList();
		userFilters.append(tag);
		settings.setValue("ResourceSettings/userFilters", userFilters);
		DkSettings::app.openFilters.append(tag);
		DkSettings::app.fileFilters.append("*." + acceptedFileInfo.suffix());
		DkSettings::app.browseFilters += acceptedFileInfo.suffix();
	}

	QDialog::accept();
}

void DkTrainDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		qDebug() << "dropping: " << url;
		url = url.toLocalFile();

		mPathEdit->setText(url.toString());
		loadFile();
	}
}

void DkTrainDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists())
			event->acceptProposedAction();
	}
}

// DkAppManagerDialog --------------------------------------------------------------------
DkAppManagerDialog::DkAppManagerDialog(DkAppManager* manager /* = 0 */, QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : QDialog(parent, flags) {

	this->manager = manager;
	setWindowTitle(tr("Manage Applications"));
	createLayout();
}

void DkAppManagerDialog::createLayout() {

	QVector<QAction* > appActions = manager->getActions();

	model = new QStandardItemModel(this);
	for (int rIdx = 0; rIdx < appActions.size(); rIdx++)
		model->appendRow(getItems(appActions.at(rIdx)));

	appTableView = new QTableView(this);
	appTableView->setModel(model);
	appTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	appTableView->verticalHeader()->hide();
	appTableView->horizontalHeader()->hide();
	//appTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	appTableView->setShowGrid(false);
	appTableView->resizeColumnsToContents();
	appTableView->resizeRowsToContents();
	appTableView->setWordWrap(false);

	QPushButton* runButton = new QPushButton(tr("&Run"), this);
	runButton->setObjectName("runButton");

	QPushButton* addButton = new QPushButton(tr("&Add"), this);
	addButton->setObjectName("addButton");

	QPushButton* deleteButton = new QPushButton(tr("&Delete"), this);
	deleteButton->setObjectName("deleteButton");
	deleteButton->setShortcut(QKeySequence::Delete);

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	buttons->addButton(runButton, QDialogButtonBox::ActionRole);
	buttons->addButton(addButton, QDialogButtonBox::ActionRole);
	buttons->addButton(deleteButton, QDialogButtonBox::ActionRole);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(appTableView);
	layout->addWidget(buttons);
	QMetaObject::connectSlotsByName(this);
}

QList<QStandardItem* > DkAppManagerDialog::getItems(QAction* action) {

	QList<QStandardItem* > items;
	QStandardItem* item = new QStandardItem(action->icon(), action->text().remove("&"));
	//item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	items.append(item);
	item = new QStandardItem(action->toolTip());
	item->setFlags(Qt::ItemIsSelectable);
	items.append(item);

	return items;
}

void DkAppManagerDialog::on_addButton_clicked() {

	// load system default open dialog
	QString appFilter;
	QString defaultPath;
#ifdef WIN32
	appFilter += tr("Executable Files (*.exe);;");
	defaultPath = getenv("PROGRAMFILES");
#elif QT_VERSION < 0x050000
	defaultPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation); // retrieves startmenu on windows?!
#else
	defaultPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);	// still retrieves startmenu on windows
#endif

	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Application"),
		defaultPath, 
		appFilter);

	if (filePath.isEmpty())
		return;

	QAction* newApp = manager->createAction(filePath);

	if (newApp)
		model->appendRow(getItems(newApp));

}

void DkAppManagerDialog::on_deleteButton_clicked() {

	QModelIndexList selRows = appTableView->selectionModel()->selectedRows();

	while (!selRows.isEmpty()) {
		model->removeRows(selRows.last().row(), 1);
		selRows.removeLast();
	}
}

void DkAppManagerDialog::on_runButton_clicked() {

	accept();

	QItemSelectionModel* sel = appTableView->selectionModel();

	if (!sel->hasSelection() && !manager->getActions().isEmpty())
		emit openWithSignal(manager->getActions().first());
	
	else if (!manager->getActions().isEmpty()) {

		QModelIndexList rows = sel->selectedRows();

		for (int idx = 0; idx < rows.size(); idx++) {
			emit openWithSignal(manager->getActions().at(rows.at(idx).row()));
		}
	}

}

void DkAppManagerDialog::accept() {

	QVector<QAction* > apps;

	for (int idx = 0; idx < model->rowCount(); idx++) {

		QString filePath = model->item(idx, 1)->text();
		QString name = model->item(idx, 0)->text();
		QAction* action = manager->findAction(filePath);

		if (!action)
			action = manager->createAction(filePath);
		// obviously I cannot create this action - should we tell user?
		if (!action)	
			continue;

		if (name != action->text().remove("&"))
			action->setText(name);

		qDebug() << "pushing back: " << action->text();

		apps.append(action);
	}

	manager->setActions(apps);

	QDialog::accept();
}

// DkSearchDialaog --------------------------------------------------------------------
DkSearchDialog::DkSearchDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkSearchDialog::init() {

	setObjectName("DkSearchDialog");
	setWindowTitle(tr("Find & Filter"));

	mEndMessage = tr("Load All");

	QVBoxLayout* layout = new QVBoxLayout(this);

	QCompleter* history = new QCompleter(DkSettings::global.searchHistory, this);
	history->setCompletionMode(QCompleter::InlineCompletion);
	mSearchBar = new QLineEdit();
	mSearchBar->setObjectName("searchBar");
	mSearchBar->setToolTip(tr("Type a search word or a regular expression"));
	mSearchBar->setCompleter(history);

	mStringModel = new QStringListModel(this);

	mResultListView = new QListView(this);
	mResultListView->setObjectName("resultListView");
	mResultListView->setModel(mStringModel);
	mResultListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	mResultListView->setSelectionMode(QAbstractItemView::SingleSelection);

	//// TODO: add cursor down - cursor up action
	//QAction* focusAction = new QAction(tr("Focus Action"), searchBar);
	//focusAction->setShortcut(Qt::Key_Down);
	//connect(focusAction, SIGNAL(triggered()), resultListView, SLOT(/*createSLOT*/));

	mFilterButton = new QPushButton(tr("&Filter"), this);
	mFilterButton->setObjectName("filterButton");

	mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	mButtons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	mButtons->button(QDialogButtonBox::Ok)->setText(tr("F&ind"));
	//mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	mButtons->addButton(mFilterButton, QDialogButtonBox::ActionRole);

	connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(mSearchBar);
	layout->addWidget(mResultListView);
	layout->addWidget(mButtons);

	mSearchBar->setFocus(Qt::MouseFocusReason);

	QMetaObject::connectSlotsByName(this);
}

void DkSearchDialog::setFiles(const QStringList& fileList) {
	mFileList = fileList;
	mResultList = fileList;
	mStringModel->setStringList(makeViewable(fileList));
}

void DkSearchDialog::setPath(const QString& dirPath) {
	mPath = dirPath;
}

bool DkSearchDialog::filterPressed() const {
	return mIsFilterPressed;
}

void DkSearchDialog::on_searchBar_textChanged(const QString& text) {

	DkTimer dt;

	if (text == mCurrentSearch)
		return;
	
	mResultList = DkUtils::filterStringList(text, mFileList);
	qDebug() << "searching takes: " << dt.getTotal();
	mCurrentSearch = text;

	if (mResultList.empty()) {
		QStringList answerList;
		answerList.append(tr("No Matching Items"));
		mStringModel->setStringList(answerList);

		mResultListView->setProperty("empty", true);
		
		mFilterButton->setEnabled(false);
		mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
	else {
		mFilterButton->setEnabled(true);
		mButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
		mStringModel->setStringList(makeViewable(mResultList));
		mResultListView->selectionModel()->setCurrentIndex(mStringModel->index(0, 0), QItemSelectionModel::SelectCurrent);
		mResultListView->setProperty("empty", false);
	}

	mResultListView->style()->unpolish(mResultListView);
	mResultListView->style()->polish(mResultListView);
	mResultListView->update();

	qDebug() << "searching takes (total): " << dt.getTotal();
}

void DkSearchDialog::on_resultListView_doubleClicked(const QModelIndex& modelIndex) {

	if (modelIndex.data().toString() == mEndMessage) {
		mStringModel->setStringList(makeViewable(mResultList, true));
		return;
	}

	emit loadFileSignal(QFileInfo(mPath, modelIndex.data().toString()).absoluteFilePath());
	close();
}

void DkSearchDialog::on_resultListView_clicked(const QModelIndex& modelIndex) {

	if (modelIndex.data().toString() == mEndMessage)
		mStringModel->setStringList(makeViewable(mResultList, true));
}

void DkSearchDialog::accept() {

	on_okButton_pressed();
	QDialog::accept();
}

void DkSearchDialog::on_okButton_pressed() {

	if (mResultListView->selectionModel()->currentIndex().data().toString() == mEndMessage) {
		mStringModel->setStringList(makeViewable(mResultList, true));
		return;
	}

	updateHistory();

	// ok load the selected file
	QString fileName = mResultListView->selectionModel()->currentIndex().data().toString();
	qDebug() << "opening filename: " << fileName;

	if (!fileName.isEmpty())
		emit loadFileSignal(QFileInfo(mPath, fileName).absoluteFilePath());
}

void DkSearchDialog::on_filterButton_pressed() {
	filterSignal(mCurrentSearch.split(" "));
	mIsFilterPressed = true;
	done(filter_button);
}

void DkSearchDialog::setDefaultButton(int defaultButton /* = find_button */) {

	if (defaultButton == find_button) {
		mButtons->button(QDialogButtonBox::Ok)->setAutoDefault(true);
		mFilterButton->setAutoDefault(false);
	}
	else if (defaultButton == filter_button) {
		mButtons->button(QDialogButtonBox::Ok)->setAutoDefault(false);
		mFilterButton->setAutoDefault(true);
	}
}

void DkSearchDialog::updateHistory() {
	
	DkSettings::global.searchHistory.append(mCurrentSearch);

	// keep the history small
	if (DkSettings::global.searchHistory.size() > 50)
		DkSettings::global.searchHistory.pop_front();

	//QCompleter* history = new QCompleter(DkSettings::global.searchHistory);
	//searchBar->setCompleter(history);
}

QStringList DkSearchDialog::makeViewable(const QStringList& resultList, bool forceAll) {
	
	QStringList answerList;
	
	// if size > 1000 it gets slow -> cut at 1000 and make an entry for 'expand'
	if (!forceAll && resultList.size() > 1000) {

		for (int idx = 0; idx < 1000; idx++)
			answerList.append(resultList[idx]);
		answerList.append(mEndMessage);

		mAllDisplayed = false;
	}
	else {
		mAllDisplayed = true;
		answerList = resultList;
	}

	return answerList;
}

// DkResizeDialog --------------------------------------------------------------------
DkResizeDialog::DkResizeDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkResizeDialog::accept() {
	saveSettings();

	QDialog::accept();
}

void DkResizeDialog::saveSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());

	settings.setValue("ResampleMethod", mResampleBox->currentIndex());
	settings.setValue("Resample", mResampleCheck->isChecked());
	settings.setValue("CorrectGamma", mGammaCorrection->isChecked());

	if (mSizeBox->currentIndex() == size_percent) {
		settings.setValue("Width", mWPixelEdit->value());
		settings.setValue("Height", mHPixelEdit->value());
	}
	else {
		settings.setValue("Width", 0);
		settings.setValue("Height", 0);
	}
	settings.endGroup();
}


void DkResizeDialog::loadSettings() {

	qDebug() << "loading new settings...";

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());

	mResampleBox->setCurrentIndex(settings.value("ResampleMethod", ipl_cubic).toInt());
	mResampleCheck->setChecked(settings.value("Resample", true).toBool());
	mGammaCorrection->setChecked(settings.value("CorrectGamma", true).toBool());

	if (settings.value("Width", 0).toDouble() != 0) {

		double w = settings.value("Width", 0).toDouble();
		double h = settings.value("Height", 0).toDouble();

		qDebug() << "width loaded from settings: " << w;

		if (w != h) {
			mLockButton->setChecked(false);
			mLockButtonDim->setChecked(false);
		}
		mSizeBox->setCurrentIndex(size_percent);

		mWPixelEdit->setValue(w);
		mHPixelEdit->setValue(h);
		
		updateWidth();
		updateHeight();
	}
	settings.endGroup();

}

void DkResizeDialog::init() {

	setObjectName("DkResizeDialog");

	mUnitFactor.resize(unit_end);
	mUnitFactor.insert(unit_cm, 1.0f);
	mUnitFactor.insert(unit_mm, 10.0f);
	mUnitFactor.insert(unit_inch, 1.0f/2.54f);

	mResFactor.resize(res_end);
	mResFactor.insert(res_ppi, 2.54f);
	mResFactor.insert(res_ppc, 1.0f);

	setWindowTitle(tr("Resize Image"));
	//setFixedSize(600, 512);
	createLayout();
	initBoxes();

	mWPixelEdit->setFocus(Qt::ActiveWindowFocusReason);

	QMetaObject::connectSlotsByName(this);
}

void DkResizeDialog::createLayout() {

	// preview
	int minPx = 1;
	int maxPx = 100000;
	double minWidth = 1;
	double maxWidth = 500000;
	int decimals = 2;

	QLabel* origLabelText = new QLabel(tr("Original"));
	origLabelText->setAlignment(Qt::AlignHCenter);
	QLabel* newLabel = new QLabel(tr("New"));
	newLabel->setAlignment(Qt::AlignHCenter);

	// shows the original image
	mOrigView = new DkBaseViewPort(this);
	mOrigView->setForceFastRendering(true);
	mOrigView->setPanControl(QPointF(0.0f, 0.0f));
	connect(mOrigView, SIGNAL(imageUpdated()), this, SLOT(drawPreview()));

	//// maybe we should report this: 
	//// if a stylesheet (with border) is set, the var
	//// cornerPaintingRect in QAbstractScrollArea (which we don't even need : )
	//// is invalid which blocks re-paints unless the widget gets a focus...
	//origView->setStyleSheet("QViewPort{border: 1px solid #888;}");

	// shows the preview
	mPreviewLabel = new QLabel(this);
	mPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	mPreviewLabel->setMinimumHeight(100);

	// all text dialogs...
	QDoubleValidator* doubleValidator = new QDoubleValidator(1, 1000000, 2, 0);
	doubleValidator->setRange(0, 100, 2);

	QWidget* resizeBoxes = new QWidget(this);
	resizeBoxes->setGeometry(QRect(QPoint(40, 300), QSize(400, 170)));

	QGridLayout* gridLayout = new QGridLayout(resizeBoxes);

	QLabel* wPixelLabel = new QLabel(tr("Width: "));
	wPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mWPixelEdit = new DkSelectAllDoubleSpinBox();
	mWPixelEdit->setObjectName("wPixelEdit");
	mWPixelEdit->setRange(minPx, maxPx);
	mWPixelEdit->setDecimals(0);

	mLockButton = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	mLockButton->setFixedSize(QSize(16,16));
	mLockButton->setObjectName("lockButton");
	mLockButton->setCheckable(true);
	mLockButton->setChecked(true);

	QLabel* hPixelLabel = new QLabel(tr("Height: "));
	hPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mHPixelEdit = new DkSelectAllDoubleSpinBox();
	mHPixelEdit->setObjectName("hPixelEdit");
	mHPixelEdit->setRange(minPx, maxPx);
	mHPixelEdit->setDecimals(0);

	mSizeBox = new QComboBox();
	QStringList sizeList;
	sizeList.insert(size_pixel, "pixel");
	sizeList.insert(size_percent, "%");
	mSizeBox->addItems(sizeList);
	mSizeBox->setObjectName("sizeBox");

	// first row
	int rIdx = 0;
	gridLayout->addWidget(wPixelLabel, 0, rIdx);
	gridLayout->addWidget(mWPixelEdit, 0, ++rIdx);
	gridLayout->addWidget(mLockButton, 0, ++rIdx);
	gridLayout->addWidget(hPixelLabel, 0, ++rIdx);
	gridLayout->addWidget(mHPixelEdit, 0, ++rIdx);
	gridLayout->addWidget(mSizeBox, 0, ++rIdx);

	// Document dimensions
	QLabel* widthLabel = new QLabel(tr("Width: "));
	widthLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mWidthEdit = new DkSelectAllDoubleSpinBox();
	mWidthEdit->setObjectName("widthEdit");
	mWidthEdit->setRange(minWidth, maxWidth);
	mWidthEdit->setDecimals(decimals);


	mLockButtonDim = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	//lockButtonDim->setIcon(QIcon(":/nomacs/img/lock.png"));
	mLockButtonDim->setFixedSize(QSize(16,16));
	mLockButtonDim->setObjectName("lockButtonDim");
	mLockButtonDim->setCheckable(true);
	mLockButtonDim->setChecked(true);

	QLabel* heightLabel = new QLabel(tr("Height: "));
	heightLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mHeightEdit = new DkSelectAllDoubleSpinBox();
	mHeightEdit->setObjectName("heightEdit");
	mHeightEdit->setRange(minWidth, maxWidth);
	mHeightEdit->setDecimals(decimals);

	mUnitBox = new QComboBox();
	QStringList unitList;
	unitList.insert(unit_cm, "cm");
	unitList.insert(unit_mm, "mm");
	unitList.insert(unit_inch, "inch");
	//unitList.insert(unit_percent, "%");
	mUnitBox->addItems(unitList);
	mUnitBox->setObjectName("unitBox");

	// second row
	rIdx = 0;
	gridLayout->addWidget(widthLabel, 1, rIdx);
	gridLayout->addWidget(mWidthEdit, 1, ++rIdx);
	gridLayout->addWidget(mLockButtonDim, 1, ++rIdx);
	gridLayout->addWidget(heightLabel, 1, ++rIdx);
	gridLayout->addWidget(mHeightEdit, 1, ++rIdx);
	gridLayout->addWidget(mUnitBox, 1, ++rIdx);

	// resolution
	QLabel* resolutionLabel = new QLabel(tr("Resolution: "));
	resolutionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mResolutionEdit = new DkSelectAllDoubleSpinBox();
	mResolutionEdit->setObjectName("resolutionEdit");
	mResolutionEdit->setRange(minWidth, maxWidth);
	mResolutionEdit->setDecimals(decimals);

	mResUnitBox = new QComboBox();
	QStringList resUnitList;
	resUnitList.insert(res_ppi, tr("pixel/inch"));
	resUnitList.insert(res_ppc, tr("pixel/cm"));
	mResUnitBox->addItems(resUnitList);
	mResUnitBox->setObjectName("resUnitBox");

	// third row
	rIdx = 0;
	gridLayout->addWidget(resolutionLabel, 2, rIdx);
	gridLayout->addWidget(mResolutionEdit, 2, ++rIdx);
	gridLayout->addWidget(mResUnitBox, 2, ++rIdx, 1, 2);

	// resample
	mResampleCheck = new QCheckBox(tr("Resample Image:"));
	mResampleCheck->setChecked(true);
	mResampleCheck->setObjectName("resampleCheck");

	// TODO: disable items if no opencv is available
	mResampleBox = new QComboBox();
	QStringList resampleList;
	resampleList.insert(ipl_nearest, tr("Nearest Neighbor"));
	resampleList.insert(ipl_area, tr("Area (best for downscaling)"));
	resampleList.insert(ipl_linear, tr("Linear"));
	resampleList.insert(ipl_cubic, tr("Bicubic (4x4 pixel interpolation)"));
	resampleList.insert(ipl_lanczos, tr("Lanczos (8x8 pixel interpolation)"));
	mResampleBox->addItems(resampleList);
	mResampleBox->setObjectName("resampleBox");
	mResampleBox->setCurrentIndex(ipl_cubic);

	// last two rows
	gridLayout->addWidget(mResampleCheck, 3, 1, 1, 3);
	gridLayout->addWidget(mResampleBox, 4, 1, 1, 3);

	mGammaCorrection = new QCheckBox(tr("Gamma Correction"));
	mGammaCorrection->setObjectName("gammaCorrection");
	mGammaCorrection->setChecked(false);	// default: false since gamma might destroy soft gradients

	gridLayout->addWidget(mGammaCorrection, 5, 1, 1, 3);

	// add stretch
	gridLayout->setColumnStretch(6, 1);

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QGridLayout* layout = new QGridLayout(this);
	layout->setColumnStretch(0,1);
	layout->setColumnStretch(1,1);

	layout->addWidget(origLabelText, 0, 0);
	layout->addWidget(newLabel, 0, 1);
	layout->addWidget(mOrigView, 1, 0);
	layout->addWidget(mPreviewLabel, 1, 1);
	layout->addWidget(resizeBoxes, 2, 0, 1, 2, Qt::AlignLeft);
	//layout->addStretch();
	layout->addWidget(buttons, 3, 0, 1, 2, Qt::AlignRight);
	
	adjustSize();
	//show();
}

void DkResizeDialog::initBoxes(bool updateSettings) {

	if (mImg.isNull())
		return;

	if (mSizeBox->currentIndex() == size_pixel) {
		mWPixelEdit->setValue(mImg.width());
		mHPixelEdit->setValue(mImg.height());
	}
	else {
		mWPixelEdit->setValue(100);
		mHPixelEdit->setValue(100);
	}

	float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
	float width = (float)mImg.width()/mExifDpi * units;
	mWidthEdit->setValue(width);

	float height = (float)mImg.height()/mExifDpi * units;
	mHeightEdit->setValue(height);

	if (updateSettings)
		loadSettings();
}

void DkResizeDialog::setImage(const QImage& img) {
	
	mImg = img;
	initBoxes(true);
	updateSnippets();
	drawPreview();
	mWPixelEdit->selectAll();
}

QImage DkResizeDialog::getResizedImage() {

	return resizeImg(mImg, false);
}

void DkResizeDialog::setExifDpi(float exifDpi) {

	mExifDpi = exifDpi;
	mResolutionEdit->blockSignals(true);
	mResolutionEdit->setValue(exifDpi);
	mResolutionEdit->blockSignals(false);
}

float DkResizeDialog::getExifDpi() {
	return mExifDpi;
}

bool DkResizeDialog::resample() {
	return mResampleCheck->isChecked();
}

void DkResizeDialog::updateWidth() {

	float pWidth = (float)mWPixelEdit->text().toDouble();

	if (mSizeBox->currentIndex() == size_percent)
		pWidth = (float)qRound(pWidth/100 * mImg.width()); 

	float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
	float width = pWidth/mExifDpi * units;
	mWidthEdit->setValue(width);
}

void DkResizeDialog::updateHeight() {

	float pHeight = (float)mHPixelEdit->text().toDouble();

	if (mSizeBox->currentIndex() == size_percent)
		pHeight = (float)qRound(pHeight/100 * mImg.height()); 

	float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
	float height = pHeight/mExifDpi * units;
	mHeightEdit->setValue(height);
}

void DkResizeDialog::updateResolution() {

	qDebug() << "updating resolution...";
	float wPixel = (float)mWPixelEdit->text().toDouble();
	float width = (float)mWidthEdit->text().toDouble();

	float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
	float resolution = wPixel/width * units;
	mResolutionEdit->setValue(resolution);
}

void DkResizeDialog::updatePixelHeight() {

	float height = (float)mHeightEdit->text().toDouble();

	// *1000/10 is for more beautiful values
	float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
	float pixelHeight = (mSizeBox->currentIndex() != size_percent) ? qRound(height*mExifDpi / units) : qRound(1000.0f*height*mExifDpi / (units * mImg.height()))/10.0f;

	mHPixelEdit->setValue(pixelHeight);
}

void DkResizeDialog::updatePixelWidth() {

	float width = (float)mWidthEdit->text().toDouble();

	float units = mResFactor.at(mResUnitBox->currentIndex()) * mUnitFactor.at(mUnitBox->currentIndex());
	float pixelWidth = (mSizeBox->currentIndex() != size_percent) ? qRound(width*mExifDpi / units) : qRound(1000.0f*width*mExifDpi / (units * mImg.width()))/10.0f;
	mWPixelEdit->setValue(pixelWidth);
}

void DkResizeDialog::on_lockButtonDim_clicked() {

	mLockButton->setChecked(mLockButtonDim->isChecked());
	if (!mLockButtonDim->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_lockButton_clicked() {

	mLockButtonDim->setChecked(mLockButton->isChecked());

	if (!mLockButton->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_wPixelEdit_valueChanged(const QString& text) {

	if (!mWPixelEdit->hasFocus())
		return;

	updateWidth();

	if (!mLockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newHeight = (mSizeBox->currentIndex() != size_percent) ? qRound((float)text.toInt()/(float)mImg.width() * mImg.height()) : qRound(text.toFloat());
	mHPixelEdit->setValue(newHeight);
	updateHeight();
	drawPreview();
}

void DkResizeDialog::on_hPixelEdit_valueChanged(const QString& text) {

	if(!mHPixelEdit->hasFocus())
		return;
	
	updateHeight();

	if (!mLockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newWidth = (mSizeBox->currentIndex() != size_percent) ? qRound((float)text.toInt()/(float)mImg.height() * (float)mImg.width()) : qRound(text.toFloat());
	mWPixelEdit->setValue(newWidth);
	updateWidth();
	drawPreview();
}

void DkResizeDialog::on_widthEdit_valueChanged(const QString& text) {

	if (!mWidthEdit->hasFocus())
		return;

	if (mResampleCheck->isChecked()) 
		updatePixelWidth();

	if (!mLockButton->isChecked()) {
		drawPreview();
		return;
	}

	mHeightEdit->setValue(text.toFloat()/(float)mImg.width() * (float)mImg.height());

	if (mResampleCheck->isChecked()) 
		updatePixelHeight();

	if (!mResampleCheck->isChecked())
		updateResolution();

	drawPreview();
}

void DkResizeDialog::on_heightEdit_valueChanged(const QString& text) {

	if (!mHeightEdit->hasFocus())
		return;

	if (mResampleCheck->isChecked()) 
		updatePixelHeight();

	if (!mLockButton->isChecked()) {
		drawPreview();
		return;
	}

	mWidthEdit->setValue(text.toFloat()/(float)mImg.height() * (float)mImg.width());

	if (mResampleCheck->isChecked()) 
		updatePixelWidth();

	if (!mResampleCheck->isChecked())
		updateResolution();
	drawPreview();
}

void DkResizeDialog::on_resolutionEdit_valueChanged(const QString& text) {

	mExifDpi = (float)text.toDouble();

	if (!mResolutionEdit->hasFocus())
		return;

	updatePixelWidth();
	updatePixelHeight();

	if (mResampleCheck->isChecked()) {
		drawPreview();
		return;
	}

	initBoxes();
}


void DkResizeDialog::on_unitBox_currentIndexChanged(int) {

	updateHeight();
	updateWidth();
	//initBoxes();
}

void DkResizeDialog::on_sizeBox_currentIndexChanged(int idx) {

	if (idx == size_pixel) {
		mWPixelEdit->setDecimals(0);
		mHPixelEdit->setDecimals(0);
	}
	else {
		mWPixelEdit->setDecimals(2);
		mHPixelEdit->setDecimals(2);
	}

	updatePixelHeight();
	updatePixelWidth();
}

void DkResizeDialog::on_resUnitBox_currentIndexChanged(int) {

	updateResolution();
	//initBoxes();
}

void DkResizeDialog::on_resampleCheck_clicked() {

	mResampleBox->setEnabled(mResampleCheck->isChecked());
	mWPixelEdit->setEnabled(mResampleCheck->isChecked());
	mHPixelEdit->setEnabled(mResampleCheck->isChecked());

	if (!mResampleCheck->isChecked()) {
		mLockButton->setChecked(true);
		mLockButtonDim->setChecked(true);
		initBoxes();
	}
	else
		drawPreview();
}

void DkResizeDialog::on_gammaCorrection_clicked() {

	drawPreview();	// diem: just update
}

void DkResizeDialog::on_resampleBox_currentIndexChanged(int) {
	drawPreview();
}

void DkResizeDialog::updateSnippets() {

	if (mImg.isNull() /*|| !isVisible()*/)
		return;

	//// fix layout issues - sorry
	//origView->setFixedWidth(width()*0.5f-30);
	//previewLabel->setFixedWidth(origView->width()-2);
	//origView->setFixedHeight(width()*0.5f-30);
	//previewLabel->setFixedHeight(width()*0.5f-30);


	mOrigView->setImage(mImg);
	mOrigView->fullView();
	mOrigView->zoomConstraints(mOrigView->get100Factor());

	qDebug() << "zoom constraint: " << mOrigView->get100Factor();

	//QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	//s *= 0.5;
	//origImg = QImage(s, QImage::Format_ARGB32);
	//origImg.fill(Qt::transparent);
	//QRect imgRect = QRect(QPoint(img.width()*0.5-origImg.width()*0.5, img.height()*0.5-origImg.height()*0.5), origImg.size());

	//QPainter painter(&origImg);
	//painter.setBackgroundMode(Qt::TransparentMode);
	//painter.drawImage(QRect(QPoint(), origImg.size()), img, imgRect;)
	//painter.end();

}

void DkResizeDialog::drawPreview() {

	if (mImg.isNull() || !isVisible()) 
		return;

	QImage newImg = mOrigView->getCurrentImageRegion();

	// TODO: thread here!
	QImage img = resizeImg(newImg);

	// TODO: is there a better way of mapping the pixels? (ipl here introduces artifacts that are not in the final image)
	img = img.scaled(mPreviewLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	mPreviewLabel->setPixmap(QPixmap::fromImage(img));
}

QImage DkResizeDialog::resizeImg(QImage img, bool silent) {

	if (img.isNull())
		return img;

	QSize newSize;

	if (mSizeBox->currentIndex() == size_percent)
		newSize = QSize(qRound(mWPixelEdit->text().toFloat()/100.0f * mImg.width()), qRound(mHPixelEdit->text().toFloat()/100.0f * mImg.height()));
	else
		newSize = QSize(mWPixelEdit->text().toInt(), mHPixelEdit->text().toInt());

	QSize imgSize = mImg.size();

	qDebug() << "new size: " << newSize;

	// nothing to do
	if (mImg.size() == newSize)
		return img;

	if (mImg.size() != img.size()) {
		// compute relative size
		float relWidth = (float)newSize.width()/(float)imgSize.width();
		float relHeight = (float)newSize.height()/(float)imgSize.height();

		qDebug() << "relative size: " << newSize;

		newSize = QSize(qRound(img.width()*relWidth), qRound(img.height()*relHeight));
	}

	if (newSize.width() < mWPixelEdit->minimum() || newSize.width() > mWPixelEdit->maximum() || 
		newSize.height() < mHPixelEdit->minimum() || newSize.height() > mHPixelEdit->maximum()) {

		if (!silent) {
			QMessageBox errorDialog(this);
			errorDialog.setIcon(QMessageBox::Critical);
			errorDialog.setText(tr("Sorry, but the image size %1 x %2 is illegal.").arg(newSize.width()).arg(newSize.height()));
			errorDialog.show();
			errorDialog.exec();
		}
	}

	QImage rImg = DkImage::resizeImage(img, newSize, 1.0f, mResampleBox->currentIndex(), mGammaCorrection->isChecked());

	if (rImg.isNull() && !silent) {
		qDebug() << "image size: " << newSize;
		QMessageBox errorDialog(this);
		errorDialog.setIcon(QMessageBox::Critical);
		errorDialog.setText(tr("Sorry, the image is too large: %1").arg(DkImage::getBufferSize(newSize, 32)));
		errorDialog.show();
		errorDialog.exec();
	}

	return rImg;
}

// DkShortcutDelegate --------------------------------------------------------------------
DkShortcutDelegate::DkShortcutDelegate(QObject* parent) : QItemDelegate(parent) {
	item = 0;
}

QWidget* DkShortcutDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	
	QWidget* w = QItemDelegate::createEditor(parent, option, index);

	if (!w)
		return w;

#if QT_VERSION < 0x050000
	connect(w, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
	connect(w, SIGNAL(editingFinished()), this, SLOT(textChanged()));
#else
	connect(w, SIGNAL(keySequenceChanged(const QKeySequence&)), this, SLOT(keySequenceChanged(const QKeySequence&)));
#endif	
	
	return w;
}

bool DkShortcutDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) {

	item = index.internalPointer();

	return QItemDelegate::editorEvent(event, model, option, index);
}

void DkShortcutDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {

	const_cast<DkShortcutDelegate*>(this)->item = index.internalPointer();
	emit clearDuplicateSignal();

	QItemDelegate::setEditorData(editor, index);
}

#if QT_VERSION < 0x050000
void DkShortcutDelegate::textChanged(const QString& text) {
	emit checkDuplicateSignal(text, item);
}

void DkShortcutDelegate::keySequenceChanged(const QKeySequence&) {}
#else

void DkShortcutDelegate::textChanged(const QString&) {}	// dummy since the moccer is to dumb to get #if defs

void DkShortcutDelegate::keySequenceChanged(const QKeySequence& keySequence) {
	emit checkDuplicateSignal(keySequence, item);
}
#endif

// fun fact: there are ~10^4500 (binary) images of size 128x128 

// DkShortcutEditor --------------------------------------------------------------------
DkShortcutEditor::DkShortcutEditor(QWidget *widget) : QLineEdit(widget) {

	installEventFilter(this);
}

QKeySequence DkShortcutEditor::shortcut() const {
	return QKeySequence(text());
}

void DkShortcutEditor::setShortcut(const QKeySequence shortcut) {
	ks = shortcut;
}

bool DkShortcutEditor::eventFilter(QObject *obj, QEvent *event) {

	// TODO: we somehow need to filter events such as ALT+F4 too
	if (event->type() == QEvent::KeyRelease) {
		
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		
		if (keyEvent->key() == Qt::Key_Control ||
			keyEvent->key() == Qt::Key_Shift ||
			keyEvent->key() == Qt::Key_Alt ||
			keyEvent->key() == Qt::Key_Meta)
			return true;

		int ksi = keyEvent->key();

		if (keyEvent->modifiers() & Qt::ShiftModifier)
			ksi += Qt::SHIFT;
		if (keyEvent->modifiers() & Qt::AltModifier)
			ksi += Qt::ALT;
		if (keyEvent->modifiers() & Qt::ControlModifier)
			ksi += Qt::CTRL;
		if (keyEvent->modifiers() & Qt::MetaModifier)
			ksi += Qt::META;

		QKeySequence lks(ksi);
		setText(lks.toString());

		qDebug() << "eating the event...";

		return true;
	}
	else if (event->type() == QEvent::KeyPress) {	// filter keypresses to avoid any dialog action on shortcut edit
		return true;
	}
	//else if (event->type() == QEvent::ShortcutOverride)
	//	return true;

	return QLineEdit::eventFilter(obj, event);
}


//void DkShortcutEditor::keyPressEvent(QKeyEvent *event) {
//
//}
//
//void DkShortcutEditor::keyReleaseEvent(QKeyEvent* event) {
//	
//	if (event->key() == Qt::Key_Control ||
//		event->key() == Qt::Key_Shift ||
//		event->key() == Qt::Key_Alt ||
//		event->key() == Qt::Key_Meta)
//		return;
//
//	int ksi = event->key();
//
//	if (event->modifiers() & Qt::ShiftModifier)
//		ksi += Qt::SHIFT;
//	if (event->modifiers() & Qt::AltModifier)
//		ksi += Qt::ALT;
//	if (event->modifiers() & Qt::ControlModifier)
//		ksi += Qt::CTRL;
//	if (event->modifiers() & Qt::MetaModifier)
//		ksi += Qt::META;
//
//	QKeySequence ks(ksi);
//	setText(ks.toString());
//}

// DkShortcutsModel --------------------------------------------------------------------
DkShortcutsModel::DkShortcutsModel(QObject* parent) : QAbstractItemModel(parent) {

	// create root
	QVector<QVariant> rootData;
	rootData << tr("Name") << tr("Shortcut");

	mRootItem = new TreeItem(rootData);

}

DkShortcutsModel::~DkShortcutsModel() {
	delete mRootItem;
}

QModelIndex DkShortcutsModel::index(int row, int column, const QModelIndex &parent) const {
	
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = mRootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);

	//qDebug() << " creating index for: " << childItem->data(0) << " row: " << row;
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex DkShortcutsModel::parent(const QModelIndex &index) const {
	
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (parentItem == mRootItem)
		return QModelIndex();

	//qDebug() << "creating index for: " << childItem->data(0);

	return createIndex(parentItem->row(), 0, parentItem);
}

int DkShortcutsModel::rowCount(const QModelIndex& parent) const {

	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = mRootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int DkShortcutsModel::columnCount(const QModelIndex& parent) const {

	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return mRootItem->columnCount();
	//return 2;
}

QVariant DkShortcutsModel::data(const QModelIndex& index, int role) const {

	if (!index.isValid()) {
		qDebug() << "invalid row: " << index.row();
		return QVariant();
	}

	//if (index.row() > rowCount())
	//	return QVariant();

	//if (index.column() > columnCount())
	//	return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole) {

		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		//qDebug() << "returning: " << item->data(0) << "row: " << index.row();

		return item->data(index.column());
	}

	return QVariant();
}


QVariant DkShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const {

	if (orientation != Qt::Horizontal || role != Qt::DisplayRole) 
		return QVariant();

	return mRootItem->data(section);
} 

bool DkShortcutsModel::setData(const QModelIndex& index, const QVariant& value, int role) {

	if (!index.isValid() || role != Qt::EditRole)
		return false;

	if (index.column() == 1) {

		QKeySequence ks = value.value<QKeySequence>();
		if (index.column() == 1) {
			TreeItem* duplicate = mRootItem->find(ks, index.column());
			if (duplicate) duplicate->setData(QKeySequence(), index.column());
			if (!duplicate) qDebug() << ks << " no duplicate found...";
		}
		
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		item->setData(ks, index.column());

	}
	else {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		item->setData(value, index.column());
	}

	//emit duplicateSignal("");		// TODO: we also have to clear if the user hits ESC
	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags DkShortcutsModel::flags(const QModelIndex& index) const {

	if (!index.isValid())
		return Qt::ItemIsEditable;

	//// no editing on root items
	//if (item->parent() == rootItem)
	//	return QAbstractTableModel::flags(index);

	Qt::ItemFlags flags;

	if (index.column() == 0)
		flags = QAbstractItemModel::flags(index);
	if (index.column() == 1)
		flags = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

	return flags;
}

void DkShortcutsModel::addDataActions(QVector<QAction*> actions, const QString& name) {

	// create root
	QVector<QVariant> menuData;
	menuData << name;

	TreeItem* menuItem = new TreeItem(menuData, mRootItem);

	for (int idx = 0; idx < actions.size(); idx++) {

		// skip NULL actions - this should never happen!
		if (actions[idx]->text().isNull()) {
			qDebug() << "NULL Action detected when creating shortcuts...";
			continue;
		}

		QString text = actions[idx]->text();
		text.remove("&");

		QVector<QVariant> actionData;
		actionData << text << actions[idx]->shortcut();

		TreeItem* dataItem = new TreeItem(actionData, menuItem);
		menuItem->appendChild(dataItem);
	}

	mRootItem->appendChild(menuItem);
	mActions.append(actions);
	//qDebug() << "menu item has: " << menuItem->childCount();

}

void DkShortcutsModel::checkDuplicate(const QString& text, void* item) {

	if (text.isEmpty()) {
		emit duplicateSignal("");
		return;
	}

	QKeySequence ks(text);
	checkDuplicate(ks, item);
}

void DkShortcutsModel::checkDuplicate(const QKeySequence& ks, void* item) {

	if (ks.isEmpty()) {
		emit duplicateSignal("");
		return;
	}

	TreeItem* duplicate = mRootItem->find(ks, 1);

	if (duplicate == item)
		return;

	if (duplicate && duplicate->parent()) {
		emit duplicateSignal(tr("%1 already used by %2 > %3\nPress ESC to undo changes")
				.arg(duplicate->data(1).toString())
				.arg(duplicate->parent()->data(0).toString())
				.arg(duplicate->data(0).toString()));
	}
	else if (duplicate) {
		emit duplicateSignal(tr("%1 already used by %2\nPress ESC to undo changes")
			.arg(duplicate->data(1).toString())
			.arg(duplicate->data(0).toString()));
	}
	else 
		emit duplicateSignal("");
}

void DkShortcutsModel::clearDuplicateInfo() const {

	qDebug() << "duplicates cleared...";
	emit duplicateSignal("");
}

void DkShortcutsModel::resetActions() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("CustomShortcuts");

	for (int pIdx = 0; pIdx < mActions.size(); pIdx++) {
		
		QVector<QAction*> cActions = mActions.at(pIdx);
		
		for (int idx = 0; idx < cActions.size(); idx++) {
			QString val = settings.value(cActions[idx]->text(), "no-shortcut").toString();

			if (val != "no-shortcut") {
				cActions[idx]->setShortcut(QKeySequence());
			}
		}
	}

	settings.endGroup();
}

void DkShortcutsModel::saveActions() const {

	if (!mRootItem)
		return;

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("CustomShortcuts");

	// loop all menu entries
	for (int idx = 0; idx < mRootItem->childCount(); idx++) {

		TreeItem* cMenu = mRootItem->child(idx);
		QVector<QAction*> cActions = mActions.at(idx);

		// loop all action entries
		for (int mIdx = 0; mIdx < cMenu->childCount(); mIdx++) {

			TreeItem* cItem = cMenu->child(mIdx);
			QKeySequence ks = cItem->data(1).value<QKeySequence>();

			// if empty try to restore
			if (ks.isEmpty() && !mRootItem->find(cActions.at(mIdx)->shortcut(), 1))
				continue;

			if (cActions.at(mIdx)->shortcut() != ks) {

				if (cActions.at(mIdx)->text().isEmpty()) {
					qDebug() << "empty action detected! shortcut is: " << ks;
					continue;
				}

				cActions.at(mIdx)->setShortcut(ks);		// assign new shortcut
				settings.setValue(cActions.at(mIdx)->text(), ks.toString());	// note this works as long as you don't change the language!
			}
		}
	}
	settings.endGroup();

}

// DkShortcutsDialog --------------------------------------------------------------------
DkShortcutsDialog::DkShortcutsDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	createLayout();
}

void DkShortcutsDialog::createLayout() {

	setWindowTitle(tr("Keyboard Shortcuts"));

	QVBoxLayout* layout = new QVBoxLayout(this);

	// register our special shortcut editor
	QItemEditorFactory *factory = new QItemEditorFactory;

#if QT_VERSION < 0x050000
	QItemEditorCreatorBase *shortcutListCreator =
		new QStandardItemEditorCreator<DkShortcutEditor>();
#else
	QItemEditorCreatorBase *shortcutListCreator =
		new QStandardItemEditorCreator<QKeySequenceEdit>();
#endif

	factory->registerEditor(QVariant::KeySequence, shortcutListCreator);

	QItemEditorFactory::setDefaultFactory(factory);

	// create our beautiful shortcut view
	mModel = new DkShortcutsModel(this);
	
	DkShortcutDelegate* scDelegate = new DkShortcutDelegate(this);

	QTreeView* treeView = new QTreeView(this);
	treeView->setModel(mModel);
	treeView->setItemDelegate(scDelegate);
	treeView->setAlternatingRowColors(true);
	treeView->setIndentation(8);

	mNotificationLabel = new QLabel(this);
	mNotificationLabel->setObjectName("DkDecentInfo");
	mNotificationLabel->setProperty("warning", true);
	//notificationLabel->setTextFormat(Qt::)

	mDefaultButton = new QPushButton(tr("Set to &Default"), this);
	mDefaultButton->setToolTip(tr("Removes All Custom Shortcuts"));
	connect(mDefaultButton, SIGNAL(clicked()), this, SLOT(defaultButtonClicked()));
	connect(mModel, SIGNAL(duplicateSignal(const QString&)), mNotificationLabel, SLOT(setText(const QString&)));

#if QT_VERSION < 0x050000
	connect(scDelegate, SIGNAL(checkDuplicateSignal(const QString&, void*)), model, SLOT(checkDuplicate(const QString&, void*)));
#else
	connect(scDelegate, SIGNAL(checkDuplicateSignal(const QKeySequence&, void*)), mModel, SLOT(checkDuplicate(const QKeySequence&, void*)));
	connect(scDelegate, SIGNAL(clearDuplicateSignal()), mModel, SLOT(clearDuplicateInfo()));
#endif

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	buttons->addButton(mDefaultButton, QDialogButtonBox::ActionRole);
	
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(treeView);
	layout->addWidget(mNotificationLabel);
	//layout->addSpacing()
	layout->addWidget(buttons);
}

void DkShortcutsDialog::addActions(const QVector<QAction*>& actions, const QString& name) {

	QString cleanName = name;
	cleanName.remove("&");
	mModel->addDataActions(actions, cleanName);

}

void DkShortcutsDialog::contextMenu(const QPoint&) {

}

void DkShortcutsDialog::defaultButtonClicked() {

	if (mModel) mModel->resetActions();

	QSettings& settings = Settings::instance().getSettings();
	settings.remove("CustomShortcuts");

	QDialog::reject();
}

void DkShortcutsDialog::accept() {

	// assign shortcuts & save them if they are changed
	if (mModel) mModel->saveActions();

	QDialog::accept();
}

// DkTextDialog --------------------------------------------------------------------
DkTextDialog::DkTextDialog(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : QDialog(parent, flags) {

	setWindowTitle(tr("Text Editor"));
	createLayout();
}

void DkTextDialog::createLayout() {

	textEdit = new QTextEdit(this);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&Save"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Close"));

	connect(buttons, SIGNAL(accepted()), this, SLOT(save()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	// dialog layout
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(textEdit);
	layout->addWidget(buttons);
}

void DkTextDialog::setText(const QStringList& text) {
	textEdit->setText(text.join("\n"));
}

void DkTextDialog::save() {

	QStringList folders = DkSettings::global.recentFolders;
	QString savePath = QDir::rootPath();

	if (folders.size() > 0)
		savePath = folders.first();

	QStringList extList;
	extList << tr("Text File (*.txt)") << tr("All Files (*.*)");
	QString saveFilters(extList.join(";;"));

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Text File"),
		savePath, saveFilters);

	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream << textEdit->toPlainText();
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("Could not save: %1\n%2").arg(fileName).arg(file.errorString()));
		return;
	}

	file.close();
	accept();
}

// DkUpdateDialog --------------------------------------------------------------------
DkUpdateDialog::DkUpdateDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	init();
}

void DkUpdateDialog::init() {
	createLayout();

	connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
}

void DkUpdateDialog::createLayout() {
	setFixedWidth(300);
	setFixedHeight(150);
	setWindowTitle(tr("nomacs updater"));

	QGridLayout* gridlayout = new QGridLayout;
	upperLabel = new QLabel;
	upperLabel->setOpenExternalLinks(true);

	QWidget* lowerWidget = new QWidget;
	QHBoxLayout* hbox = new QHBoxLayout;
	okButton = new QPushButton(tr("Install Now"));
	cancelButton = new QPushButton(tr("Cancel"));
	hbox->addStretch();
	hbox->addWidget(okButton);
	hbox->addWidget(cancelButton);
	lowerWidget->setLayout(hbox);

	gridlayout->addWidget(upperLabel, 0, 0);
	gridlayout->addWidget(lowerWidget, 1, 0);
	
	setLayout(gridlayout);

}

void DkUpdateDialog::okButtonClicked() {
	emit startUpdate();
	close();
}

// DkPrintPreviewDialog --------------------------------------------------------------------

DkPrintPreviewDialog::DkPrintPreviewDialog(const QImage& img, float dpi, QPrinter* printer, QWidget* parent, Qt::WindowFlags flags) 
		: QMainWindow(parent, flags) {
	
	mImg = img;
	mPrinter = printer;
	mDpi = dpi;
	mOrigDpi = dpi;
	mPrintDialog = 0;
	mImgTransform = QTransform();
	init();
	if (!img.isNull() && img.width() > img.height()) 
		mPreview->setLandscapeOrientation();

	scaleImage();
}

void DkPrintPreviewDialog::setImage(const QImage& img, float dpi) {
	
	mImg = img;
	mDpi = dpi;
	mImgTransform = QTransform();
	scaleImage();
}

void DkPrintPreviewDialog::scaleImage() {
	QRectF rect = mPrinter->pageRect();
	qreal scaleFactor;
	QSizeF paperSize = mPrinter->paperSize(QPrinter::Inch);
	QRectF pageRectInch = mPrinter->pageRect(QPrinter::Inch);

	// scale image to fit on paper
	if (rect.width()/mImg.width() < rect.height()/mImg.height()) {
		scaleFactor = rect.width()/(mImg.width()+FLT_EPSILON);		
	} else {
		scaleFactor = rect.height()/(mImg.height()+FLT_EPSILON);
	}

	float inchW = (float)mPrinter->pageRect(QPrinter::Inch).width();
	float pxW = (float)mPrinter->pageRect().width();
	mDpi = (pxW/inchW)/(float)scaleFactor;

	// use at least 150 dpi 
	if (mDpi < 150 && scaleFactor > 1) {
		mDpi = 150;
		scaleFactor = (pxW/inchW)/mDpi;
		qDebug() << "new scale Factor:" << scaleFactor;
	}


	mImgTransform.scale(scaleFactor, scaleFactor);

	mDpiFactor->lineEdit()->setText(QString().sprintf("%.0f", mDpi)+mDpiEditorSuffix);
	centerImage();
	updateZoomFactor();
}

void DkPrintPreviewDialog::init() {
	
	if (!mPrinter) {
#ifdef WIN32
		mPrinter = new QPrinter(QPrinter::HighResolution);
#else
		mPrinter = new QPrinter;
#endif
	}
	
	mPreview = new DkPrintPreviewWidget(mPrinter, this);

	connect(mPreview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintRequested(QPrinter*)));
	connect(mPreview, SIGNAL(zoomChanged()), this, SLOT(updateZoomFactor()));
	
	createIcons();
	setupActions();
	createLayout();
	setMinimumHeight(600);
	setMinimumWidth(800);

}

void DkPrintPreviewDialog::createIcons() {

	mIcons.resize(print_end);

	mIcons[print_fit_width]	= QIcon(":/nomacs/img/fit-width.png");
	mIcons[print_fit_page]	= QIcon(":/nomacs/img/zoomReset.png");
	mIcons[print_zoom_in]	= QIcon(":/nomacs/img/zoom-in.png");
	mIcons[print_zoom_out]	= QIcon(":/nomacs/img/zoom-out.png");
	mIcons[print_reset_dpi]	= QIcon(":/nomacs/img/zoom100.png");
	mIcons[print_landscape]	= QIcon(":/nomacs/img/landscape.png");
	mIcons[print_portrait]	= QIcon(":/nomacs/img/portrait.png");
	mIcons[print_setup]		= QIcon(":/nomacs/img/print-setup.png");
	mIcons[print_printer]	= QIcon(":/nomacs/img/printer.png");

	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < mIcons.size(); idx++)
			mIcons[idx].addPixmap(DkImage::colorizePixmap(mIcons[idx].pixmap(100), DkSettings::display.iconColor));
	}
}

void DkPrintPreviewDialog::setupActions() {

	mFitGroup = new QActionGroup(this);

	mFitWidthAction = mFitGroup->addAction(mIcons[print_fit_width], tr("Fit width"));
	mFitPageAction = mFitGroup->addAction(mIcons[print_fit_page], tr("Fit page"));
	mFitWidthAction->setObjectName(QLatin1String("fitWidthAction"));
	mFitPageAction->setObjectName(QLatin1String("fitPageAction"));
	mFitWidthAction->setCheckable(true);
	mFitPageAction->setCheckable(true);
	//setIcon(fitWidthAction, QLatin1String("fit-width"));
	//setIcon(fitPageAction, QLatin1String("fit-page"));
	QObject::connect(mFitGroup, SIGNAL(triggered(QAction*)), this, SLOT(fitImage(QAction*)));

	// Zoom
	mZoomGroup = new QActionGroup(this);

	mZoomInAction = mZoomGroup->addAction(mIcons[print_zoom_in], tr("Zoom in"));
	mZoomInAction->setShortcut(Qt::Key_Plus);
	//preview->addAction(zoomInAction);
	//addAction(zoomInAction);
	//zoomInAction->setShortcut(QKeySequence::AddTab);
	//addAction(zoomInAction);
	mZoomOutAction = mZoomGroup->addAction(mIcons[print_zoom_out], tr("Zoom out"));
	mZoomOutAction->setShortcut(QKeySequence(Qt::Key_Minus));
	//addAction(zoomOutAction);
	//preview->addAction(zoomOutAction);
	//setIcon(zoomInAction, QLatin1String("zoom-in"));
	//setIcon(zoomOutAction, QLatin1String("zoom-out"));

	// Portrait/Landscape
	mOrientationGroup = new QActionGroup(this);
	mPortraitAction = mOrientationGroup->addAction(mIcons[print_portrait], tr("Portrait"));
	mLandscapeAction = mOrientationGroup->addAction(mIcons[print_landscape], tr("Landscape"));
	mPortraitAction->setCheckable(true);
	mLandscapeAction->setCheckable(true);
	//setIcon(portraitAction, QLatin1String("layout-portrait"));
	//setIcon(landscapeAction, QLatin1String("layout-landscape"));
	QObject::connect(mPortraitAction, SIGNAL(triggered(bool)), mPreview, SLOT(setPortraitOrientation()));
	QObject::connect(mPortraitAction, SIGNAL(triggered(bool)), this, SLOT(centerImage()));
	QObject::connect(mLandscapeAction, SIGNAL(triggered(bool)), mPreview, SLOT(setLandscapeOrientation()));
	QObject::connect(mLandscapeAction, SIGNAL(triggered(bool)), this, SLOT(centerImage()));


	// Print
	mPrinterGroup = new QActionGroup(this);
	mPrintAction = mPrinterGroup->addAction(mIcons[print_printer], tr("Print"));
	mPageSetupAction = mPrinterGroup->addAction(mIcons[print_setup], tr("Page setup"));
	//setIcon(printAction, QLatin1String("print"));
	//setIcon(pageSetupAction, QLatin1String("page-setup"));
	QObject::connect(mPrintAction, SIGNAL(triggered(bool)), this, SLOT(print()));
	QObject::connect(mPageSetupAction, SIGNAL(triggered(bool)), this, SLOT(pageSetup()));

	mDpiGroup = new QActionGroup(this);
	mResetDpiAction = mDpiGroup->addAction(mIcons[print_reset_dpi], tr("Reset dpi"));
	//setIcon(resetDpiAction, QLatin1String("fit-width"));
	QObject::connect(mResetDpiAction, SIGNAL(triggered(bool)), this, SLOT(resetDpi()));

}

void DkPrintPreviewDialog::createLayout() {
	
	mZoomFactor = new QComboBox(this);
	mZoomFactor->setEditable(true);
	mZoomFactor->setMinimumContentsLength(7);
	mZoomFactor->setInsertPolicy(QComboBox::NoInsert);
	QLineEdit *zoomEditor = new QLineEdit(this);
	zoomEditor->setValidator(new DkPrintPreviewValidator("%", 1,1000, 1, zoomEditor));
	mZoomFactor->setLineEdit(zoomEditor);
	static const short factorsX2[] = { 25, 50, 100, 200, 250, 300, 400, 800, 1600 };
	for (int i = 0; i < int(sizeof(factorsX2) / sizeof(factorsX2[0])); ++i)
		mZoomFactor->addItem(QString::number(factorsX2[i] / 2.0)+"%");
	QObject::connect(mZoomFactor->lineEdit(), SIGNAL(editingFinished()), this, SLOT(zoomFactorChanged()));
	QObject::connect(mZoomFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(zoomFactorChanged()));

	QString zoomTip = tr("keep ALT key pressed to zoom with the mouse wheel");
	mZoomFactor->setToolTip(zoomTip);
	zoomEditor->setToolTip(zoomTip);
	mZoomOutAction->setToolTip(zoomTip);
	mZoomInAction->setToolTip(zoomTip);

	// dpi selection
	mDpiFactor = new QComboBox;
	mDpiFactor->setEditable(true);
	mDpiFactor->setMinimumContentsLength(5);
	mDpiFactor->setInsertPolicy(QComboBox::NoInsert);

	QLineEdit* dpiEditor = new QLineEdit;
	mDpiEditorSuffix = " dpi";
	dpiEditor->setValidator(new DkPrintPreviewValidator(mDpiEditorSuffix, 1,1000, 1, zoomEditor));
	mDpiFactor->setLineEdit(dpiEditor);
	static const short dpiFactors[] = {72, 150, 300, 600};
	for (int i = 0; i < int(sizeof(dpiFactors) / sizeof(dpiFactors[0])); i++)
		mDpiFactor->addItem(QString::number(dpiFactors[i])+mDpiEditorSuffix);
	QObject::connect(mDpiFactor->lineEdit(), SIGNAL(editingFinished()), this, SLOT(dpiFactorChanged()));
	QObject::connect(mDpiFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(dpiFactorChanged()));

	QToolBar *toolbar = new QToolBar(tr("Print Preview"), this);
	toolbar->addAction(mFitWidthAction);
	toolbar->addAction(mFitPageAction);
	toolbar->addSeparator();
	toolbar->addWidget(mZoomFactor);
	toolbar->addAction(mZoomInAction);
	toolbar->addAction(mZoomOutAction);
	toolbar->addSeparator();
	toolbar->addWidget(mDpiFactor);
	toolbar->addAction(mResetDpiAction);
	toolbar->addSeparator();
	toolbar->addAction(mPortraitAction);
	toolbar->addAction(mLandscapeAction);
	toolbar->addSeparator();
	//toolbar->addAction(firstPageAction);
	//toolbar->addAction(prevPageAction);
	//toolbar->addSeparator();
	toolbar->addAction(mPageSetupAction);
	toolbar->addAction(mPrintAction);

	if (DkSettings::display.toolbarGradient)
		toolbar->setObjectName("toolbarWithGradient");

	if (DkSettings::display.smallIcons)
		toolbar->setIconSize(QSize(16, 16));
	else
		toolbar->setIconSize(QSize(32, 32));


	// Cannot use the actions' triggered signal here, since it doesn't autorepeat
	QToolButton *zoomInButton = static_cast<QToolButton *>(toolbar->widgetForAction(mZoomInAction));
	QToolButton *zoomOutButton = static_cast<QToolButton *>(toolbar->widgetForAction(mZoomOutAction));
	zoomInButton->setAutoRepeat(true);
	zoomInButton->setAutoRepeatInterval(200);
	zoomInButton->setAutoRepeatDelay(200);
	zoomOutButton->setAutoRepeat(true);
	zoomOutButton->setAutoRepeatInterval(200);
	zoomOutButton->setAutoRepeatDelay(200);
	QObject::connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
	QObject::connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));


	this->addToolBar(toolbar);

	this->setCentralWidget(mPreview);
}

void DkPrintPreviewDialog::setIcon(QAction* action, const QLatin1String &name) {
	QLatin1String imagePrefix(":/trolltech/dialogs/qprintpreviewdialog/images/");
	QIcon icon;
	icon.addFile(imagePrefix + name + QLatin1String("-24.png"), QSize(24, 24));
	icon.addFile(imagePrefix + name + QLatin1String("-32.png"), QSize(32, 32));
	action->setIcon(icon);
}

void DkPrintPreviewDialog::paintRequested(QPrinter* printer) {
	
	QPainter painter(printer);
	QRect rect = painter.viewport();
	QSize size = mImg.size();
	painter.setWorldTransform(mImgTransform);
	painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
	painter.setWindow(mImg.rect());
	painter.drawImage(0, 0, mImg);

	painter.end();

}

void DkPrintPreviewDialog::fitImage(QAction* action) {
	setFitting(true);
	if (action == mFitPageAction)
		mPreview->fitInView();
	else
		mPreview->fitToWidth();
	updateZoomFactor();
}

bool DkPrintPreviewDialog::isFitting() {
	return (mFitGroup->isExclusive() && (mFitWidthAction->isChecked() || mFitPageAction->isChecked()));
}

void DkPrintPreviewDialog::centerImage() {
	
	QRect imgRect = mImg.rect();
	QRectF transRect = mImgTransform.mapRect(imgRect);
	qreal xtrans = 0, ytrans = 0;
	xtrans = ((mPrinter->pageRect().width() - transRect.width())/2);
	ytrans = (mPrinter->pageRect().height() - transRect.height())/2;

	mImgTransform.translate(-mImgTransform.dx()/(mImgTransform.m11()+DBL_EPSILON), -mImgTransform.dy()/(mImgTransform.m22()+DBL_EPSILON)); // reset old transformation

	mImgTransform.translate(xtrans/(mImgTransform.m11()+DBL_EPSILON), ytrans/(mImgTransform.m22()+DBL_EPSILON));
	mPreview->updatePreview();
}

void DkPrintPreviewDialog::setFitting(bool on) {
	if (isFitting() == on)
		return;
	mFitGroup->setExclusive(on);
	if (on) {
		QAction* action = mFitWidthAction->isChecked() ? mFitWidthAction : mFitPageAction;
		action->setChecked(true);
		if (mFitGroup->checkedAction() != action) {
			// work around exclusitivity problem
			mFitGroup->removeAction(action);
			mFitGroup->addAction(action);
		}
	} else {
		mFitWidthAction->setChecked(false);
		mFitPageAction->setChecked(false);
	}
}

void DkPrintPreviewDialog::zoomIn() {
	setFitting(false);
	mPreview->zoomIn();
	updateZoomFactor();
}

void DkPrintPreviewDialog::zoomOut() {
	setFitting(false);
	mPreview->zoomOut();
	updateZoomFactor();
}

void DkPrintPreviewDialog::zoomFactorChanged() {
	QString text = mZoomFactor->lineEdit()->text();
	bool ok;
	qreal factor = text.remove(QLatin1Char('%')).toFloat(&ok);
	factor = qMax(qreal(1.0), qMin(qreal(1000.0), factor));
	if (ok) {
		mPreview->setZoomFactor(factor/100.0);
		mZoomFactor->setEditText(QString::fromLatin1("%1%").arg(factor));
		setFitting(false);
		updateZoomFactor();
	}
	updateZoomFactor();
}

void DkPrintPreviewDialog::updateZoomFactor()
{
	mZoomFactor->lineEdit()->setText(QString().sprintf("%.1f%%", mPreview->zoomFactor()*100));
}

void DkPrintPreviewDialog::dpiFactorChanged() {
	
	QString text = mDpiFactor->lineEdit()->text();
	bool ok;
	qreal factor = text.remove(mDpiEditorSuffix).toFloat(&ok);
	if (ok) {
		mImgTransform.reset();

		float inchW = (float)mPrinter->pageRect(QPrinter::Inch).width();
		float pxW = (float)mPrinter->pageRect().width();
		float scaleFactor = (float)((pxW/inchW)/factor);

		mImgTransform.scale(scaleFactor, scaleFactor);

	}
	centerImage();
	mPreview->updatePreview();
}

void DkPrintPreviewDialog::updateDpiFactor(qreal dpi) {
	mDpiFactor->lineEdit()->setText(QString().sprintf("%.0f", dpi)+mDpiEditorSuffix);
}

void DkPrintPreviewDialog::resetDpi() {
	updateDpiFactor(mOrigDpi);
	dpiFactorChanged();
}

void DkPrintPreviewDialog::pageSetup() {
	QPageSetupDialog pageSetup(mPrinter, this);
	if (pageSetup.exec() == QDialog::Accepted) {
		// update possible orientation changes
		if (mPreview->orientation() == QPrinter::Portrait) {
			mPortraitAction->setChecked(true);
			mPreview->setPortraitOrientation();
		}else {
			mLandscapeAction->setChecked(true);
			mPreview->setLandscapeOrientation();
		}
		centerImage();
	}
}

void DkPrintPreviewDialog::print() {
	if (!mPrintDialog)
		mPrintDialog = new QPrintDialog(mPrinter, this);
	if (mPrintDialog->exec() == QDialog::Accepted) {
		mPreview->print();
		close();
	}
}

// DkPrintPreviewWidget --------------------------------------------------------------------
DkPrintPreviewWidget::DkPrintPreviewWidget(QPrinter* printer, QWidget* parent, Qt::WindowFlags flags) : QPrintPreviewWidget(printer, parent, flags) {
	// do nothing atm - bis zum bankomat
}

//void DkPrintPreviewWidget::paintEvent(QPaintEvent * event) {
//	qDebug() << "paintEvent";
//	QPrintPreviewWidget::paintEvent(event);
//}

void DkPrintPreviewWidget::wheelEvent(QWheelEvent *event) {

	if (event->modifiers() != Qt::AltModifier) {
		QPrintPreviewWidget::wheelEvent(event);
		return;
	}


	qreal delta = event->delta();
	if (DkSettings::display.invertZoom)
		delta *= -1;
	if (event->delta() > 0)
		zoomIn();
	else
		zoomOut();
	emit zoomChanged();

	QPrintPreviewWidget::wheelEvent(event);	
}

// DkOpacityDialog --------------------------------------------------------------------
DkOpacityDialog::DkOpacityDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {

	createLayout();
}

void DkOpacityDialog::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);

	slider = new DkSlider(tr("Window Opacity"), this);
	slider->setMinimum(5);

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(slider);
	layout->addWidget(buttons);
}

int DkOpacityDialog::value() const {
	return slider->value();
}

// DkExportTiffDialog --------------------------------------------------------------------
DkExportTiffDialog::DkExportTiffDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	setWindowTitle(tr("Export Multi-Page TIFF"));
	createLayout();
	setAcceptDrops(true);

	connect(this, SIGNAL(updateImage(const QImage&)), mViewport, SLOT(setImage(const QImage&)));
	connect(&mWatcher, SIGNAL(finished()), this, SLOT(processingFinished()));
	connect(this, SIGNAL(infoMessage(const QString&)), mMsgLabel, SLOT(setText(const QString&)));
	connect(this, SIGNAL(updateProgress(int)), mProgress, SLOT(setValue(int)));
	QMetaObject::connectSlotsByName(this);
}

void DkExportTiffDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		setFile(url.toString());
	}
}

void DkExportTiffDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists() && file.suffix().indexOf(QRegExp("tif"), Qt::CaseInsensitive) != -1)
			event->acceptProposedAction();
	}

}


void DkExportTiffDialog::createLayout() {

	// progress bar
	mProgress = new QProgressBar(this);
	mProgress->hide();

	mMsgLabel = new QLabel(this);
	mMsgLabel->setObjectName("DkWarningInfo");
	mMsgLabel->hide();

	// open handles
	QLabel* openLabel = new QLabel(tr("Multi-Page TIFF:"), this);
	openLabel->setAlignment(Qt::AlignRight);

	QPushButton* openButton = new QPushButton(tr("&Browse"), this);
	openButton->setObjectName("openButton");

	mTiffLabel = new QLabel(tr("No Multi-Page TIFF loaded"), this);

	// save handles
	QLabel* saveLabel = new QLabel(tr("Save Folder:"), this);
	saveLabel->setAlignment(Qt::AlignRight);

	QPushButton* saveButton = new QPushButton(tr("&Browse"), this);
	saveButton->setObjectName("saveButton");

	mFolderLabel = new QLabel(tr("Specify a Save Folder"), this);

	// file name handles
	QLabel* fileLabel = new QLabel(tr("Filename:"), this);
	fileLabel->setAlignment(Qt::AlignRight);

	mFileEdit = new QLineEdit("tiff_page", this);
	mFileEdit->setObjectName("fileEdit");

	mSuffixBox = new QComboBox(this);
	mSuffixBox->addItems(DkSettings::app.saveFilters);
	mSuffixBox->setCurrentIndex(DkSettings::app.saveFilters.indexOf(QRegExp(".*tif.*")));

	// export handles
	QLabel* exportLabel = new QLabel(tr("Export Pages"));
	exportLabel->setAlignment(Qt::AlignRight);

	mFromPage = new QSpinBox(0);

	mToPage = new QSpinBox(0);

	mOverwrite = new QCheckBox(tr("Overwrite"));

	mControlWidget = new QWidget(this);
	QGridLayout* controlLayout = new QGridLayout(mControlWidget);
	controlLayout->addWidget(openLabel, 0, 0);
	controlLayout->addWidget(openButton, 0, 1, 1, 2);
	controlLayout->addWidget(mTiffLabel, 0, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(saveLabel, 1, 0);
	controlLayout->addWidget(saveButton, 1, 1, 1, 2);
	controlLayout->addWidget(mFolderLabel, 1, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(fileLabel, 2, 0);
	controlLayout->addWidget(mFileEdit, 2, 1, 1, 2);
	controlLayout->addWidget(mSuffixBox, 2, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(exportLabel, 3, 0);
	controlLayout->addWidget(mFromPage, 3, 1);
	controlLayout->addWidget(mToPage, 3, 2);
	controlLayout->addWidget(mOverwrite, 3, 3);
	controlLayout->setColumnStretch(5, 1);

	// shows the image if it could be loaded
	mViewport = new DkBaseViewPort(this);
	mViewport->setForceFastRendering(true);
	mViewport->setPanControl(QPointF(0.0f, 0.0f));

	// mButtons
	mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	mButtons->button(QDialogButtonBox::Ok)->setText(tr("&Export"));
	mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mViewport);
	layout->addWidget(mProgress);
	layout->addWidget(mMsgLabel);
	layout->addWidget(mControlWidget);
	layout->addWidget(mButtons);

	enableTIFFSave(false);
}

void DkExportTiffDialog::on_openButton_pressed() {

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open TIFF"),
		mFilePath, 
		DkSettings::app.saveFilters.filter(QRegExp(".*tif.*")).join(";;"));

	setFile(fileName);
}

void DkExportTiffDialog::on_saveButton_pressed() {
	qDebug() << "save triggered...";

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		mSaveDirPath);

	if (QDir(dirName).exists()) {
		mSaveDirPath = dirName;
		mFolderLabel->setText(mSaveDirPath);
	}
}

void DkExportTiffDialog::on_fileEdit_textChanged(const QString& filename) {

	qDebug() << "new file name: " << filename;
}

void DkExportTiffDialog::reject() {

	// not sure if this is a nice way to do: but we change cancel behavior while processing
	if (mProcessing)
		mProcessing = false;
	else
		QDialog::reject();

}

void DkExportTiffDialog::accept() {

	mProgress->setMinimum(mFromPage->value()-1);
	mProgress->setMaximum(mToPage->value());
	mProgress->setValue(mProgress->minimum());
	mProgress->show();
	mMsgLabel->show();

	enableAll(false);

	QString suffix = mSuffixBox->currentText();

	for (int idx = 0; idx < DkSettings::app.fileFilters.size(); idx++) {
		if (suffix.contains("(" + DkSettings::app.fileFilters.at(idx))) {
			suffix = DkSettings::app.fileFilters.at(idx);
			suffix.replace("*","");
			break;
		}
	}

	QFileInfo sFile(mSaveDirPath, mFileEdit->text() + "-" + suffix);
	
	emit infoMessage("");
	
	QFuture<int> future = QtConcurrent::run(this, 
		&nmc::DkExportTiffDialog::exportImages,
		sFile.absoluteFilePath(), 
		mFromPage->value(), 
		mToPage->value(),
		mOverwrite->isChecked());
	mWatcher.setFuture(future);
}

void DkExportTiffDialog::processingFinished() {

	enableAll(true);
	mProgress->hide();
	mMsgLabel->hide();

	if (mWatcher.future() == QDialog::Accepted)
		QDialog::accept();
}

int DkExportTiffDialog::exportImages(const QString& saveFilePath, int from, int to, bool overwrite) {

	mProcessing = true;

	QFileInfo saveInfo(saveFilePath);

	// Do your job
	for (int idx = from; idx <= to; idx++) {

		QFileInfo cInfo(saveInfo.absolutePath(), saveInfo.baseName() + QString::number(idx) + "." + saveInfo.suffix());
		qDebug() << "trying to save: " << cInfo.absoluteFilePath();

		emit updateProgress(idx-1);

		// user wants to overwrite files
		if (cInfo.exists() && overwrite) {
			QFile f(cInfo.absoluteFilePath());
			f.remove();
		}
		else if (cInfo.exists()) {
			emit infoMessage(tr("%1 exists, skipping...").arg(cInfo.fileName()));
			continue;
		}

		if (!mLoader.loadPageAt(idx)) {	// load next
			emit infoMessage(tr("Sorry, I could not load page: %1").arg(idx));
			continue;
		}

		QString lSaveFilePath = mLoader.save(cInfo.absoluteFilePath(), mLoader.image(), 90);		//TODO: ask user for compression?
		QFileInfo lSaveInfo = QFileInfo(lSaveFilePath);

		if (!lSaveInfo.exists() || !lSaveInfo.isFile())
			emit infoMessage(tr("Sorry, I could not save: %1").arg(cInfo.fileName()));

		emit updateImage(mLoader.image());
		emit updateProgress(idx);

		// user canceled?
		if (!mProcessing)
			return QDialog::Rejected;
	}

	mProcessing = false;

	return QDialog::Accepted;
}

void DkExportTiffDialog::setFile(const QString& filePath) {
	
	if (!QFileInfo(filePath).exists())
		return;
	
	QFileInfo fInfo(filePath);
	mFilePath = filePath;
	mSaveDirPath = fInfo.absolutePath();
	mFolderLabel->setText(mSaveDirPath);
	mTiffLabel->setText(filePath);
	mFileEdit->setText(fInfo.baseName());

	mLoader.loadGeneral(filePath, true);
	mViewport->setImage(mLoader.image());

	enableTIFFSave(mLoader.getNumPages() > 1);

	mFromPage->setRange(1, mLoader.getNumPages());
	mToPage->setRange(1, mLoader.getNumPages());

	mFromPage->setValue(1);
	mToPage->setValue(mLoader.getNumPages());
}

void DkExportTiffDialog::enableAll(bool enable) {

	enableTIFFSave(enable);
	mControlWidget->setEnabled(enable);
}

void DkExportTiffDialog::enableTIFFSave(bool enable) {

	mFileEdit->setEnabled(enable);
	mSuffixBox->setEnabled(enable);
	mFromPage->setEnabled(enable);
	mToPage->setEnabled(enable);
	mButtons->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

#ifdef WITH_OPENCV
// DkUnsharpDialog --------------------------------------------------------------------
DkUnsharpDialog::DkUnsharpDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	mProcessing = false;

	setWindowTitle(tr("Sharpen Image"));
	createLayout();
	setAcceptDrops(true);

	connect(this, SIGNAL(updateImage(const QImage&)), mViewport, SLOT(setImage(const QImage&)));
	connect(&mUnsharpWatcher, SIGNAL(finished()), this, SLOT(unsharpFinished()));
	connect(mViewport, SIGNAL(imageUpdated()), this, SLOT(computePreview()));
	QMetaObject::connectSlotsByName(this);
}

void DkUnsharpDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		setFile(url.toString());
	}
}

void DkUnsharpDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists() && DkUtils::isValid(file))
			event->acceptProposedAction();
	}
}

void DkUnsharpDialog::createLayout() {

	// post processing sliders
	mSigmaSlider = new DkSlider(tr("Sigma"), this);
	mSigmaSlider->setObjectName("sigmaSlider");
	mSigmaSlider->setValue(30);
	//darkenSlider->hide();

	mAmountSlider = new DkSlider(tr("Amount"), this);
	mAmountSlider->setObjectName("amountSlider");
	mAmountSlider->setValue(45);

	QWidget* sliderWidget = new QWidget(this);
	QVBoxLayout* sliderLayout = new QVBoxLayout(sliderWidget);
	sliderLayout->addWidget(mSigmaSlider);
	sliderLayout->addWidget(mAmountSlider);

	// shows the image if it could be loaded
	mViewport = new DkBaseViewPort(this);
	mViewport->setForceFastRendering(true);
	mViewport->setPanControl(QPointF(0.0f, 0.0f));

	mPreview = new QLabel(this);
	mPreview->setScaledContents(true);
	mPreview->setMinimumSize(QSize(200,200));
	//preview->setForceFastRendering(true);
	//preview->setPanControl(QPointF(0.0f, 0.0f));

	QWidget* viewports = new QWidget(this);
	QGridLayout* viewLayout = new QGridLayout(viewports);
	viewLayout->setColumnStretch(0,1);
	viewLayout->setColumnStretch(1,1);

	viewLayout->addWidget(mViewport, 0,0);
	viewLayout->addWidget(mPreview, 0, 1);

	// mButtons
	mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	//mButtons->button(QDialogButtonBox::Save)->setText(tr("&Save"));
	//mButtons->button(QDialogButtonBox::Apply)->setText(tr("&Generate"));
	//mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	//connect(mButtons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));
	//mButtons->button(QDialogButtonBox::Save)->setEnabled(false);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(viewports);
	layout->addWidget(sliderWidget);
	layout->addWidget(mButtons);
}

void DkUnsharpDialog::on_sigmaSlider_valueChanged(int) {

	computePreview();
}

void DkUnsharpDialog::on_amountSlider_valueChanged(int) {

	computePreview();
}

QImage DkUnsharpDialog::getImage() {

	return computeUnsharp(mImg, mSigmaSlider->value(), mAmountSlider->value());
}

void DkUnsharpDialog::reject() {

	QDialog::reject();

}

//void DkMosaicDialog::buttonClicked(QAbstractButton* button) {
//
//	if (button == mButtons->button(QDialogButtonBox::Save)) {
//
//		// render the full image
//		if (!mosaic.isNull()) {
//			sliderWidget->hide();
//			progress->setValue(progress->minimum());
//			progress->show();
//			enableAll(false);
//			button->setEnabled(false);
//
//			QFuture<bool> future = QtConcurrent::run(this, 
//				&nmc::DkMosaicDialog::postProcessMosaic,
//				darkenSlider->value()/100.0f,
//				lightenSlider->value()/100.0f, 
//				saturationSlider->value()/100.0f,
//				false);
//			postProcessWatcher.setFuture(future);
//		}
//	}
//	else if (button == mButtons->button(QDialogButtonBox::Apply))
//		compute();
//}

void DkUnsharpDialog::computePreview() {
		
	if (mProcessing)
		return;

	QFuture<QImage> future = QtConcurrent::run(this, 
		&nmc::DkUnsharpDialog::computeUnsharp,
		mViewport->getCurrentImageRegion(),
		mSigmaSlider->value(),
		mAmountSlider->value()); 
	mUnsharpWatcher.setFuture(future);
	mProcessing = true;
}

void DkUnsharpDialog::unsharpFinished() {

	QImage img = mUnsharpWatcher.result();
	img = img.scaled(mPreview->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	mPreview->setPixmap(QPixmap::fromImage(img));

	//update();
	mProcessing = false;
}

QImage DkUnsharpDialog::computeUnsharp(const QImage& img, int sigma, int amount) {

	QImage imgC = img.copy();
	DkImage::unsharpMask(imgC, (float)sigma, 1.0f+amount/100.0f);
	return imgC;
}

void DkUnsharpDialog::setImage(const QImage& img) {
	mImg = img;
	mViewport->setImage(img);
	mViewport->fullView();
	mViewport->zoomConstraints(mViewport->get100Factor());
	computePreview();
}

void DkUnsharpDialog::setFile(const QString& filePath) {

	DkBasicLoader loader;
	loader.loadGeneral(filePath, true);
	setImage(loader.image());
}

// DkTinyPlanetDialog --------------------------------------------------------------------
DkTinyPlanetDialog::DkTinyPlanetDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	mProcessing = false;

	setWindowTitle(tr("Tiny Planet"));
	createLayout();
	setAcceptDrops(true);

	connect(this, SIGNAL(updateImage(const QImage&)), this, SLOT(updateImageSlot(const QImage&)));
	connect(&mUnsharpWatcher, SIGNAL(finished()), this, SLOT(tinyPlanetFinished()));
	QMetaObject::connectSlotsByName(this);
}

void DkTinyPlanetDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		setFile(url.toString());
	}
}

void DkTinyPlanetDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists() && DkUtils::isValid(file))
			event->acceptProposedAction();
	}
}

void DkTinyPlanetDialog::createLayout() {

	// post processing sliders
	mScaleLogSlider = new DkSlider(tr("Planet Size"), this);
	mScaleLogSlider->setObjectName("scaleLogSlider");
	mScaleLogSlider->setMinimum(1);
	mScaleLogSlider->setMaximum(1000);
	mScaleLogSlider->setValue(30);

	//darkenSlider->hide();

	mScaleSlider = new DkSlider(tr("Scale"), this);
	mScaleSlider->setObjectName("scaleSlider");
	mScaleSlider->setMinimum(1);
	mScaleSlider->setMaximum(3000);
	mScaleSlider->setValue(300);

	mAngleSlider = new DkSlider(tr("Angle"), this);
	mAngleSlider->setObjectName("angleSlider");
	mAngleSlider->setValue(0);
	mAngleSlider->setMinimum(-180);
	mAngleSlider->setMaximum(179);

	mInvertBox = new QCheckBox(tr("Invert Planet"), this);
	mInvertBox->setObjectName("invertBox");

	QWidget* sliderWidget = new QWidget(this);
	QVBoxLayout* sliderLayout = new QVBoxLayout(sliderWidget);
	sliderLayout->addWidget(mScaleLogSlider);
	sliderLayout->addWidget(mScaleSlider);
	sliderLayout->addWidget(mAngleSlider);
	sliderLayout->addWidget(mInvertBox);

	// shows the image if it could be loaded
	mImgPreview = new QLabel(this);
	//imgPreview->setScaledContents(true);
	mImgPreview->setMinimumSize(QSize(200, 200));

	mPreviewLabel = new QLabel(this);
	//preview->setScaledContents(true);
	mPreviewLabel->setMinimumSize(QSize(200, 200));
	//preview->setForceFastRendering(true);
	//preview->setPanControl(QPointF(0.0f, 0.0f));

	QWidget* viewports = new QWidget(this);
	QHBoxLayout* viewLayout = new QHBoxLayout(viewports);
	//viewLayout->setColumnStretch(0,1);
	//viewLayout->setColumnStretch(1,1);
	viewLayout->addStretch();
	viewLayout->addWidget(mImgPreview);
	viewLayout->addWidget(mPreviewLabel);
	viewLayout->addStretch();

	// mButtons
	mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(viewports);
	layout->addWidget(sliderWidget);
	layout->addWidget(mButtons);
}

void DkTinyPlanetDialog::on_scaleSlider_valueChanged(int) {

	computePreview();
}

void DkTinyPlanetDialog::on_scaleLogSlider_valueChanged(int) {

	mScaleSlider->setValue(std::max(mScaleLogSlider->value()*10, 200));
	//computePreview();
}

void DkTinyPlanetDialog::on_angleSlider_valueChanged(int) {

	computePreview();
}

void DkTinyPlanetDialog::on_invertBox_toggled(bool) {

	computePreview();
}

void DkTinyPlanetDialog::resizeEvent(QResizeEvent *event) {

	updateImageSlot(mImg);
	QDialog::resizeEvent(event);
}

QImage DkTinyPlanetDialog::getImage() {
	
	int mSize = std::max(mImg.width(), mImg.height());
	if (mSize > 7000)
		mSize = 7000;	// currently max supported size (x86)
	float f = (mSize > 1000) ? mSize/1000 : 1.0f;
	QSize s(mSize, mSize);

	float slInv = mScaleLogSlider->value()*f;
	if (mInvertBox->isChecked())
		slInv *= -1.0f;

	return computeTinyPlanet(mImg, slInv, mScaleSlider->value()*f, mAngleSlider->value()*DK_DEG2RAD, s);
}

void DkTinyPlanetDialog::reject() {

	QDialog::reject();

}

void DkTinyPlanetDialog::updateImageSlot(const QImage& img) {

	mImgPreview->setPixmap(QPixmap::fromImage(img.scaled(mImgPreview->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
	computePreview();
}

void DkTinyPlanetDialog::computePreview() {

	if (mProcessing)
		return;

	QImage rImg = mImg;
	int mSide = qMax(mImg.width(), mImg.height()) > 1000 ? 1000 : qMax(mImg.width(), mImg.height());
	rImg = rImg.scaled(QSize(mSide, mSide), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	int slVal = mScaleLogSlider->value();
	
	// encode invert bool into sign
	if (mInvertBox->isChecked())
		slVal *= -1;

	QFuture<QImage> future = QtConcurrent::run(this, 
		&nmc::DkTinyPlanetDialog::computeTinyPlanet,
		rImg,
		slVal,
		mScaleSlider->value(),
		mAngleSlider->value()*DK_DEG2RAD,
		QSize(mSide, mSide)); 
	mUnsharpWatcher.setFuture(future);
	mProcessing = true;
}

void DkTinyPlanetDialog::tinyPlanetFinished() {

	QImage img = mUnsharpWatcher.result();
	img = img.scaled(mPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	mPreviewLabel->setPixmap(QPixmap::fromImage(img));

	//update();
	mProcessing = false;
}

QImage DkTinyPlanetDialog::computeTinyPlanet(const QImage& img, float scaleLog, float scale, double angle, QSize s) {

	bool inverted = scaleLog < 0;
	scaleLog = fabs(scaleLog);

	QImage imgC = img.copy();
	DkImage::tinyPlanet(imgC, (double)scaleLog, (double) scale, angle, s, inverted);
	return imgC;
}

void DkTinyPlanetDialog::setImage(const QImage& img) {
	mImg = img;
	updateImageSlot(img);
	//mViewport->fullView();
	//mViewport->zoomConstraints(mViewport->get100Factor());
	computePreview();
}

void DkTinyPlanetDialog::setFile(const QString& filePath) {

	DkBasicLoader loader;
	loader.loadGeneral(filePath, true);
	setImage(loader.image());
}

// DkMosaicDialog --------------------------------------------------------------------
DkMosaicDialog::DkMosaicDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	mProcessing = false;
	mPostProcessing = false;
	mUpdatePostProcessing = false;

	setWindowTitle(tr("Create Mosaic Image"));
	createLayout();
	setAcceptDrops(true);

	connect(this, SIGNAL(updateImage(const QImage&)), mPreview, SLOT(setImage(const QImage&)));
	connect(&mMosaicWatcher, SIGNAL(finished()), this, SLOT(mosaicFinished()));
	connect(&mPostProcessWatcher, SIGNAL(finished()), this, SLOT(postProcessFinished()));
	connect(&mPostProcessWatcher, SIGNAL(canceled()), this, SLOT(postProcessFinished()));
	connect(this, SIGNAL(infoMessage(const QString&)), mMsgLabel, SLOT(setText(const QString&)));
	connect(this, SIGNAL(updateProgress(int)), mProgress, SLOT(setValue(int)));
	QMetaObject::connectSlotsByName(this);
}

void DkMosaicDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		setFile(url.toString());
	}
}

void DkMosaicDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists() && DkUtils::isValid(file))
			event->acceptProposedAction();
	}
}

void DkMosaicDialog::createLayout() {

	// progress bar
	mProgress = new QProgressBar(this);
	mProgress->hide();

	mMsgLabel = new QLabel(this);
	mMsgLabel->setObjectName("DkWarningInfo");
	mMsgLabel->hide();

	// post processing sliders
	mDarkenSlider = new QSlider(Qt::Horizontal, this);
	mDarkenSlider->setObjectName("darkenSlider");
	mDarkenSlider->setValue(40);
	//darkenSlider->hide();

	mLightenSlider = new QSlider(Qt::Horizontal, this);
	mLightenSlider->setObjectName("lightenSlider");
	mLightenSlider->setValue(40);
	//lightenSlider->hide();

	mSaturationSlider = new QSlider(Qt::Horizontal, this);
	mSaturationSlider->setObjectName("saturationSlider");
	mSaturationSlider->setValue(60);
	//saturationSlider->hide();

	mSliderWidget = new QWidget(this);
	QGridLayout* sliderLayout = new QGridLayout(mSliderWidget);
	sliderLayout->addWidget(new QLabel(tr("Darken")), 0, 0);
	sliderLayout->addWidget(new QLabel(tr("Lighten")), 0, 1);
	sliderLayout->addWidget(new QLabel(tr("Saturation")), 0, 2);

	sliderLayout->addWidget(mDarkenSlider, 1, 0);
	sliderLayout->addWidget(mLightenSlider, 1, 1);
	sliderLayout->addWidget(mSaturationSlider, 1, 2);
	mSliderWidget->hide();

	// open handles
	QLabel* openLabel = new QLabel(tr("Mosaic Image:"), this);
	openLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	QPushButton* openButton = new QPushButton(tr("&Browse"), this);
	openButton->setObjectName("openButton");
	openButton->setToolTip(tr("Specify the Root Folder of the Image Database Desired."));

	mFileLabel = new QLabel(tr("No Image loaded"), this);

	// save handles
	QLabel* saveLabel = new QLabel(tr("Image Database:"), this);
	saveLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	QPushButton* dbButton = new QPushButton(tr("&Browse"), this);
	dbButton->setObjectName("dbButton");

	mFolderLabel = new QLabel(tr("Specify an Image Database"), this);

	// resolution handles
	QLabel* sizeLabel = new QLabel(tr("Resolution:"));
	sizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mNewWidthBox = new QSpinBox();
	mNewWidthBox->setObjectName("newWidthBox");
	mNewWidthBox->setToolTip(tr("Pixel Width"));
	mNewWidthBox->setMinimum(100);
	mNewWidthBox->setMaximum(30000);
	mNewHeightBox = new QSpinBox();
	mNewHeightBox->setObjectName("newHeightBox");
	mNewHeightBox->setToolTip(tr("Pixel Height"));
	mNewHeightBox->setMinimum(100);
	mNewHeightBox->setMaximum(30000);
	mRealResLabel = new QLabel("");
	//realResLabel->setToolTip(tr("."));

	// num patch handles
	QLabel* patchLabel = new QLabel(tr("Patches:"));
	patchLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mNumPatchesH = new QSpinBox(this);
	mNumPatchesH->setObjectName("numPatchesH");
	mNumPatchesH->setToolTip(tr("Number of Horizontal Patches"));
	mNumPatchesH->setMinimum(1);
	mNumPatchesH->setMaximum(1000);
	mNumPatchesV = new QSpinBox(this);
	mNumPatchesV->setObjectName("numPatchesV");
	mNumPatchesV->setToolTip(tr("Number of Vertical Patches"));
	mNumPatchesV->setMinimum(1);
	mNumPatchesV->setMaximum(1000);
	mPatchResLabel = new QLabel("", this);
	mPatchResLabel->setObjectName("DkDecentInfo");
	mPatchResLabel->setToolTip(tr("If this label turns red, the computation might be slower."));

	// file filters
	QLabel* filterLabel = new QLabel(tr("Filters:"), this);
	filterLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	mFilterEdit = new QLineEdit("", this);
	mFilterEdit->setObjectName("fileEdit");
	mFilterEdit->setToolTip(tr("You can split multiple ignore words with ;"));

	QStringList filters = DkSettings::app.openFilters;
	filters.pop_front();	// replace for better readability
	filters.push_front(tr("All Images"));
	mSuffixBox = new QComboBox(this);
	mSuffixBox->addItems(filters);
	//suffixBox->setCurrentIndex(DkImageLoader::saveFilters.indexOf(QRegExp(".*tif.*")));

	mControlWidget = new QWidget(this);
	QGridLayout* controlLayout = new QGridLayout(mControlWidget);
	controlLayout->addWidget(openLabel, 0, 0);
	controlLayout->addWidget(openButton, 0, 1, 1, 2);
	controlLayout->addWidget(mFileLabel, 0, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(saveLabel, 1, 0);
	controlLayout->addWidget(dbButton, 1, 1, 1, 2);
	controlLayout->addWidget(mFolderLabel, 1, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(sizeLabel, 2, 0);
	controlLayout->addWidget(mNewWidthBox, 2, 1);
	controlLayout->addWidget(mNewHeightBox, 2, 2);
	controlLayout->addWidget(mRealResLabel, 2, 3);

	controlLayout->addWidget(patchLabel, 4, 0);
	controlLayout->addWidget(mNumPatchesH, 4, 1);
	controlLayout->addWidget(mNumPatchesV, 4, 2);
	controlLayout->addWidget(mPatchResLabel, 4, 3);

	controlLayout->addWidget(filterLabel, 5, 0);
	controlLayout->addWidget(mFilterEdit, 5, 1, 1, 2);
	controlLayout->addWidget(mSuffixBox, 5, 3, 1, 2);
	controlLayout->setColumnStretch(5, 1);

	// shows the image if it could be loaded
	mViewport = new DkBaseViewPort(this);
	mViewport->setForceFastRendering(true);
	mViewport->setPanControl(QPointF(0.0f, 0.0f));

	mPreview = new DkBaseViewPort(this);
	mPreview->setForceFastRendering(true);
	mPreview->setPanControl(QPointF(0.0f, 0.0f));
	mPreview->hide();

	QWidget* viewports = new QWidget(this);
	QHBoxLayout* viewLayout = new QHBoxLayout(viewports);
	viewLayout->addWidget(mViewport);
	viewLayout->addWidget(mPreview);

	// mButtons
	mButtons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	mButtons->button(QDialogButtonBox::Save)->setText(tr("&Save"));
	mButtons->button(QDialogButtonBox::Apply)->setText(tr("&Generate"));
	mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	//connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mButtons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));
	mButtons->button(QDialogButtonBox::Save)->setEnabled(false);
	
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(viewports);
	layout->addWidget(mProgress);
	layout->addWidget(mSliderWidget);
	layout->addWidget(mMsgLabel);
	layout->addWidget(mControlWidget);
	layout->addWidget(mButtons);

	enableMosaicSave(false);
}

void DkMosaicDialog::on_openButton_pressed() {

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open TIFF"),
		mFilePath, DkSettings::app.openFilters.join(";;"));

	setFile(fileName);
}

void DkMosaicDialog::on_dbButton_pressed() {
	qDebug() << "save triggered...";

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"), mSavePath);

	if (QFileInfo(dirName).exists()) {
		mSavePath = dirName;
		mFolderLabel->setText(mSavePath);
	}
}

void DkMosaicDialog::on_fileEdit_textChanged(const QString& filename) {

	qDebug() << "new file name: " << filename;
}

void DkMosaicDialog::on_newWidthBox_valueChanged(int) {

	if (!mLoader.hasImage())
		return;

	mNewHeightBox->blockSignals(true);
	mNewHeightBox->setValue(qRound((float)mNewWidthBox->value()/mLoader.image().width()*mLoader.image().height()));
	mNewHeightBox->blockSignals(false);
	mRealResLabel->setText(tr("%1 x %2 cm @150 dpi").arg(mNewWidthBox->value()/150.0*2.54, 0, 'f', 1).arg(mNewHeightBox->value()/150.0*2.54, 0, 'f', 1));
	updatePatchRes();
}

void DkMosaicDialog::on_newHeightBox_valueChanged(int) {

	if (!mLoader.hasImage())
		return;

	mNewWidthBox->blockSignals(true);
	mNewWidthBox->setValue(qRound((float)mNewHeightBox->value()/mLoader.image().height()*mLoader.image().width()));
	mNewWidthBox->blockSignals(false);
	mRealResLabel->setText(tr("%1 x %2 cm @150 dpi").arg(mNewWidthBox->value()/150.0*2.54, 0, 'f', 1).arg(mNewHeightBox->value()/150.0*2.54, 0, 'f', 1));
	updatePatchRes();
}

void DkMosaicDialog::on_numPatchesH_valueChanged(int) {

	if (!mLoader.hasImage())
		return;

	mNumPatchesV->blockSignals(true);
	mNumPatchesV->setValue(qFloor((float)mLoader.image().height()/((float)mLoader.image().width()/mNumPatchesH->value())));
	mNumPatchesV->blockSignals(false);
	updatePatchRes();
}

void DkMosaicDialog::on_numPatchesV_valueChanged(int) {
	
	if (!mLoader.hasImage())
		return;

	mNumPatchesH->blockSignals(true);
	mNumPatchesH->setValue(qFloor((float)mLoader.image().width()/((float)mLoader.image().height()/mNumPatchesV->value())));
	mNumPatchesH->blockSignals(false);
	updatePatchRes();
}

void DkMosaicDialog::on_darkenSlider_valueChanged(int) {

	updatePostProcess();
}

void DkMosaicDialog::on_lightenSlider_valueChanged(int) {

	updatePostProcess();
}

void DkMosaicDialog::on_saturationSlider_valueChanged(int) {

	updatePostProcess();
}

void DkMosaicDialog::updatePatchRes() {

	int patchResD = qFloor((float)mNewWidthBox->value()/mNumPatchesH->value());

	mPatchResLabel->setText(tr("Patch Resolution: %1 px").arg(patchResD));
	mPatchResLabel->show();

	// show the user if we can work with the thumbnails or not
	if (patchResD > 97)
		mPatchResLabel->setProperty("warning", true);
	else
		mPatchResLabel->setProperty("warning", false);

	mPatchResLabel->style()->unpolish(mPatchResLabel);
	mPatchResLabel->style()->polish(mPatchResLabel);
	mPatchResLabel->update();
}

QImage DkMosaicDialog::getImage() {

	if (mMosaic.isNull() && !mMosaicMat.empty())
		return DkImage::mat2QImage(mMosaicMat);

	return mMosaic;
}

void DkMosaicDialog::reject() {

	// not sure if this is a nice way to do: but we change cancel behavior while processing
	if (mProcessing)
		mProcessing = false;
	else if (!mMosaic.isNull() && !mButtons->button(QDialogButtonBox::Apply)->isEnabled()) {
		mButtons->button(QDialogButtonBox::Apply)->setEnabled(true);
		enableAll(true);
		mViewport->show();
		mSliderWidget->hide();
	}
	else
		QDialog::reject();

}

void DkMosaicDialog::buttonClicked(QAbstractButton* button) {

	if (button == mButtons->button(QDialogButtonBox::Save)) {

		// render the full image
		if (!mMosaic.isNull()) {
			mSliderWidget->hide();
			mProgress->setValue(mProgress->minimum());
			mProgress->show();
			enableAll(false);
			button->setEnabled(false);
			
			QFuture<bool> future = QtConcurrent::run(this, 
				&nmc::DkMosaicDialog::postProcessMosaic,
				mDarkenSlider->value()/100.0f,
				mLightenSlider->value()/100.0f, 
				mSaturationSlider->value()/100.0f,
				false);
			mPostProcessWatcher.setFuture(future);
		}
	}
	else if (button == mButtons->button(QDialogButtonBox::Apply))
		compute();
}

void DkMosaicDialog::compute() {

	if (mPostProcessing)
		return;

	mProgress->setValue(mProgress->minimum());
	mProgress->show();
	mMsgLabel->setText("");
	mMsgLabel->show();
	mMosaicMatSmall.release();
	mMosaicMat.release();
	mOrigImg.release();
	mMosaic = QImage();
	mSliderWidget->hide();
	mViewport->show();
	mPreview->setForceFastRendering(true);
	mPreview->show();

	enableAll(false);

	QString suffixTmp = mSuffixBox->currentText();
	QString suffix;

	for (int idx = 0; idx < DkSettings::app.fileFilters.size(); idx++) {
		if (suffixTmp.contains("(" + DkSettings::app.fileFilters.at(idx))) {
			suffix = DkSettings::app.fileFilters.at(idx);
			break;
		}
	}

	QString filter = mFilterEdit->text();
	mFilesUsed.clear();

	mProcessing = true;
	QFuture<int> future = QtConcurrent::run(this, 
		&nmc::DkMosaicDialog::computeMosaic,
		filter,
		suffix, 
		mNewWidthBox->value(), 
		mNumPatchesH->value());
	mMosaicWatcher.setFuture(future);

	//// debug
	//computeMosaic(
	//	cFile,
	//	filter,
	//	suffix,
	//	fromPage->value(), 
	//	toPage->value(),
	//	overwrite->isChecked());

}

void DkMosaicDialog::mosaicFinished() {

	mProgress->hide();
	//msgLabel->hide();
	
	if (!mMosaicMat.empty()) {
		mSliderWidget->show();
		mMsgLabel->hide();
		mViewport->hide();
		mPreview->setForceFastRendering(false);

		updatePostProcess();	// add values
		mButtons->button(QDialogButtonBox::Save)->setEnabled(true);
	}
	else
		enableAll(true);
}

int DkMosaicDialog::computeMosaic(const QString& filter, const QString& suffix, int newWidth, int numPatchesH) {

	DkTimer dt;

	// compute new image size
	cv::Mat mImg = DkImage::qImage2Mat(mLoader.image());

	QSize numPatches = QSize(numPatchesH, 0);

	// compute new image size
	//float aratio = (float)mImg.rows/mImg.cols;
	int patchResO = qFloor((float)mImg.cols/numPatches.width());
	numPatches.setHeight(qFloor((float)mImg.rows/patchResO));

	int patchResD = qFloor(newWidth/numPatches.width());

	float shR = (mImg.rows-patchResO*numPatches.height())/2.0f;
	float shC = (mImg.cols-patchResO*numPatches.width())/2.0f;

	mImg = mImg.rowRange(qFloor(shR), mImg.rows-qCeil(shR)).colRange(qFloor(shC), mImg.cols-qCeil(shC));
	cv::Mat mImgLab;
	cv::cvtColor(mImg, mImgLab, CV_RGB2Lab);
	std::vector<cv::Mat> channels;
	cv::split(mImgLab, channels);
	cv::Mat imgL = channels[0];

	// keeps track of the weights
	cv::Mat cc(numPatches.height(), numPatches.width(), CV_32FC1);
	cc.setTo(0);
	cv::Mat ccD(numPatches.height(), numPatches.width(), CV_8UC1);	// tells us if we have already computed the real patch

	mFilesUsed.resize(numPatches.height()*numPatches.width());

	// destination image
	cv::Mat dImg(patchResD*numPatches.height(), patchResD*numPatches.width(), CV_8UC1);
	dImg = 255;

	// patch image (preview)
	cv::Mat pImg(patchResO*numPatches.height(), patchResO*numPatches.width(), CV_8UC1);
	pImg = 255;

	qDebug() << "mosaic data --------------------------------";
	qDebug() << "patchRes: " << patchResD;
	qDebug() << "new resolution: " << dImg.cols << " x " << dImg.rows;
	qDebug() << "num patches: " << numPatches.width() << " x " << numPatches.height();
	qDebug() << "mosaic data --------------------------------";

	// progress bar
	int pIdx = 0;
	int maxP = numPatches.width()*numPatches.height();

	int iDidNothing = 0;
	bool force = false;
	bool useTwice = false;

	while (iDidNothing < 10000) {

		if (!mProcessing)
			return QDialog::Rejected;

		if (iDidNothing > 100) {
			force = true;

			if (!useTwice)
				emit infoMessage(tr("Filling empty areas..."));
		}

		if (iDidNothing > 400 && !useTwice) {
			emit infoMessage(tr("I need to use some images twice - maybe the database is too small?"));
			iDidNothing = 0;
			useTwice = true;
		}
		else if (iDidNothing > 400) {
			emit infoMessage(tr("Sorry, it seems that i cannot create your mosaic with this database."));
			return QDialog::Rejected;
		}

		QString imgPath = getRandomImagePath(mSavePath, filter, suffix);

		if (!useTwice && mFilesUsed.contains(QFileInfo(imgPath))) {
			iDidNothing++;
			continue;
		}

		try {

			DkThumbNail thumb = DkThumbNail(imgPath);
			thumb.setMinThumbSize(patchResO);
			//thumb.setRescale(false);
			thumb.compute();

			if (!thumb.hasImage()) {
				iDidNothing++;
				continue;
			}

			cv::Mat ccTmp(cc.size(), cc.depth());
		
			if (!force)
				ccTmp.setTo(0);
			else
				ccTmp = cc.clone();

			cv::Mat thumbPatch = createPatch(thumb, patchResO);
		
			if (thumbPatch.rows != patchResO || thumbPatch.cols != patchResO) {
				iDidNothing++;
				continue;
			}
		
			matchPatch(imgL, thumbPatch, patchResO, ccTmp);

			if (force) {
				cv::Mat mask = (cc == 0);
				mask.convertTo(mask, CV_32FC1, 1.0f/255.0f);
				ccTmp = ccTmp.mul(mask);
			}
				
			double maxVal = 0;
			cv::Point maxIdx;
			cv::minMaxLoc(ccTmp, 0, &maxVal, 0, &maxIdx);
			float* ccPtr = cc.ptr<float>(maxIdx.y);

			if (maxVal > ccPtr[maxIdx.x]) {

				cv::Mat pPatch = pImg.rowRange(maxIdx.y*patchResO, maxIdx.y*patchResO+patchResO)
					.colRange(maxIdx.x*patchResO, maxIdx.x*patchResO+patchResO);
				thumbPatch.copyTo(pPatch);
			
				// visualize
				if (pIdx % 10 == 0) {
				
					channels[0] = pImg;
				
					//debug
					cv::Mat imgT3;
					cv::merge(channels, imgT3);
					cv::cvtColor(imgT3, imgT3, CV_Lab2BGR);
					emit updateImage(DkImage::mat2QImage(imgT3));
				}

				if (ccPtr[maxIdx.x] == 0) {
					pIdx++;
					emit updateProgress(qRound((float)pIdx/maxP*100));
				}

				// compute it now if we already have the full image loaded
				if (thumb.getImage().width() > patchResD && thumb.getImage().height() > patchResD) {
					thumbPatch = createPatch(thumb, patchResD);

					cv::Mat dPatch = dImg.rowRange(maxIdx.y*patchResD, maxIdx.y*patchResD+patchResD)
						.colRange(maxIdx.x*patchResD, maxIdx.x*patchResD+patchResD);
					thumbPatch.copyTo(dPatch);
					ccD.ptr<unsigned char>(maxIdx.y)[maxIdx.x] = 1;
				}
				else
					ccD.ptr<unsigned char>(maxIdx.y)[maxIdx.x] = 0;

				// update cc
				ccPtr[maxIdx.x] = (float)maxVal;

				mFilesUsed[maxIdx.y*numPatchesH+maxIdx.x] = thumb.getFilePath();	// replaces additionally the old file
				iDidNothing = 0;
			}
			else
				iDidNothing++;
		} 
		// catch cv exceptions e.g. out of memory
		catch(...) {
			iDidNothing++;
		}

		// are we done yet?
		double minVal = 0;
		cv::minMaxLoc(cc, &minVal);

		if (minVal > 0)
			break;
		
	}

	pIdx = 0;

	// compute real resolution
	for (int rIdx = 0; rIdx < ccD.rows; rIdx++) {

		const unsigned char* ccDPtr = ccD.ptr<unsigned char>(rIdx);

		for (int cIdx = 0; cIdx < ccD.cols; cIdx++) {

			// is the patch already computed?
			if (ccDPtr[cIdx])
				continue;

			QFileInfo cFile = mFilesUsed.at(rIdx*ccD.cols+cIdx);

			if (!cFile.exists()) {
				emit infoMessage(tr("Something is seriously wrong, I could not load: %1").arg(cFile.absoluteFilePath()));
				continue;
			}

			cv::Mat thumbPatch = createPatch(DkThumbNail(cFile.absoluteFilePath()), patchResD);

			cv::Mat dPatch = dImg.rowRange(rIdx*patchResD, rIdx*patchResD+patchResD)
				.colRange(cIdx*patchResD, cIdx*patchResD+patchResD);
			thumbPatch.copyTo(dPatch);
			emit updateProgress(qRound((float)pIdx/maxP*100));
			pIdx++;
		}
	}

	qDebug() << "I fully rendered: " << ccD.rows*ccD.cols-cv::sum(ccD)[0] << " images";

	// create final images
	mOrigImg = mImgLab;
	mMosaicMat = dImg;
	mMosaicMatSmall = pImg;

	mProcessing = false;

	qDebug() << "mosaic computed in: " << dt.getTotal();

	return QDialog::Accepted;
}

void DkMosaicDialog::matchPatch(const cv::Mat& img, const cv::Mat& thumb, int patchRes, cv::Mat& cc) {

	for (int rIdx = 0; rIdx < cc.rows; rIdx++) {

		float* ccPtr = cc.ptr<float>(rIdx);
		const cv::Mat imgStrip = img.rowRange(rIdx*patchRes, rIdx*patchRes+patchRes);

		for (int cIdx = 0; cIdx < cc.cols; cIdx++) {

			// already computed?
			if (ccPtr[cIdx] != 0)
				continue;

			const cv::Mat cPatch = imgStrip.colRange(cIdx*patchRes, cIdx*patchRes+patchRes);
			
			cv::Mat absDiff;
			cv::absdiff(cPatch, thumb, absDiff);
			ccPtr[cIdx] = 1.0f-((float)cv::sum(absDiff)[0]/(patchRes*patchRes*255));
		}
	}
}

cv::Mat DkMosaicDialog::createPatch(const DkThumbNail& thumb, int patchRes) {

	QImage img;

	// load full image if we have not enough resolution
	if (qMin(thumb.getImage().width(), thumb.getImage().height()) < patchRes) {
		DkBasicLoader loader;
		loader.loadGeneral(thumb.getFilePath(), true, true);
		img = loader.image();
	}
	else
		img = thumb.getImage();

	cv::Mat cvThumb = DkImage::qImage2Mat(img);
	cv::cvtColor(cvThumb, cvThumb, CV_RGB2Lab);
	std::vector<cv::Mat> channels;
	cv::split(cvThumb, channels);
	cvThumb = channels[0];
	channels.clear();

	// make square
	if (cvThumb.rows != cvThumb.cols) {

		if (cvThumb.rows > cvThumb.cols) {
			float sh = (cvThumb.rows - cvThumb.cols)/2.0f;
			cvThumb = cvThumb.rowRange(qFloor(sh), cvThumb.rows-qCeil(sh));
		}
		else {
			float sh = (cvThumb.cols - cvThumb.rows)/2.0f;
			cvThumb = cvThumb.colRange(qFloor(sh), cvThumb.cols-qCeil(sh));
		}
	}

	if (cvThumb.rows < patchRes || cvThumb.cols < patchRes)
		qDebug() << "enlarging thumbs!!";

	cv::resize(cvThumb, cvThumb, cv::Size(patchRes, patchRes), 0.0, 0.0, CV_INTER_AREA);

	return cvThumb;
}

QString DkMosaicDialog::getRandomImagePath(const QString& cPath, const QString& ignore, const QString& suffix) {

	// TODO: remove hierarchy

	QStringList fileFilters = (suffix.isEmpty()) ? DkSettings::app.fileFilters : QStringList(suffix);

	// get all dirs
	QFileInfoList entries = QDir(cPath).entryInfoList(QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
	//qDebug() << entries;
	// get all files
	entries += QDir(cPath).entryInfoList(fileFilters);
	//qDebug() << "current entries: " << e;

	if (!ignore.isEmpty()) {

		QStringList ignoreList = ignore.split(";");
		QFileInfoList entriesTmp = entries;
		entries.clear();

		for (int idx = 0; idx < entriesTmp.size(); idx++) {
			
			bool lIgnore = false;
			QString p = entriesTmp.at(idx).absoluteFilePath();

			for (int iIdx = 0; iIdx < ignoreList.size(); iIdx++) {
				if (p.contains(ignoreList.at(iIdx))) {
					lIgnore = true;
					break;
				}
			}

			if (!lIgnore)
				entries.append(entriesTmp.at(idx));
		}
	}
		

	if (entries.isEmpty())
		return QString();

	int rIdx = qRound((float)qrand()/RAND_MAX*(entries.size()-1));

	//qDebug() << "rand index: " << rIdx;

	QFileInfo rPath = entries.at(rIdx);
	//qDebug() << rPath.absoluteFilePath();

	if (rPath.isDir())
		return getRandomImagePath(rPath.absoluteFilePath(), ignore, suffix);
	else
		return rPath.absoluteFilePath();
}

void DkMosaicDialog::updatePostProcess() {
	
	if (mMosaicMat.empty() || mProcessing)
		return;

	if (mPostProcessing) {
		mUpdatePostProcessing = true;
		return;
	}

	mButtons->button(QDialogButtonBox::Apply)->setEnabled(false);
	mButtons->button(QDialogButtonBox::Save)->setEnabled(false);

	QFuture<bool> future = QtConcurrent::run(this, 
		&nmc::DkMosaicDialog::postProcessMosaic,
		mDarkenSlider->value()/100.0f,
		mLightenSlider->value()/100.0f, 
		mSaturationSlider->value()/100.0f,
		true);
	mPostProcessWatcher.setFuture(future);

	mUpdatePostProcessing = false;
	//postProcessMosaic(darkenSlider->value()/100.0f, lightenSlider->value()/100.0f, saturationSlider->value()/100.0f);
}

void DkMosaicDialog::postProcessFinished() {

	if (mPostProcessWatcher.result()) {
		QDialog::accept();
	}
	else if (mUpdatePostProcessing)
		updatePostProcess();
	else {
		mButtons->button(QDialogButtonBox::Save)->setEnabled(true);
	}
}

bool DkMosaicDialog::postProcessMosaic(float multiply /* = 0.3 */, float screen /* = 0.5 */, float saturation, bool computePreview) {

	mPostProcessing = true;

	qDebug() << "darken: " << multiply << " lighten: " << screen;

	cv::Mat origR;
	cv::Mat mosaicR;

	try {
		if (computePreview) {
			origR = mOrigImg.clone();
			mosaicR = mMosaicMatSmall.clone();
		}
		else {
			cv::resize(mOrigImg, origR, mMosaicMat.size(), 0, 0, CV_INTER_LANCZOS4);
			mosaicR = mMosaicMat;
			mOrigImg.release();
		}

		// multiply the two images
		for (int rIdx = 0; rIdx < origR.rows; rIdx++) {

			const unsigned char* mosaicPtr = mosaicR.ptr<unsigned char>(rIdx);
			unsigned char* origPtr = origR.ptr<unsigned char>(rIdx);

			if (!computePreview)
				emit updateProgress(qRound((float)rIdx/origR.rows*100));

			for (int cIdx = 0; cIdx < origR.cols; cIdx++) {

				// mix the luminance channel
				float mosaic = mosaicPtr[cIdx]/255.0f;
				float luminance = (*origPtr)/255.0f;

			
				float lighten = (1.0f-luminance)*screen + (1.0f-screen);
				lighten *= 1.0f-mosaic;	// multiply inverse
				lighten = 1.0f-lighten;

				float darken = luminance*multiply + (1.0f-multiply);
				darken *= lighten;	// mix with the mosaic pixel

				// now stretch to the dynamic range and save it
				*origPtr = (unsigned char)qRound(darken*255.0f);

				// now adopt the saturation
				origPtr++;
				*origPtr = (unsigned char)qRound((*origPtr-128) * saturation)+128;
				origPtr++;
				*origPtr = (unsigned char)qRound((*origPtr-128) * saturation)+128;
				origPtr++;
			}
		}

		//if (!computePreview)
		//	mosaicMat.release();
		cv::cvtColor(origR, origR, CV_Lab2BGR);
		qDebug() << "color converted";

		mMosaic = DkImage::mat2QImage(origR);
		qDebug() << "mosaicing computed...";

	}
	catch(...) {
		origR.release();

		QMessageBox::critical(QApplication::activeWindow(), tr("Error"), tr("Sorry, I could not mix the image..."));
		qDebug() << "exception caught...";
		mMosaic = DkImage::mat2QImage(mMosaicMat);
	}
	
	if (computePreview)
		mPreview->setImage(mMosaic);

	mPostProcessing = false;

	return !computePreview;

}

void DkMosaicDialog::setFile(const QString& filePath) {

	QFileInfo fInfo(filePath);
	if (!fInfo.exists())
		return;

	mFilePath = filePath;
	mSavePath = fInfo.absolutePath();
	mFolderLabel->setText(mSavePath);
	mFileLabel->setText(filePath);

	mLoader.loadGeneral(filePath, true);
	mViewport->setImage(mLoader.image());

	enableMosaicSave(mLoader.hasImage());

	//newWidthBox->blockSignals(true);
	//newHeightBox->blockSignals(true);
	mNewWidthBox->setValue(mLoader.image().width());
	mNumPatchesH->setValue(qFloor((float)mLoader.image().width()/90));	// 130 is a pretty good patch resolution
	mNumPatchesH->setMaximum(qMin(1000, qFloor(mLoader.image().width()*0.5f)));
	mNumPatchesV->setMaximum(qMin(1000, qFloor(mLoader.image().height()*0.5f)));
	//newHeightBox->setValue(loader.image().height());
	//newWidthBox->blockSignals(false);
	//newHeightBox->blockSignals(false);

	//fromPage->setRange(1, loader.getNumPages());
	//toPage->setRange(1, loader.getNumPages());

	//fromPage->setValue(1);
	//toPage->setValue(loader.getNumPages());
}

void DkMosaicDialog::enableAll(bool enable) {

	enableMosaicSave(enable);
	mControlWidget->setEnabled(enable);
}

void DkMosaicDialog::enableMosaicSave(bool enable) {

	mFilterEdit->setEnabled(enable);
	mSuffixBox->setEnabled(enable);
	mNewWidthBox->setEnabled(enable);
	mNewHeightBox->setEnabled(enable);
	mNumPatchesH->setEnabled(enable);
	mNumPatchesV->setEnabled(enable);
	mButtons->button(QDialogButtonBox::Apply)->setEnabled(enable);

	if (!enable)
		mButtons->button(QDialogButtonBox::Save)->setEnabled(enable);
}
#endif
// DkForceThumbDialog --------------------------------------------------------------------
DkForceThumbDialog::DkForceThumbDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	createLayout();
}

void DkForceThumbDialog::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);

	infoLabel = new QLabel();
	infoLabel->setAlignment(Qt::AlignHCenter);

	cbForceSave = new QCheckBox(tr("Overwrite Existing Thumbnails"));
	cbForceSave->setToolTip("If checked, existing thumbnails will be replaced");

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(infoLabel);
	layout->addWidget(cbForceSave);
	layout->addWidget(buttons);

}

bool DkForceThumbDialog::forceSave() const {

	return cbForceSave->isChecked();
}

void DkForceThumbDialog::setDir(const QDir& fileInfo) {

	infoLabel->setText(tr("Compute thumbnails for all images in:\n %1\n").arg(fileInfo.absolutePath()));
}

// Welcome dialog --------------------------------------------------------------------
DkWelcomeDialog::DkWelcomeDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {

	setWindowTitle(tr("Welcome"));
	createLayout();
	languageChanged = false;
}

void DkWelcomeDialog::createLayout() {

	QGridLayout* layout = new QGridLayout(this);

	QLabel* welcomeLabel = new QLabel(tr("Welcome to nomacs, please choose your preferred language below."), this);

	languageCombo = new QComboBox(this);
	DkUtils::addLanguages(languageCombo, languages);

	registerFilesCheckBox = new QCheckBox(tr("Register File Associations"), this);
	registerFilesCheckBox->setChecked(true);

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	
	layout->addItem(new QSpacerItem(10, 10), 0, 0, -1, -1);
	layout->addWidget(welcomeLabel, 1, 0, 1, 3);
	layout->addItem(new QSpacerItem(10, 10), 2, 0, -1, -1);
	layout->addWidget(languageCombo, 3, 1);

#ifdef WIN32
	layout->addWidget(registerFilesCheckBox, 4, 1);
#else
	registerFilesCheckBox->setChecked(false);
	registerFilesCheckBox->hide();
#endif
	
	layout->addWidget(buttons, 5, 0, 1, 3);
}

void DkWelcomeDialog::accept() {

	DkFileFilterHandling fh;

	// register file associations
	if (registerFilesCheckBox->isChecked()) {

		QStringList rFilters = DkSettings::app.openFilters;
		
		for (const QString& filter : DkSettings::app.containerFilters)
			rFilters.removeAll(filter);

		for (const QString& filter : rFilters) {

			// remove the icon file -> otherwise icons might be destroyed (e.g. acrobat)
			if (!filter.contains("ico"))	
				fh.registerFileType(filter, tr("Image"), true);
		}
	}
	
	fh.registerNomacs(true);	// register nomacs again - to be save

	// change language
	if (languageCombo->currentIndex() != languages.indexOf(DkSettings::global.language) && languageCombo->currentIndex() >= 0) {
		DkSettings::global.language = languages.at(languageCombo->currentIndex());
		languageChanged = true;
	}

	QDialog::accept();
}

bool DkWelcomeDialog::isLanguageChanged() {

	return languageChanged;
}


// archive extraction dialog --------------------------------------------------------------------
#ifdef WITH_QUAZIP
DkArchiveExtractionDialog::DkArchiveExtractionDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	
	mFileList = QStringList();
	setWindowTitle(tr("Extract images from an archive"));
	createLayout();
	setMinimumSize(340, 400);
	setAcceptDrops(true);
}

void DkArchiveExtractionDialog::createLayout() {

	// archive file path
	QLabel* archiveLabel = new QLabel(tr("Archive (%1)").arg(DkSettings::app.containerRawFilters.replace(" *", ", *")), this);
	mArchivePathEdit = new QLineEdit(this);
	mArchivePathEdit->setObjectName("DkWarningEdit");
	mArchivePathEdit->setValidator(&mFileValidator);
	connect(mArchivePathEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
	connect(mArchivePathEdit, SIGNAL(editingFinished()), this, SLOT(loadArchive()));

	QPushButton* openArchiveButton = new QPushButton(tr("&Browse"));
	connect(openArchiveButton, SIGNAL(pressed()), this, SLOT(openArchive()));

	// dir file path
	QLabel* dirLabel = new QLabel(tr("Extract to"));
	mDirPathEdit = new QLineEdit();
	mDirPathEdit->setValidator(&mFileValidator);
	connect(mDirPathEdit, SIGNAL(textChanged(const QString&)), this, SLOT(dirTextChanged(const QString&)));

	QPushButton* openDirButton = new QPushButton(tr("&Browse"));
	connect(openDirButton, SIGNAL(pressed()), this, SLOT(openDir()));

	mFeedbackLabel = new QLabel("", this);
	mFeedbackLabel->setObjectName("DkDecentInfo");

	mFileListDisplay = new QListWidget(this);

	mRemoveSubfolders = new QCheckBox(tr("Remove Subfolders"), this);
	mRemoveSubfolders->setChecked(false);
	connect(mRemoveSubfolders, SIGNAL(stateChanged(int)), this, SLOT(checkbocChecked(int)));

	// mButtons
	mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	mButtons->button(QDialogButtonBox::Ok)->setText(tr("&Extract"));
	mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
	mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));

	QWidget* extractWidget = new QWidget(this);
	QGridLayout* gdLayout = new QGridLayout(extractWidget);
	gdLayout->addWidget(archiveLabel, 0, 0);
	gdLayout->addWidget(mArchivePathEdit, 1, 0);
	gdLayout->addWidget(openArchiveButton, 1, 1);
	gdLayout->addWidget(dirLabel, 2, 0);
	gdLayout->addWidget(mDirPathEdit, 3, 0);
	gdLayout->addWidget(openDirButton, 3, 1);
	gdLayout->addWidget(mFeedbackLabel, 4, 0, 1, 2);
	gdLayout->addWidget(mFileListDisplay, 5, 0, 1, 2);
	gdLayout->addWidget(mRemoveSubfolders, 6, 0, 1, 2);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(extractWidget);
	layout->addWidget(mButtons);
}

void DkArchiveExtractionDialog::setCurrentFile(const QString& filePath, bool isZip) {

	userFeedback("", false);
	mArchivePathEdit->setText("");
	mDirPathEdit->setText("");
	mFileListDisplay->clear();
	mRemoveSubfolders->setChecked(false);

	mFilePath = filePath;
	if (isZip) {
		mArchivePathEdit->setText(mFilePath);
		loadArchive();
	}
};

void DkArchiveExtractionDialog::textChanged(const QString& text) {
	
	bool oldStyle = mArchivePathEdit->property("error").toBool();
	bool newStyle = false;

	if (QFileInfo(text).exists() && DkBasicLoader::isContainer(text)) {
		newStyle = false;
		mArchivePathEdit->setProperty("error", newStyle);
		loadArchive(text);
	}
	else {
		newStyle = true;
		mArchivePathEdit->setProperty("error", newStyle);
		userFeedback("", false);	
		mFileListDisplay->clear();
		mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
	}

	if (oldStyle != newStyle) {
		mArchivePathEdit->style()->unpolish(mArchivePathEdit);
		mArchivePathEdit->style()->polish(mArchivePathEdit);
		mArchivePathEdit->update();
	}


}

void DkArchiveExtractionDialog::dirTextChanged(const QString& text) {
	
	if (text.isEmpty()) {
		userFeedback("", false);
		mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

void DkArchiveExtractionDialog::checkbocChecked(int) {

	loadArchive();
}

void DkArchiveExtractionDialog::openArchive() {

	// load system default open dialog
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Archive"),
		(mArchivePathEdit->text().isEmpty()) ? QFileInfo(mFilePath).absolutePath() : mArchivePathEdit->text(), tr("Archives (%1)").arg(DkSettings::app.containerRawFilters.remove(",")));

	if (QFileInfo(filePath).exists()) {
		mArchivePathEdit->setText(filePath);
		loadArchive(filePath);
	}
}

void DkArchiveExtractionDialog::openDir() {

	// load system default open dialog
	QString filePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		(mDirPathEdit->text().isEmpty()) ? QFileInfo(mFilePath).absolutePath() : mDirPathEdit->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (QFileInfo(filePath).exists())
		mDirPathEdit->setText(filePath);
}

void DkArchiveExtractionDialog::userFeedback(const QString& msg, bool error) {

	if (!error)
		mFeedbackLabel->setProperty("warning", false);
	else
		mFeedbackLabel->setProperty("warning", true);

	mFeedbackLabel->setText(msg);
	mFeedbackLabel->style()->unpolish(mFeedbackLabel);
	mFeedbackLabel->style()->polish(mFeedbackLabel);
	mFeedbackLabel->update();
}

void DkArchiveExtractionDialog::loadArchive(const QString& filePath) {

	mFileList = QStringList();
	mFileListDisplay->clear();

	QString lFilePath = filePath;
	if (lFilePath.isEmpty())
		lFilePath = mArchivePathEdit->text();

	QFileInfo fileInfo(lFilePath);
	if (!fileInfo.exists())
		return;

	if (!DkBasicLoader::isContainer(lFilePath)) {
		userFeedback(tr("Not a valid archive."), true);
		return;
	}

	if (mDirPathEdit->text().isEmpty()) {

		mDirPathEdit->setText(lFilePath.remove("." + fileInfo.suffix()));
		mDirPathEdit->setFocus();
	}

	QStringList fileNameList = JlCompress::getFileList(lFilePath);
	
	// remove the * in fileFilters
	QStringList fileFiltersClean = DkSettings::app.browseFilters;
	for (int idx = 0; idx < fileFiltersClean.size(); idx++)
		fileFiltersClean[idx].replace("*", "");

	for (int idx = 0; idx < fileNameList.size(); idx++) {
		
		for (int idxFilter = 0; idxFilter < fileFiltersClean.size(); idxFilter++) {

			if (fileNameList.at(idx).contains(fileFiltersClean[idxFilter], Qt::CaseInsensitive)) {
				mFileList.append(fileNameList.at(idx));
				break;
			}
		}
	}

	if (mFileList.size() > 0)
		userFeedback(tr("Number of images: ") + QString::number(mFileList.size()), false);
	else {
		userFeedback(tr("The archive does not contain any images."), false);
		return;
	}

	mFileListDisplay->addItems(mFileList);

	if (mRemoveSubfolders->checkState() == Qt::Checked) {
		for (int i = 0; i < mFileListDisplay->count(); i++) {

			QFileInfo fi(mFileListDisplay->item(i)->text());
			mFileListDisplay->item(i)->setText(fi.fileName());
		}
	}
	mFileListDisplay->update();

	mButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void DkArchiveExtractionDialog::accept() {

	QStringList extractedFiles = extractFilesWithProgress(mArchivePathEdit->text(), mFileList, mDirPathEdit->text(), mRemoveSubfolders->isChecked());

	if ((extractedFiles.isEmpty() || extractedFiles.size() != mFileList.size()) && !extractedFiles.contains("userCanceled")) {
		
		QMessageBox msgBox(this);
		msgBox.setText(tr("The images could not be extracted!"));
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
	}

	QDialog::accept();
}

void DkArchiveExtractionDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		qDebug() << "dropping: " << url;
		url = url.toLocalFile();

		if (QFileInfo(url.toString()).isFile()) {
			mArchivePathEdit->setText(url.toString());
			loadArchive(url.toString());
		}
		else
			mDirPathEdit->setText(url.toString());
	}
}

void DkArchiveExtractionDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists())
			event->acceptProposedAction();
	}

}

QStringList DkArchiveExtractionDialog::extractFilesWithProgress(const QString& fileCompressed, const QStringList& files, const QString& dir, bool removeSubfolders) {

    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size() - 1);
    progressDialog.setWindowTitle(tr("Extracting files..."));
	progressDialog.setWindowModality(Qt::WindowModal);
	progressDialog.setModal(true);
	progressDialog.hide();
	progressDialog.show();

    QStringList extracted;
    for (int i=0; i<files.count(); i++) {
		progressDialog.setValue(i);
		progressDialog.setLabelText(tr("Extracting file %1 of %2").arg(i + 1).arg(files.size()));

		QString absPath;
		if(removeSubfolders)
			absPath = QDir(dir).absoluteFilePath(QFileInfo(files.at(i)).fileName());
		else
			absPath = QDir(dir).absoluteFilePath(files.at(i));

		if (JlCompress::extractFile(fileCompressed, files.at(i), absPath).isEmpty()) {
			qDebug() << "unable to extract:" << files.at(i);
			//return QStringList();
		}
        extracted.append(absPath);
		if(progressDialog.wasCanceled()) {
			return QStringList("userCanceled");
		}
    }

	progressDialog.close();

    return extracted;
}

#endif

} // close namespace
