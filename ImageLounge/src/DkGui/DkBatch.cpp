/*******************************************************************************************************
 DkNoMacs.cpp
 Created on:	26.10.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include "DkBatch.h"
#include "DkProcess.h"
#include "DkDialog.h"
#include "DkWidgets.h"
#include "DkThumbsWidgets.h"
#include "DkUtils.h"
#include "DkImageLoader.h"
#include "DkSettings.h"
#include "DkMessageBox.h"
#include "DkPluginManager.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDialogButtonBox>
#include <QLabel>
#include <QListView>
#include <QLineEdit>
#include <QFileDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QButtonGroup>
#include <QProgressBar>
#include <QTextEdit>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QMessageBox>
#include <QApplication>
#include <QTextBlock>
#include <QDropEvent>
#include <QMimeData>
#include <QSplitter>
#include <QListWidget>
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkBatchWidget --------------------------------------------------------------------
DkBatchWidget::DkBatchWidget(const QString& titleString, const QString& headerString, QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f) {
	mTitleString = titleString;
	mHeaderString = headerString;
	createLayout();
}

void DkBatchWidget::createLayout() {
	
	mShowButton = new DkButton(QIcon(":/nomacs/img/minus.svg"), QIcon(":/nomacs/img/plus.svg"), tr("Plus"));
	mShowButton->setFixedSize(QSize(16,16));
	mShowButton->setObjectName("showSelectionButton");
	mShowButton->setCheckable(true);
	mShowButton->setChecked(true);

	mTitleLabel = new QLabel(mTitleString);
	mTitleLabel->setObjectName("DkBatchTitle");
	mTitleLabel->setAlignment(Qt::AlignBottom);
	mHeaderLabel = new QLabel(mHeaderString);
	mHeaderLabel->setObjectName("DkDecentInfo");
	mHeaderLabel->setAlignment(Qt::AlignBottom);
	
	QWidget* headerWidget = new QWidget(this);
	QHBoxLayout* headerWidgetLayout = new QHBoxLayout(headerWidget);
	headerWidgetLayout->setContentsMargins(0,0,0,0);
	headerWidgetLayout->addWidget(mShowButton);
	headerWidgetLayout->addWidget(mTitleLabel);
	headerWidgetLayout->addWidget(mHeaderLabel);
	headerWidgetLayout->addStretch();

	mBatchWidgetLayout = new QVBoxLayout(this);
	mBatchWidgetLayout->setContentsMargins(0,0,0,0);
	mBatchWidgetLayout->addWidget(headerWidget);
	//mBatchWidgetLayout->addWidget(contentWidget);
	//mBatchWidgetLayout->addStretch();
	setLayout(mBatchWidgetLayout);
}

void DkBatchWidget::setContentWidget(QWidget* batchContent) {
	
	mBatchContent = dynamic_cast<DkBatchContent*>(batchContent);

	mBatchWidgetLayout->addWidget(batchContent);
	connect(mShowButton, SIGNAL(toggled(bool)), batchContent, SLOT(setVisible(bool)));
	connect(batchContent, SIGNAL(newHeaderText(const QString&)), this, SLOT(setHeader(const QString&)));
}

QWidget* DkBatchWidget::contentWidget() const {
	
	return dynamic_cast<QWidget*>(mBatchContent);
}

void DkBatchWidget::showContent(bool) {

	mShowButton->click();
	//contentWidget()->setVisible(show);
}

void DkBatchWidget::setTitle(const QString& titleString) {
	mTitleString = titleString;
	mTitleLabel->setText(titleString);
}

void DkBatchWidget::setHeader(const QString& headerString) {
	mHeaderString = headerString;
	mHeaderLabel->setText(headerString);
}

// DkInputTextEdit --------------------------------------------------------------------
DkInputTextEdit::DkInputTextEdit(QWidget* parent /* = 0 */) : QTextEdit(parent) {

	setAcceptDrops(true);
	connect(this, SIGNAL(textChanged()), this, SIGNAL(fileListChangedSignal()));
}

void DkInputTextEdit::appendFiles(const QStringList& fileList) {

	QStringList cFileList = getFileList();
	QStringList newFiles;

	// unique!
	for (const QString& cStr : fileList) {

		if (!cFileList.contains(cStr))
			newFiles.append(cStr);
	}

	if (!newFiles.empty()) {
		append(newFiles.join("\n"));
		fileListChangedSignal();
	}
}

void DkInputTextEdit::appendDir(const QString& newDir, bool recursive) {

	if (recursive) {
		qDebug() << "adding recursive...";
		QDir tmpDir = newDir;
		QFileInfoList subDirs = tmpDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);

		for (QFileInfo cDir : subDirs)
			appendDir(cDir.absoluteFilePath(), recursive);
	}

	QDir tmpDir = newDir;
	tmpDir.setSorting(QDir::LocaleAware);
	QFileInfoList fileList = tmpDir.entryInfoList(Settings::param().app().fileFilters);
	QStringList strFileList;

	for (QFileInfo entry : fileList) {
		strFileList.append(entry.absoluteFilePath());
	}

	qDebug() << "appending " << strFileList.size() << " files";

	appendFiles(strFileList);
}

void DkInputTextEdit::appendFromMime(const QMimeData* mimeData, bool recursive) {

	if (!mimeData || !mimeData->hasUrls())
		return;

	QStringList cFiles;

	for (QUrl url : mimeData->urls()) {

		QFileInfo cFile = DkUtils::urlToLocalFile(url);

		if (cFile.isDir())
			appendDir(cFile.absoluteFilePath(), recursive);
		else if (cFile.exists() && DkUtils::isValid(cFile))
			cFiles.append(cFile.absoluteFilePath());
	}

	if (!cFiles.empty())
		appendFiles(cFiles);
}

void DkInputTextEdit::insertFromMimeData(const QMimeData* mimeData) {

	appendFromMime(mimeData);
	QTextEdit::insertFromMimeData(mimeData);
}

void DkInputTextEdit::dragEnterEvent(QDragEnterEvent *event) {

	QTextEdit::dragEnterEvent(event);

	if (event->source() == this)
		event->acceptProposedAction();
	else if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void DkInputTextEdit::dragMoveEvent(QDragMoveEvent *event) {

	QTextEdit::dragMoveEvent(event);

	if (event->source() == this)
		event->acceptProposedAction();
	else if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}


void DkInputTextEdit::dropEvent(QDropEvent *event) {
	
	if (event->source() == this) {
		event->accept();
		return;
	}

	appendFromMime(event->mimeData(), (event->keyboardModifiers() & Qt::ControlModifier) != 0);

	// do not propagate!
	//QTextEdit::dropEvent(event);
}

QStringList DkInputTextEdit::getFileList() const {

	QStringList fileList;
	QString textString;
	QTextStream textStream(&textString);
	textStream << toPlainText();

	QString line;
	do
	{
		line = textStream.readLine();	// we don't want to get into troubles with carriage returns of different OS
		if (!line.isNull() && !line.trimmed().isEmpty())
			fileList.append(line);
	} while(!line.isNull());

	return fileList;
}

void DkInputTextEdit::clear() {
	
	mResultList.clear();
	QTextEdit::clear();
}

// File Selection --------------------------------------------------------------------
DkFileSelection::DkFileSelection(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QWidget(parent, f) {

	setObjectName("DkFileSelection");
	createLayout();
	setMinimumHeight(300);

}

void DkFileSelection::createLayout() {
	
	mDirectoryEdit = new DkDirectoryEdit(this);

	QWidget* upperWidget = new QWidget(this);
	QGridLayout* upperWidgetLayout = new QGridLayout(upperWidget);
	upperWidgetLayout->setContentsMargins(0,0,0,0);
	upperWidgetLayout->addWidget(mDirectoryEdit, 0, 1);

	mInputTextEdit = new DkInputTextEdit(this);

	mResultTextEdit = new QTextEdit(this);
	mResultTextEdit->setReadOnly(true);
	mResultTextEdit->setVisible(false);

	mThumbScrollWidget = new DkThumbScrollWidget(this);
	mThumbScrollWidget->setVisible(true);
	mThumbScrollWidget->getThumbWidget()->setImageLoader(mLoader);

	mInfoLabel = new QLabel(tr("No Files Selected"), this);

	// add explorer
	mExplorer = new DkExplorer(tr("File Explorer"));
	mExplorer->getModel()->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs);
	mExplorer->getModel()->setNameFilters(QStringList());
	mExplorer->setMaximumWidth(300);

	QStringList folders = Settings::param().global().recentFiles;

	if (folders.size() > 0)
		mExplorer->setCurrentPath(folders[0]);

	// tab widget
	mInputTabs = new QTabWidget(this);
	mInputTabs->addTab(mThumbScrollWidget,  QIcon(":/nomacs/img/thumbs-view.svg"), tr("Thumbnails"));
	mInputTabs->addTab(mInputTextEdit, QIcon(":/nomacs/img/batch-processing.svg"), tr("File List"));

	QGridLayout* widgetLayout = new QGridLayout(this);
	widgetLayout->addWidget(mExplorer, 0, 0, 3, 1);
	widgetLayout->addWidget(upperWidget, 0, 1);
	widgetLayout->addWidget(mInputTabs, 1, 1);
	widgetLayout->addWidget(mInfoLabel, 2, 1);
	setLayout(widgetLayout);

	connect(mThumbScrollWidget->getThumbWidget(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
	connect(mThumbScrollWidget, SIGNAL(batchProcessFilesSignal(const QStringList&)), mInputTextEdit, SLOT(appendFiles(const QStringList&)));
	connect(mThumbScrollWidget, SIGNAL(updateDirSignal(const QString&)), this, SLOT(setDir(const QString&)));
	connect(mThumbScrollWidget, SIGNAL(filterChangedSignal(const QString &)), mLoader.data(), SLOT(setFolderFilter(const QString&)), Qt::UniqueConnection);
	
	connect(mInputTextEdit, SIGNAL(fileListChangedSignal()), this, SLOT(selectionChanged()));

	connect(mDirectoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(emitChangedSignal()));
	connect(mDirectoryEdit, SIGNAL(directoryChanged(const QString&)), this, SLOT(setDir(const QString&)));
	connect(mExplorer, SIGNAL(openDir(const QString&)), this, SLOT(setDir(const QString&)));
	connect(mLoader.data(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), mThumbScrollWidget, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));

}

void DkFileSelection::changeTab(int tabIdx) const {

	if (tabIdx < 0 || tabIdx >= mInputTabs->count())
		return;

	mInputTabs->setCurrentIndex(tabIdx);
}

void DkFileSelection::updateDir(QVector<QSharedPointer<DkImageContainerT> > thumbs) {
	qDebug() << "emitting updateDirSignal";
	emit updateDirSignal(thumbs);
}

void DkFileSelection::setVisible(bool visible) {

	QWidget::setVisible(visible);
	mThumbScrollWidget->getThumbWidget()->updateLayout();
}

void DkFileSelection::browse() {

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		mCDirPath);

	if (dirName.isEmpty())
		return;

	setDir(dirName);
}

QString DkFileSelection::getDir() const {

	return mDirectoryEdit->existsDirectory() ? QDir(mDirectoryEdit->text()).absolutePath() : "";
}

QStringList DkFileSelection::getSelectedFiles() const {
	
	QStringList textList = mInputTextEdit->getFileList();

	if (textList.empty())
		return mThumbScrollWidget->getThumbWidget()->getSelectedFiles();
	else
		return textList;
}

QStringList DkFileSelection::getSelectedFilesBatch() {

	QStringList textList = mInputTextEdit->getFileList();

	if (textList.empty()) {
		textList = mThumbScrollWidget->getThumbWidget()->getSelectedFiles();
		mInputTextEdit->appendFiles(textList);
	}

	return textList;
}


DkInputTextEdit* DkFileSelection::getInputEdit() const {

	return mInputTextEdit;
}

void DkFileSelection::setFileInfo(QFileInfo file) {

	setDir(file.absoluteFilePath());
}

void DkFileSelection::setDir(const QString& dirPath) {

	mExplorer->setCurrentPath(dirPath);

	mCDirPath = dirPath;
	qDebug() << "setting directory to:" << dirPath;
	mDirectoryEdit->setText(mCDirPath);
	emit newHeaderText(mCDirPath);
	emit updateInputDir(mCDirPath);
	mLoader->loadDir(mCDirPath, false);
	mThumbScrollWidget->updateThumbs(mLoader->getImages());
}

void DkFileSelection::selectionChanged() {

	if (getSelectedFiles().empty())
		mInfoLabel->setText(tr("No Files Selected"));
	else if (getSelectedFiles().size() == 1)
		mInfoLabel->setText(tr("%1 File Selected").arg(getSelectedFiles().size()));
	else
		mInfoLabel->setText(tr("%1 Files Selected").arg(getSelectedFiles().size()));

	emit changed();
}

void DkFileSelection::emitChangedSignal() {
	
	QString newDirPath = mDirectoryEdit->text();
		
	qDebug() << "edit text newDir: " << newDirPath << " mCDir " << mCDirPath;

	if (QDir(newDirPath).exists() && newDirPath != mCDirPath) {
		setDir(newDirPath);
		emit changed();
	}
}

void DkFileSelection::setResults(const QStringList& results) {

	if (mInputTabs->count() < 3) {
		mInputTabs->addTab(mResultTextEdit, tr("Results"));
	}

	mResultTextEdit->clear();
	mResultTextEdit->setHtml(results.join("<br> "));
	QTextCursor c = mResultTextEdit->textCursor();
	c.movePosition(QTextCursor::End);
	mResultTextEdit->setTextCursor(c);
	mResultTextEdit->setVisible(true);
}

void DkFileSelection::startProcessing() {

	if (mInputTabs->count() < 3) {
		mInputTabs->addTab(mResultTextEdit, tr("Results"));
	}

	changeTab(tab_results);
	mInputTextEdit->setEnabled(false);
	mResultTextEdit->clear();
}

void DkFileSelection::stopProcessing() {

	mInputTextEdit->clear();
	mInputTextEdit->setEnabled(true);
}

// DkFileNameWdiget --------------------------------------------------------------------
DkFilenameWidget::DkFilenameWidget(QWidget* parent) : QWidget(parent) {

	createLayout();
	showOnlyFilename();
}

void DkFilenameWidget::createLayout() {
	
	mLayout = new QGridLayout(this);
	mLayout->setContentsMargins(0,0,0,5);
	setMaximumWidth(500);

	mCbType = new QComboBox(this);
	mCbType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	mCbType->insertItem(fileNameTypes_fileName, tr("Current Filename"));
	mCbType->insertItem(fileNameTypes_Text, tr("Text"));
	mCbType->insertItem(fileNameTypes_Number, tr("Number"));
	connect(mCbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeCBChanged(int)));
	connect(mCbType, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForUserInput()));
	connect(mCbType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));

	mCbCase = new QComboBox(this);
	mCbCase->addItem(tr("Keep Case"));
	mCbCase->addItem(tr("To lowercase"));
	mCbCase->addItem(tr("To UPPERCASE"));
	connect(mCbCase, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForUserInput()));
	connect(mCbCase, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));

	mSbNumber = new QSpinBox(this);
	mSbNumber->setValue(1);
	mSbNumber->setMinimum(0);
	mSbNumber->setMaximum(999);	// changes - if cbDigits->setCurrentIndex() is changed!
	connect(mSbNumber, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));

	mCbDigits = new QComboBox(this);
	mCbDigits->addItem(tr("1 digit"));
	mCbDigits->addItem(tr("2 digits"));
	mCbDigits->addItem(tr("3 digits"));
	mCbDigits->addItem(tr("4 digits"));
	mCbDigits->addItem(tr("5 digits"));
	mCbDigits->setCurrentIndex(2);	// see sBNumber->setMaximum()
	connect(mCbDigits, SIGNAL(currentIndexChanged(int)), this, SLOT(digitCBChanged(int)));

	mLeText = new QLineEdit(this);
	connect(mCbCase, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(mLeText, SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));

	mPbPlus = new QPushButton("+", this);
	mPbPlus->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	mPbPlus->setMinimumSize(10,10);
	mPbPlus->setMaximumSize(30,30);
	mPbMinus = new QPushButton("-", this);
	mPbMinus->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	mPbMinus->setMinimumSize(10,10);
	mPbMinus->setMaximumSize(30,30);
	connect(mPbPlus, SIGNAL(clicked()), this, SLOT(pbPlusPressed()));
	connect(mPbMinus, SIGNAL(clicked()), this, SLOT(pbMinusPressed()));
	connect(mPbPlus, SIGNAL(clicked()), this, SIGNAL(changed()));
	connect(mPbMinus, SIGNAL(clicked()), this, SIGNAL(changed()));
}

void DkFilenameWidget::typeCBChanged(int index) {
	switch (index) {
		case fileNameTypes_fileName: {showOnlyFilename(); break;};
		case fileNameTypes_Text: {showOnlyText(); break;};
		case fileNameTypes_Number: {showOnlyNumber(); break;};
		default:
			break;
	}
}

void DkFilenameWidget::showOnlyFilename() {
	mCbCase->show();

	mSbNumber->hide();
	mCbDigits->hide();
	mLeText->hide();

	mLayout->addWidget(mCbType, 0, fileNameWidget_type);
	mLayout->addWidget(mCbCase, 0, fileNameWidget_input1);
	//curLayout->addWidget(new QWidget(this), 0, fileNameWidget_input2 );
	mLayout->addWidget(mPbPlus, 0, fileNameWidget_plus);
	mLayout->addWidget(mPbMinus, 0, fileNameWidget_minus);

}

void DkFilenameWidget::showOnlyNumber() {
	mSbNumber->show();
	mCbDigits->show();

	mCbCase->hide();
	mLeText->hide();

	mLayout->addWidget(mCbType, 0, fileNameWidget_type);
	mLayout->addWidget(mSbNumber, 0, fileNameWidget_input1);
	mLayout->addWidget(mCbDigits, 0, fileNameWidget_input2);
	mLayout->addWidget(mPbPlus, 0, fileNameWidget_plus);
	mLayout->addWidget(mPbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::showOnlyText() {
	mLeText->show();

	mSbNumber->hide();
	mCbDigits->hide();
	mCbCase->hide();
	

	mLayout->addWidget(mCbType, 0, fileNameWidget_type);
	mLayout->addWidget(mLeText, 0, fileNameWidget_input1);
	//curLayout->addWidget(new QWidget(this), 0, fileNameWidget_input2);
	mLayout->addWidget(mPbPlus, 0, fileNameWidget_plus);
	mLayout->addWidget(mPbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::pbPlusPressed() {
	emit plusPressed(this);
}

void DkFilenameWidget::pbMinusPressed() {
	emit minusPressed(this);
}

void DkFilenameWidget::enableMinusButton(bool enable) {
	mPbMinus->setEnabled(enable);
}

void DkFilenameWidget::enablePlusButton(bool enable) {
	mPbPlus->setEnabled(enable);
}

void DkFilenameWidget::checkForUserInput() {
	if(mCbType->currentIndex() == 0 && mCbCase->currentIndex() == 0)
		hasChanged = false;
	else
		hasChanged = true;
	//emit changed();
}

void DkFilenameWidget::digitCBChanged(int index) {
	mSbNumber->setMaximum(qRound(std::pow(10, index+1)-1));
	emit changed();
}

QString DkFilenameWidget::getTag() const {

	QString tag;

	switch (mCbType->currentIndex()) {
		
	case fileNameTypes_Number: 
		{
			tag += "<d:"; 
			tag += QString::number(mCbDigits->currentIndex());	// is sensitive to the index
			tag += ":" + QString::number(mSbNumber->value());
			tag += ">";
			break;
		}
	case fileNameTypes_fileName: 
		{
			tag += "<c:"; 
			tag += QString::number(mCbCase->currentIndex());	// is sensitive to the index
			tag += ">";
			break;
		}
	case fileNameTypes_Text:
		{
			tag += mLeText->text();
		}
	}

	return tag;
}

// DkBatchOutput --------------------------------------------------------------------
DkBatchOutput::DkBatchOutput(QWidget* parent , Qt::WindowFlags f ) : QWidget(parent, f) {

	setObjectName("DkBatchOutput");
	createLayout();
}

void DkBatchOutput::createLayout() {

	// Output Directory Groupbox
	QGroupBox* outDirGroupBox = new QGroupBox(this);
	outDirGroupBox->setTitle(tr("Output Directory"));

	mOutputBrowseButton = new QPushButton(tr("Browse"));
	mOutputlineEdit = new DkDirectoryEdit(this);
	mOutputlineEdit->setPlaceholderText(tr("Select a Directory"));
	connect(mOutputBrowseButton , SIGNAL(clicked()), this, SLOT(browse()));
	connect(mOutputlineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(setDir(const QString&)));

	// overwrite existing
	mCbOverwriteExisting = new QCheckBox(tr("Overwrite Existing Files"));
	mCbOverwriteExisting->setToolTip(tr("If checked, existing files are overwritten.\nThis option might destroy your images - so be careful!"));
	connect(mCbOverwriteExisting, SIGNAL(clicked()), this, SIGNAL(changed()));

	// Use Input Folder
	mCbUseInput = new QCheckBox(tr("Use Input Folder"));
	mCbUseInput->setToolTip(tr("If checked, the batch is applied to the input folder - so be careful!"));
	connect(mCbUseInput, SIGNAL(clicked(bool)), this, SLOT(useInputFolderChanged(bool)));

	// delete original
	mCbDeleteOriginal = new QCheckBox(tr("Delete Input Files"));
	mCbDeleteOriginal->setToolTip(tr("If checked, the original file will be deleted if the conversion was successful.\n So be careful!"));

	QWidget* cbWidget = new QWidget(this);
	QHBoxLayout* cbLayout = new QHBoxLayout(cbWidget);
	cbLayout->setContentsMargins(0,0,0,0);
	cbLayout->addWidget(mCbUseInput);
	cbLayout->addWidget(mCbOverwriteExisting);
	cbLayout->addWidget(mCbDeleteOriginal);
	cbLayout->addStretch();

	QGridLayout* outDirLayout = new QGridLayout(outDirGroupBox);
	//outDirLayout->setContentsMargins(0, 0, 0, 0);
	outDirLayout->addWidget(mOutputBrowseButton, 0, 0);
	outDirLayout->addWidget(mOutputlineEdit, 0, 1);
	outDirLayout->addWidget(cbWidget, 1, 1);

	// Filename Groupbox
	QGroupBox* filenameGroupBox = new QGroupBox(this);
	filenameGroupBox->setTitle(tr("Filename"));
	mFilenameVBLayout = new QVBoxLayout(filenameGroupBox);
	mFilenameVBLayout->setSpacing(0);
	//filenameVBLayout->setContentsMargins(0,0,0,0);
	DkFilenameWidget* fwidget = new DkFilenameWidget(this);
	fwidget->enableMinusButton(false);
	mFilenameWidgets.push_back(fwidget);
	mFilenameVBLayout->addWidget(fwidget);
	connect(fwidget, SIGNAL(plusPressed(DkFilenameWidget*)), this, SLOT(plusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(minusPressed(DkFilenameWidget*)), this, SLOT(minusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(changed()), this, SLOT(emitChangedSignal()));

	QWidget* extensionWidget = new QWidget(this);
	QHBoxLayout* extensionLayout = new QHBoxLayout(extensionWidget);
	//extensionLayout->setSpacing(0);
	extensionLayout->setContentsMargins(0,0,0,0);
	mCbExtension = new QComboBox(this);
	mCbExtension->addItem(tr("Keep Extension"));
	mCbExtension->addItem(tr("Convert To"));
	connect(mCbExtension, SIGNAL(currentIndexChanged(int)), this, SLOT(extensionCBChanged(int)));

	mCbNewExtension = new QComboBox(this);
	mCbNewExtension->addItems(Settings::param().app().saveFilters);
	mCbNewExtension->setFixedWidth(150);
	mCbNewExtension->setEnabled(false);

	QLabel* compressionLabel = new QLabel(tr("Compression"), this);

	mSbCompression = new QSpinBox(this);
	mSbCompression->setValue(90);
	mSbCompression->setMinimum(1);
	mSbCompression->setMaximum(100);
	mSbCompression->setEnabled(false);

	extensionLayout->addWidget(mCbExtension);
	extensionLayout->addWidget(mCbNewExtension);
	extensionLayout->addWidget(compressionLabel);
	extensionLayout->addWidget(mSbCompression);
	//extensionLayout->addStretch();
	mFilenameVBLayout->addWidget(extensionWidget);
	
	QLabel* oldLabel = new QLabel(tr("Old: "));
	mOldFileNameLabel = new QLabel("");

	QLabel* newLabel = new QLabel(tr("New: "));
	mNewFileNameLabel = new QLabel("");

	// Preview Widget
	QGroupBox* previewGroupBox = new QGroupBox(this);
	previewGroupBox->setTitle(tr("Filename Preview"));
	QGridLayout* previewGBLayout = new QGridLayout(previewGroupBox);
	//previewGroupBox->hide();	// show if more than 1 file is selected
	previewGBLayout->addWidget(oldLabel, 0, 0);
	previewGBLayout->addWidget(mOldFileNameLabel, 0, 1);
	previewGBLayout->addWidget(newLabel, 1, 0);
	previewGBLayout->addWidget(mNewFileNameLabel, 1, 1);
	previewGBLayout->setColumnStretch(3, 10);
	previewGBLayout->setAlignment(Qt::AlignTop);
	
	QGridLayout* contentLayout = new QGridLayout(this);
	contentLayout->addWidget(outDirGroupBox, 0, 0, 1, 2);
	contentLayout->addWidget(filenameGroupBox, 1, 0);
	contentLayout->addWidget(previewGroupBox, 1, 1);
	setLayout(contentLayout);
}

void DkBatchOutput::browse() {

	QString dirGuess = (mOutputlineEdit->text().isEmpty()) ? mInputDirectory : mOutputlineEdit->text();
	
	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		dirGuess);

	if (dirName.isEmpty())
		return;

	setDir(dirName);
}

void DkBatchOutput::setDir(const QString& dirPath, bool updateLineEdit) {

	mOutputDirectory = dirPath;
	emit newHeaderText(dirPath);
	
	if (updateLineEdit)
		mOutputlineEdit->setText(dirPath);
}

void DkBatchOutput::setInputDir(const QString& dirPath) {
	mInputDirectory = dirPath;

	if (mCbUseInput->isChecked())
		setDir(mInputDirectory);
}

void DkBatchOutput::useInputFolderChanged(bool checked) {

	mOutputlineEdit->setEnabled(!checked);
	mOutputBrowseButton->setEnabled(!checked);

	if (checked)
		setDir(mInputDirectory);
}

void DkBatchOutput::plusPressed(DkFilenameWidget* widget) {
	
	int index = mFilenameVBLayout->indexOf(widget);
	DkFilenameWidget* fwidget = new DkFilenameWidget(this);
	mFilenameWidgets.insert(index + 1, fwidget);
	if (mFilenameWidgets.size() > 4) {
		for (int i = 0; i  < mFilenameWidgets.size(); i++)
			mFilenameWidgets[i]->enablePlusButton(false);
	}
	mFilenameVBLayout->insertWidget(index + 1, fwidget); // append to current widget
	connect(fwidget, SIGNAL(plusPressed(DkFilenameWidget*)), this, SLOT(plusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(minusPressed(DkFilenameWidget*)), this, SLOT(minusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(changed()), this, SLOT(emitChangedSignal()));

	emitChangedSignal();
}

void DkBatchOutput::minusPressed(DkFilenameWidget* widget) {
	
	mFilenameVBLayout->removeWidget(widget);
	mFilenameWidgets.remove(mFilenameWidgets.indexOf(widget));
	if (mFilenameWidgets.size() <= 4) {
		for (int i = 0; i  < mFilenameWidgets.size(); i++)
			mFilenameWidgets[i]->enablePlusButton(true);
	}

	widget->hide();

	emitChangedSignal();
}

void DkBatchOutput::extensionCBChanged(int index) {
	
	mCbNewExtension->setEnabled(index > 0);
	mSbCompression->setEnabled(index > 0);
	emitChangedSignal();
}


bool DkBatchOutput::hasUserInput() const {
	// TODO add output directory 
	return mFilenameWidgets.size() > 1 || mFilenameWidgets[0]->hasUserInput() || mCbExtension->currentIndex() == 1;
}

void DkBatchOutput::emitChangedSignal() {

	updateFileLabelPreview();
	emit changed();
}

void DkBatchOutput::updateFileLabelPreview() {

	qDebug() << "updating file label, example name: " << mExampleName;

	if (mExampleName.isEmpty())
		return;

	DkFileNameConverter converter(mExampleName, getFilePattern(), 0);

	mOldFileNameLabel->setText(mExampleName);
	mNewFileNameLabel->setText(converter.getConvertedFileName());
}

QString DkBatchOutput::getOutputDirectory() {
	qDebug() << "ouptut dir: " << QDir(mOutputlineEdit->text()).absolutePath();

	return mOutputlineEdit->text();
}

QString DkBatchOutput::getFilePattern() {

	QString pattern = "";

	for (int idx = 0; idx < mFilenameWidgets.size(); idx++)
		pattern += mFilenameWidgets.at(idx)->getTag();	

	if (mCbExtension->currentIndex() == 0) {
		pattern += ".<old>";
	}
	else {
		QString ext = mCbNewExtension->itemText(mCbNewExtension->currentIndex());

		QStringList tmp = ext.split("(");

		if (tmp.size() == 2) {

			QString filters = tmp.at(1);
			filters.replace(")", "");
			filters.replace("*", "");

			QStringList extList = filters.split(" ");

			if (!extList.empty())
				pattern += extList[0];
		}
	}

	return pattern;
}

int DkBatchOutput::getCompression() const {
	return mSbCompression->value();
}

int DkBatchOutput::overwriteMode() const {

	if (mCbOverwriteExisting->isChecked())
		return DkBatchConfig::mode_overwrite;

	return DkBatchConfig::mode_skip_existing;
}

bool DkBatchOutput::useInputDir() const {
	return mCbUseInput->isChecked();
}

bool DkBatchOutput::deleteOriginal() const {

	return mCbDeleteOriginal->isChecked();
}

void DkBatchOutput::setExampleFilename(const QString& exampleName) {

	mExampleName = exampleName;
	qDebug() << "example name: " << exampleName;
	updateFileLabelPreview();
}

// DkResizeWidget --------------------------------------------------------------------
DkBatchResizeWidget::DkBatchResizeWidget(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QWidget(parent, f) {

	createLayout();
	modeChanged(0);	// init gui
}

void DkBatchResizeWidget::createLayout() {

	mComboMode = new QComboBox(this);
	QStringList modeItems;
	modeItems << tr("Percent") << tr("Long Side") << tr("Short Side") << tr("Width") << tr("Height");
	mComboMode->addItems(modeItems);

	mComboProperties = new QComboBox(this);
	QStringList propertyItems;
	propertyItems << tr("Transform All") << tr("Shrink Only") << tr("Enlarge Only");
	mComboProperties->addItems(propertyItems);

	mSbPercent = new QDoubleSpinBox(this);
	mSbPercent->setSuffix(tr("%"));
	mSbPercent->setMaximum(1000);
	mSbPercent->setMinimum(0.1);
	mSbPercent->setValue(100.0);

	mSbPx = new QSpinBox(this);
	mSbPx->setSuffix(tr(" px"));
	mSbPx->setMaximum(SHRT_MAX);
	mSbPx->setMinimum(1);
	mSbPx->setValue(1920);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(mComboMode);
	layout->addWidget(mSbPercent);
	layout->addWidget(mSbPx);
	layout->addWidget(mComboProperties);
	layout->addStretch();

	connect(mComboMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
	connect(mSbPercent, SIGNAL(valueChanged(double)), this, SLOT(percentChanged(double)));
	connect(mSbPx, SIGNAL(valueChanged(int)), this, SLOT(pxChanged(int)));
}

void DkBatchResizeWidget::modeChanged(int) {

	if (mComboMode->currentIndex() == DkResizeBatch::mode_default) {
		mSbPx->hide();
		mSbPercent->show();
		mComboProperties->hide();
		percentChanged(mSbPercent->value());
	}
	else {
		mSbPx->show();
		mSbPercent->hide();
		mComboProperties->show();
		pxChanged(mSbPx->value());
	}
}

void DkBatchResizeWidget::percentChanged(double val) {

	if (val == 100.0)
		emit newHeaderText(tr("inactive"));
	else
		emit newHeaderText(QString::number(val) + "%");
}

void DkBatchResizeWidget::pxChanged(int val) {

	emit newHeaderText(mComboMode->itemText(mComboMode->currentIndex()) + ": " + QString::number(val) + " px");
}

void DkBatchResizeWidget::transferProperties(QSharedPointer<DkResizeBatch> batchResize) const {

	if (mComboMode->currentIndex() == DkResizeBatch::mode_default) {
		batchResize->setProperties((float)mSbPercent->value()/100.0f, mComboMode->currentIndex());
	}
	else {
		batchResize->setProperties((float)mSbPx->value(), mComboMode->currentIndex(), mComboProperties->currentIndex());
	}
}

bool DkBatchResizeWidget::hasUserInput() const {

	return !(mComboMode->currentIndex() == DkResizeBatch::mode_default && mSbPercent->value() == 100.0);
}

bool DkBatchResizeWidget::requiresUserInput() const {

	return false;
}

#ifdef WITH_PLUGINS
// DkBatchPlugin --------------------------------------------------------------------
DkBatchPluginWidget::DkBatchPluginWidget(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QWidget(parent, f) {

	createLayout();
}

void DkBatchPluginWidget::transferProperties(QSharedPointer<DkPluginBatch> batchPlugin) const {

	QStringList pluginList;
	for (int idx = 0; idx < mPluginListWidget->count(); idx++) {
		pluginList.append(mPluginListWidget->item(idx)->text());
	}

	batchPlugin->setProperties(pluginList);
}

bool DkBatchPluginWidget::hasUserInput() const {
	return false;	// TODO
}

bool DkBatchPluginWidget::requiresUserInput() const {
	return false;
}

void DkBatchPluginWidget::createLayout() {

	DkListWidget* pluginSelectionWidget = new DkListWidget(this);
	pluginSelectionWidget->setEmptyText(tr("Sorry, no Plugins found."));
	pluginSelectionWidget->addItems(getPluginActionNames());

	mPluginListWidget = new DkListWidget(this);
	mPluginListWidget->setEmptyText(tr("Drag Plugin Actions here."));

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(pluginSelectionWidget);
	layout->addWidget(mPluginListWidget);
	layout->addStretch();

	// connections
	connect(pluginSelectionWidget, SIGNAL(dataDroppedSignal()), this, SLOT(updateHeader()));
	connect(mPluginListWidget, SIGNAL(dataDroppedSignal()), this, SLOT(updateHeader()));
}

QStringList DkBatchPluginWidget::getPluginActionNames() const {

	QStringList pluginActions;
	QVector<DkPluginInterface*> plugins = DkPluginManager::instance().getBasicPlugins();

	for (const DkPluginInterface* p : plugins) {

		QList<QAction*> actions = p->pluginActions();

		for (const QAction* a : actions) {
			pluginActions.append(p->pluginName() + " | " + a->text());
		}
	}

	return pluginActions;
}

void DkBatchPluginWidget::updateHeader() const {
	
	int c = mPluginListWidget->count();
	if (!c)
		emit newHeaderText(tr("inactive"));
	else
		emit newHeaderText(tr("%1 plugins selected").arg(c));

	// TODO: counting is wrong! (if you remove plugins
}
#endif

// DkBatchTransform --------------------------------------------------------------------
DkBatchTransformWidget::DkBatchTransformWidget(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QWidget(parent, f) {

	createLayout();
}

void DkBatchTransformWidget::createLayout() {

	mRbRotate0 = new QRadioButton(tr("Do &Not Rotate"));
	mRbRotate0->setChecked(true);
	mRbRotateLeft = new QRadioButton(tr("9&0%1 Counter Clockwise").arg(dk_degree_str));
	mRbRotateRight = new QRadioButton(tr("&90%1 Clockwise").arg(dk_degree_str));
	mRbRotate180 = new QRadioButton(tr("&180%1").arg(dk_degree_str));

	mRotateGroup = new QButtonGroup(this);

	mRotateGroup->addButton(mRbRotate0);
	mRotateGroup->addButton(mRbRotateLeft);
	mRotateGroup->addButton(mRbRotateRight);
	mRotateGroup->addButton(mRbRotate180);

	mCbFlipH = new QCheckBox(tr("Flip &Horizontal"));
	mCbFlipV = new QCheckBox(tr("Flip &Vertical"));

	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(mRbRotate0, 0, 0);
	layout->addWidget(mRbRotateRight, 1, 0);
	layout->addWidget(mRbRotateLeft, 2, 0);
	layout->addWidget(mRbRotate180, 3, 0);

	layout->addWidget(mCbFlipH, 0, 1);
	layout->addWidget(mCbFlipV, 1, 1);
	layout->setColumnStretch(3, 10);

	connect(mRotateGroup, SIGNAL(buttonClicked(int)), this, SLOT(radioButtonClicked(int)));
	connect(mCbFlipV, SIGNAL(clicked()), this, SLOT(checkBoxClicked()));
	connect(mCbFlipH, SIGNAL(clicked()), this, SLOT(checkBoxClicked()));
}

bool DkBatchTransformWidget::hasUserInput() const {
	
	return !mRbRotate0->isChecked() || mCbFlipH->isChecked() || mCbFlipV->isChecked();
}

bool DkBatchTransformWidget::requiresUserInput() const {

	return false;
}

void DkBatchTransformWidget::radioButtonClicked(int) {

	updateHeader();
}

void DkBatchTransformWidget::checkBoxClicked() {

	updateHeader();
}

void DkBatchTransformWidget::updateHeader() const {

	if (!hasUserInput())
		emit newHeaderText(tr("inactive"));
	else {
		
		QString txt;
		if (getAngle() != 0)
			txt += tr("Rotating by: %1").arg(getAngle());
		if (mCbFlipH->isChecked() || mCbFlipV->isChecked()) {
			txt += tr(" Flipping");
		}
		
		emit newHeaderText(txt);
	}
}

void DkBatchTransformWidget::transferProperties(QSharedPointer<DkBatchTransform> batchTransform) const {

	batchTransform->setProperties(getAngle(), mCbFlipH->isChecked(), mCbFlipV->isChecked());
}

int DkBatchTransformWidget::getAngle() const {

	if (mRbRotate0->isChecked())
		return 0;
	else if (mRbRotateLeft->isChecked())
		return -90;
	else if (mRbRotateRight->isChecked())
		return 90;
	else if (mRbRotate180->isChecked())
		return 180;

	return 0;
}

// Batch Dialog --------------------------------------------------------------------
DkBatchDialog::DkBatchDialog(const QString& currentDirectory, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {
	
	mCurrentDirectory = currentDirectory;
	mBatchProcessing = new DkBatchProcessing(DkBatchConfig(), this);

	connect(mBatchProcessing, SIGNAL(progressValueChanged(int)), this, SLOT(updateProgress(int)));
	connect(mBatchProcessing, SIGNAL(finished()), this, SLOT(processingFinished()));

	setWindowTitle(tr("Batch Conversion"));
	createLayout();
	connect(mFileSelection, SIGNAL(updateInputDir(const QString&)), mOutputSelection, SLOT(setInputDir(const QString&)));
	
	connect(&mLogUpdateTimer, SIGNAL(timeout()), this, SLOT(updateLog()));

	mFileSelection->setDir(currentDirectory);
	mOutputSelection->setInputDir(currentDirectory);
	//mOutputSelection->setDir(currentDirectory);
}

void DkBatchDialog::createLayout() {

	//setStyleSheet("QWidget{border: 1px solid #000000;}");

	mWidgets.resize(batchWidgets_end);

	// Input Directory
	mWidgets[batch_input] = new DkBatchWidget(tr("Input Directory"), tr("directory not set"), this);
	mFileSelection  = new DkFileSelection(mWidgets[batch_input]);
	mWidgets[batch_input]->setContentWidget(mFileSelection);
	mFileSelection->setDir(mCurrentDirectory);
	
	// fold content
	mWidgets[batch_resize] = new DkBatchWidget(tr("Resize"), tr("inactive"), this);
	mResizeWidget = new DkBatchResizeWidget(mWidgets[batch_resize]);
	mWidgets[batch_resize]->setContentWidget(mResizeWidget);
	mWidgets[batch_resize]->showContent(false);

	mWidgets[batch_transform] = new DkBatchWidget(tr("Transform"), tr("inactive"), this);
	mTransformWidget = new DkBatchTransformWidget(mWidgets[batch_transform]);
	mWidgets[batch_transform]->setContentWidget(mTransformWidget);
	mWidgets[batch_transform]->showContent(false);

#ifdef WITH_PLUGINS
	mWidgets[batch_plugin] = new DkBatchWidget(tr("Plugins"), tr("inactive"), this);
	mPluginWidget = new DkBatchPluginWidget(mWidgets[batch_plugin]);
	mWidgets[batch_plugin]->setContentWidget(mPluginWidget);
	mWidgets[batch_plugin]->showContent(false);
#endif

	mWidgets[batch_output] = new DkBatchWidget(tr("Output"), tr("not set"), this);
	mOutputSelection = new DkBatchOutput(mWidgets[batch_output]);
	mWidgets[batch_output]->setContentWidget(mOutputSelection);

	mProgressBar = new QProgressBar(this);
	mProgressBar->setVisible(false);

	mSummaryLabel = new QLabel("", this);
	mSummaryLabel->setObjectName("DkDecentInfo");
	mSummaryLabel->setVisible(false);
	mSummaryLabel->setAlignment(Qt::AlignRight);

	// mButtons
	mLogButton = new QPushButton(tr("Show &Log"), this);
	mLogButton->setToolTip(tr("Shows detailed status messages."));
	mLogButton->setEnabled(false);
	connect(mLogButton, SIGNAL(clicked()), this, SLOT(logButtonClicked()));

	mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	mButtons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	mButtons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
	mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Close"));
	mButtons->addButton(mLogButton, QDialogButtonBox::ActionRole);

	connect(mButtons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mButtons, SIGNAL(rejected()), this, SLOT(reject()));


	QWidget* batchWidget = new QWidget(this);
	QVBoxLayout* batchLayout = new QVBoxLayout(batchWidget);
	batchLayout->setAlignment(Qt::AlignTop);
	for (int i=0; i < mWidgets.size(); i++) {

		if (i != batch_input)
			batchLayout->addWidget(mWidgets[i]);
		//connect(widgets[i]->contentWidget(), SIGNAL(changed()), this, SLOT(widgetChanged())); // most widgets do not have (and need) a changed signal ... perhaps uncomment this again 
	}
	connect(mWidgets[batch_input]->contentWidget(), SIGNAL(changed()), this, SLOT(widgetChanged()));
	connect(mWidgets[batch_output]->contentWidget(), SIGNAL(changed()), this, SLOT(widgetChanged())); 

	QSplitter* splitter = new QSplitter(Qt::Vertical, this);
	splitter->addWidget(mWidgets[batch_input]);
	splitter->addWidget(batchWidget);

	QVBoxLayout* dialogLayout = new QVBoxLayout(this);
	dialogLayout->addWidget(splitter);		// almost everything
	dialogLayout->addWidget(mProgressBar);
	dialogLayout->addWidget(mSummaryLabel);
	//dialogLayout->addStretch(10);
	dialogLayout->addWidget(mButtons);

	setLayout(dialogLayout);
}

void DkBatchDialog::accept() {
	
	// check if we are good to go
	if (mFileSelection->getSelectedFiles().empty()) {
		QMessageBox::information(this, tr("Wrong Configuration"), tr("Please select files for processing."), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	DkBatchOutput* outputWidget = dynamic_cast<DkBatchOutput*>(mWidgets[batch_output]->contentWidget());

	if (!outputWidget) {
		qDebug() << "FATAL ERROR: could not cast output widget";
		QMessageBox::critical(this, tr("Fatal Error"), tr("I am missing a widget."), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	if (mWidgets[batch_output] && mWidgets[batch_input])  {
		bool outputChanged = dynamic_cast<DkBatchContent*>(mWidgets[batch_output]->contentWidget())->hasUserInput();
		QString inputDirPath = dynamic_cast<DkFileSelection*>(mWidgets[batch_input]->contentWidget())->getDir();
		QString outputDirPath = dynamic_cast<DkBatchOutput*>(mWidgets[batch_output]->contentWidget())->getOutputDirectory();
		
		if (!outputChanged && inputDirPath.toLower() == outputDirPath.toLower() && dynamic_cast<DkBatchOutput*>(mWidgets[batch_output]->contentWidget())->overwriteMode() != DkBatchConfig::mode_overwrite) {
			QMessageBox::information(this, tr("Wrong Configuration"), tr("Please check 'Overwrite Existing Files' or choose a different output directory."), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
	}

	DkBatchConfig config(mFileSelection->getSelectedFilesBatch(), outputWidget->getOutputDirectory(), outputWidget->getFilePattern());
	config.setMode(outputWidget->overwriteMode());
	config.setDeleteOriginal(outputWidget->deleteOriginal());
	config.setInputDirIsOutputDir(outputWidget->useInputDir());
	config.setCompression(outputWidget->getCompression());

	if (!config.getOutputDirPath().isEmpty() && !QDir(config.getOutputDirPath()).exists()) {

		DkMessageBox* msgBox = new DkMessageBox(QMessageBox::Question, tr("Create Output Directory"), 
			tr("Should I create:\n%1").arg(config.getOutputDirPath()), 
			(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), qApp->activeWindow());

		msgBox->setDefaultButton(QMessageBox::Yes);
		msgBox->setObjectName("batchOutputDirDialog");

		int answer = msgBox->exec();

		if (answer != QMessageBox::Accepted && answer != QMessageBox::Yes) {
			return;
		}
	}

	if (!config.isOk()) {

		if (config.getOutputDirPath().isEmpty()) {
			QMessageBox::critical(this, tr("Fatal Error"), tr("Please select an output directory."), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		else if (!QDir(config.getOutputDirPath()).exists()) {
			QMessageBox::critical(this, tr("Fatal Error"), tr("Sorry, I cannot create %1.").arg(config.getOutputDirPath()), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		else if (config.getFileList().empty()) {
			QMessageBox::critical(this, tr("Fatal Error"), tr("Sorry, I cannot find files to process."), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		else if (config.getFileNamePattern().isEmpty()) {
			QMessageBox::critical(this, tr("Fatal Error"), tr("Sorry, the file pattern is empty."), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		//else if (config.getOutputDir() == QDir()) {
		//	QMessageBox::information(this, tr("Input Missing"), tr("Please choose a valid output directory\n%1").arg(config.getOutputDir().absolutePath()), QMessageBox::Ok, QMessageBox::Ok);
		//	return;
		//}

		qDebug() << "config not ok - canceling";
		QMessageBox::critical(this, tr("Fatal Error"), tr("Sorry, the file pattern is empty."), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	// create processing functions
	QSharedPointer<DkResizeBatch> resizeBatch(new DkResizeBatch);
	mResizeWidget->transferProperties(resizeBatch);

	// create processing functions
	QSharedPointer<DkBatchTransform> transformBatch(new DkBatchTransform);
	mTransformWidget->transferProperties(transformBatch);

#ifdef WITH_PLUGINS
	// create processing functions
	QSharedPointer<DkPluginBatch> pluginBatch(new DkPluginBatch);
	mPluginWidget->transferProperties(pluginBatch);
#endif

	QVector<QSharedPointer<DkAbstractBatch> > processFunctions;
	
	if (resizeBatch->isActive())
		processFunctions.append(resizeBatch);

	if (transformBatch->isActive())
		processFunctions.append(transformBatch);

#ifdef WITH_PLUGINS
	if (pluginBatch->isActive())
		processFunctions.append(pluginBatch);
#endif

	config.setProcessFunctions(processFunctions);
	mBatchProcessing->setBatchConfig(config);

	startProcessing();
	mBatchProcessing->compute();
}

void DkBatchDialog::reject() {

	if (mBatchProcessing->isComputing()) {
		mBatchProcessing->cancel();
		mButtons->button(QDialogButtonBox::Cancel)->setEnabled(false);
		//stopProcessing();
	}
	else
		QDialog::reject();
}

void DkBatchDialog::processingFinished() {

	stopProcessing();
}

void DkBatchDialog::startProcessing() {

	mFileSelection->startProcessing();

	mProgressBar->show();
	mProgressBar->reset();
	mProgressBar->setMaximum(mFileSelection->getSelectedFiles().size());
	mLogButton->setEnabled(false);
	mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
	mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

	mLogUpdateTimer.start(1000);
}

void DkBatchDialog::stopProcessing() {

	mFileSelection->stopProcessing();

	mProgressBar->hide();
	mProgressBar->reset();
	mLogButton->setEnabled(true);
	mButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
	mButtons->button(QDialogButtonBox::Cancel)->setEnabled(true);
	mButtons->button(QDialogButtonBox::Cancel)->setText(tr("&Close"));

	int numFailures = mBatchProcessing->getNumFailures();
	int numProcessed = mBatchProcessing->getNumProcessed();
	int numItems = mBatchProcessing->getNumItems();

	mSummaryLabel->setText(tr("%1/%2 files processed... %3 failed.").arg(numProcessed).arg(numItems).arg(numFailures));
	mSummaryLabel->show();

	mSummaryLabel->setProperty("warning", numFailures > 0);
	mSummaryLabel->style()->unpolish(this);
	mSummaryLabel->style()->polish(this);
	update();

	mLogNeedsUpdate = false;
	mLogUpdateTimer.stop();

	updateLog();
}

void DkBatchDialog::updateLog() {

	mFileSelection->setResults(mBatchProcessing->getResultList());
}

void DkBatchDialog::updateProgress(int progress) {

	mProgressBar->setValue(progress);
	mLogNeedsUpdate = true;
}

void DkBatchDialog::logButtonClicked() {

	QStringList log = mBatchProcessing->getLog();

	DkTextDialog* textDialog = new DkTextDialog(this);
	textDialog->getTextEdit()->setReadOnly(true);
	textDialog->setText(log);

	textDialog->exec();
}

void DkBatchDialog::setSelectedFiles(const QStringList& selFiles) {

	if (!selFiles.empty()) {
		mFileSelection->getInputEdit()->appendFiles(selFiles);
		mFileSelection->changeTab(DkFileSelection::tab_text_input);
	}

}

void DkBatchDialog::widgetChanged() {
	
	if (mWidgets[batch_output] && mWidgets[batch_input])  {
		QString inputDirPath = dynamic_cast<DkFileSelection*>(mWidgets[batch_input]->contentWidget())->getDir();
		QString outputDirPath = dynamic_cast<DkBatchOutput*>(mWidgets[batch_output]->contentWidget())->getOutputDirectory();
		
		if (inputDirPath == "" || outputDirPath == "")
			mButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
		else
			mButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
	}

	if (!mFileSelection->getSelectedFiles().isEmpty()) {

		QUrl url = mFileSelection->getSelectedFiles().first();
		QString fString = url.toString();
		fString = fString.replace("file:///", "");

		QFileInfo cFileInfo = QFileInfo(fString);
		if (!cFileInfo.exists())	// try an alternative conversion
			cFileInfo = QFileInfo(url.toLocalFile());

		dynamic_cast<DkBatchOutput*>(mWidgets[batch_output]->contentWidget())->setExampleFilename(cFileInfo.fileName());
		mButtons->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
}

}
