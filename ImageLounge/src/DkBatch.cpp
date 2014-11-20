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
	titleLabel->setStyleSheet("QLabel{font-size: 16px;}");
	titleLabel->setAlignment(Qt::AlignBottom);
	headerLabel = new QLabel(headerString);
	headerLabel->setStyleSheet("QLabel{color: #666;}");
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
	setLayout(batchWidgetLayout);
}

void DkBatchWidget::setContentWidget(QWidget* batchContent) {
	batchWidgetLayout->addWidget(batchContent);
	connect(showButton, SIGNAL(toggled(bool)), batchContent, SLOT(setVisible(bool)));
	connect(batchContent, SIGNAL(newHeaderText(QString)), this, SLOT(setHeader(QString)));
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
	setStyleSheet("QLabel#DkFileSelection{border-radius: 5px; border: 1px solid #AAAAAA;}");
	createLayout();
	setMinimumHeight(300);

	loader = new DkImageLoader();
	connect(loader, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), this, SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT> >)));
}

void DkFileSelection::createLayout() {
	
	directoryEdit = new DkDirectoryEdit(this);

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
	connect(this, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), thumbScrollWidget, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));

	QVBoxLayout* widgetLayout = new QVBoxLayout();
	widgetLayout->addWidget(upperWidget);
	widgetLayout->addWidget(thumbScrollWidget);
	setLayout(widgetLayout);
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
	//indexDir();

	setDir(cDir);
}

void DkFileSelection::indexDir() {

	emit dirSignal(cDir.absolutePath());

	cDir.setSorting(QDir::LocaleAware);
	thumbScrollWidget->setDir(QFileInfo(cDir.absolutePath()));
	
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

		indexDir();
}

// DkBatchOutput --------------------------------------------------------------------
DkBatchOutput::DkBatchOutput(QWidget* parent , Qt::WindowFlags f ) : QWidget(parent, f) {
	this->hUserInput = false;
	this->rUserInput = false;
	setObjectName("DkBatchOutput");
	createLayout();
}

void DkBatchOutput::createLayout() {

	// Output Directory Groupbox
	QGroupBox* outDirGroupBox = new QGroupBox(this);
	outDirGroupBox->setTitle(tr("Output Directory"));
	QHBoxLayout* outDirGBLayout = new QHBoxLayout(outDirGroupBox);

	DkDirectoryEdit* outputlineEdit= new DkDirectoryEdit();
	QPushButton* outputBrowseButton = new QPushButton(tr("Browse"));
	// TODO
	//connect(outputBrowseButton , SIGNAL(clicked()), this, SLOT(browseOutputDir()));
	outDirGBLayout->addWidget(outputlineEdit);
	outDirGBLayout->addWidget(outputBrowseButton);

	// Filename Groupbox
	QGroupBox* filenameGroupBox = new QGroupBox(this);
	filenameGroupBox->setTitle(tr("Filename"));
	QHBoxLayout* filenameGBLayout = new QHBoxLayout(filenameGroupBox);



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

// Batch Dialog --------------------------------------------------------------------

DkBatchDialog::DkBatchDialog(QDir currentDirectory, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {
	this->currentDirectory  = currentDirectory;

	setWindowTitle(tr("Batch Conversion"));
	createLayout();


}


void DkBatchDialog::createLayout() {
	widgets.resize(batchWdidgets_end);
	// Input Directory
	widgets[batchWdidgets_input] = new DkBatchWidget(tr("Input Directory"), tr("directory not set"), this);
	DkFileSelection* fileSelection  = new DkFileSelection(widgets[batchWdidgets_input]);
	widgets[batchWdidgets_input]->setContentWidget(fileSelection);
	fileSelection->setDir(currentDirectory);
	

	widgets[batchWdidgets_output] = new DkBatchWidget(tr("Output"), tr("not set"), this);
	DkBatchOutput* outputSelection = new DkBatchOutput(widgets[batchWdidgets_output]);
	widgets[batchWdidgets_output]->setContentWidget(outputSelection);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* dialogLayout = new QVBoxLayout();
	for (int i=0; i < widgets.size(); i++) {
		dialogLayout->addWidget(widgets[i]);
	}
	//dialogLayout->addWidget(fileSelection);
	//dialogLayout->addWidget(outputSelection);
	dialogLayout->addStretch();
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

}