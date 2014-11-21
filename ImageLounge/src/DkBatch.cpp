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
	
	contentWidget = new QWidget();

	DkButton* showButton = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	showButton->setFixedSize(QSize(16,16));
	showButton->setObjectName("showSelectionButton");
	showButton->setCheckable(true);
	showButton->setChecked(true);
	connect(showButton, SIGNAL(toggled(bool)), contentWidget, SLOT(setVisible(bool)));

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

	QVBoxLayout* batchWidgetLayout = new QVBoxLayout;
	batchWidgetLayout->addWidget(headerWidget);
	batchWidgetLayout->addWidget(contentWidget);
	//batchWidgetLayout->addStretch();
	setLayout(batchWidgetLayout);
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
DkFileSelection::DkFileSelection(QString titleString, QString headerString, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : DkBatchWidget(titleString, headerString, parent, f) {

	setObjectName("DkFileSelection");
	createLayout();
	setMinimumHeight(300);
}

void DkFileSelection::createLayout() {

	QLineEdit* filterEdit = new QLineEdit();

	QPushButton* browseButton = new QPushButton(tr("Browse"));
	connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

	//fileModel = new DkFileModel();

	//fileWidget = new QListView();
	//fileWidget->setStyleSheet("QListView::item:alternate{background: #BBB;}");
	//fileWidget->setModel(fileModel);
	thumbScrollWidget = new DkThumbScrollWidget(this);
	thumbScrollWidget->setVisible(true);
	
	connect(this, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), thumbScrollWidget, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));
	//contentWidget = new QWidget(this);
	QGridLayout* fsLayout = new QGridLayout(contentWidget);
	fsLayout->addWidget(filterEdit, 0, 0, 1, 4);
	fsLayout->addWidget(browseButton, 0, 4);
	fsLayout->addWidget(thumbScrollWidget, 1, 0, 1, 5);	// change to 4 if we support thumbs
	//setContentWidget(contentWidget);
	//fsLayout->setRowStretch(2, 300);

	//setLayout(fsLayout);
}

void DkFileSelection::updateDir(QVector<QSharedPointer<DkImageContainerT> > thumbs) {
	qDebug() << "emitting updateDirSignal";
	emit updateDirSignal(thumbs);
}

void DkFileSelection::browse() {

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		cDir.absolutePath());

	if (dirName.isEmpty())
		return;

	cDir = QDir(dirName);

	indexDir();
}

void DkFileSelection::indexDir() {

	emit dirSignal(cDir.absolutePath());

	cDir.setSorting(QDir::LocaleAware);
	thumbScrollWidget->setDir(QFileInfo(cDir.absolutePath()));
	
}

QList<QUrl> DkFileSelection::getSelectedFiles() {
	return thumbScrollWidget->getThumbWidget()->getSelectedUrls();
}

// DkBatchOutput --------------------------------------------------------------------
DkBatchOutput::DkBatchOutput(QString titleString, QString headerString, QWidget* parent , Qt::WindowFlags f ) : DkBatchWidget(titleString, headerString, parent, f) {
	setObjectName("DkBatchOutput");
	createLayout();
}

void DkBatchOutput::createLayout() {
	QGridLayout* contentLayout = new QGridLayout(contentWidget);

	QLineEdit* outputlineEdit= new QLineEdit();

	QPushButton* outputBrowseButton = new QPushButton(tr("Browse"));
	connect(outputBrowseButton , SIGNAL(clicked()), this, SLOT(browseOutputDir()));

	contentLayout->addWidget(outputlineEdit, 0, 0);
	contentLayout->addWidget(outputBrowseButton, 0, 1);
}

// Batch Dialog --------------------------------------------------------------------

DkBatchDialog::DkBatchDialog(QDir currentDirectory, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {
	setWindowTitle(tr("Batch Conversion"));
	createLayout();

	this->currentDirectory  = currentDirectory;
	loader = new DkImageLoader();
	connect(loader, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), fileSelection, SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT> >)));

	fileSelection->setDir(currentDirectory);
}

void DkBatchDialog::createLayout() {
	// Input Directory
	fileSelection = new DkFileSelection("Input Directory", "directory not set", this);
	connect(fileSelection, SIGNAL(dirSignal(const QString&)), this, SLOT(setInputDir(const QString&)));
	//fileSelection->hide();
	


	outputSelection = new DkBatchOutput("Output", "not set", this);


	// File Output
	//QWidget* outputSelection = new QWidget();

	//DkButton* outputButton = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	//outputButton->setFixedSize(QSize(16,16));
	//outputButton->setObjectName("showOutputButton");
	//outputButton->setCheckable(true);
	//outputButton->setChecked(true);
	//connect(outputButton, SIGNAL(toggled(bool)), outputSelection, SLOT(setVisible(bool)));	// TODO

	//QLabel* outputLabel = new QLabel(tr("File Output"));
	//outputLabel->setStyleSheet("QLabel{font-size: 15px;}");
	//outputDirLabel = new QLabel(tr("No Directory selected"));
	//outputDirLabel->setStyleSheet("QLabel{color: #333;}");

	//QWidget* outputWidget = new QWidget();
	//QHBoxLayout* outputLayout = new QHBoxLayout(outputWidget);
	//outputLayout->setContentsMargins(0,0,0,0);
	//outputLayout->addWidget(outputButton);
	//outputLayout->addWidget(outputLabel);
	//outputLayout->addWidget(outputDirLabel);
	//outputLayout->addStretch();

	//QLineEdit* outputlineEdit= new QLineEdit();

	//QPushButton* outputBrowseButton = new QPushButton(tr("Browse"));
	//connect(outputBrowseButton , SIGNAL(clicked()), this, SLOT(browseOutputDir()));
	//
	//QHBoxLayout* outputSelLayout = new QHBoxLayout(outputSelection);
	//outputSelLayout->addWidget(outputlineEdit);
	//outputSelLayout->addWidget(outputBrowseButton);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* dialogLayout = new QVBoxLayout();
	//dialogLayout->addWidget(fileSelectionWidget);
	dialogLayout->addWidget(fileSelection);
	dialogLayout->addWidget(outputSelection);
	//dialogLayout->addWidget(outputSelection);
	//dialogLayout->addStretch();
	dialogLayout->addWidget(buttons);

	setLayout(dialogLayout);
}

void DkBatchDialog::setInputDir(const QString& dirName) {
	qDebug() << "BatchDialog: inputDir set to " << dirName;
	fileSelection->setHeader(dirName);
	loader->loadDir(QDir(dirName));
}

void DkBatchDialog::accept() {
	qDebug() << "processing images:";
	QList<QUrl> urls = fileSelection->getSelectedFiles();
	for (int i = 0; i < urls.size(); i++) {
		qDebug() << urls[i];
	}
}

void DkBatchDialog::browseOutputDir() {
	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		currentDirectory.absolutePath());

	if (dirName.isEmpty())
		return;

	outputDir = QDir(dirName);
	outputDirLabel->setText(dirName);
}

}