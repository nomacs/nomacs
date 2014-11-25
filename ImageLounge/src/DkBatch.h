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

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListView>
#include <QLineEdit>
#include <QFileDialog>
#include <QGroupBox>
#include <QComboBox>

#include "DkWidgets.h"
#include "DkThumbsWidgets.h"



namespace nmc {

enum batchWidgets {
	batchWdidgets_input,
	batchWdidgets_output,

	batchWidgets_end
};

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


class DkBatchWidget : public QWidget {
	Q_OBJECT
	public:
		DkBatchWidget(QString titleString, QString headerString, QWidget* parent = 0, Qt::WindowFlags f = 0);
	
		void setContentWidget(QWidget* batchContent);
		QWidget* contentWidget() const;

	public slots:
		void setTitle(QString title);
		void setHeader(QString header);
		

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

		void setDir(const QDir& dir);

		QList<QUrl> getSelectedFiles();

		virtual bool hasUserInput() {return hUserInput;};
		virtual bool requiresUserInput() {return rUserInput;};

	public slots:
		void browse();
		void updateDir(QVector<QSharedPointer<DkImageContainerT> >);
		void setVisible(bool visible);
		void emitChangedSignal();

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
		DkDirectoryEdit* directoryEdit;

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
		bool hasUserInput() {return hasChanged;};

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

		virtual bool hasUserInput();
		virtual bool requiresUserInput()  {return rUserInput;};
		QString getOutputDirectory();

	signals:
		void newHeaderText(QString);
		void changed();

	protected slots:
		void browse();
		void plusPressed(DkFilenameWidget* widget);
		void minusPressed(DkFilenameWidget* widget);
		void extensionCBChanged(int index);
		void emitChangedSignal();

	protected:
		virtual void createLayout();
		void setDir(QDir dir);

	private:
		bool hUserInput;
		bool rUserInput;
		QDir outputDirectory;
		DkDirectoryEdit* outputlineEdit;
		QVector<DkFilenameWidget*> filenameWidgets;
		QVBoxLayout* filenameVBLayout;

		QComboBox* cBExtension;
		QComboBox* cBNewExtension;

};

class DkBatchDialog : public QDialog {
	Q_OBJECT

	public:
		DkBatchDialog(QDir currentDirectory = QDir(), QWidget* parent = 0, Qt::WindowFlags f = 0);

	public slots:
		virtual void accept();
		void widgetChanged();

	protected:
		void createLayout();
		
	private:
		QVector<DkBatchWidget*> widgets;
		
		QDir currentDirectory;
		QDialogButtonBox* buttons;
};
}
