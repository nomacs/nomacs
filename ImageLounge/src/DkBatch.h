/*******************************************************************************************************
DkNoMacs.h
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
#pragma once;

#include "DkWidgets.h"
#include "DkThumbsWidgets.h"

// Qt defines
class QListView;

namespace nmc {

// TODO: these enums are global - they should be put into the respective classes
enum fileNameTypes {
fileNameTypes_fileName,
fileNameTypes_Number,
fileNameTypes_Text,

fileNameTypes_end
};

enum fileNameWidget {
fileNameWidget_type,
fileNameWidget_input1,
fileNameWidget_input2,
fileNameWidget_plus,
fileNameWidget_minus,

fileNameWidget_end
};

// from Process.h
class DkResizeBatch;
class DkBatchProcessing;
class DkBatchTransform;


class DkBatchWidget : public QWidget {
	Q_OBJECT

public:
	DkBatchWidget(QString titleString, QString headerString, QWidget* parent = 0, Qt::WindowFlags f = 0);
	
	void setContentWidget(QWidget* batchContent);
	QWidget* contentWidget() const;

public slots:
	void setTitle(QString title);
	void setHeader(QString header);
	void showContent(bool show);

protected:
	virtual void createLayout();

private:
	DkBatchContent* batchContent;
	QVBoxLayout* batchWidgetLayout;
	QString titleString, headerString;
	QLabel* titleLabel; 
	QLabel* headerLabel;
	DkButton* showButton;
};


class DkFileSelection : public QWidget, public DkBatchContent  {
Q_OBJECT

public:
	DkFileSelection(QWidget* parent = 0, Qt::WindowFlags f = 0);

	QString getDir() {
		return directoryEdit->existsDirectory() ? QDir(directoryEdit->text()).absolutePath() : "";
	};



	QList<QUrl> getSelectedFiles();

	virtual bool hasUserInput() const {return hUserInput;};
	virtual bool requiresUserInput() const {return rUserInput;};

public slots:
	void setDir(QDir dir);
	void browse();
	void updateDir(QVector<QSharedPointer<DkImageContainerT> >);
	void setVisible(bool visible);
	void emitChangedSignal();
	void selectionChanged();
	void filterTextChanged(const QString& filterText);
	void setFileInfo(QFileInfo file);

signals:
	void updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >);
	void newHeaderText(QString);
	void changed();

protected:
	virtual void createLayout();

	QDir cDir;
	QListView* fileWidget;
	DkThumbScrollWidget* thumbScrollWidget;
	DkImageLoader* loader;
	DkExplorer* explorer;
	DkDirectoryEdit* directoryEdit;
	QLabel* infoLabel;

private:
	bool hUserInput;
	bool rUserInput;

};

class DkFilenameWidget : public QWidget {
Q_OBJECT

public:	
	DkFilenameWidget(QWidget* parent = 0);
	void enableMinusButton(bool enable);
	void enablePlusButton(bool enable);
	bool hasUserInput() const {return hasChanged;};
	QString getTag() const;

signals:
	void plusPressed(DkFilenameWidget*);
	void minusPressed(DkFilenameWidget*);
	void changed();

private slots:
	void typeCBChanged(int index);
	void pbPlusPressed();
	void pbMinusPressed();
	void checkForUserInput();
	void digitCBChanged(int index);

private:
	void createLayout();
	void showOnlyText();
	void showOnlyNumber();
	void showOnlyFilename();

	QComboBox* cBType;
		
	QLineEdit* lEText;
	QComboBox* cBCase;

	QSpinBox* sBNumber;
	QComboBox* cBDigits;
		
	QPushButton* pbPlus;
	QPushButton* pbMinus;

	QGridLayout* curLayout;

	bool hasChanged;
};

class DkBatchOutput : public QWidget, public DkBatchContent {
Q_OBJECT

public:
	DkBatchOutput(QWidget* parent = 0, Qt::WindowFlags f = 0);

	virtual bool hasUserInput() const;
	virtual bool requiresUserInput() const {return rUserInput;};
	int overwriteMode();
	QString getOutputDirectory();
	QString getFilePattern();
	void setExampleFilename(const QString& exampleName);
	void setDir(QDir);

signals:
	void newHeaderText(QString);
	void changed();

protected slots:
	void browse();
	void plusPressed(DkFilenameWidget* widget);
	void minusPressed(DkFilenameWidget* widget);
	void extensionCBChanged(int index);
	void emitChangedSignal();
	void updateFileLabelPreview();

protected:
	virtual void createLayout();
	

private:

	bool hUserInput;
	bool rUserInput;
	QDir outputDirectory;
	DkDirectoryEdit* outputlineEdit;
	QVector<DkFilenameWidget*> filenameWidgets;
	QVBoxLayout* filenameVBLayout;
	QCheckBox* cbOverwriteExisting;

	QComboBox* cBExtension;
	QComboBox* cBNewExtension;
	QLabel* oldFileNameLabel;
	QLabel* newFileNameLabel;
	QString exampleName;

};

class DkBatchResizeWidget : public QWidget, public DkBatchContent {
	Q_OBJECT

public:
	DkBatchResizeWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

	void transferProperties(QSharedPointer<DkResizeBatch> batchResize) const;
	bool hasUserInput() const;
	bool requiresUserInput() const;

public slots:
	void modeChanged(int idx);
	void percentChanged(double val);
	void pxChanged(int val);

signals:
	void newHeaderText(QString txt);

protected:
	void createLayout();

	QComboBox* comboMode;
	QComboBox* comboProperties;
	QSpinBox* sbPx;
	QDoubleSpinBox* sbPercent;
};

class DkBatchTransformWidget : public QWidget, public DkBatchContent {
	Q_OBJECT

public:
	DkBatchTransformWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

	void transferProperties(QSharedPointer<DkBatchTransform> batchTransform) const;
	bool hasUserInput() const;
	bool requiresUserInput() const;

public slots:
	void radioButtonClicked(int id);
	void checkBoxClicked();

signals:
	void newHeaderText(QString txt) const;

protected:
	void createLayout();
	void updateHeader() const;
	int getAngle() const;

	QButtonGroup* rotateGroup;
	QRadioButton* rbRotate0;
	QRadioButton* rbRotateLeft;
	QRadioButton* rbRotateRight;
	QRadioButton* rbRotate180;

	QCheckBox* cbFlipH;
	QCheckBox* cbFlipV;
};


class DkBatchDialog : public QDialog {
	Q_OBJECT

public:
	DkBatchDialog(QDir currentDirectory = QDir(), QWidget* parent = 0, Qt::WindowFlags f = 0);

	enum batchWidgets {
		batch_input,
		batch_resize,
		batch_transform,
		batch_output,

		batchWidgets_end
	};

public slots:
	virtual void accept();
	virtual void reject();
	void widgetChanged();
	void logButtonClicked();
	void processingFinished();
	void updateProgress(int progress);

protected:
	void createLayout();
		
private:
	QVector<DkBatchWidget*> widgets;
		
	QDir currentDirectory;
	QDialogButtonBox* buttons;
	DkFileSelection* fileSelection;
	DkBatchOutput* outputSelection;
	DkBatchResizeWidget* resizeWidget;
	DkBatchTransformWidget* transformWidget;
	DkBatchProcessing* batchProcessing;
	QPushButton* logButton;
	QProgressBar* progressBar;

	void startProcessing();
	void stopProcessing();
};

}
