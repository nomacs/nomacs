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

#include <fstream>

#include "DkOcrPlugin.h"
#include "DkUtils.h"
#include "DkOcr.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end
#include "TesseractSettingsDialog.h"

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

namespace nmc {
	DkOcrPlugin::DkOcrPlugin(QObject* parent) : QObject(parent) {
		qDebug() << "DkOcrPlugin ctor";

		// create run IDs
		QVector<QString> runIds;
		runIds.resize(id_end);
		runIds[ACTION_IMG2TXT] = "OCR_PLUGIN_IMG2TXT";
		runIds[ACTION_IMG2CLIP] = "OCR_PLUGIN_IMG2CLIP";
		runIds[ACTION_LANGUAGEDIALOG] = "OCR_PLUGIN_LANGUAGESETTINGS";
		mRunIDs = runIds.toList();

		// create menu actions
		QVector<QString> menuNames;
		menuNames.resize(id_end);

		menuNames[ACTION_IMG2TXT] = tr("Text to file");
		menuNames[ACTION_IMG2CLIP] = tr("Text to clipboard");
		menuNames[ACTION_LANGUAGEDIALOG] = tr("Language Settings");
		mMenuNames = menuNames.toList();

		// create menu status tips
		QVector<QString> statusTips;
		statusTips.resize(id_end);

		statusTips[ACTION_IMG2TXT] = tr("dontknow1");
		statusTips[ACTION_IMG2CLIP] = tr("dontknow2");
		statusTips[ACTION_LANGUAGEDIALOG] = tr("dontknow3");
		mMenuStatusTips = statusTips.toList();

		// save default settings
		nmc::DefaultSettings settings;
		loadSettings(settings);
		saveSettings(settings);
	}

	void DkOcrPlugin::loadSettings(QSettings & settings) {
		settings.beginGroup(name());
		int mIdx = settings.value("Test", 1).toInt();
		mTessConfigFile = settings.value("Tesseract Configuration", "").toString();
		mSelectedLanguages = settings.value("Selected Languages", "").toStringList();
		settings.endGroup();
	}

	void DkOcrPlugin::saveSettings(QSettings & settings) const {
		settings.beginGroup(name());
		settings.setValue("Tesseract Configuration", mTessConfigFile);
		settings.setValue("Selected Languages", mSelectedLanguages);
		settings.endGroup();
	}

	void DkOcrPlugin::preLoadPlugin() const {
		qDebug() << "preLoadPlugin";
	}

	void DkOcrPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
		/*qDebug() << "postLoadPlugin:";
		for (auto bi : batchInfo)
		{
			qDebug() << "filePath: " << bi->filePath();
		}*/
	}

	QString DkOcrPlugin::name() const {
		return "DkOcrPlugin";
	}

	DkOcrPlugin::~DkOcrPlugin() {
	}

	/*QListWidget* DkOcrPlugin::buildLanguageList(const QList<QString>& langList) const {
		auto* languagelist_widget = new QListWidget();

		for (const QString lang : langList)
		{
			QListWidgetItem* langListItem = new QListWidgetItem(lang);
			langListItem->setFlags(langListItem->flags() | Qt::ItemIsUserCheckable); // set checkable flag
			langListItem->setCheckState(Qt::Unchecked); // AND initialize check state
			languagelist_widget->addItem(langListItem);
		}

		return languagelist_widget;
	}*/

	QList<QAction*> DkOcrPlugin::createActions(QWidget* parent)  {

		if (mActions.empty()) {			
			
			QAction* ca = new QAction(mMenuNames[ACTION_IMG2TXT], parent);
			ca->setObjectName(mMenuNames[ACTION_IMG2TXT]);
			ca->setStatusTip(mMenuStatusTips[ACTION_IMG2TXT]);
			ca->setData(mRunIDs[ACTION_IMG2TXT]);
			mActions.append(ca);

			ca = new QAction(mMenuNames[ACTION_IMG2CLIP], parent);
			ca->setObjectName(mMenuNames[ACTION_IMG2CLIP]);
			ca->setStatusTip(mMenuStatusTips[ACTION_IMG2CLIP]);
			ca->setData(mRunIDs[ACTION_IMG2CLIP]);
			mActions.append(ca);

			ca = new QAction(mMenuNames[ACTION_LANGUAGEDIALOG], parent);
			ca->setObjectName(mMenuNames[ACTION_LANGUAGEDIALOG]);
			ca->setStatusTip(mMenuStatusTips[ACTION_LANGUAGEDIALOG]);
			ca->setData(mRunIDs[ACTION_LANGUAGEDIALOG]);
			mActions.append(ca);
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
		QSharedPointer<DkBatchInfo>& batchInfo) const {

		if (!imgC)
			return imgC;

		qDebug() << "runPlugin";

		if (runID == mRunIDs[ACTION_IMG2TXT]) {
			auto txtOutputPath = saveInfo.outputFilePath() + ".txt";

			auto api = new Ocr::TesseractApi();
			if (!api->initialize(mSelectedLanguages, mTessConfigFile))
				return imgC;

			auto text = api->runOcr(imgC->image());

			std::ofstream oFile(txtOutputPath.toStdString());
			oFile << text.toStdString();
			oFile.close();

			nmc::DkUtils::showViewportMessage(
				QObject::tr("Text saved to %1").arg(QFileInfo(txtOutputPath).fileName()));

		}
		else if (runID == mRunIDs[ACTION_IMG2CLIP]) {

			auto api = new Ocr::TesseractApi();
			api->initialize(mSelectedLanguages, mTessConfigFile);
			auto text = api->runOcr(imgC->image());

			QClipboard *p_Clipboard = QApplication::clipboard();
			p_Clipboard->setText(text);

			nmc::DkUtils::showViewportMessage(
				QObject::tr("Text copied to clipboard..."));
		}
		else if (runID == mRunIDs[ACTION_LANGUAGEDIALOG]) {

			auto* dialog = new TesseractSettingsDialog(getMainWindow(), mSelectedLanguages);
			dialog->init();
			dialog->show();
			dialog->setModal(true);
			dialog->raise();
			dialog->activateWindow();

			auto p = mSelectedLanguages;
			connect(dialog, &TesseractSettingsDialog::closeSignal, this, &DkOcrPlugin::languageSelectionChanged_);
		}

		return imgC;
	}

	void DkOcrPlugin::languageSelectionChanged_(QStringList langs)
	{
		mSelectedLanguages = langs;
		nmc::DefaultSettings settings;
		saveSettings(settings);
	}

	QImage DkOcrPlugin::image() const {

		return QImage(":/OcrPlugin/img/description.png");
	}
}