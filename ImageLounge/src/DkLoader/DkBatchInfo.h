/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#pragma once

#pragma warning(push, 0)	// no warnings from includes
#include <QObject>
#pragma warning(pop)

#ifndef DllLoaderExport
#ifdef DK_LOADER_DLL_EXPORT
#define DllLoaderExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllLoaderExport Q_DECL_IMPORT
#else
#define DllLoaderExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QFileInfo;
class QSettings;

namespace nmc {

// nomacs defines

class DllLoaderExport DkBatchInfo {

public:
	DkBatchInfo(const QString& id = QString(), const QString& filePath = QString());

	virtual bool isEmpty() const;
	
	virtual QString filePath() const;
	virtual QFileInfo fileInfo() const;
	virtual void setFilePath(const QString& filePath);

	virtual void setId(const QString& id);
	virtual QString id() const;

	QString toString() const;

	static QVector<QSharedPointer<DkBatchInfo> > filter(const QVector<QSharedPointer<DkBatchInfo> >& infos, const QString& id);
	friend DllLoaderExport QDataStream& operator<<(QDataStream& s, const DkBatchInfo& b);
	friend DllLoaderExport QDebug operator<< (QDebug d, const DkBatchInfo &b);

private:
	QString mFilePath;
	QString mId;

};

class DllLoaderExport DkSaveInfo {

public:
	DkSaveInfo(const QString& filePathIn = QString(), const QString& filePathOut = QString());

	enum OverwriteMode {
		mode_overwrite,
		mode_skip_existing,

		mode_end
	};

	void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings) const;

	void setInputFilePath(const QString& inputFilePath);
	void setOutputFilePath(const QString& outputFilePath);

	void setMode(OverwriteMode mode);
	void setDeleteOriginal(bool deleteOriginal);
	void setCompression(int compression);
	void setInputDirIsOutputDir(bool isOutputDir);

	QString inputFilePath() const;
	QString outputFilePath() const;
	QString backupFilePath() const;

	QFileInfo inputFileInfo() const;
	QFileInfo outputFileInfo() const;
	QFileInfo backupFileInfo() const;

	OverwriteMode mode() const;
	bool isDeleteOriginal() const;
	bool isInputDirOutputDir() const;
	int compression() const;

	void createBackupFilePath();
	void clearBackupFilePath();

private:
	QString mFilePathIn;
	QString mFilePathOut;
	QString mBackupPath;

	OverwriteMode mMode = mode_skip_existing;
	int mCompression = -1;
	bool mDeleteOriginal = false;
	bool mInputDirIsOutputDir = false;

};


}