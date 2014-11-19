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

#include "DkThumbsWidgets.h"



namespace nmc {

class DkFileSelection : public QLabel {
	Q_OBJECT

	public:
		DkFileSelection(QWidget* parent = 0, Qt::WindowFlags f = 0);

	QDir getDir() {
		return cDir;
	};

	void setDir(const QDir& dir) {
		cDir = dir;
		indexDir();
	};

	QList<QUrl> getSelectedFiles();

	public slots:
		void browse();
		void indexDir();
		void updateDir(QVector<QSharedPointer<DkImageContainerT> >);

	signals:
		void dirSignal(const QString& dir);
		void updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >);

	protected:
		void createLayout();

		QDir cDir;
		QListView* fileWidget;
		DkThumbScrollWidget* thumbScrollWidget;
};

class DkBatchDialog : public QDialog {
		Q_OBJECT

	public:
		DkBatchDialog(QDir currentDirectory = QDir(), QWidget* parent = 0, Qt::WindowFlags f = 0);

	public slots:
		virtual void accept();
		void browseOutputDir();
		void setInputDir(const QString& dirName);

	protected:
		void createLayout();

		
		QLabel* inputDirLabel;
		QLabel* outputDirLabel;
		DkFileSelection* fileSelection;
		DkImageLoader* loader;
		QDir outputDir;
		QDir currentDirectory;
};
}
