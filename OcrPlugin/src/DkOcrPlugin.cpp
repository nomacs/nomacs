/*******************************************************************************************************
 DkOcrPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 Dominik Schoerkhuber

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

#include "DkOcrPlugin.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

 /*******************************************************************************************************
  * DkOcrPlugin     	- enter the plugin class name (e.g. DkPageExtractionPlugin)
  * Dominik Schoerkhuber- your name/pseudonym whatever
  * 21.10.2016			- today...
  * Optical Character Recognition Plugin - describe your plugin in one sentence
  * #MENU_NAME			- a user friendly name (e.g. Flip Image)
  * #MENU_STATUS_TIPP	- status tip of your plugin
  * #RUN_ID_1			- generate an ID using: GUID without hyphens generated at http://www.guidgenerator.com/
  * ID_ACTION1			- your action name (e.g. id_flip_horizontally)
  * #ACTION_NAME1		- your action name (e.g. Flip Horizotally - user friendly!)
  * #ACTION_TIPP1		- your action status tip (e.g. Flips an image horizontally - user friendly!)
  *******************************************************************************************************/

#include <iostream>
#include <fstream>

//#include <QLibrary>
#include "DkOcr.h"
#include "DkSettings.h"
#include <DkBaseViewPort.h>
//#include <DkLoader/DkImageContainer.h>

namespace nmc {

	/**
	*	Constructor
	**/
	DkOcrPlugin::DkOcrPlugin(QObject* parent) : QObject(parent) {
		
		/*DefaultSettings s;
		s.beginGroup(name());
		s.endGroup();*/

		qDebug() << "DkOcrPlugin ctor";

		// create run IDs
		QVector<QString> runIds;
		runIds.resize(id_end);
		runIds[ACTION_TESTRUN] = "OCR_PLUGIN_TEST_RUN";
		mRunIDs = runIds.toList();

		// create menu actions
		QVector<QString> menuNames;
		menuNames.resize(id_end);

		menuNames[ACTION_TESTRUN] = tr("Testrun");
		mMenuNames = menuNames.toList();

		// create menu status tips
		QVector<QString> statusTips;
		statusTips.resize(id_end);

		statusTips[ACTION_TESTRUN] = tr("#ACTION_TIPP2");
		mMenuStatusTips = statusTips.toList();

		// save default settings
		nmc::DefaultSettings settings;
		saveSettings(settings);
		loadSettings(settings);
	}

	void DkOcrPlugin::loadSettings(QSettings & settings) {
		settings.beginGroup(name());
		int mIdx = settings.value("Test", 1).toInt();
		settings.endGroup();
	}

	void DkOcrPlugin::saveSettings(QSettings & settings) const {
		settings.beginGroup(name());
		settings.setValue("Test", 1);
		settings.endGroup();
	}

	void DkOcrPlugin::preLoadPlugin() const {
		qDebug() << "preLoadPlugin";
	}

	void DkOcrPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
		// Create Settings Toolbar

		/**/

		qDebug() << "postLoadPlugin:";
		for (auto bi : batchInfo)
		{
			qDebug() << "filePath: " << bi->filePath();
		}
	}

	void DkOcrPlugin::createUi()
	{
		QMainWindow* mainWindow = getMainWindow();
		mDockWidgetSettings = new QDockWidget(tr("Ocr Plugin Settings"), mainWindow);
		mainWindow->addDockWidget(Qt::RightDockWidgetArea, mDockWidgetSettings);

		te_resultText = new QTextEdit();
		//te_resultText->minimumHeight(100);

		QPushButton* btn_runocr = new QPushButton(tr("Run Ocr"));
		QPushButton* btn_copytoclipboard = new QPushButton(tr("Copy to Clipboard"));
		QPushButton* btn_sendtoeditor = new QPushButton(tr("Open in Editor"));

		connect(btn_runocr, &QPushButton::pressed, [&]()
		{
			qDebug("run ocr pressed");
			mActions[ACTION_TESTRUN]->trigger();
		});

		connect(btn_copytoclipboard, &QPushButton::pressed, [&]()
		{
			qDebug("copy to clip pressed");

			QApplication::clipboard()->setText(te_resultText->toPlainText());
		});

		connect(btn_sendtoeditor, &QPushButton::pressed, [&]()
		{
			qDebug("open in editor pressed");
			QString filename = GetRandomString() + ".txt";
			auto saveloc = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + filename;
			qDebug("temp file location %s", saveloc);

			QFile outputFile(saveloc);
			outputFile.open(QIODevice::WriteOnly);

			if (!outputFile.isOpen()) {
				qDebug() << "Error, unable to open" << saveloc;
				return;
			}

			QTextStream outStream(&outputFile);
			outStream << te_resultText->toPlainText();
			outputFile.close();

			QDesktopServices::openUrl(QUrl("file:///" + saveloc));
		});

		QVBoxLayout* layout = new QVBoxLayout();
		layout->addWidget(te_resultText);

		QHBoxLayout* btn_layout = new QHBoxLayout();
		btn_layout->addWidget(btn_runocr);
		btn_layout->addWidget(btn_copytoclipboard);
		btn_layout->addWidget(btn_sendtoeditor);
		layout->addLayout(btn_layout);

		auto api = new Ocr::TesseractApi();
		api->initialize({});

		auto* langlist = buildLanguageList(api->getAvailableLanguages());
		connect(langlist, &QListWidget::itemChanged, [&](QListWidgetItem* item) {
			qInfo() << "selection changed";

			std::vector<std::string> langs;
			for (int i = 0; i < langlist->count(); ++i) {
				auto* items = langlist->item(i);
				if (items->checkState() == Qt::Checked) {
					langs.push_back(items->text().toStdString());
				}
			}

			//api->initialize(langs);
		});

		auto* languagelist_layout = new QHBoxLayout();
		languagelist_layout->addWidget(langlist);
		layout->addLayout(languagelist_layout);


		//mDockWidgetSettings->setLayout(layout);
		QGroupBox* widget = new QGroupBox();
		widget->setLayout(layout);
		//mDockWidgetSettings->setWidget(widget);

		


			//		

			/*QLibrary libTesseract("E:/dev/tesseract/build_x86/bin/Debug/tesseract305d.dll");
			libTesseract.load();
			if(!libTesseract.isLoaded())
			{
			std::cout << "could not load lib Tesseract" << std::endl;
			exit(1);
			}

			QLibrary libLept("E:/dev/tesseract/build_x86/bin/Debug/liblept171.dll");
			libLept.load();
			if (!libLept.isLoaded())
			{
			std::cout << "could not load lib lib Leptonica" << std::endl;
			exit(1);
			}*/
	}

	QString DkOcrPlugin::name() const
	{
		return "DkOcrPlugin";
	}

	/*bool DkOcrPlugin::createViewPort(QWidget* parent)
	{
		mViewport = new DkPluginViewPort(parent);
		return true;
	}

	DkPluginViewPort* DkOcrPlugin::getViewPort()
	{
		return mViewport;
	}

	void DkOcrPlugin::setVisible(bool visible)
	{
		if (mViewport)
			mViewport->setVisible(visible);
	}*/

	/**
	*	Destructor
	**/
	DkOcrPlugin::~DkOcrPlugin() {
	}

	QListWidget* DkOcrPlugin::buildLanguageList(const QList<QString>& langList) const
	{
		auto* languagelist_widget = new QListWidget();

		for (const QString lang : langList)
		{
			QListWidgetItem* langListItem = new QListWidgetItem(lang);
			langListItem->setFlags(langListItem->flags() | Qt::ItemIsUserCheckable); // set checkable flag
			langListItem->setCheckState(Qt::Unchecked); // AND initialize check state
			languagelist_widget->addItem(langListItem);
		}

		return languagelist_widget;
	}

	QList<QAction*> DkOcrPlugin::createActions(QWidget* parent)  {

		//createUi();

		if (mActions.empty()) {			
			
			QAction* ca = new QAction(mMenuNames[ACTION_TESTRUN], parent);
			ca->setObjectName(mMenuNames[ACTION_TESTRUN]);
			ca->setStatusTip(mMenuStatusTips[ACTION_TESTRUN]);
			ca->setData(mRunIDs[ACTION_TESTRUN]);	// runID needed for calling function runPlugin()
			mActions.append(ca);

			// additional action
			//mActions.append(mDockWidgetSettings->toggleViewAction());
		}

		return mActions;
	}

	QList<QAction*> DkOcrPlugin::pluginActions() const {
		return mActions;
	}

	QSharedPointer<nmc::DkImageContainer> DkOcrPlugin::runPlugin(
		const QString & runID,
		QSharedPointer<DkImageContainer> imgC,
		const DkSaveInfo& saveInfo,
		QSharedPointer<DkBatchInfo>& batchInfo) const
	{

		qDebug() << "runPlugin";

		auto txtOutputPath = saveInfo.outputFilePath() + ".txt";

		auto api = new Ocr::TesseractApi();
		api->initialize({});
		auto text = api->runOcr(imgC->image());

		std::ofstream oFile(txtOutputPath.toStdString());
		oFile << text.toStdString();
		oFile.close();


		/*QMainWindow* mainWindow = getMainWindow();
		QDockWidget* mDockWidgetSettings = new QDockWidget(tr("Ocr Plugin Settings"), mainWindow);

		if (runID == mRunIDs[ACTION_TESTRUN]) {

			qInfo("testrun action");

			auto img = imgC->image();

			auto api = new Ocr::TesseractApi();
			api->initialize({ "eng" });
			auto text = api->runOcr(img);
			te_resultText->setText(text);
			imgC->setImage(img, "OCR Image");
		}

		// wrong runID? - do nothing*/
		return imgC;
	}

	QString DkOcrPlugin::GetRandomString() const
	{
		const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
		const int randomStringLength = 12; // assuming you want random strings of 12 characters

		QString randomString;
		for (int i = 0; i<randomStringLength; ++i)
		{
			int index = qrand() % possibleCharacters.length();
			QChar nextChar = possibleCharacters.at(index);
			randomString.append(nextChar);
		}
		return randomString;
	}

	QImage DkOcrPlugin::image() const {

		return QImage(":/nomacsPluginPaint/img/description.png");
	}
}