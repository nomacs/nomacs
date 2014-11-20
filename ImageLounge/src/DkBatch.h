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

#include "DkWidgets.h"
#include "DkThumbsWidgets.h"



namespace nmc {

enum batchWidgets {
	batchWdidgets_input,
	batchWdidgets_output,

	batchWdidgets_end
};


class DkBatchWidget : public QWidget {
	Q_OBJECT
	public:
		DkBatchWidget(QString titleString, QString headerString, QWidget* parent = 0, Qt::WindowFlags f = 0);
	
		void setContentWidget(QWidget* batchContent);

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

		QDir getDir() {
			return cDir;
		};

		void setDir(const QDir& dir);

		QList<QUrl> getSelectedFiles();

		virtual bool hasUserInput() {return hUserInput;};
		virtual bool requiresUserInput() {return rUserInput;};

	public slots:
		void browse();
		void indexDir();
		void updateDir(QVector<QSharedPointer<DkImageContainerT> >);

	signals:
		void dirSignal(const QString& dir);
		void updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >);
		void newHeaderText(QString);

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

class DkBatchOutput : public QWidget, public DkBatchContent {
	Q_OBJECT

	public:
		DkBatchOutput(QWidget* parent = 0, Qt::WindowFlags f = 0);

		virtual bool hasUserInput()  {return hUserInput;};;
		virtual bool requiresUserInput()  {return rUserInput;};;

	signals:
		void newHeaderText(QString);

	protected:
		virtual void createLayout();

	private:
		bool hUserInput;
		bool rUserInput;

};

class DkBatchDialog : public QDialog {
		Q_OBJECT

	public:
		DkBatchDialog(QDir currentDirectory = QDir(), QWidget* parent = 0, Qt::WindowFlags f = 0);

	public slots:
		virtual void accept();

	protected:
		void createLayout();
		
	private:
		QVector<DkBatchWidget*> widgets;
		
		QDir currentDirectory;
};
}
