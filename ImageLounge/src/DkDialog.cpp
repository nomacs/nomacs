/*******************************************************************************************************
 DkImage.h
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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


namespace nmc {

// DkSplashScreen --------------------------------------------------------------------
DkSplashScreen::DkSplashScreen(QWidget* parent, Qt::WFlags flags) : QDialog(0, flags) {

	QPixmap img(":/nomacs/img/splash-screen.png");
	setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_TranslucentBackground);

	imgLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	imgLabel->setPixmap(img);
	imgLabel->show();

	// create exit shortcuts
	QShortcut* escExit = new QShortcut(Qt::Key_Escape, this);
	QObject::connect(escExit, SIGNAL( activated ()), this, SLOT( close() ));

	// set the text
	text = 
		QString("Flo was here und wünscht<br>" 
		"Stefan fiel Spaß während<br>" 
		"Markus rockt... <br><br>" 

		"<a href=\"http://www.nomacs.org\">www.nomacs.org</a><br>"
		"<a href=\"mailto:developers@nomacs.org\">developers@nomacs.org</a><br><br>" 

		"This program is licensed under GNU Public License v3<br>"
		"&#169; Markus Diem, Stefan Fiel and Florian Kleber, 2011-2013<br><br>"

		"Press [ESC] to exit");

	textLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	textLabel->setScaledContents(true);
	textLabel->setTextFormat(Qt::RichText);
	textLabel->setText(text);
	textLabel->move(131, 280);
	textLabel->setOpenExternalLinks(true);

	QLabel* versionLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	versionLabel->setTextFormat(Qt::RichText);

	QString platform = "";
#ifdef _WIN64
	platform = " [x64] ";
#elif defined _WIN32
	platform = " [x86] ";
#endif

	versionLabel->setText("Version: " % QApplication::applicationVersion() % platform %  "<br>"
#ifdef WITH_OPENCV
		"RAW support: Yes"
#else
		"RAW support: No"
#endif  
		);

	versionLabel->move(360, 280);

	qDebug() << "splash screen created...";
}

//void DkSplashScreen::mousePressEvent(QMouseEvent* event) {
//
//	close();
//}




// tiff dialog --------------------------------------------------------------------
DkTifDialog::DkTifDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	init();
}

void DkTifDialog::init() {

	isOk = false;
	setWindowTitle("TIF compression");
	setFixedSize(270, 146);
	setLayout(new QVBoxLayout(this));

	//QWidget* buttonWidget = new QWidget(this);
	QGroupBox* buttonWidget = new QGroupBox(tr("TIF compression"), this);
	QVBoxLayout* vBox = new QVBoxLayout(buttonWidget);
	QButtonGroup* bGroup = new QButtonGroup(buttonWidget);
	noCompressionButton = new QRadioButton( tr("&no compression"), this);
	compressionButton = new QRadioButton(tr("&LZW compression (lossless)"), this);
	compressionButton->setChecked(true);
	bGroup->addButton(noCompressionButton);
	bGroup->addButton(compressionButton);

	vBox->addWidget(noCompressionButton);
	vBox->addWidget(compressionButton);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout()->addWidget(buttonWidget);
	layout()->addWidget(buttons);
}


//
DkCompressDialog::DkCompressDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	dialogMode = jpg_dialog;	// default
	bgCol = QColor(255,255,255);

	setFixedSize(600, 450);
	createLayout();
	init();

}

void DkCompressDialog::init() {

	hasAlpha = false;
	img = 0;

	if (dialogMode == jpg_dialog) {
		setWindowTitle("JPG Settings");
		cbLossless->hide();
		slider->setEnabled(true);
	}
	else if (dialogMode == webp_dialog) {
		setWindowTitle("WEBP Settings");
		colChooser->setEnabled(false);
		//colChooser->hide();
		cbLossless->show();
		losslessCompression(cbLossless->isChecked());
	}	

}

void DkCompressDialog::createLayout() {

	QLabel* origLabelText = new QLabel(tr("Original"));
	QLabel* newLabel = new QLabel(tr("New"));

	// shows the original image
	origView = new DkBaseViewPort(this);
	origView->setForceFastRendering(true);
	origView->setPanControl(QPointF(0.0f, 0.0f));
	connect(origView, SIGNAL(imageUpdated()), this, SLOT(drawPreview()));

	//// maybe we should report this: 
	//// if a stylesheet (with border) is set, the var
	//// cornerPaintingRect in QAbstractScrollArea (which we don't even need : )
	//// is invalid which blocks re-paints unless the widget gets a focus...
	//origView->setStyleSheet("QViewPort{border: 1px solid #888;}");

	// shows the preview
	previewLabel = new QLabel();
	//previewLabel->setStyleSheet("QLabel{border: 1px solid #888;}");

	// slider
	slider = new DkSlider(tr("Image Quality"));
	slider->setValue(80);
	slider->setTickInterval(10);
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(drawPreview()));

	// lossless
	cbLossless = new QCheckBox(tr("Lossless Compression"));
	connect(cbLossless, SIGNAL(toggled(bool)), this, SLOT(losslessCompression(bool)));
	
	previewSizeLabel = new QLabel();

	// color chooser
	colChooser = new DkColorChooser(bgCol, tr("Background Color"));
	colChooser->setEnabled(hasAlpha);
	colChooser->enableAlpha(false);
	connect(colChooser, SIGNAL(accepted()), this, SLOT(newBgCol()));

	QWidget* dummy = new QWidget();
	QHBoxLayout* dummyLayout = new QHBoxLayout(dummy);
	dummyLayout->addWidget(colChooser);
	dummyLayout->addStretch();
	dummyLayout->addWidget(previewSizeLabel);

	QWidget* previewWidget = new QWidget();
	QGridLayout* previewLayout = new QGridLayout(previewWidget);
	previewLayout->setAlignment(Qt::AlignHCenter);
	previewLayout->setHorizontalSpacing(20);

	previewLayout->addWidget(origLabelText, 0, 0);
	previewLayout->addWidget(newLabel, 0, 1);
	previewLayout->addWidget(origView, 1, 0);
	previewLayout->addWidget(previewLabel, 1, 1);
	previewLayout->addWidget(slider, 2, 0);
	previewLayout->addWidget(dummy, 2, 1);
	previewLayout->addWidget(cbLossless, 3, 0);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(previewWidget);
	//layout->addStretch(30);
	layout->addWidget(buttons);

}

void DkCompressDialog::updateSnippets() {

	if (!img)
		return;

	// TODO: no pointer -> threads
	origView->setImage(*img);
	origView->fullView();
	origView->zoomConstraints(origView->get100Factor());

	// fix layout issues - sorry
	origView->setFixedWidth(width()*0.5f-30);
	previewLabel->setFixedWidth(width()*0.5f-30);
}

void DkCompressDialog::drawPreview() {

	if (!img)
		return;

	QImage origImg = origView->getCurrentImageRegion();
	qDebug() << "orig img size: " << origImg.size();
	newImg = QImage(origImg.size(), QImage::Format_ARGB32);

	if (dialogMode == jpg_dialog && hasAlpha)
		newImg.fill(bgCol.rgb());
	
	QPainter bgPainter(&newImg);
	bgPainter.drawImage(origImg.rect(), origImg, origImg.rect());
	bgPainter.end();

	if (dialogMode == jpg_dialog) {
		// pre-compute the jpg compression
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::ReadWrite);
		newImg.save(&buffer, "JPG", slider->value());
		newImg.loadFromData(ba, "JPG");
		updateFileSizeLabel(buffer.size());
	}
	else if (dialogMode == webp_dialog && getCompression() != -1) {
		// pre-compute the webp compression
		DkBasicLoader loader;
		QByteArray buffer;
		loader.encodeWebP(buffer, newImg, getCompression(), 0);
		loader.decodeWebP(buffer);
		newImg = loader.image();
		updateFileSizeLabel(buffer.size());
	}
	else {
		updateFileSizeLabel();
	}

	previewLabel->setScaledContents(true);
	QImage img = newImg.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	previewLabel->setPixmap(QPixmap::fromImage(img));
}

void DkCompressDialog::updateFileSizeLabel(float bufferSize) {

	if (img == 0 || bufferSize == -1) {
		previewSizeLabel->setText(tr("File Size: --"));
		previewSizeLabel->setEnabled(false);
		return;
	}
	previewSizeLabel->setEnabled(true);

	float depth = (dialogMode == jpg_dialog) ? 24 : img->depth();	// jpg uses always 24 bit

	float rawBufferSize = newImg.width()*newImg.height()*depth/8.0f;
	float rawImgSize = img->width()*img->height()*depth/8.0f;

	previewSizeLabel->setText(tr("File Size: ~%1").arg(DkUtils::readableByte(rawImgSize*bufferSize/rawBufferSize)));
}

// OpenWithDialog --------------------------------------------------------------------
DkOpenWithDialog::DkOpenWithDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkOpenWithDialog::init() {

	defaultApp = (DkSettings::Global::defaultAppIdx < 0) ? 0 : DkSettings::Global::defaultAppIdx;
	numDefaultApps = 0;

	// TODO: qt obviously saves the settings if the keys are not found...
	// TODO: add GIMP & other software

	// the order must be correct!
	organizations = (QStringList()	<< "Adobe"				<< "Google"			<< "Google"						<< "IrfanView"		<< "");
	applications =	(QStringList()	<< "Photoshop"			<< "Picasa"			<< "Picasa"						<< "shell"			<< "");
	pathKeys =		(QStringList()	<< "ApplicationPath"	<< "Directory"		<< "Directory"					<< ""				<< "");
	exeNames =		(QStringList()	<< ""					<< ""				<< "PicasaPhotoViewer.exe"		<< ""				<< "");
	screenNames =	(QStringList()	<< tr("&Photoshop")		<< tr("Pi&casa")	<< tr("Picasa Ph&oto Viewer")	<< tr("&IrfanView")	<< tr("&Explorer"));

	// find paths to pre-defined software
	for (int idx = 0; idx < organizations.size(); idx++) {

		appPaths.append(searchForSoftware(idx));
		appIcons.append(getIcon(appPaths[idx]));
	}

	// dirty hack - but locating it in the registry is not that easy
	QFileInfo expPath("C:/Windows/explorer.exe");
	if (expPath.exists() && !appPaths.empty()) {
		appPaths.last() = expPath.absoluteFilePath();
		appIcons.last() = getIcon(appPaths.last());
	}

	createLayout();
	setWindowTitle(tr("Open With..."));

}

void DkOpenWithDialog::createLayout() {
	userRadiosGroup = new QButtonGroup;
	userRadiosGroup->setExclusive(true);

	layout = new QBoxLayout(QBoxLayout::TopToBottom);

	QGroupBox* groupBox = new QGroupBox(tr("3rd Party Software"));
	groupBox->setObjectName("softwareGroupBox");
	QGridLayout* bl = new QGridLayout();

	// add default applications
	bool first = true;

	for (int idx = 0; idx < appPaths.size(); idx++) {

		if (!appPaths[idx].isEmpty()) {

			// create
			QRadioButton* radio = new QRadioButton(screenNames[idx]);
			radio->setObjectName(screenNames[idx]);
			radio->setIcon(appIcons[idx]);
			userRadiosGroup->addButton(radio);

			qDebug() << "appPath: " << appPaths[idx];

			connect(radio, SIGNAL(clicked()), this, SLOT(softwareSelectionChanged()));			

			// always check first one
			if (DkSettings::Global::defaultAppIdx == -1 && first ||
				DkSettings::Global::defaultAppIdx == idx ) {

					radio->setChecked(true);
					first = false;
					defaultApp = idx;	// set to default app
			}

			bl->addWidget(radio, numDefaultApps, 0);
		}
		numDefaultApps++;

	}

	QStringList tmpUserPaths = DkSettings::Global::userAppPaths; // shortcut

	for (int idx = 0; idx < DkSettings::Global::numUserChoices; idx++) {

		// default initialization
		userRadios.append(new QRadioButton(tr("Choose Application")));
		connect(userRadios[idx], SIGNAL(clicked()), this, SLOT(softwareSelectionChanged()));
		userRadios[idx]->setDisabled(true);
		userRadiosGroup->addButton(userRadios[idx]);

		QIcon iconX = QIcon(":/nomacs/img/close.png"); 
		userCleanButtons.append(new QPushButton(this));
		userCleanButtons[idx]->setFlat(true);
		//userCleanButtons[idx]->setStyleSheet("QPushButton:pressed {border:0px; margin:0px;};"); // stay flat when pressed
		userCleanButtons[idx]->setIcon(iconX);
		userCleanButtons[idx]->setVisible(false);
		userCleanButtons[idx]->setFixedWidth(16);
		connect(userCleanButtons[idx], SIGNAL(clicked()), this, SLOT(softwareCleanClicked()));

		userCleanSpace.append(new QLabel(this));
		userCleanSpace[idx]->setFixedWidth(16);
		userCleanSpace[idx]->setVisible(true);


		QPushButton* userBrowse = new QPushButton(tr("Browse..."));
		userBrowse->setObjectName("browse-" % QString::number(idx));
		connect(userBrowse, SIGNAL(clicked()), this, SLOT(browseAppFile()));

		screenNames.append("");
		userAppPaths.append("");

		int userIdx = idx + numDefaultApps;

		if (DkSettings::Global::defaultAppIdx == userIdx)
			userRadios[idx]->setChecked(true);

		// is an application set & is it still installed?
		if (idx < tmpUserPaths.size() &&
			QFileInfo(tmpUserPaths[idx]).exists()) {

				// remove file extension for GUI
				QFileInfo filePathInfo = QFileInfo(tmpUserPaths[idx]);
				screenNames[userIdx] = filePathInfo.fileName();
				screenNames[userIdx].replace("." + filePathInfo.suffix(), "");	

				userAppPaths[idx] = tmpUserPaths[idx];
				userRadios[idx]->setObjectName(screenNames[userIdx]);
				userRadios[idx]->setText(screenNames[userIdx]);
				userRadios[idx]->setIcon(getIcon(tmpUserPaths[idx]));
				userRadios[idx]->setEnabled(true);

				userCleanButtons[idx]->setVisible(true);
				userCleanSpace[idx]->setVisible(false);
		}

		bl->addWidget(userRadios[idx], numDefaultApps+idx, 0);
		bl->addWidget(userCleanButtons[idx], numDefaultApps+idx,1);
		bl->addWidget(userCleanSpace[idx], numDefaultApps+idx,1);
		bl->addWidget(userBrowse, numDefaultApps+idx, 2);
	}

	// never again checkbox
	neverAgainBox = new QCheckBox(tr("Never show this dialog again"));
	neverAgainBox->setObjectName("neverAgainBox");
	neverAgainBox->setChecked(!DkSettings::Global::showDefaultAppDialog);
	neverAgainBox->setToolTip(tr("Do not be scared, you can always open this window in Preferences -> Global Settings"));

	// ok, cancel button
	QWidget* bottomWidget = new QWidget(this);
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	groupBox->setLayout(bl);
	layout->addWidget(groupBox);
	layout->addWidget(neverAgainBox);
	layout->addStretch();
	layout->addWidget(buttons);

	setLayout(layout);
}

void DkOpenWithDialog::browseAppFile() {

	QString sender = QObject::sender()->objectName();

	if (!sender.contains("browse"))
		return;

	// identify the browse button
	QStringList splitSender = sender.split("-");

	// was it really a browse button?
	if (splitSender.size() != 2)
		return;

	int senderIdx = splitSender[1].toInt();

	// is the sender valid?
	if (senderIdx < 0 || senderIdx >= userRadios.size())
		return;

	// load system default open dialog
	QString appFilter;
	QString defaultPath = userAppPaths[senderIdx];
#ifdef Q_WS_WIN
	appFilter += tr("Executable Files (*.exe);;");
	if (!QFileInfo(defaultPath).exists())
		defaultPath = getenv("PROGRAMFILES");
#else
	defaultPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation); // retrieves startmenu on windows?!
#endif

	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Application"),
		defaultPath, 
		appFilter);

	if (filePath.isEmpty())
		return;

	int userIdx = senderIdx+numDefaultApps;

	// remove file extension for GUI
	QFileInfo filePathInfo = QFileInfo(filePath);
	screenNames[userIdx] = filePathInfo.fileName();
	screenNames[userIdx].replace("." % filePathInfo.suffix(), "");	

	userAppPaths[senderIdx] = filePath;
	userRadios[senderIdx]->setObjectName(screenNames[userIdx]);	// needed for slot	
	userRadios[senderIdx]->setText(screenNames[userIdx]);
	userRadios[senderIdx]->setIcon(getIcon(filePath));
	userRadios[senderIdx]->setEnabled(true);
	userRadios[senderIdx]->setChecked(true);
	defaultApp = userIdx;

	userCleanButtons[senderIdx]->setVisible(true);
	userCleanSpace[senderIdx]->setVisible(false);
}

void DkOpenWithDialog::accept() {

	// store everything
	DkSettings::Global::showDefaultAppDialog = !neverAgainBox->isChecked();
	DkSettings::Global::defaultAppIdx = defaultApp;
	DkSettings::Global::defaultAppPath = getPath();
	DkSettings::Global::userAppPaths = userAppPaths;

	QDialog::accept();
}

void DkOpenWithDialog::softwareSelectionChanged() {

	QString sender = QObject::sender()->objectName();

	for (int idx = 0; idx < screenNames.size(); idx++) {

		if (screenNames[idx] == sender)
			defaultApp = idx;
	}
	
	qDebug() << "default app idx..." << defaultApp;
}

void DkOpenWithDialog::softwareCleanClicked() {
	QPushButton* button = (QPushButton*)QObject::sender();
	int idx = userCleanButtons.indexOf(button);
	if (idx == -1)
		return;

	userAppPaths.replace(idx, QString());

	if(userRadios[idx]->isChecked()) {
		userRadiosGroup->setExclusive(false);
		userRadiosGroup->checkedButton()->setChecked(false);
		userRadiosGroup->setExclusive(true);

		QList<QAbstractButton*> buttons = userRadiosGroup->buttons();
		if (buttons.size() > 0)
			buttons[0]->setChecked(true);
		defaultApp = 0;
	}

	userRadios[idx]->setText(tr("Choose Application"));
	userRadios[idx]->setDisabled(true);
	userRadios[idx]->setIcon(QIcon());



	userCleanButtons[idx]->setVisible(false);
	userCleanSpace[idx]->setVisible(true);
}

QString DkOpenWithDialog::searchForSoftware(int softwareIdx) {

	if (softwareIdx < 0 || softwareIdx >= organizations.size())
		return "";

	qDebug() << "\n\nsearching for: " << organizations[softwareIdx] << " " << applications[softwareIdx];

	// locate the settings entry
	QSettings* softwareSettings = new QSettings(QSettings::UserScope, organizations[softwareIdx], applications[softwareIdx]);
	QStringList keys = softwareSettings->allKeys();

	//// debug
	//for (int idx = 0; idx < keys.size(); idx++) {
	//	qDebug() << keys[idx] << " - " << softwareSettings->value(keys[idx]).toString();
	//}
	//// debug

	QString appPath;

	for (int idx = 0; idx < keys.length(); idx++) {

		// find the path
		if (keys[idx].contains(pathKeys[softwareIdx])) {
			appPath = softwareSettings->value(keys[idx]).toString();
			break;
		}
	}

	// if we did not find it -> return
	if (appPath.isEmpty()) {
		// clean up
		delete softwareSettings;
		return appPath;
	}

	if (exeNames[softwareIdx].isEmpty()) {

		// locate the exe
		QDir appFile = appPath.replace("\"", "");	// the string must not have extra quotes
		QFileInfoList apps = appFile.entryInfoList(QStringList() << "*.exe");

		for (int idx = 0; idx < apps.size(); idx++) {

			if (apps[idx].fileName().contains(applications[softwareIdx])) {
				appPath = apps[idx].absoluteFilePath();
				break;
			}
		}

		qDebug() << appPath;
	}
	else
		appPath = QFileInfo(appPath, exeNames[softwareIdx]).absoluteFilePath();	// for correct separators

	// clean up
	delete softwareSettings;

	return appPath;
}

QPixmap DkOpenWithDialog::getIcon(QFileInfo file) {


#ifdef Q_WS_WIN
#include <windows.h>

	// icon extraction should take between 2ms and 13ms
	QPixmap appIcon;
	QString winPath = QDir::toNativeSeparators(file.absoluteFilePath());

	WCHAR* wDirName = new WCHAR[winPath.length()];

	// CMakeLists.txt:
	// if compile error that toWCharArray is not recognized:
	// in msvc: Project Properties -> C/C++ -> Language -> Treat WChar_t as built-in type: set to No (/Zc:wchar_t-)
	int dirLength = winPath.toWCharArray(wDirName);
	wDirName[dirLength] = L'\0';	// append null character

	int nIcons = ExtractIconExW(wDirName, 0, NULL, NULL, 0);

	if (!nIcons)
		return appIcon;

	HICON largeIcon;
	HICON smallIcon;
	int err = ExtractIconExW(wDirName, 0, &largeIcon, &smallIcon, 1);

	if (nIcons != 0 && largeIcon != NULL)
		appIcon = QPixmap::fromWinHICON(largeIcon);

	DestroyIcon(largeIcon);
	DestroyIcon(smallIcon);

	return appIcon;

#endif

	return QPixmap();
}

// DkSearchDialaog --------------------------------------------------------------------
DkSearchDialog::DkSearchDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkSearchDialog::init() {

	setObjectName("DkSearchDialog");
	setWindowTitle(tr("Find & Filter"));

	endMessage = tr("Load All");
	allDisplayed = true;
	isFilterPressed = false;

	QVBoxLayout* layout = new QVBoxLayout(this);

	QCompleter* history = new QCompleter(DkSettings::Global::searchHistory);
	history->setCompletionMode(QCompleter::InlineCompletion);
	searchBar = new QLineEdit();
	searchBar->setObjectName("searchBar");
	searchBar->setToolTip(tr("Type a search word or a regular expression"));
	searchBar->setCompleter(history);

	stringModel = new QStringListModel();

	resultListView = new QListView();
	resultListView->setObjectName("resultListView");
	resultListView->setModel(stringModel);
	resultListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	resultListView->setSelectionMode(QAbstractItemView::SingleSelection);
	defaultStyleSheet = resultListView->styleSheet();

	//// TODO: add cursor down - cursor up action
	//QAction* focusAction = new QAction(tr("Focus Action"), searchBar);
	//focusAction->setShortcut(Qt::Key_Down);
	//connect(focusAction, SIGNAL(triggered()), resultListView, SLOT(/*createSLOT*/));


	// buttons
	findButton = new QPushButton(tr("F&ind"), this);
	findButton->setObjectName("okButton");
	findButton->setDefault(true);

	filterButton = new QPushButton(tr("&Filter"), this);
	filterButton->setObjectName("filterButton");

	cancelButton = new QPushButton(tr("&Cancel"), this);
	cancelButton->setObjectName("cancelButton");

	QWidget* buttonWidget = new QWidget();
	QBoxLayout* buttonLayout = new QBoxLayout(QBoxLayout::RightToLeft, buttonWidget);
	buttonLayout->addWidget(cancelButton);
	buttonLayout->addWidget(filterButton);
	buttonLayout->addWidget(findButton);

	layout->addWidget(searchBar);
	layout->addWidget(resultListView);
	layout->addWidget(buttonWidget);

	QMetaObject::connectSlotsByName(this);
}

void DkSearchDialog::on_searchBar_textChanged(const QString& text) {

	qDebug() << " you wrote: " << text;

	DkTimer dt;

	if (text == currentSearch)
		return;
	
	// white space is the magic thingy
	QStringList queries = text.split(" ");
	
	if (queries.size() == 1) {
		// if characters are added, use the result list -> speed-up
		// I think we can't do that anymore for the sake of list view cropping...
		resultList = (!text.contains(currentSearch) || !allDisplayed) ? fileList.filter(text, Qt::CaseInsensitive) : resultList.filter(text, Qt::CaseInsensitive);
	}
	else {
		resultList = fileList;
		for (int idx = 0; idx < queries.size(); idx++) {
			resultList = resultList.filter(queries[idx], Qt::CaseInsensitive);
			qDebug() << "query: " << queries[idx];
		}
	}

	// if string match returns nothing -> try a regexp
	if (resultList.empty())
		resultList = fileList.filter(QRegExp(text));

	qDebug() << "searching takes: " << QString::fromStdString(dt.getTotal());
	currentSearch = text;

	if (resultList.empty()) {
		QStringList answerList;
		answerList.append(tr("No Matching Items"));
		stringModel->setStringList(answerList);

		resultListView->setStyleSheet("QListView{color: #777777; font-style: italic;}");
		filterButton->setEnabled(false);
		findButton->setEnabled(false);
		//cancelButton->setFocus();
	}
	else {
		filterButton->setEnabled(true);
		findButton->setEnabled(true);
		stringModel->setStringList(makeViewable(resultList));
		resultListView->selectionModel()->setCurrentIndex(stringModel->index(0, 0), QItemSelectionModel::SelectCurrent);
		resultListView->setStyleSheet(defaultStyleSheet);
	}

	qDebug() << "searching takes (total): " << QString::fromStdString(dt.getTotal());
}

void DkSearchDialog::on_resultListView_doubleClicked(const QModelIndex& modelIndex) {

	if (modelIndex.data().toString() == endMessage) {
		stringModel->setStringList(makeViewable(resultList, true));
		return;
	}

	emit loadFileSignal(QFileInfo(path, modelIndex.data().toString()));
	close();
}

void DkSearchDialog::on_resultListView_clicked(const QModelIndex& modelIndex) {

	if (modelIndex.data().toString() == endMessage)
		stringModel->setStringList(makeViewable(resultList, true));
}

void DkSearchDialog::on_okButton_pressed() {

	if (resultListView->selectionModel()->currentIndex().data().toString() == endMessage) {
		stringModel->setStringList(makeViewable(resultList, true));
		return;
	}

	updateHistory();

	// ok load the selected file
	QString fileName = resultListView->selectionModel()->currentIndex().data().toString();
	qDebug() << "opening filename: " << fileName;

	if (!fileName.isEmpty())
		emit loadFileSignal(QFileInfo(path, fileName));
	close();
}

void DkSearchDialog::on_filterButton_pressed() {
	filterSignal(currentSearch.split(" "));
	isFilterPressed = true;
	close();
}

void DkSearchDialog::on_cancelButton_pressed() {

	close();
}

void DkSearchDialog::updateHistory() {
	
	DkSettings::Global::searchHistory.append(currentSearch);

	// keep the history small
	if (DkSettings::Global::searchHistory.size() > 50)
		DkSettings::Global::searchHistory.pop_front();

	//QCompleter* history = new QCompleter(DkSettings::Global::searchHistory);
	//searchBar->setCompleter(history);
}

QStringList DkSearchDialog::makeViewable(const QStringList& resultList, bool forceAll) {
	
	QStringList answerList;
	
	// if size > 1000 it gets slow -> cut at 1000 and make an entry for 'expand'
	if (!forceAll && resultList.size() > 1000) {

		for (int idx = 0; idx < 1000; idx++)
			answerList.append(resultList[idx]);
		answerList.append(endMessage);

		allDisplayed = false;
	}
	else {
		allDisplayed = true;
		answerList = resultList;
	}

	return answerList;
}

// DkResizeDialog --------------------------------------------------------------------
DkResizeDialog::DkResizeDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkResizeDialog::init() {

	leftSpacing = 40;
	margin = 10;
	exifDpi = 72;

	unitFactor.resize(unit_end);
	unitFactor.insert(unit_cm, 1.0f);
	unitFactor.insert(unit_mm, 10.0f);
	unitFactor.insert(unit_inch, 1.0f/2.54f);

	resFactor.resize(res_end);
	resFactor.insert(res_ppi, 2.54f);
	resFactor.insert(res_ppc, 1.0f);

	setWindowTitle(tr("Resize Image"));
	setFixedSize(600, 512);
	createLayout();
	initBoxes();

	QMetaObject::connectSlotsByName(this);

}

void DkResizeDialog::createLayout() {

	// central widget
	centralWidget = new QWidget(this);

	QWidget* previewWidget = new QWidget();
	QGridLayout* previewLayout = new QGridLayout(previewWidget);

	// preview
	QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	s *= 0.5;
	int minPx = 1;
	int maxPx = 100000;
	int minWidth = 0.1;
	int maxWidth = 500000;
	int decimals = 2;

	QLabel* origLabelText = new QLabel(tr("Original"));
	QLabel* newLabel = new QLabel(tr("New"));

	// shows the original image
	origView = new DkBaseViewPort(this);
	origView->setForceFastRendering(true);
	origView->setPanControl(QPointF(0.0f, 0.0f));
	connect(origView, SIGNAL(imageUpdated()), this, SLOT(drawPreview()));

	//// maybe we should report this: 
	//// if a stylesheet (with border) is set, the var
	//// cornerPaintingRect in QAbstractScrollArea (which we don't even need : )
	//// is invalid which blocks re-paints unless the widget gets a focus...
	//origView->setStyleSheet("QViewPort{border: 1px solid #888;}");

	// shows the preview
	previewLabel = new QLabel();
	//previewLabel->setStyleSheet("QLabel{border: 1px solid #888;}");

	previewLayout->addWidget(origLabelText, 0, 0);
	previewLayout->addWidget(newLabel, 0, 1);
	previewLayout->addWidget(origView, 1, 0);
	previewLayout->addWidget(previewLabel, 1, 1);

	// all text dialogs... // TODO: go on here...
	QIntValidator* intValidator = new QIntValidator(1, 100000, 0);
	QDoubleValidator* doubleValidator = new QDoubleValidator(1, 1000000, 2, 0);
	doubleValidator->setRange(0, 100, 2);

	QWidget* resizeBoxes = new QWidget();
	resizeBoxes->setGeometry(QRect(QPoint(leftSpacing, 300), QSize(400, 170)));

	QGridLayout* gridLayout = new QGridLayout(resizeBoxes);

	QLabel* wPixelLabel = new QLabel(tr("Width: "));
	wPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	wPixelEdit = new QDoubleSpinBox();
	wPixelEdit->setObjectName("wPixelEdit");
	wPixelEdit->setRange(minPx, maxPx);
	wPixelEdit->setDecimals(0);

	QWidget* lockWidget = new QWidget();
	QHBoxLayout* boxLayout = new QHBoxLayout(); 
	lockButton = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	lockButton->setFixedSize(QSize(16,16));
	lockButton->setObjectName("lockButton");
	lockButton->setCheckable(true);
	lockButton->setChecked(true);

	QLabel* hPixelLabel = new QLabel(tr("Height: "));
	hPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	hPixelEdit = new QDoubleSpinBox();
	hPixelEdit->setObjectName("hPixelEdit");
	hPixelEdit->setRange(minPx, maxPx);
	hPixelEdit->setDecimals(0);

	sizeBox = new QComboBox();
	QStringList sizeList;
	sizeList.insert(size_pixel, "pixel");
	sizeList.insert(size_percent, "%");
	sizeBox->addItems(sizeList);
	sizeBox->setObjectName("sizeBox");

	// first row
	int rIdx = 0;
	gridLayout->addWidget(wPixelLabel, 0, rIdx);
	gridLayout->addWidget(wPixelEdit, 0, ++rIdx);
	gridLayout->addWidget(lockButton, 0, ++rIdx);
	gridLayout->addWidget(hPixelLabel, 0, ++rIdx);
	gridLayout->addWidget(hPixelEdit, 0, ++rIdx);
	gridLayout->addWidget(sizeBox, 0, ++rIdx);

	// Document dimensions
	QLabel* widthLabel = new QLabel(tr("Width: "));
	widthLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	widthEdit = new QDoubleSpinBox();
	widthEdit->setObjectName("widthEdit");
	widthEdit->setRange(minWidth, maxWidth);
	widthEdit->setDecimals(decimals);

	lockButtonDim = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	//lockButtonDim->setIcon(QIcon(":/nomacs/img/lock.png"));
	lockButtonDim->setFixedSize(QSize(16,16));
	lockButtonDim->setObjectName("lockButtonDim");
	lockButtonDim->setCheckable(true);
	lockButtonDim->setChecked(true);

	QLabel* heightLabel = new QLabel(tr("Height: "));
	heightLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	heightEdit = new QDoubleSpinBox();
	heightEdit->setObjectName("heightEdit");
	heightEdit->setRange(minWidth, maxWidth);
	heightEdit->setDecimals(decimals);

	unitBox = new QComboBox();
	QStringList unitList;
	unitList.insert(unit_cm, "cm");
	unitList.insert(unit_mm, "mm");
	unitList.insert(unit_inch, "inch");
	//unitList.insert(unit_percent, "%");
	unitBox->addItems(unitList);
	unitBox->setObjectName("unitBox");

	// second row
	rIdx = 0;
	gridLayout->addWidget(widthLabel, 1, rIdx);
	gridLayout->addWidget(widthEdit, 1, ++rIdx);
	gridLayout->addWidget(lockButtonDim, 1, ++rIdx);
	gridLayout->addWidget(heightLabel, 1, ++rIdx);
	gridLayout->addWidget(heightEdit, 1, ++rIdx);
	gridLayout->addWidget(unitBox, 1, ++rIdx);

	// resolution
	QLabel* resolutionLabel = new QLabel(tr("Resolution: "));
	resolutionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	resolutionEdit = new QDoubleSpinBox();
	resolutionEdit->setObjectName("resolutionEdit");
	resolutionEdit->setRange(minWidth, maxWidth);
	resolutionEdit->setDecimals(decimals);

	resUnitBox = new QComboBox();
	QStringList resUnitList;
	resUnitList.insert(res_ppi, "pixel/inch");
	resUnitList.insert(res_ppc, "pixel/cm");
	resUnitBox->addItems(resUnitList);
	resUnitBox->setObjectName("resUnitBox");

	// third row
	rIdx = 0;
	gridLayout->addWidget(resolutionLabel, 2, rIdx);
	gridLayout->addWidget(resolutionEdit, 2, ++rIdx);
	gridLayout->addWidget(resUnitBox, 2, ++rIdx, 1, 2);

	// resample
	resampleCheck = new QCheckBox(tr("Resample Image:"));
	resampleCheck->setChecked(true);
	resampleCheck->setObjectName("resampleCheck");

	// TODO: disable items if no opencv is available
	resampleBox = new QComboBox();
	QStringList resampleList;
	resampleList.insert(ipl_nearest, tr("Nearest Neighbor"));
	resampleList.insert(ipl_area, tr("Area (best for downscaling)"));
	resampleList.insert(ipl_linear, tr("Linear"));
	resampleList.insert(ipl_cubic, tr("Bicubic (4x4 pixel interpolation)"));
	resampleList.insert(ipl_lanczos, tr("Lanczos (8x8 pixel interpolation)"));
	resampleBox->addItems(resampleList);
	resampleBox->setObjectName("resampleBox");
	resampleBox->setCurrentIndex(ipl_cubic);

	// last two rows
	gridLayout->addWidget(resampleCheck, 3, 1, 1, 3);
	gridLayout->addWidget(resampleBox, 4, 1, 1, 3);

	// add stretch
	gridLayout->setColumnStretch(6, 1000);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(previewWidget);
	layout->addWidget(resizeBoxes);
	layout->addStretch();
	layout->addWidget(buttons);
	show();
}

void DkResizeDialog::initBoxes() {

	if (img.isNull())
		return;

	if (sizeBox->currentIndex() == size_pixel) {
		wPixelEdit->setValue(img.width());
		hPixelEdit->setValue(img.height());
	}
	else {
		wPixelEdit->setValue(100);
		hPixelEdit->setValue(100);
	}

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float width = (float)img.width()/exifDpi * units;
	widthEdit->setValue(width);

	float height = (float)img.height()/exifDpi * units;
	heightEdit->setValue(height);
}

void DkResizeDialog::updateWidth() {

	float pWidth = wPixelEdit->text().toDouble();

	if (sizeBox->currentIndex() == size_percent)
		pWidth = qRound(pWidth/100 * img.width()); 

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float width = pWidth/exifDpi * units;
	widthEdit->setValue(width);
}

void DkResizeDialog::updateHeight() {

	float pHeight = hPixelEdit->text().toDouble();

	if (sizeBox->currentIndex() == size_percent)
		pHeight = qRound(pHeight/100 * img.height()); 

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float height = pHeight/exifDpi * units;
	heightEdit->setValue(height);
}

void DkResizeDialog::updateResolution() {

	qDebug() << "updating resolution...";
	float wPixel = wPixelEdit->text().toDouble();
	float width = widthEdit->text().toDouble();

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float resolution = wPixel/width * units;
	resolutionEdit->setValue(resolution);
}

void DkResizeDialog::updatePixelHeight() {

	float height = heightEdit->text().toDouble();

	// *1000/10 is for more beautiful values
	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float pixelHeight = (sizeBox->currentIndex() != size_percent) ? qRound(height*exifDpi / units) : qRound(1000.0f*height*exifDpi / (units * img.height()))/10.0f;

	hPixelEdit->setValue(pixelHeight);
}

void DkResizeDialog::updatePixelWidth() {

	float width = widthEdit->text().toDouble();

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float pixelWidth = (sizeBox->currentIndex() != size_percent) ? qRound(width*exifDpi / units) : qRound(1000.0f*width*exifDpi / (units * img.width()))/10.0f;
	wPixelEdit->setValue(pixelWidth);
}

void DkResizeDialog::on_lockButtonDim_clicked() {

	lockButton->setChecked(lockButtonDim->isChecked());
	if (!lockButtonDim->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_lockButton_clicked() {

	lockButtonDim->setChecked(lockButton->isChecked());

	if (!lockButton->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_wPixelEdit_valueChanged(QString text) {

	if (!wPixelEdit->hasFocus())
		return;

	updateWidth();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newHeight = (sizeBox->currentIndex() != size_percent) ? qRound((float)text.toInt()/(float)img.width() * img.height()) : text.toFloat();
	hPixelEdit->setValue(newHeight);
	updateHeight();
	drawPreview();
}

void DkResizeDialog::on_hPixelEdit_valueChanged(QString text) {

	if(!hPixelEdit->hasFocus())
		return;
	
	updateHeight();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newWidth = (sizeBox->currentIndex() != size_percent) ? qRound((float)text.toInt()/(float)img.height() * (float)img.width()) : text.toFloat();
	wPixelEdit->setValue(newWidth);
	updateWidth();
	drawPreview();
}

void DkResizeDialog::on_widthEdit_valueChanged(QString text) {

	if (!widthEdit->hasFocus())
		return;

	if (resampleCheck->isChecked()) 
		updatePixelWidth();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	heightEdit->setValue(text.toFloat()/(float)img.width() * (float)img.height());

	if (resampleCheck->isChecked()) 
		updatePixelHeight();

	if (!resampleCheck->isChecked())
		updateResolution();

	drawPreview();
}

void DkResizeDialog::on_heightEdit_valueChanged(QString text) {

	if (!heightEdit->hasFocus())
		return;

	if (resampleCheck->isChecked()) 
		updatePixelHeight();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	widthEdit->setValue(text.toFloat()/(float)img.height() * (float)img.width());

	if (resampleCheck->isChecked()) 
		updatePixelWidth();

	if (!resampleCheck->isChecked())
		updateResolution();
	drawPreview();
}

void DkResizeDialog::on_resolutionEdit_valueChanged(QString text) {

	exifDpi = text.toDouble();

	if (!resolutionEdit->hasFocus())
		return;

	updatePixelWidth();
	updatePixelHeight();

	if (resampleCheck->isChecked()) {
		drawPreview();
		return;
	}

	initBoxes();
}


void DkResizeDialog::on_unitBox_currentIndexChanged(int idx) {

	updateHeight();
	updateWidth();
	//initBoxes();
}

void DkResizeDialog::on_sizeBox_currentIndexChanged(int idx) {

	if (idx == size_pixel) {
		wPixelEdit->setDecimals(0);
		hPixelEdit->setDecimals(0);
	}
	else {
		wPixelEdit->setDecimals(2);
		hPixelEdit->setDecimals(2);
	}

	updatePixelHeight();
	updatePixelWidth();
}

void DkResizeDialog::on_resUnitBox_currentIndexChanged(int idx) {

	updateResolution();
	//initBoxes();
}

void DkResizeDialog::on_resampleCheck_clicked() {

	resampleBox->setEnabled(resampleCheck->isChecked());
	wPixelEdit->setEnabled(resampleCheck->isChecked());
	hPixelEdit->setEnabled(resampleCheck->isChecked());

	if (!resampleCheck->isChecked()) {
		lockButton->setChecked(true);
		lockButtonDim->setChecked(true);
		initBoxes();
	}
	else
		drawPreview();
}

void DkResizeDialog::on_resampleBox_currentIndexChanged(int idx) {
	drawPreview();
}

void DkResizeDialog::updateSnippets() {

	if (img.isNull())
		return;

	// fix layout issues - sorry
	origView->setFixedWidth(width()*0.5f-30);
	previewLabel->setFixedWidth(width()*0.5f-30);
	origView->setFixedHeight(width()*0.5f-30);
	previewLabel->setFixedHeight(width()*0.5f-30);


	origView->setImage(img);
	origView->fullView();
	origView->zoomConstraints(origView->get100Factor());

	//QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	//s *= 0.5;
	//origImg = QImage(s, QImage::Format_ARGB32);
	//origImg.fill(Qt::transparent);
	//QRect imgRect = QRect(QPoint(img.width()*0.5-origImg.width()*0.5, img.height()*0.5-origImg.height()*0.5), origImg.size());

	//QPainter painter(&origImg);
	//painter.setBackgroundMode(Qt::TransparentMode);
	//painter.drawImage(QRect(QPoint(), origImg.size()), img, imgRect);
	//painter.end();

}

void DkResizeDialog::drawPreview() {

	if (img.isNull())
		return;

	newImg = origView->getCurrentImageRegion();
	newImg = resizeImg(newImg);

	previewLabel->setScaledContents(true);
	QImage img = newImg.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	previewLabel->setPixmap(QPixmap::fromImage(img));


	//QImage preview = QImage(origImg.width()*2 + 10, origImg.height(), QImage::Format_ARGB32);
	//preview.fill(Qt::transparent);
	//QRect pos = QRect(QPoint(), origImg.size());
	//QRect posM = pos;
	//posM.setSize(QSize(pos.size().width()-1, pos.size().height()-1));

	//QPainter painter(&preview);
	//painter.setBackgroundMode(Qt::TransparentMode);
	//painter.drawImage(pos, origImg, QRect(QPoint(), origImg.size()));
	//painter.setPen(QColor(0,0,0,30));
	//painter.drawRect(posM);
	//pos.moveTopLeft(QPoint(origImg.width()+10, 0));
	//posM.moveTopLeft(QPoint(origImg.width()+10, 0));
	//painter.drawImage(pos, newImg, QRect(QPoint(), newImg.size()));
	//painter.drawRect(posM);

	////previewLabel->setGeometry(QRect(QPoint(slider->geometry().left(), slider->geometry().bottom() + margin*4), preview.size()));
	//previewLabel->setGeometry(QRect(QPoint(40, margin*4), preview.size()));
	//previewLabel->setPixmap(QPixmap::fromImage(preview));

	//updateSnippets();
}

QImage DkResizeDialog::resizeImg(QImage img, bool silent) {

	if (img.isNull())
		return img;

	QSize newSize;

	if (sizeBox->currentIndex() == size_percent)
		newSize = QSize(wPixelEdit->text().toFloat()/100.0f * this->img.width(), hPixelEdit->text().toFloat()/100.0f * this->img.height());
	else
		newSize = QSize(wPixelEdit->text().toInt(), hPixelEdit->text().toInt());

	QSize imgSize = this->img.size();

	qDebug() << "new size: " << newSize;

	// nothing to do
	if (this->img.size() == newSize)
		return img;

	if (this->img.size() != img.size()) {
		// compute relative size
		float relWidth = (float)newSize.width()/(float)imgSize.width();
		float relHeight = (float)newSize.height()/(float)imgSize.height();

		qDebug() << "relative size: " << newSize;

		newSize = QSize(img.width()*relWidth, img.height()*relHeight);
	}

	if (newSize.width() < wPixelEdit->minimum() || newSize.width() > wPixelEdit->maximum() || 
		newSize.height() < hPixelEdit->minimum() || newSize.height() > hPixelEdit->maximum()) {

		if (!silent) {
			QMessageBox errorDialog(this);
			errorDialog.setIcon(QMessageBox::Critical);
			errorDialog.setText(tr("Sorry, but the image size %1 x %2 is illegal.").arg(newSize.width()).arg(newSize.height()));
			errorDialog.show();
			errorDialog.exec();
		}
	}

	Qt::TransformationMode iplQt;
	switch(resampleBox->currentIndex()) {
	case ipl_nearest:	
	case ipl_area:		iplQt = Qt::FastTransformation; break;
	case ipl_linear:	
	case ipl_cubic:		
	case ipl_lanczos:	iplQt = Qt::SmoothTransformation; break;
	}
#ifdef WITH_OPENCV

	int ipl = CV_INTER_CUBIC;
	switch(resampleBox->currentIndex()) {
	case ipl_nearest:	ipl = CV_INTER_NN; break;
	case ipl_area:		ipl = CV_INTER_AREA; break;
	case ipl_linear:	ipl = CV_INTER_LINEAR; break;
	case ipl_cubic:		ipl = CV_INTER_CUBIC; break;
#ifdef DISABLE_LANCZOS
	case ipl_lanczos:	ipl = CV_INTER_CUBIC; break;
#else
	case ipl_lanczos:	ipl = CV_INTER_LANCZOS4; break;
#endif
	}

	try {
		Mat resizeImage = DkImage::qImage2Mat(img);

		// is the image convertable?
		if (resizeImage.empty() || newSize.width() < 1 || newSize.height() < 1) {

			return img.scaled(newSize, Qt::IgnoreAspectRatio, iplQt);
		}
		else {
						
			QVector<QRgb> colTable = img.colorTable();
			qDebug() << "resizing..." << colTable.size();
			Mat tmp;
			cv::resize(resizeImage, tmp, cv::Size(newSize.width(), newSize.height()), 0, 0, ipl);

			QImage rImg = DkImage::mat2QImage(tmp);
					
			rImg.setColorTable(img.colorTable());
			return rImg;

		}

	}catch (std::exception se) {

		if (!silent) {

			qDebug() << "image size: " << newSize;
			QMessageBox errorDialog(this);
			errorDialog.setIcon(QMessageBox::Critical);
			errorDialog.setText(tr("Sorry, the image is too large: %1").arg(DkImage::getBufferSize(newSize, 32)));
			errorDialog.show();
			errorDialog.exec();
		}

		return QImage();
	}

	return QImage();
#else

	return img.scaled(newSize, Qt::IgnoreAspectRatio, iplQt);

#endif

}

// DkShortcutDelegate --------------------------------------------------------------------
DkShortcutDelegate::DkShortcutDelegate(QVector<QPair<QString, QKeySequence> >* actions, QObject* parent) : QItemDelegate(parent) {
	this->actions = actions;
	//cModelIndex = 0;
}

bool DkShortcutDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) {

	cRow = index.row();
	return QItemDelegate::editorEvent(event, model, option, index);
}
//
//void DkShortcutDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
//
//	qDebug() << "setting editor data...";
//
//}

QWidget* DkShortcutDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	

	//if (!index.isValid())
	//	return 0;
	//QVariant::Type t = static_cast<QVariant::Type>(index.data(Qt::EditRole).userType());
	//const QItemEditorFactory *factory = d->f;
	//if (factory == 0)
	//	factory = QItemEditorFactory::defaultFactory();
	//return factory->createEditor(t, parent);

	QWidget* w = QItemDelegate::createEditor(parent, option, index);
	connect(w, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
	connect(w, SIGNAL(editingFinished()), this, SLOT(hideNotification()));

	return w;
}

void DkShortcutDelegate::textChanged(QString text) {
	qDebug() << "shortcut text is: " << text;

	if (!actions)
		return;

	QKeySequence ks(text);

	for (int idx = 0; idx < actions->size(); idx++) {
		if (idx != cRow && actions->at(idx).second == ks) {
			emit notifyDuplicate(tr("%1 already used by %2\n Press ESC to undo changes").arg(actions->at(idx).second.toString()).arg(actions->at(idx).first));
			break;
		}
	}
}

void DkShortcutDelegate::hideNotification() {
	emit notifyDuplicate("");	// remove notification
}

DkShortcutEditor::DkShortcutEditor(QWidget *widget) : QLineEdit(widget) {

}

QKeySequence DkShortcutEditor::shortcut() const {
	//qDebug() << "returning: "
	return QKeySequence(text());
}

void DkShortcutEditor::setShortcut(const QKeySequence shortcut) {
	this->ks = shortcut;
}

void DkShortcutEditor::keyPressEvent(QKeyEvent *event) {

}

void DkShortcutEditor::keyReleaseEvent(QKeyEvent* event) {
	
	if (event->key() == Qt::Key_Control ||
		event->key() == Qt::Key_Shift ||
		event->key() == Qt::Key_Alt ||
		event->key() == Qt::Key_Meta)
		return;

	int ksi = event->key();

	if (event->modifiers() & Qt::ShiftModifier)
		ksi += Qt::SHIFT;
	if (event->modifiers() & Qt::AltModifier)
		ksi += Qt::ALT;
	if (event->modifiers() & Qt::ControlModifier)
		ksi += Qt::CTRL;
	if (event->modifiers() & Qt::MetaModifier)
		ksi += Qt::META;

	QKeySequence ks(ksi);
	setText(ks.toString());
}


// TreeItem --------------------------------------------------------------------
TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent) {
	parentItem = parent;
	itemData = data;
}

TreeItem::~TreeItem() {
	qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item) {
	childItems.append(item);
	//item->setParent(this);
}

TreeItem* TreeItem::child(int row) {
	
	if (row < 0 || row >= childItems.size())
		return 0;

	return childItems[row];
}

int TreeItem::childCount() const {
	return childItems.size();
}

int TreeItem::row() const {
	
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

int TreeItem::columnCount() const {
	
	int columns = itemData.size();

	for (int idx = 0; idx < childItems.size(); idx++)
		columns = qMax(columns, childItems[idx]->columnCount());
	
	return columns;
}

QVariant TreeItem::data(int column) const {
	return itemData.value(column);
}

void TreeItem::setData(const QVariant& value, int column) {

	if (column < 0 || column >= itemData.size())
		return;

	qDebug() << "replacing: " << itemData[0] << " with: " << value;
	itemData.replace(column, value);
}

TreeItem* TreeItem::find(const QVariant& value, int column) {

	if (column < 0)
		return 0;

	if (column < itemData.size() && itemData[column] == value)
		return this;

	for (int idx = 0; idx < childItems.size(); idx++) 
		if (TreeItem* child = childItems[idx]->find(value, column))
			return child;

	return 0;
}

TreeItem* TreeItem::parent() const {
	return parentItem;
}

void TreeItem::setParent(TreeItem* parent) {
	parentItem = parent;
}

// DkShortcutsModel --------------------------------------------------------------------
DkShortcutsModel::DkShortcutsModel(QObject* parent) : QAbstractTableModel(parent) {

	// create root
	QVector<QVariant> rootData;
	rootData << tr("Name") << tr("Shortcut");

	rootItem = new TreeItem(rootData);

}

DkShortcutsModel::~DkShortcutsModel() {
	delete rootItem;
}

//DkShortcutsModel::DkShortcutsModel(QVector<QPair<QString, QKeySequence> > actions, QObject *parent) : QAbstractTableModel(parent) {
//	this->actions = actions;
//}

QModelIndex DkShortcutsModel::index(int row, int column, const QModelIndex &parent) const {
	
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
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

	if (parentItem == rootItem)
		return QModelIndex();

	//qDebug() << "creating index for: " << childItem->data(0);

	return createIndex(parentItem->row(), 0, parentItem);
}

int DkShortcutsModel::rowCount(const QModelIndex& parent) const {

	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int DkShortcutsModel::columnCount(const QModelIndex& parent) const {

	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
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

	return rootItem->data(section);
} 

bool DkShortcutsModel::setData(const QModelIndex& index, const QVariant& value, int role) {

	if (!index.isValid() || role != Qt::EditRole)
		return false;

	if (index.column() == 1) {

		QKeySequence ks = value.value<QKeySequence>();
		if (index.column() == 1) {
			TreeItem* duplicate = rootItem->find(ks, index.column());
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

	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags DkShortcutsModel::flags(const QModelIndex& index) const {

	if (!index.isValid())
		return Qt::ItemIsEditable;

	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

	//// no editing on root items
	//if (item->parent() == rootItem)
	//	return QAbstractTableModel::flags(index);

	Qt::ItemFlags flags;

	if (index.column() == 0)
		flags = QAbstractTableModel::flags(index);
	if (index.column() == 1)
		flags = QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

	return flags;
}

void DkShortcutsModel::addDataActions(QVector<QAction*> actions, QString name) {

	// create root
	QVector<QVariant> menuData;
	menuData << name;

	TreeItem* menuItem = new TreeItem(menuData, rootItem);


	for (int idx = 0; idx < actions.size(); idx++) {

		QString text = actions[idx]->text();
		text.remove("&");

		QVector<QVariant> actionData;
		actionData << text << actions[idx]->shortcut();

		TreeItem* dataItem = new TreeItem(actionData, menuItem);
		menuItem->appendChild(dataItem);

		qDebug() << "adding: " << text;
		
	}

	rootItem->appendChild(menuItem);

	qDebug() << "menu item has: " << menuItem->childCount();

}

// DkShortcutsDialog --------------------------------------------------------------------
DkShortcutsDialog::DkShortcutsDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	createLayout();
}

void DkShortcutsDialog::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);


	//qDebug() << "rows: " << model->rowCount() << " column: " << model->columnCount();
	

	// register our special shortcut editor
	QItemEditorFactory *factory = new QItemEditorFactory;

	QItemEditorCreatorBase *shortcutListCreator =
		new QStandardItemEditorCreator<DkShortcutEditor>();

	factory->registerEditor(QVariant::KeySequence, shortcutListCreator);

	QItemEditorFactory::setDefaultFactory(factory);

	model = new DkShortcutsModel();

	treeView = new QTreeView();
	treeView->setModel(model);

	notificationLabel = new QLabel();
	notificationLabel->setStyleSheet("QLabel{color: #AA4242;}");
	//notificationLabel->setTextFormat(Qt::)
	
	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(treeView);
	layout->addWidget(notificationLabel);
	//layout->addSpacing()
	layout->addWidget(buttons);
}

void DkShortcutsDialog::addActions(const QVector<QAction*> actions, const QString name) {

	// add our own 'shortcut delegate	TODO: update
	//DkShortcutDelegate* scDelegate = new DkShortcutDelegate();

	//actionTable->verticalHeader()->hide();
	//actionTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	//actionTable->setMidLineWidth(width());
	//actionTable->setItemDelegate(scDelegate);
	//connect(scDelegate, SIGNAL(notifyDuplicate(QString)), notificationLabel, SLOT(setText(QString)));

	//treeView->setModel(model);

	model->addDataActions(actions, name);

}

void DkShortcutsDialog::contextMenu(const QPoint& cur) {

}

QVector<QPair<QString, QKeySequence> > DkShortcutsDialog::convertActions(const QVector<QAction*>& actions) {

	QVector<QPair<QString, QKeySequence> > cvtActions;

	for (int idx = 0; idx < actions.size(); idx++) {
		QString text = actions[idx]->text();
		text.remove("&");
		cvtActions.push_back(QPair<QString, QKeySequence>(text, actions[idx]->shortcut()));
	}

	return cvtActions;
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

DkPrintPreviewDialog::DkPrintPreviewDialog(QImage img, float dpi, QPrinter* printer, QWidget* parent, Qt::WindowFlags flags) 
		: QMainWindow(parent, flags) {
	this->img = img;
	this->printer = printer;
	this->dpi = dpi;
	this->origdpi = dpi;
	printDialog = 0;
	imgTransform = QTransform();
	init();
}

void DkPrintPreviewDialog::init() {
	
	if (!printer) {
#ifdef Q_WS_WIN
		printer = new QPrinter(QPrinter::HighResolution);
#else
		printer = new QPrinter;
#endif
	}
	
	preview = new DkPrintPreviewWidget(printer, this);
	connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintRequested(QPrinter*)));
	connect(preview, SIGNAL(zoomChanged()), this, SLOT(updateZoomFactor()));
	
	createIcons();
	setup_Actions();
	createLayout();
	setMinimumHeight(600);
	setMinimumWidth(800);

	QRectF rect = printer->pageRect();
	qreal scaleFactor;
	QSizeF paperSize = printer->paperSize(QPrinter::Inch);
	QRectF pageRectInch = printer->pageRect(QPrinter::Inch);
	
	// scale image to fit on paper
	if (rect.width()/img.width() < rect.height()/img.height()) {
		scaleFactor = rect.width()/(img.width()+FLT_EPSILON);		
	} else {
		scaleFactor = rect.height()/(img.height()+FLT_EPSILON);
	}

	float inchW = printer->pageRect(QPrinter::Inch).width();
	float pxW = printer->pageRect().width();
	dpi = (pxW/inchW)/scaleFactor;


	qDebug() << "img:" << img.size();
	qDebug() << "paperInch:" << paperSize;
	qDebug() << "rect:" << rect;
	qDebug() << "rectInch:" << pageRectInch;
	qDebug() << "scaleFactor:" << scaleFactor;
	qDebug() << "dpi:" << dpi;
	qDebug() << "origDpi:" << origdpi;

	// use at least 150 dpi as default if image has less then 150
	if ((origdpi < 150 || dpi < 150) && scaleFactor > 1) {
		dpi = 150;
		scaleFactor = (pxW/inchW)/dpi;
	}

	imgTransform.scale(scaleFactor, scaleFactor);

	dpiFactor->lineEdit()->setText(QString().sprintf("%.0f", dpi)+dpiEditorSuffix);
	centerImage();
	updateZoomFactor();
}

void DkPrintPreviewDialog::createIcons() {

	icons.resize(print_end);

	icons[print_fit_width]	= QIcon(":/nomacs/img/fit-width.png");
	icons[print_fit_page]	= QIcon(":/nomacs/img/zoomReset.png");
	icons[print_zoom_in]	= QIcon(":/nomacs/img/zoom-in.png");
	icons[print_zoom_out]	= QIcon(":/nomacs/img/zoom-out.png");
	icons[print_reset_dpi]	= QIcon(":/nomacs/img/zoom100.png");
	icons[print_landscape]	= QIcon(":/nomacs/img/landscape.png");
	icons[print_portrait]	= QIcon(":/nomacs/img/portrait.png");
	icons[print_setup]		= QIcon(":/nomacs/img/print-setup.png");
	icons[print_printer]	= QIcon(":/nomacs/img/printer.png");

	if (!DkSettings::Display::defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++)
			icons[idx].addPixmap(DkUtils::colorizePixmap(icons[idx].pixmap(100), DkSettings::Display::iconColor));
	}
}

void DkPrintPreviewDialog::setup_Actions() {
	
	fitGroup = new QActionGroup(this);
	
	fitWidthAction = fitGroup->addAction(icons[print_fit_width], tr("Fit width"));
	fitPageAction = fitGroup->addAction(icons[print_fit_page], tr("Fit page"));
	fitWidthAction->setObjectName(QLatin1String("fitWidthAction"));
	fitPageAction->setObjectName(QLatin1String("fitPageAction"));
	fitWidthAction->setCheckable(true);
	fitPageAction->setCheckable(true);
	//setIcon(fitWidthAction, QLatin1String("fit-width"));
	//setIcon(fitPageAction, QLatin1String("fit-page"));
	QObject::connect(fitGroup, SIGNAL(triggered(QAction*)), this, SLOT(fitImage(QAction*)));

	// Zoom
	zoomGroup = new QActionGroup(this);
	
	zoomInAction = zoomGroup->addAction(icons[print_zoom_in], tr("Zoom in"));
	zoomInAction->setShortcut(Qt::Key_Plus);
	//preview->addAction(zoomInAction);
	//addAction(zoomInAction);
	//zoomInAction->setShortcut(QKeySequence::AddTab);
		//addAction(zoomInAction);
	zoomOutAction = zoomGroup->addAction(icons[print_zoom_out], tr("Zoom out"));
	zoomOutAction->setShortcut(QKeySequence(Qt::Key_Minus));
	//addAction(zoomOutAction);
	//preview->addAction(zoomOutAction);
	//setIcon(zoomInAction, QLatin1String("zoom-in"));
	//setIcon(zoomOutAction, QLatin1String("zoom-out"));

	// Portrait/Landscape
	orientationGroup = new QActionGroup(this);
	portraitAction = orientationGroup->addAction(icons[print_portrait], tr("Portrait"));
	landscapeAction = orientationGroup->addAction(icons[print_landscape], tr("Landscape"));
	portraitAction->setCheckable(true);
	landscapeAction->setCheckable(true);
	//setIcon(portraitAction, QLatin1String("layout-portrait"));
	//setIcon(landscapeAction, QLatin1String("layout-landscape"));
	QObject::connect(portraitAction, SIGNAL(triggered(bool)), preview, SLOT(setPortraitOrientation()));
	QObject::connect(portraitAction, SIGNAL(triggered(bool)), this, SLOT(centerImage()));
	QObject::connect(landscapeAction, SIGNAL(triggered(bool)), preview, SLOT(setLandscapeOrientation()));
	QObject::connect(landscapeAction, SIGNAL(triggered(bool)), this, SLOT(centerImage()));


	// Print
	printerGroup = new QActionGroup(this);
	printAction = printerGroup->addAction(icons[print_printer], tr("Print"));
	pageSetupAction = printerGroup->addAction(icons[print_setup], tr("Page setup"));
	//setIcon(printAction, QLatin1String("print"));
	//setIcon(pageSetupAction, QLatin1String("page-setup"));
	QObject::connect(printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
	QObject::connect(pageSetupAction, SIGNAL(triggered(bool)), this, SLOT(pageSetup()));

	dpiGroup = new QActionGroup(this);
	resetDpiAction = dpiGroup->addAction(icons[print_reset_dpi], tr("Reset dpi"));
	//setIcon(resetDpiAction, QLatin1String("fit-width"));
	QObject::connect(resetDpiAction, SIGNAL(triggered(bool)), this, SLOT(resetDpi()));

}

void DkPrintPreviewDialog::createLayout() {

	zoomFactor = new QComboBox;
	zoomFactor->setEditable(true);
	zoomFactor->setMinimumContentsLength(7);
	zoomFactor->setInsertPolicy(QComboBox::NoInsert);
	QLineEdit *zoomEditor = new QLineEdit;
	zoomEditor->setValidator(new DkPrintPreviewValidator("%", 1,1000, 1, zoomEditor));
	zoomFactor->setLineEdit(zoomEditor);
	static const short factorsX2[] = { 25, 50, 100, 200, 250, 300, 400, 800, 1600 };
	for (int i = 0; i < int(sizeof(factorsX2) / sizeof(factorsX2[0])); ++i)
		zoomFactor->addItem(QString::number(factorsX2[i] / 2.0)+"%");
	QObject::connect(zoomFactor->lineEdit(), SIGNAL(editingFinished()), this, SLOT(zoomFactorChanged()));
	QObject::connect(zoomFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(zoomFactorChanged()));

	QString zoomTip = tr("keep ALT key pressed to zoom with the mouse wheel");
	zoomFactor->setToolTip(zoomTip);
	zoomEditor->setToolTip(zoomTip);
	zoomOutAction->setToolTip(zoomTip);
	zoomInAction->setToolTip(zoomTip);

	// dpi selection
	dpiFactor = new QComboBox;
	dpiFactor->setEditable(true);
	dpiFactor->setMinimumContentsLength(5);
	dpiFactor->setInsertPolicy(QComboBox::NoInsert);
	QLineEdit* dpiEditor = new QLineEdit;
	dpiEditorSuffix = " dpi";
	dpiEditor->setValidator(new DkPrintPreviewValidator(dpiEditorSuffix, 1,1000, 1, zoomEditor));
	dpiFactor->setLineEdit(dpiEditor);
	static const short dpiFactors[] = {72, 150, 300, 600};
	for (int i = 0; i < int(sizeof(dpiFactors) / sizeof(dpiFactors[0])); i++)
		dpiFactor->addItem(QString::number(dpiFactors[i])+dpiEditorSuffix);
	QObject::connect(dpiFactor->lineEdit(), SIGNAL(editingFinished()), this, SLOT(dpiFactorChanged()));
	QObject::connect(dpiFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(dpiFactorChanged()));

	QToolBar *toolbar = new QToolBar(tr("Print Preview"), this);
	toolbar->addAction(fitWidthAction);
	toolbar->addAction(fitPageAction);
	toolbar->addSeparator();
	toolbar->addWidget(zoomFactor);
	toolbar->addAction(zoomInAction);
	toolbar->addAction(zoomOutAction);
	toolbar->addSeparator();
	toolbar->addWidget(dpiFactor);
	toolbar->addAction(resetDpiAction);
	toolbar->addSeparator();
	toolbar->addAction(portraitAction);
	toolbar->addAction(landscapeAction);
	toolbar->addSeparator();
	//toolbar->addAction(firstPageAction);
	//toolbar->addAction(prevPageAction);
	//toolbar->addSeparator();
	toolbar->addAction(pageSetupAction);
	toolbar->addAction(printAction);

	if (DkSettings::Display::toolbarGradient) {

		QColor hCol = DkSettings::Display::highlightColor;
		hCol.setAlpha(80);

		toolbar->setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}

	if (DkSettings::Display::smallIcons)
		toolbar->setIconSize(QSize(16, 16));
	else
		toolbar->setIconSize(QSize(32, 32));


	// Cannot use the actions' triggered signal here, since it doesn't autorepeat
	QToolButton *zoomInButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomInAction));
	QToolButton *zoomOutButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomOutAction));
	zoomInButton->setAutoRepeat(true);
	zoomInButton->setAutoRepeatInterval(200);
	zoomInButton->setAutoRepeatDelay(200);
	zoomOutButton->setAutoRepeat(true);
	zoomOutButton->setAutoRepeatInterval(200);
	zoomOutButton->setAutoRepeatDelay(200);
	QObject::connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
	QObject::connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));


	this->addToolBar(toolbar);

	this->setCentralWidget(preview);
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
	QSize size = img.size();
	painter.setWorldTransform(imgTransform);
	painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
	painter.setWindow(img.rect());
	painter.drawImage(0, 0, img);

	painter.end();

}

void DkPrintPreviewDialog::fitImage(QAction* action) {
	setFitting(true);
	if (action == fitPageAction)
		preview->fitInView();
	else
		preview->fitToWidth();
	updateZoomFactor();
}

void DkPrintPreviewDialog::centerImage() {
	QRect imgRect = img.rect();
	QRectF transRect = imgTransform.mapRect(imgRect);
	qreal xtrans = 0, ytrans = 0;
	xtrans = ((printer->pageRect().width() - transRect.width())/2);
	ytrans = (printer->pageRect().height() - transRect.height())/2;

	imgTransform.translate(-imgTransform.dx()/(imgTransform.m11()+DBL_EPSILON), -imgTransform.dy()/(imgTransform.m22()+DBL_EPSILON)); // reset old transformation

	imgTransform.translate(xtrans/(imgTransform.m11()+DBL_EPSILON), ytrans/(imgTransform.m22()+DBL_EPSILON));
	preview->updatePreview();
}

void DkPrintPreviewDialog::setFitting(bool on) {
	if (isFitting() == on)
		return;
	fitGroup->setExclusive(on);
	if (on) {
		QAction* action = fitWidthAction->isChecked() ? fitWidthAction : fitPageAction;
		action->setChecked(true);
		if (fitGroup->checkedAction() != action) {
			// work around exclusitivity problem
			fitGroup->removeAction(action);
			fitGroup->addAction(action);
		}
	} else {
		fitWidthAction->setChecked(false);
		fitPageAction->setChecked(false);
	}
}

void DkPrintPreviewDialog::zoomIn() {
	setFitting(false);
	preview->zoomIn();
	updateZoomFactor();
}

void DkPrintPreviewDialog::zoomOut() {
	setFitting(false);
	preview->zoomOut();
	updateZoomFactor();
}

void DkPrintPreviewDialog::zoomFactorChanged() {
	QString text = zoomFactor->lineEdit()->text();
	bool ok;
	qreal factor = text.remove(QLatin1Char('%')).toFloat(&ok);
	factor = qMax(qreal(1.0), qMin(qreal(1000.0), factor));
	if (ok) {
		preview->setZoomFactor(factor/100.0);
		zoomFactor->setEditText(QString::fromLatin1("%1%").arg(factor));
		setFitting(false);
		updateZoomFactor();
	}
	updateZoomFactor();
}

void DkPrintPreviewDialog::updateZoomFactor()
{
	zoomFactor->lineEdit()->setText(QString().sprintf("%.1f%%", preview->zoomFactor()*100));
}

void DkPrintPreviewDialog::dpiFactorChanged() {
	
	QString text = dpiFactor->lineEdit()->text();
	bool ok;
	qreal factor = text.remove(dpiEditorSuffix).toFloat(&ok);
	if (ok) {
		imgTransform.reset();

		float inchW = printer->pageRect(QPrinter::Inch).width();
		float pxW = printer->pageRect().width();
		float scaleFactor = (((pxW/inchW)/factor));

		imgTransform.scale(scaleFactor, scaleFactor);

	}
	centerImage();
	preview->updatePreview();
}

void DkPrintPreviewDialog::updateDpiFactor(qreal dpi) {
	dpiFactor->lineEdit()->setText(QString().sprintf("%.0f", dpi)+dpiEditorSuffix);
}

void DkPrintPreviewDialog::resetDpi() {
	updateDpiFactor(origdpi);
	dpiFactorChanged();
}

void DkPrintPreviewDialog::pageSetup() {
	QPageSetupDialog pageSetup(printer, this);
	if (pageSetup.exec() == QDialog::Accepted) {
		// update possible orientation changes
		if (preview->orientation() == QPrinter::Portrait) {
			portraitAction->setChecked(true);
			preview->setPortraitOrientation();
		}else {
			landscapeAction->setChecked(true);
			preview->setLandscapeOrientation();
		}
		centerImage();
	}
}

void DkPrintPreviewDialog::print() {
//#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
//	if (printer->outputFormat() != QPrinter::NativeFormat) {
//		QString title;
//		QString suffix;
//		if (printer->outputFormat() == QPrinter::PdfFormat) {
//			title = QCoreApplication::translate("QPrintPreviewDialog", "Export to PDF");
//			suffix = QLatin1String(".pdf");
//		} else {
//			title = QCoreApplication::translate("QPrintPreviewDialog", "Export to PostScript");
//			suffix = QLatin1String(".ps");
//		}
//		QString fileName = QFileDialog::getSaveFileName(this, title, printer->outputFileName(),
//			QLatin1Char('*') + suffix);
//		if (!fileName.isEmpty()) {
//			if (QFileInfo(fileName).suffix().isEmpty())
//				fileName.append(suffix);
//			printer->setOutputFileName(fileName);
//		}
//		if (!printer->outputFileName().isEmpty())
//			preview->print();
//		this->accept();
//		return;
//	}
//#endif

	if (!printDialog)
		printDialog = new QPrintDialog(printer, this);
	if (printDialog->exec() == QDialog::Accepted) {
		preview->print();
		this->close();
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
	if (DkSettings::Display::invertZoom)
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

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(slider);
	layout->addWidget(buttons);

	//setStandardButton 

}

} // close namespace


