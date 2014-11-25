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

namespace nmc {

// DkBatchWidget --------------------------------------------------------------------
DkBatchWidget::DkBatchWidget(QString titleString, QString headerString, QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f) {
	this->titleString = titleString;
	this->headerString = headerString;
	createLayout();

}

void DkBatchWidget::createLayout() {
	
	showButton = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	showButton->setFixedSize(QSize(16,16));
	showButton->setObjectName("showSelectionButton");
	showButton->setCheckable(true);
	showButton->setChecked(true);
	

	titleLabel = new QLabel(titleString);
	titleLabel->setObjectName("DkBatchTitle");
	titleLabel->setAlignment(Qt::AlignBottom);
	headerLabel = new QLabel(headerString);
	headerLabel->setObjectName("DkDecentInfo");
	headerLabel->setAlignment(Qt::AlignBottom);
	
	QWidget* headerWidget = new QWidget();
	QHBoxLayout* headerWidgetLayout = new QHBoxLayout(headerWidget);
	headerWidgetLayout->setContentsMargins(0,0,0,0);
	headerWidgetLayout->addWidget(showButton);
	headerWidgetLayout->addWidget(titleLabel);
	headerWidgetLayout->addWidget(headerLabel);
	headerWidgetLayout->addStretch();

	batchWidgetLayout = new QVBoxLayout;
	batchWidgetLayout->addWidget(headerWidget);
	//batchWidgetLayout->addWidget(contentWidget);
	//batchWidgetLayout->addStretch();
	setLayout(batchWidgetLayout);
}

void DkBatchWidget::setContentWidget(QWidget* batchContent) {
	
	this->batchContent = dynamic_cast<DkBatchContent*>(batchContent);
	batchWidgetLayout->addWidget(batchContent);
	connect(showButton, SIGNAL(toggled(bool)), batchContent, SLOT(setVisible(bool)));
	connect(batchContent, SIGNAL(newHeaderText(QString)), this, SLOT(setHeader(QString)));
}

QWidget* DkBatchWidget::contentWidget() const {
	
	return dynamic_cast<QWidget*>(batchContent);
}

void DkBatchWidget::setTitle(QString titleString) {
	this->titleString = titleString;
	titleLabel->setText(titleString);
}

void DkBatchWidget::setHeader(QString headerString) {
	this->headerString = headerString;
	headerLabel->setText(headerString);
}


// File Selection --------------------------------------------------------------------
DkFileSelection::DkFileSelection(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QWidget(parent, f) {
	this->hUserInput = false;
	this->rUserInput = false;
	
	setObjectName("DkFileSelection");
	createLayout();
	setMinimumHeight(300);

	loader = new DkImageLoader();
	//connect(loader, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), this, SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT> >)));
	connect(loader, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), thumbScrollWidget, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));
}

void DkFileSelection::createLayout() {
	
	directoryEdit = new DkDirectoryEdit(this);
	connect(directoryEdit, SIGNAL(textChanged(QString)), this, SLOT(emitChangedSignal()));

	QPushButton* browseButton = new QPushButton(tr("Browse"));
	connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

	QLineEdit* filterEdit = new QLineEdit("not implemented yet", this);
	QPushButton* filterButton = new QPushButton(tr("Apply Filter"));

	QWidget* upperWidget = new QWidget(this);
	QGridLayout* upperWidgetLayout = new QGridLayout(upperWidget);
	upperWidgetLayout->addWidget(directoryEdit, 0,0);
	upperWidgetLayout->addWidget(browseButton, 0, 1);
	upperWidgetLayout->addWidget(filterEdit, 1, 0);
	upperWidgetLayout->addWidget(filterButton, 1, 1);

	thumbScrollWidget = new DkThumbScrollWidget(this);
	thumbScrollWidget->setVisible(true);
	//connect(this, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), thumbScrollWidget, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));

	QVBoxLayout* widgetLayout = new QVBoxLayout();
	widgetLayout->addWidget(upperWidget);
	widgetLayout->addWidget(thumbScrollWidget);
	setLayout(widgetLayout);
}

void DkFileSelection::updateDir(QVector<QSharedPointer<DkImageContainerT> > thumbs) {
	qDebug() << "emitting updateDirSignal";
	emit updateDirSignal(thumbs);
}

void DkFileSelection::setVisible(bool visible) {

	QWidget::setVisible(visible);
	thumbScrollWidget->getThumbWidget()->updateLayout();
}

void DkFileSelection::browse() {

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		cDir.absolutePath());

	if (dirName.isEmpty())
		return;

	setDir(QDir(dirName));
}


QList<QUrl> DkFileSelection::getSelectedFiles() {
	return thumbScrollWidget->getThumbWidget()->getSelectedUrls();
}

void DkFileSelection::setDir(const QDir& dir) {
		cDir = dir;
		qDebug() << "setting directory to:" << dir;
		directoryEdit->setText(cDir.absolutePath());
		emit newHeaderText(cDir.absolutePath());
		loader->setDir(cDir);

}

void DkFileSelection::emitChangedSignal() {
	emit changed();
}
// DkFileNameWdiget --------------------------------------------------------------------
DkFilenameWidget::DkFilenameWidget(QWidget* parent) : QWidget(parent) {
	createLayout();
	showOnlyFilename();
	hasChanged = false;
}

void DkFilenameWidget::createLayout() {
	curLayout = new QGridLayout(this);

	cBType = new QComboBox(this);
	cBType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	cBType->insertItem(fileNameTypes_fileName, tr("current filename"));
	cBType->insertItem(fileNameTypes_Text, tr("text"));
	cBType->insertItem(fileNameTypes_Number, tr("number"));
	connect(cBType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeCBChanged(int)));
	connect(cBType, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForUserInput()));

	cBCase = new QComboBox(this);
	cBCase->addItem(tr("keep case"));
	cBCase->addItem(tr("to lowercase"));
	cBCase->addItem(tr("to UPPERCASE"));
	connect(cBCase, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForUserInput()));

	sBNumber = new QSpinBox(this);
	sBNumber->setValue(1);
	sBNumber->setMinimum(0);
	sBNumber->setMaximum(9);

	cBDigits = new QComboBox(this);
	cBDigits->addItem(tr("1 digit"));
	cBDigits->addItem(tr("2 digits"));
	cBDigits->addItem(tr("3 digits"));
	cBDigits->addItem(tr("4 digits"));
	cBDigits->addItem(tr("5 digits"));
	connect(cBDigits, SIGNAL(currentIndexChanged(int)), this, SLOT(digitCBChanged(int)));

	lEText = new QLineEdit(this);

	pbPlus = new QPushButton("+", this);
	pbPlus->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	pbPlus->setMinimumSize(10,10);
	pbPlus->setMaximumSize(30,30);
	pbMinus = new QPushButton("-", this);
	pbMinus->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	pbMinus->setMinimumSize(10,10);
	pbMinus->setMaximumSize(30,30);
	connect(pbPlus, SIGNAL(clicked()), this, SLOT(pbPlusPressed()));
	connect(pbMinus, SIGNAL(clicked()), this, SLOT(pbMinusPressed()));
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
	cBCase->show();

	sBNumber->hide();
	cBDigits->hide();
	lEText->hide();

	curLayout->addWidget(cBType, 0, fileNameWidget_type);
	curLayout->addWidget(cBCase, 0, fileNameWidget_input1);
	//curLayout->addWidget(new QWidget(this), 0, fileNameWidget_input2 );
	curLayout->addWidget(pbPlus, 0, fileNameWidget_plus);
	curLayout->addWidget(pbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::showOnlyNumber() {
	sBNumber->show();
	cBDigits->show();

	cBCase->hide();
	lEText->hide();

	curLayout->addWidget(cBType, 0, fileNameWidget_type);
	curLayout->addWidget(sBNumber, 0, fileNameWidget_input1);
	curLayout->addWidget(cBDigits, 0, fileNameWidget_input2);
	curLayout->addWidget(pbPlus, 0, fileNameWidget_plus);
	curLayout->addWidget(pbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::showOnlyText() {
	lEText->show();

	sBNumber->hide();
	cBDigits->hide();
	cBCase->hide();
	

	curLayout->addWidget(cBType, 0, fileNameWidget_type);
	curLayout->addWidget(lEText, 0, fileNameWidget_input1);
	//curLayout->addWidget(new QWidget(this), 0, fileNameWidget_input2);
	curLayout->addWidget(pbPlus, 0, fileNameWidget_plus);
	curLayout->addWidget(pbMinus, 0, fileNameWidget_minus);
}

void DkFilenameWidget::pbPlusPressed() {
	emit plusPressed(this);
}

void DkFilenameWidget::pbMinusPressed() {
	emit minusPressed(this);
}

void DkFilenameWidget::enableMinusButton(bool enable) {
	pbMinus->setEnabled(enable);
}

void DkFilenameWidget::enablePlusButton(bool enable) {
	pbPlus->setEnabled(enable);
}

void DkFilenameWidget::checkForUserInput() {
	if(cBType->currentIndex() == 0 && cBCase->currentIndex() == 0)
		hasChanged = false;
	else
		hasChanged = true;
	emit changed();
}

void DkFilenameWidget::digitCBChanged(int index) {
	sBNumber->setMaximum(std::pow(10, index+1)-1);
}

// DkBatchOutput --------------------------------------------------------------------
DkBatchOutput::DkBatchOutput(QWidget* parent , Qt::WindowFlags f ) : QWidget(parent, f) {
	this->rUserInput = false;
	setObjectName("DkBatchOutput");
	createLayout();

	outputDirectory = QDir();
}

void DkBatchOutput::createLayout() {

	// Output Directory Groupbox
	QGroupBox* outDirGroupBox = new QGroupBox(this);
	outDirGroupBox->setTitle(tr("Output Directory"));
	QHBoxLayout* outDirGBLayout = new QHBoxLayout(outDirGroupBox);

	outputlineEdit = new DkDirectoryEdit();
	QPushButton* outputBrowseButton = new QPushButton(tr("Browse"));
	// TODO
	connect(outputBrowseButton , SIGNAL(clicked()), this, SLOT(browse()));
	connect(outputlineEdit, SIGNAL(textChanged(QString)), this, SLOT(emitChangedSignal()));
	outDirGBLayout->addWidget(outputlineEdit);
	outDirGBLayout->addWidget(outputBrowseButton);

	// Filename Groupbox
	QGroupBox* filenameGroupBox = new QGroupBox(this);
	filenameGroupBox->setTitle(tr("Filename"));
	filenameVBLayout = new QVBoxLayout(filenameGroupBox);
	DkFilenameWidget* fwidget = new DkFilenameWidget(this);
	fwidget->enableMinusButton(false);
	filenameWidgets.push_back(fwidget);
	filenameVBLayout->addWidget(fwidget);
	connect(fwidget, SIGNAL(plusPressed(DkFilenameWidget*)), this, SLOT(plusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(minusPressed(DkFilenameWidget*)), this, SLOT(minusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(changed()), this, SLOT(emitChangedSignal()));

	QWidget* extensionWidget = new QWidget(this);
	QHBoxLayout* extensionLayout = new QHBoxLayout(extensionWidget);
	cBExtension = new QComboBox(this);
	cBExtension->addItem(tr("keep extension"));
	cBExtension->addItem(tr("convert to"));
	connect(cBExtension, SIGNAL(currentIndexChanged(int)), this, SLOT(extensionCBChanged(int)));

	cBNewExtension = new QComboBox(this);
	cBNewExtension->addItem("add extensions here");
	cBNewExtension->setEnabled(false);

	extensionLayout->addWidget(cBExtension);
	extensionLayout->addWidget(cBNewExtension);
	extensionLayout->addStretch();
	filenameVBLayout->addWidget(extensionWidget);


	// Preview Widget
	QGroupBox* previewGroupBox = new QGroupBox(this);
	previewGroupBox->setTitle(tr("Filename Preview"));
	QHBoxLayout* previewGBLayout = new QHBoxLayout(previewGroupBox);

	QVBoxLayout* contentLayout = new QVBoxLayout(this);
	contentLayout->addWidget(outDirGroupBox);
	contentLayout->addWidget(filenameGroupBox);
	contentLayout->addWidget(previewGroupBox);
	setLayout(contentLayout);
}

void DkBatchOutput::browse() {

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		outputDirectory.absolutePath());

	if (dirName.isEmpty())
		return;

	setDir(QDir(dirName));
}

void DkBatchOutput::setDir(QDir dir) {
	outputDirectory = dir;
	emit newHeaderText(dir.absolutePath());
	outputlineEdit->setText(dir.absolutePath());
}

void DkBatchOutput::plusPressed(DkFilenameWidget* widget) {
	DkFilenameWidget* fwidget = new DkFilenameWidget(this);
	filenameWidgets.push_back(fwidget);
	if (filenameWidgets.size() > 4) {
		for (int i = 0; i  < filenameWidgets.size(); i++)
			filenameWidgets[i]->enablePlusButton(false);
	}
	filenameVBLayout->insertWidget(filenameWidgets.size()-1, fwidget);
	connect(fwidget, SIGNAL(plusPressed(DkFilenameWidget*)), this, SLOT(plusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(minusPressed(DkFilenameWidget*)), this, SLOT(minusPressed(DkFilenameWidget*)));
	connect(fwidget, SIGNAL(changed()), this, SLOT(emitChangedSignal()));

	emit changed();
}

void DkBatchOutput::minusPressed(DkFilenameWidget* widget) {
	filenameVBLayout->removeWidget(widget);
	filenameWidgets.remove(filenameWidgets.indexOf(widget));
	if (filenameWidgets.size() <= 4) {
		for (int i = 0; i  < filenameWidgets.size(); i++)
			filenameWidgets[i]->enablePlusButton(true);
	}

	widget->hide();

	emit changed();
}

void DkBatchOutput::extensionCBChanged(int index) {
	//index > 0 ? cBNewExtension->show() : cBNewExtension->hide();
	cBNewExtension->setEnabled(index > 0);
	emit changed();
}


bool DkBatchOutput::hasUserInput() {
	// TODO add output directory 
	return filenameWidgets.size() > 1 || filenameWidgets[0]->hasUserInput() || cBExtension->currentIndex() == 1;
}

void DkBatchOutput::emitChangedSignal() {
	emit changed();
}

QString DkBatchOutput::getOutputDirectory() {
	return outputlineEdit->existsDirectory() ? QDir(outputlineEdit->text()).absolutePath() : "";
}

// Batch Dialog --------------------------------------------------------------------

DkBatchDialog::DkBatchDialog(QDir currentDirectory, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {
	this->currentDirectory  = currentDirectory;

	setWindowTitle(tr("Batch Conversion"));
	createLayout();
}


void DkBatchDialog::createLayout() {
	widgets.resize(batchWidgets_end);
	// Input Directory
	widgets[batchWdidgets_input] = new DkBatchWidget(tr("Input Directory"), tr("directory not set"), this);
	DkFileSelection* fileSelection  = new DkFileSelection(widgets[batchWdidgets_input]);
	widgets[batchWdidgets_input]->setContentWidget(fileSelection);
	fileSelection->setDir(currentDirectory);
	

	widgets[batchWdidgets_output] = new DkBatchWidget(tr("Output"), tr("not set"), this);
	DkBatchOutput* outputSelection = new DkBatchOutput(widgets[batchWdidgets_output]);
	widgets[batchWdidgets_output]->setContentWidget(outputSelection);

	// buttons
	buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* dialogLayout = new QVBoxLayout();
	for (int i=0; i < widgets.size(); i++) {
		dialogLayout->addWidget(widgets[i]);
		connect(widgets[i]->contentWidget(), SIGNAL(changed()), this, SLOT(widgetChanged()));
	}
	dialogLayout->addWidget(buttons);

	setLayout(dialogLayout);
}

void DkBatchDialog::accept() {
	qDebug() << "accept is currently empty";
	//QList<QUrl> urls = fileSelection->getSelectedFiles();
	//for (int i = 0; i < urls.size(); i++) {
	//	qDebug() << urls[i];
	//}
}

void  DkBatchDialog::widgetChanged() {
	if (widgets[batchWdidgets_output] != 0 && widgets[batchWdidgets_input])  {
		bool outputChanged = dynamic_cast<DkBatchContent*>(widgets[batchWdidgets_output]->contentWidget())->hasUserInput();
		QString inputDirPath = dynamic_cast<DkFileSelection*>(widgets[batchWdidgets_input]->contentWidget())->getDir();
		QString outputDirPath = dynamic_cast<DkBatchOutput*>(widgets[batchWdidgets_output]->contentWidget())->getOutputDirectory();
		
		if (inputDirPath == "" || outputDirPath == "") {
			qDebug() << "inputDir or outputDir empty ... input:" << inputDirPath << " output:" << outputDirPath;
			return;
		}
		qDebug() << "outputDir:" << outputDirPath ;
		qDebug() << "inputDir:" << inputDirPath;

		bool enableButton = false;
		if (!outputChanged && inputDirPath.toLower() != outputDirPath.toLower())
			enableButton = true;
		else if (outputChanged)
			enableButton = true;

		buttons->button(QDialogButtonBox::Ok)->setEnabled(enableButton);
	}
}

}