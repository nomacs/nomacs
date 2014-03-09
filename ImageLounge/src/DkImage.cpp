/*******************************************************************************************************
 DkImage.cpp
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

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

#include "DkImage.h"
#include "DkNoMacs.h"
#include <QPluginLoader>

namespace nmc {

// well this is pretty shitty... but we need the filter without description too
QStringList DkImageLoader::fileFilters = QStringList();

// formats we can save
QStringList DkImageLoader::saveFilters = QStringList();

// formats we can load
QStringList DkImageLoader::openFilters = QStringList();

// DkImageLoader -> is nomacs file handling routine --------------------------------------------------------------------
/**
 * Default constructor.
 * Creates a DkImageLoader instance with a given file.
 * @param file the file to be loaded.
 **/ 
DkImageLoader::DkImageLoader(QFileInfo file) {

	qRegisterMetaType<QFileInfo>("QFileInfo");

	watcher = 0;
	// init the watcher
	//watcher = new QFileSystemWatcher();
	//connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));
	
	// this seems to be unnecessary complicated, but otherwise we create the watcher in different threads
	// (depending on if loadFile() is called threaded) which is not a very good ides
	connect(this, SIGNAL(updateFileWatcherSignal(QFileInfo)), this, SLOT(updateFileWatcher(QFileInfo)), Qt::QueuedConnection);

	dirWatcher = new QFileSystemWatcher();
	connect(dirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));

	folderUpdated = false;
	tmpFileIdx = 0;

	delayedUpdateTimer.setSingleShot(true);
	connect(&delayedUpdateTimer, SIGNAL(timeout()), this, SLOT(directoryChanged()));
	timerBlockedUpdate = false;

	//saveDir = DkSettings::global.lastSaveDir;	// loading save dir is obsolete ?!
	saveDir = "";

	if (file.exists())
		loadDir(file);
	else
		dir = DkSettings::global.lastDir;

	// init cacher
	initFileFilters();
}

/**
 * Default destructor.
 **/ 
DkImageLoader::~DkImageLoader() {

	delete dirWatcher;	// needed?

	qDebug() << "dir open: " << dir.absolutePath();
	qDebug() << "filepath: " << saveDir.absolutePath();
}

void DkImageLoader::initFileFilters() {

	//// load plugins
	//QDir pluginFolder(QCoreApplication::applicationDirPath());
	//pluginFolder.cd("imageformats");

	//QStringList pluginFilenames = pluginFolder.entryList(QStringList("*.dll"));
	//qDebug() << "searching for plugins: " << pluginFolder.absolutePath();
	//qDebug() << "plugins found: " << pluginFilenames;

	//for (int idx = 0; idx < pluginFilenames.size(); idx++) {
	//	QPluginLoader p(QFileInfo(pluginFolder, pluginFilenames[idx]).absoluteFilePath());
	//	if (!p.load())
	//		qDebug() << "sorry, I could NOT load " << pluginFilenames[idx] << " since:\n" << p.errorString();
	//	
	//}

	if (!openFilters.empty())
		return;


	QList<QByteArray> qtFormats = QImageReader::supportedImageFormats();

	// formats we can save
	if (qtFormats.contains("png"))		saveFilters.append("Portable Network Graphics (*.png)");
	if (qtFormats.contains("jpg"))		saveFilters.append("JPEG (*.jpg *.jpeg)");
	if (qtFormats.contains("j2k"))		saveFilters.append("JPEG 2000 (*.jp2 *.j2k *.jpf *.jpx *.jpm *.jpgx)");
	if (qtFormats.contains("tif"))		saveFilters.append("TIFF (*.tif *.tiff)");
	if (qtFormats.contains("bmp"))		saveFilters.append("Windows Bitmap (*.bmp)");
	if (qtFormats.contains("ppm"))		saveFilters.append("Portable Pixmap (*.ppm)");
	if (qtFormats.contains("xbm"))		saveFilters.append("X11 Bitmap (*.xbm)");
	if (qtFormats.contains("xpm"))		saveFilters.append("X11 Pixmap (*.xpm)");

	// internal filters
#ifdef WITH_WEBP
	saveFilters.append("WebP (*.webp)");
#endif

	// formats we can load
	openFilters += saveFilters;
	if (qtFormats.contains("gif"))		openFilters.append("Graphic Interchange Format (*.gif)");
	if (qtFormats.contains("pbm"))		openFilters.append("Portable Bitmap (*.pbm)");
	if (qtFormats.contains("pgm"))		openFilters.append("Portable Graymap (*.pgm)");
	if (qtFormats.contains("ico"))		openFilters.append("Icon Files (*.ico)");
	if (qtFormats.contains("tga"))		openFilters.append("Targa Image File (*.tga)");
	if (qtFormats.contains("mng"))		openFilters.append("Multi-Image Network Graphics (*.mng)");

#ifdef WITH_OPENCV
	// raw format
	openFilters.append("Nikon Raw (*.nef)");
	openFilters.append("Canon Raw (*.crw *.cr2)");
	openFilters.append("Sony Raw (*.arw)");
	openFilters.append("Digital Negativ (*.dng)");
	openFilters.append("Panasonic Raw (*.rw2)");
	openFilters.append("Minolta Raw (*.mrw)");
#endif

	// stereo formats
	openFilters.append("JPEG Stereo (*.jps)");
	openFilters.append("PNG Stereo (*.pns)");
	openFilters.append("Multi Picture Object (*.mpo)");
	
	// other formats
	openFilters.append("Adobe Photoshop (*.psd)");
	openFilters.append("Large Document Format (*.psb)");
	openFilters.append("Rohkost (*.roh)");

	// load user filters
	QSettings settings;
	openFilters += settings.value("ResourceSettings/userFilters", QStringList()).toStringList();

	for (int idx = 0; idx < openFilters.size(); idx++) {

		QString cFilter = openFilters[idx];
		cFilter = cFilter.section(QRegExp("(\\(|\\))"), 1);
		cFilter = cFilter.replace(")", "");
		DkImageLoader::fileFilters += cFilter.split(" ");
	}

	QString allFilters = DkImageLoader::fileFilters.join(" ");

	// add unknown formats from Qt plugins
	for (int idx = 0; idx < qtFormats.size(); idx++) {

		if (!allFilters.contains(qtFormats.at(idx))) {
			openFilters.append("Image Format (*." + qtFormats.at(idx) + ")");
			DkImageLoader::fileFilters.append("*." + qtFormats.at(idx));
		}
	}

	openFilters.prepend("Image Files (" + fileFilters.join(" ") + ")");

	qDebug() << "supported: " << qtFormats;

#ifdef Q_OS_WIN
	DkImageLoader::fileFilters.append("*.lnk");
#endif
}

/**
 * Clears the path.
 * Calling this method makes the loader forget
 * about the current directory. It also destroys
 * the currently loaded image.
 **/ 
void DkImageLoader::clearPath() {

	// lastFileLoaded must exist
	if (!currentImage.isNull() && currentImage->exists())
		lastImageLoaded = currentImage;

	currentImage.clear();
}

/**
 * Clears the current file watch.
 **/ 
void DkImageLoader::clearFileWatcher() {

	if (!watcher)
		return;

	if (!watcher->files().isEmpty())
		watcher->removePaths(watcher->files());	// remove all files previously watched
}

bool DkImageLoader::loadDir(QFileInfo newFile, bool scanRecursive /* = true */) {

	newFile.refresh();
	if (!newFile.exists())
		return false;

	return loadDir(newFile.absoluteDir(), scanRecursive);
}

/**
 * Loads a given directory and the first image in this directory.
 * @param newDir the directory to be loaded.
 **/ 
bool DkImageLoader::loadDir(QDir newDir, bool scanRecursive) {

	// folder changed signal was emitted
	if (folderUpdated && newDir.absolutePath() == dir.absolutePath()) {

		QFileInfoList files = getFilteredFileInfoList(dir, ignoreKeywords, keywords, folderKeywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		createImages(files);

		// might get empty too (e.g. someone deletes all images
		if (files.empty()) {
			emit showInfoSignal(tr("%1 \n does not contain any image").arg(dir.absolutePath()), 4000);	// stop showing
			return false;
		}

		emit updateDirSignal(images);
		folderUpdated = false;
		qDebug() << "getting file list.....";
	}
	// new folder is loaded
	else if ((newDir.absolutePath() != dir.absolutePath() || images.empty()) && newDir.exists()) {

		QFileInfoList files;

		// update save directory
		dir = newDir;
		dir.setNameFilters(fileFilters);
		dir.setSorting(QDir::LocaleAware);		// TODO: extend
		folderUpdated = false;

		folderKeywords.clear();	// delete key words -> otherwise user may be confused
		emit folderFiltersChanged(folderKeywords);

		if (scanRecursive && DkSettings::global.scanSubFolders)
			files = updateSubFolders(dir);
		else 
			files = getFilteredFileInfoList(dir, ignoreKeywords, keywords, folderKeywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)

		if (files.empty()) {
			emit showInfoSignal(tr("%1 \n does not contain any image").arg(dir.absolutePath()), 4000);	// stop showing
			return false;
		}

		// ok new folder, this should speed-up loading
		images.clear();
		createImages(files);

		emit updateDirSignal(images);

		if (dirWatcher) {
			if (!dirWatcher->directories().isEmpty())
				dirWatcher->removePaths(dirWatcher->directories());
			dirWatcher->addPath(dir.absolutePath());
		}
	}
	//else
	//	qDebug() << "ignoring... old dir: " << dir.absolutePath() << " newDir: " << newDir << " file size: " << images.size();

	return true;
}

void DkImageLoader::createImages(const QFileInfoList& files) {

	DkTimer dt;
	QVector<QSharedPointer<DkImageContainerT > > oldImages = images;
	images.clear();

	for (int idx = 0; idx < files.size(); idx++) {

		int oIdx = findFileIdx(files.at(idx), oldImages);

		if (oIdx != -1 && oldImages.at(oIdx)->file().lastModified() == files.at(idx).lastModified())
			images.append(oldImages.at(oIdx));
		else
			images.append(QSharedPointer<DkImageContainerT >(new DkImageContainerT(files.at(idx))));
	}

	qSort(images.begin(), images.end(), imageContainerLessThanPtr);
	qDebug() << "[DkImageLoader] " << images.size() << " indexed in " << QString::fromStdString(dt.getTotal());
}

/**
 * Loads the next file.
 * @param silent if true, no status information will be displayed.
 **/ 
void DkImageLoader::nextFile(bool silent) {
	
	qDebug() << "loading next file";
	changeFile(1, silent);
}

/**
 * Loads the previous file.
 * @param silent if true, no status information will be displayed.
 **/ 
void DkImageLoader::previousFile(bool silent) {
	
	qDebug() << "loading previous file";
	changeFile(-1, silent);
}

/**
 * Loads the ancesting or subsequent file.
 * @param skipIdx the number of files that should be skipped after/before the current file.
 * @param silent if true, no status information will be displayed.
 **/ 
void DkImageLoader::changeFile(int skipIdx, bool silent, int cacheState) {

	//if (!img.isNull() && !file.exists())
	//	return;
	//if (!file.exists() && !virtualFile.exists()) {
	//	qDebug() << virtualFile.absoluteFilePath() << "does not exist...!!!";
	//	return;
	//}

	if (skipIdx == 0) {
		reloadImage();
		return;
	}

	// update dir
	loadDir(dir);

	QSharedPointer<DkImageContainerT> imgC = getSkippedImage(skipIdx);

	load(imgC, silent);
}

///**
// * Loads the ancesting or subsequent thumbnail file.
// * @param skipIdx the number of files that should be skipped after/before the current file.
// * @param silent if true, no status information will be displayed.
// **/ 
//QImage DkImageLoader::changeFileFast(int skipIdx, QFileInfo& fileInfo, bool silent) {
//
//	fileInfo = getSkippedImage(skipIdx);
//
//	//if (loadFile.exists())
//	// no threading here
//	return loadThumb(fileInfo, silent);
//}

///**
// * Returns the file info of the ancesting/subsequent file + skipIdx.
// * @param skipIdx the number of files to be skipped from the current file.
// * @param silent if true, no status information will be displayed.
// * @return QFileInfo the file info of the demanded file
// **/ 
//QFileInfo DkImageLoader::getChangedFileInfo(int skipIdx, bool silent, bool searchFile) {
//
//	file.refresh();
//	virtualFile.refresh();
//	bool virtualExists = files.contains(virtualFile.fileName()); // old code here is a bug if the image is e.g. renamed
//
//	qDebug() << "virtual file: " << virtualFile.absoluteFilePath();
//	qDebug() << "file: " << file.absoluteFilePath();
//	qDebug() << "files: " << files;
//
//	if (!virtualExists && !file.exists())
//		return QFileInfo();
//
//	DkTimer dt;
//
//	// load a page (e.g. within a tiff file)
//	if (basicLoader.setPageIdx(skipIdx))
//		return basicLoader.getFile();
//
//	//if (folderUpdated) {
//	//	bool loaded = loadDir((virtualExists) ? virtualFile.absoluteDir() : file.absoluteDir(), false);
//	//	if (!loaded)
//	//		return QFileInfo();
//	//}
//	
//	if (searchFile && !file.absoluteFilePath().isEmpty()) {
//		QDir newDir = (virtualExists && virtualFile.absoluteDir() != dir) ? virtualFile.absoluteDir() : file.absoluteDir();
//		qDebug() << "loading new dir: " << newDir.absolutePath();
//		qDebug() << "old dir: " << dir.absolutePath();
//		bool loaded = loadDir(newDir, false);
//		if (!loaded)
//			return QFileInfo();
//	}
//
//	// locate the current file
//	QString cFilename = (virtualExists) ? virtualFile.fileName() : file.fileName();
//	int newFileIdx = 0;
//	
//	if (searchFile) cFileIdx = 0;
//
//	//qDebug() << "virtual file " << virtualFile.absoluteFilePath();
//	//qDebug() << "file" << file.absoluteFilePath();
//
//	if (virtualExists || file.exists()) {
//
//		if (searchFile) {
//			for ( ; cFileIdx < files.size(); cFileIdx++) {
//
//				if (files[cFileIdx] == cFilename)
//					break;
//			}
//		}
//		newFileIdx = cFileIdx + skipIdx;
//
//		// could not locate the file -> it was deleted?!
//		if (searchFile && cFileIdx == files.size()) {
//			
//			// see if the file was deleted
//			QStringList filesTmp = files;
//			filesTmp.append(cFilename);
//			filesTmp = sort(filesTmp, dir);
//
//			cFileIdx = 0;
//			
//			for ( ; cFileIdx < filesTmp.size(); cFileIdx++) {
//
//				if (filesTmp[cFileIdx] == cFilename)
//					break;
//			}
//
//			if (filesTmp.size() != cFileIdx) {
//				newFileIdx = cFileIdx + skipIdx;
//				if (skipIdx > 0) newFileIdx--;	// -1 because the current file does not exist
//			}
//		}		
//
//		//qDebug() << "subfolders: " << DkSettings::global.scanSubFolders << "subfolder size: " << (subFolders.size() > 1);
//
//		if (DkSettings::global.scanSubFolders && subFolders.size() > 1 && (newFileIdx < 0 || newFileIdx >= files.size())) {
//
//			int folderIdx = 0;
//
//			// locate folder
//			for (int idx = 0; idx < subFolders.size(); idx++) {
//				if (subFolders[idx] == dir.absolutePath()) {
//					folderIdx = idx;
//					break;
//				}
//			}
//
//			if (newFileIdx < 0)
//				folderIdx = getPrevFolderIdx(folderIdx);
//			else
//				folderIdx = getNextFolderIdx(folderIdx);
//
//			qDebug() << "new folder idx: " << folderIdx;
//			
//			//if (DkSettings::global.loop)
//			//	folderIdx %= subFolders.size();
//
//			if (folderIdx >= 0 && folderIdx < subFolders.size()) {
//				
//				int oldFileSize = files.size();
//				loadDir(QDir(subFolders[folderIdx]), false);	// don't scan recursive again
//				qDebug() << "loading new folder: " << subFolders[folderIdx];
//
//				if (newFileIdx >= oldFileSize) {
//					newFileIdx -= oldFileSize;
//					cFileIdx = 0;
//					qDebug() << "new skip idx: " << newFileIdx << "cFileIdx: " << cFileIdx << " -----------------------------";
//					getChangedFileInfo(newFileIdx, silent, false);
//				}
//				else if (newFileIdx < 0) {
//					newFileIdx += cFileIdx;
//					cFileIdx = files.size()-1;
//					qDebug() << "new skip idx: " << newFileIdx << "cFileIdx: " << cFileIdx << " -----------------------------";
//					getChangedFileInfo(newFileIdx, silent, false);
//				}
//			}
//			//// dir up
//			//else if (folderIdx == subFolders.size()) {
//
//			//	qDebug() << "going one up";
//			//	dir.cd("..");
//			//	loadDir(dir, false);	// don't scan recursive again
//			//	newFileIdx += cFileIdx;
//			//	cFileIdx = 0;
//			//	getChangedFileInfo(newFileIdx, silent, false);
//			//}
//			//// get root files
//			//else if (folderIdx < 0) {
//			//	loadDir(dir, false);
//			//}
//
//		}
//
//		// this should never happen!
//		if (files.empty()) {
//			qDebug() << "file list is empty, where it should not be";
//			return QFileInfo();
//		}
//
//		// loop the directory
//		if (DkSettings::global.loop) {
//			newFileIdx %= files.size();
//
//			while (newFileIdx < 0)
//				newFileIdx = files.size() + newFileIdx;
//
//		}
//		// clip to pos1 if skipIdx < -1
//		else if (cFileIdx > 0 && newFileIdx < 0) {
//			newFileIdx = 0;
//		}
//		// clip to end if skipIdx > 1
//		else if (cFileIdx < files.size()-1 && newFileIdx >= files.size()) {
//			newFileIdx = files.size()-1;
//		}
//		// tell user that there is nothing left to display
//		else if (newFileIdx < 0) {
//			QString msg = tr("You have reached the beginning");
//			if (!silent)
//				updateInfoSignal(msg, 1000);
//			return QFileInfo();
//		}
//		// tell user that there is nothing left to display
//		else if (newFileIdx >= files.size()) {
//			QString msg = tr("You have reached the end");
//			
//			qDebug() << " you have reached the end ............";
//
//			if (!DkSettings::global.loop)
//				emit(setPlayer(false));
//
//			if (!silent)
//				updateInfoSignal(msg, 1000);
//			return QFileInfo();
//		}
//	}
//
//	//qDebug() << "file idx changed in: " << QString::fromStdString(dt.getTotal());
//
//	cFileIdx = newFileIdx;
//
//	// file requested becomes current file
//	return (files.isEmpty()) ? QFileInfo() : QFileInfo(dir, files[newFileIdx]);
//	
//}

/**
 * Returns the file info of the ancesting/subsequent file + skipIdx.
 * @param skipIdx the number of files to be skipped from the current file.
 * @param silent if true, no status information will be displayed.
 * @return QFileInfo the file info of the demanded file
 **/ 
QSharedPointer<DkImageContainerT> DkImageLoader::getSkippedImage(int skipIdx, bool silent, bool searchFile) {

	QSharedPointer<DkImageContainerT> imgC;

	if (!currentImage)
		return imgC;

	DkTimer dt;

	// load a page (e.g. within a tiff file)
	if (currentImage->setPageIdx(skipIdx))
		return currentImage;

	//if (searchFile && currentImage->file().absoluteDir() != dir.absolutePath()) {
	//	qDebug() << "loading new dir: " << currentImage->file().absolutePath();
	//	qDebug() << "old dir: " << dir.absolutePath();
	bool loaded = loadDir(currentImage->file(), false);
	//}

	// locate the current file
	int newFileIdx = 0;
	
	if (searchFile) tmpFileIdx = 0;

	//qDebug() << "virtual file " << virtualFile.absoluteFilePath();
	//qDebug() << "file" << file.absoluteFilePath();

	//if (virtualExists || file.exists()) {

	if (searchFile) {

		QFileInfo file = (currentImage->exists()) ? currentImage->file() : DkSettings::global.recentFiles.first();

		qDebug() << "current image: " << currentImage->file().absoluteFilePath() << " last image: " << DkSettings::global.recentFiles.first();

		tmpFileIdx = findFileIdx(file, images);
		if (tmpFileIdx == -1)
			tmpFileIdx = 0;
	}
	newFileIdx = tmpFileIdx + skipIdx;

	// could not locate the file -> it was deleted?!
	if (searchFile && tmpFileIdx == images.size()) {
			
		tmpFileIdx = 0;
		for (; tmpFileIdx < images.size(); tmpFileIdx++) {

			if (images[tmpFileIdx].data() <= currentImage.data())
				break;
		}

		if (images.size() != tmpFileIdx) {
			newFileIdx = tmpFileIdx + skipIdx;
			if (skipIdx > 0) newFileIdx--;	// -1 because the current file does not exist
		}
	}		

	//qDebug() << "subfolders: " << DkSettings::global.scanSubFolders << "subfolder size: " << (subFolders.size() > 1);

	if (DkSettings::global.scanSubFolders && subFolders.size() > 1 && (newFileIdx < 0 || newFileIdx >= images.size())) {

		int folderIdx = 0;

		// locate folder
		for (int idx = 0; idx < subFolders.size(); idx++) {
			if (subFolders[idx] == dir.absolutePath()) {
				folderIdx = idx;
				break;
			}
		}

		if (newFileIdx < 0)
			folderIdx = getPrevFolderIdx(folderIdx);
		else
			folderIdx = getNextFolderIdx(folderIdx);

		qDebug() << "new folder idx: " << folderIdx;
			
		//if (DkSettings::global.loop)
		//	folderIdx %= subFolders.size();

		if (folderIdx >= 0 && folderIdx < subFolders.size()) {
				
			int oldFileSize = images.size();
			loadDir(QDir(subFolders[folderIdx]), false);	// don't scan recursive again
			qDebug() << "loading new folder: " << subFolders[folderIdx];

			if (newFileIdx >= oldFileSize) {
				newFileIdx -= oldFileSize;
				tmpFileIdx = 0;
				qDebug() << "new skip idx: " << newFileIdx << "cFileIdx: " << tmpFileIdx << " -----------------------------";
				getSkippedImage(newFileIdx, silent, false);
			}
			else if (newFileIdx < 0) {
				newFileIdx += tmpFileIdx;
				tmpFileIdx = images.size()-1;
				qDebug() << "new skip idx: " << newFileIdx << "cFileIdx: " << tmpFileIdx << " -----------------------------";
				getSkippedImage(newFileIdx, silent, false);
			}
		}
		//// dir up
		//else if (folderIdx == subFolders.size()) {

		//	qDebug() << "going one up";
		//	dir.cd("..");
		//	loadDir(dir, false);	// don't scan recursive again
		//	newFileIdx += cFileIdx;
		//	cFileIdx = 0;
		//	getChangedFileInfo(newFileIdx, silent, false);
		//}
		//// get root files
		//else if (folderIdx < 0) {
		//	loadDir(dir, false);
		//}

	}

	// this should never happen!
	if (images.empty()) {
		qDebug() << "file list is empty, where it should not be";
		return imgC;
	}

	// loop the directory
	if (DkSettings::global.loop) {
		newFileIdx %= images.size();

		while (newFileIdx < 0)
			newFileIdx = images.size() + newFileIdx;

	}
	// clip to pos1 if skipIdx < -1
	else if (tmpFileIdx > 0 && newFileIdx < 0) {
		newFileIdx = 0;
	}
	// clip to end if skipIdx > 1
	else if (tmpFileIdx < images.size()-1 && newFileIdx >= images.size()) {
		newFileIdx = images.size()-1;
	}
	// tell user that there is nothing left to display
	else if (newFileIdx < 0) {
		QString msg = tr("You have reached the beginning");
		if (!silent)
			showInfoSignal(msg, 1000);
		return imgC;
	}
	// tell user that there is nothing left to display
	else if (newFileIdx >= images.size()) {
		QString msg = tr("You have reached the end");
			
		qDebug() << " you have reached the end ............";

		if (!DkSettings::global.loop)
			emit(setPlayer(false));

		if (!silent)
			showInfoSignal(msg, 1000);
		return imgC;
	}
	//}

	//qDebug() << "file idx changed in: " << QString::fromStdString(dt.getTotal());

	tmpFileIdx = newFileIdx;

	if (newFileIdx >= 0 && newFileIdx < images.size())
		imgC = images.at(newFileIdx);


	// file requested becomes current file
	return imgC;
	
}


/**
* Loads the file at index idx.
* @param idx the file index of the file which should be loaded.
**/ 
void DkImageLoader::loadFileAt(int idx) {

	//if (basicLoader.hasImage() && !file.exists())
	//	return;

	if (currentImage && !dir.exists())
		loadDir(currentImage->file());

	if (dir.exists()) {

		if (idx == -1) {
			idx = images.size()-1;
		}
		else if (DkSettings::global.loop) {
			idx %= images.size();

			while (idx < 0)
				idx = images.size() + idx;

		}
		else if (idx < 0 && !DkSettings::global.loop) {
			QString msg = tr("You have reached the beginning");
			emit showInfoSignal(msg, 1000);
			return;
		}
		else if (idx >= images.size()) {
			QString msg = tr("You have reached the end");
			if (!DkSettings::global.loop)
				emit(setPlayer(false));
			emit showInfoSignal(msg, 1000);
			return;
		}

	}

	// file requested becomes current file
	setCurrentImage(images.at(idx));

	load(currentImage);

}

QSharedPointer<DkImageContainerT> DkImageLoader::findOrCreateFile(const QFileInfo& file) const {

	QSharedPointer<DkImageContainerT> imgC = findFile(file);

	if (!imgC)
		imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(file));

	return imgC;
}

QSharedPointer<DkImageContainerT> DkImageLoader::findFile(const QFileInfo& file) const {

	for (int idx = 0; idx < images.size(); idx++) {

		if (images[idx]->file() == file)
			return images[idx];
	}

	return QSharedPointer<DkImageContainerT>();
}

int DkImageLoader::findFileIdx(const QFileInfo& file, const QVector<QSharedPointer<DkImageContainerT> >& images) const {

	for (int idx = 0; idx < images.size(); idx++) {

		if (images[idx]->file().absoluteFilePath() == file.absoluteFilePath())
			return idx;
	}

	return -1;
}


//
///**
// * Returns all files of the current directory.
// * @return QStringList empty list if no directory is set.
// **/ 
//QStringList DkImageLoader::getFiles() {
//
//	// guarantee that the file list is up-to-date
//	loadDir(dir);
//	
//	return files;
//}

QStringList DkImageLoader::getFileNames() {

	QStringList fileNames;

	for (int idx = 0; idx < images.size(); idx++)
		fileNames.append(images[idx]->file().fileName());

	return fileNames;
}

QVector<QSharedPointer<DkImageContainerT> > DkImageLoader::getImages() {

	loadDir(dir);
	return images;
}

/**
 * Loads the first file of the current directory.
 **/ 
void DkImageLoader::firstFile() {

	loadFileAt(0);
}

/**
 * Loads the last file of the current directory.
 **/ 
void DkImageLoader::lastFile() {
	
	loadFileAt(-1);
}

void DkImageLoader::unloadFile() {

	if (!currentImage)
		return;

	//if (currentImage->isEdited())
		// ask user for saving

	// TODO: add save metadata (rating etc...)

}

void DkImageLoader::setCurrentImage(QSharedPointer<DkImageContainerT> newImg) {

	loadDir(newImg->file());
	
	if (newImg && currentImage && newImg->file().absoluteFilePath() == currentImage->file().absoluteFilePath()) {
		qDebug() << "new image " << newImg->file().absoluteFilePath() << " is current image: " << currentImage->file().absoluteFilePath();
		return;
	}

	if (currentImage) {
		currentImage->cancel();

		if (currentImage->imgLoaded() == DkImageContainer::loading_canceled)// {
			emit showInfoSignal(newImg->file().fileName(), 3000, 1);
			//QCoreApplication::sendPostedEvents();
		//}

		currentImage->saveMetaDataThreaded();
		if (!DkSettings::resources.cacheMemory) 
			currentImage->clear();

		// disconnect old image
		disconnect(currentImage.data(), SIGNAL(errorDialogSignal(const QString&)), this, SLOT(errorDialogSignal(const QString&)));
		disconnect(currentImage.data(), SIGNAL(fileLoadedSignal(bool)), this, SLOT(imageLoaded(bool)));
		disconnect(currentImage.data(), SIGNAL(showInfoSignal(QString, int, int)), this, SIGNAL(showInfoSignal(QString, int, int)));
		disconnect(currentImage.data(), SIGNAL(fileSavedSignal(QFileInfo)), this, SLOT(imageSaved(QFileInfo)));
	}

	currentImage = newImg;

	connect(currentImage.data(), SIGNAL(errorDialogSignal(const QString&)), this, SIGNAL(errorDialogSignal(const QString&)));
	connect(currentImage.data(), SIGNAL(fileLoadedSignal(bool)), this, SLOT(imageLoaded(bool)));
	connect(currentImage.data(), SIGNAL(showInfoSignal(QString, int, int)), this, SIGNAL(showInfoSignal(QString, int, int)));
	connect(currentImage.data(), SIGNAL(fileSavedSignal(QFileInfo, bool)), this, SLOT(imageSaved(QFileInfo, bool)));
}

//QImage DkImageLoader::loadThumb(QFileInfo& file, bool silent) {
//		
//	DkTimer dt;
//
//	if (cacher)
//		cacher->pause();	// loadFile re-starts the cacher again
//
//	virtualFile = file;
//
//	// see if we can read the thumbnail from the exif data
//	DkMetaData dataExif(file);
//	QImage thumb = dataExif.getThumbnail();
//	int orientation = dataExif.getOrientation();
//
//	qDebug() << "thumb size: " << thumb.size();
//
//	//// as found at: http://olliwang.com/2010/01/30/creating-thumbnail-images-in-qt/
//	//QString filePath = (file.isSymLink()) ? file.symLinkTarget() : file.absoluteFilePath();
//			
//	if (orientation != -1 && !dataExif.isTiff()) {
//		QTransform rotationMatrix;
//		rotationMatrix.rotate((double)orientation);
//		thumb = thumb.transformed(rotationMatrix);
//	}
//
//	if (!thumb.isNull()) {
//		file = virtualFile;
//		qDebug() << "[thumb] " << file.fileName() << " loaded in: " << QString::fromStdString(dt.getTotal());
//
//		if (file.exists())
//			emit updateFileSignal(file, thumb.size());
//
//	}
//
//	return thumb;
//}

///**
// * Loads the file specified in a thread.
// * @param file the file to be loaded.
// * @param silent if true, no status will be displayed.
// **/ 
//void DkImageLoader::load(QFileInfo file, bool silent, int cacheState) {
//
//	// if the locker is in load file we get dead locks if loading is not threaded
//	// is it save to lock the mutex before setting up the thread??
//	/*QMutexLocker locker(&mutex);*/
//	
//	// TODO: use QtConcurrent here...
//	QMetaObject::invokeMethod(this, "loadFile", Qt::QueuedConnection, Q_ARG(QFileInfo, file), Q_ARG(bool, silent), Q_ARG(int, cacheState));
//}

void DkImageLoader::reloadImage() {

	if(!currentImage)
		return;

	if (!currentImage->exists()) {
		// message when reloaded
		QString msg = tr("sorry, %1 does not exist anymore...").arg(currentImage->file().fileName());
		emit showInfoSignal(msg, 4000);
		return;
	}

	dir = QDir();
	images.clear();
	currentImage->clear();
	setCurrentImage(currentImage);
	load(currentImage);
}

void DkImageLoader::load(const QFileInfo& file, bool silent /* = false */) {

	loadDir(file);


	if (file.isFile()) {
		QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(file);
		setCurrentImage(newImg);
		load(currentImage);
	}
	else 
		firstFile();
	
}

void DkImageLoader::load(QSharedPointer<DkImageContainerT> image /* = QSharedPointer<DkImageContainerT> */, bool silent) {

	if (!image)
		return;

	setCurrentImage(image);

	emit updateSpinnerSignalDelayed(true);
	bool loaded = currentImage->loadImageThreaded();	// loads file threaded
	
	updateCacher(image);

	if (!loaded)
		emit updateSpinnerSignalDelayed(false);
	
	// if loaded is false, we definitively know that the file does not exist -> early exception here?

}

void DkImageLoader::imageLoaded(bool loaded /* = false */) {

	// TODO: cacher routines
	emit updateSpinnerSignalDelayed(false);
	emit imageLoadedSignal(currentImage, loaded);

	if (loaded)
		emit imageUpdatedSignal(currentImage);

	updateHistory();
}


///**
// * Loads the file specified (not threaded!)
// * @param file the file to be ljoaded.
// * @return bool true if the file could be loaded.
// **/ 
//bool DkImageLoader::loadFile(QFileInfo file, bool silent, int cacheState) {
//	
//	DkTimer dtt;
//
//	// null file?
//	if (file.fileName().isEmpty()) {
//		this->file = lastFileLoaded;
//		return false;
//	}
//	else if (!file.exists()) {
//		
//		if (!silent) {
//			QString msg = tr("Sorry, the file: %1 does not exist... ").arg(file.fileName());
//			updateInfoSignal(msg);
//		}
//		
//		fileNotLoadedSignal(file);
//		this->file = lastFileLoaded;	// revert to last file
//
//		int fPos = files.indexOf(file.fileName());
//		if (fPos >= 0)
//			files.removeAt(fPos);
//
//		return false;
//	}
//	else if (!file.permission(QFile::ReadUser)) {
//		
//		if (!silent) {
//			QString msg = tr("Sorry, you are not allowed to read: %1").arg(file.fileName());
//			updateInfoSignal(msg);
//		}
//		
//		fileNotLoadedSignal(file);
//		this->file = lastFileLoaded;	// revert to last file
//
//		return false;
//	}
//
//	if (cacher)
//		cacher->pause();
//
//	DkTimer dt;
//
//	//test exif
//	//DkExif dataExif = DkExif(file);
//	//int orientation = dataExif.getOrientation();
//	//dataExif.getExifKeys();
//	//dataExif.getExifValues();
//	//dataExif.getIptcKeys();
//	//dataExif.getIptcValues();
//	//dataExif.saveOrientation(90);
//
//	if (!silent)
//		emit updateSpinnerSignalDelayed(true);
//
//	qDebug() << "loading: " << file.absoluteFilePath();
//
//	bool imgLoaded = false;
//	bool imgRotated = false;
//	
//	DkTimer dtc;
//
//	// critical section -> threads
//	if (cacher && cacheState != cache_force_load && !basicLoader.isDirty()) {
//		
//		QVector<DkImageCache> cache = cacher->getCache();
//		//QMutableVectorIterator<DkImageCache> cIter(cacher->getCache());
//
//		for (int idx = 0; idx < cache.size(); idx++) {
//			
//			//cIter.next();
//
//			if (cache.at(idx).getFile() == file) {
//
//				if (cache.at(idx).getCacheState() == DkImageCache::cache_loaded) {
//					DkImageCache cCache = cache.at(idx);
//					QImage tmp = cCache.getImage();
//					basicLoader.setImage(tmp, file);
//					imgLoaded = basicLoader.hasImage();
//
//					// nothing todo here
//					if (!imgLoaded)
//						break;
//
//					imgRotated = cCache.isRotated();
//				}
//				break; 
//			}
//			//cIter.next();
//		}
//	}
//
//	qDebug() << "loading from cache takes: " << QString::fromStdString(dtc.getTotal());
//	
//	if (!imgLoaded) {
//		try {
//			imgLoaded = basicLoader.loadGeneral(file);
//		} catch(...) {
//			imgLoaded = false;
//		}
//	}
//
//	this->virtualFile = file;
//
//	if (!silent)
//		emit updateSpinnerSignalDelayed(false);	// stop showing
//
//	qDebug() << "image loaded in: " << QString::fromStdString(dt.getTotal());
//	
//	if (imgLoaded) {
//		
//		DkMetaData imgMetaData(file);		
//		int orientation = imgMetaData.getOrientation();
//
//		//QStringList keys = imgMetaData.getExifKeys();
//		//qDebug() << keys;
//
//		if (!imgMetaData.isTiff() && !imgRotated && !DkSettings::metaData.ignoreExifOrientation)
//			basicLoader.rotate(orientation);
//		
//		if (cacher && cacheState != cache_disable_update) 
//			cacher->setCurrentFile(file, basicLoader.image());
//
//		qDebug() << "orientation set in: " << QString::fromStdString(dt.getIvl());
//			
//		// update watcher
//		emit updateFileWatcherSignal(file);		
//		this->file = file;
//		lastFileLoaded = file;
//		editFile = QFileInfo();
//		loadDir(file.absoluteDir(), false);
//		
//		emit updateImageSignal();
//		emit updateDirSignal(file);	// this should call updateFileSignal too
//		sendFileSignal();
//
//		// update history
//		updateHistory();
//	}
//	else {
//		//if (!silent) {
//			QString msg = tr("Sorry, I could not load: %1").arg(file.fileName());
//			updateInfoSignal(msg);
//			this->file = lastFileLoaded;	// revert to last file
//			qDebug() << "reverting to: " << lastFileLoaded.fileName();
//			loadDir(this->file.absoluteDir(), false);
//		//}
//		fileNotLoadedSignal(file);
//		
//		startStopCacher();
//		if (cacher) {
//			cacher->start();
//			cacher->play();
//		}
//
//		qDebug() << "I did load it silent: " << silent;
//		return false;
//	}
//
//	startStopCacher();
//	if (cacher) {
//		cacher->start();
//		cacher->play();
//	}
//
//	qDebug() << "total loading time: " << QString::fromStdString(dtt.getTotal());
//
//	return true;
//}


/**
 * Saves the file specified in a thread.
 * If the file already exists, it will be replaced.
 * @param file the (new) filename.
 * @param fileFilter the file extenstion (e.g. *.jpg)
 * @param saveImg the image to be saved
 * @param compression the compression method (for *.jpg or *.tif images)
 **/ 
void DkImageLoader::saveFile(QFileInfo file, QImage saveImg, QString fileFilter, int compression) {

	saveFileIntern(file, saveImg, fileFilter, compression);
}

///**
// * Saves a file in a thread with no status information.
// * @param file the file name/path
// * @param img the image to be saved
// **/ 
//void DkImageLoader::saveFileSilentThreaded(QFileInfo file, QImage img) {
//
//	QMetaObject::invokeMethod(this, "saveFileSilentIntern", Qt::QueuedConnection, Q_ARG(QFileInfo, file), Q_ARG(QImage, img));
//}

/**
 * Saves a temporary file to the folder specified in Settings.
 * @param img the image (which was in most cases pasted to nomacs)
 **/ 
QFileInfo DkImageLoader::saveTempFile(QImage img, QString name, QString fileExt, bool force, bool threaded) {

	QFileInfo tmpPath = QFileInfo(DkSettings::global.tmpPath + "\\");
	
	if (!force && (!DkSettings::global.useTmpPath || !tmpPath.exists())) {
		qDebug() << tmpPath.absolutePath() << "does not exist";
		return QFileInfo();
	}
	else if ((!DkSettings::global.useTmpPath || !tmpPath.exists())) {

#ifdef WIN32
		
		// TODO: this path seems to be perfectly ok (you can copy it to windows explorer) - however Qt thinks it does not exist??
		QString defaultPath = getenv("HOMEPATH");
		defaultPath = "C:" + defaultPath + "\\My Pictures\\";
		tmpPath = defaultPath;

		qDebug() << "default path: " << tmpPath.absoluteFilePath();
#endif

		if (!tmpPath.isDir()) {
			// load system default open dialog
			QString dirName = QFileDialog::getExistingDirectory(DkNoMacs::getDialogParent(), tr("Save Directory"),
				getDir().absolutePath());

			tmpPath = dirName + "/";

			if (!tmpPath.exists())
				return QFileInfo();
		}
	}

	qDebug() << "tmpPath: " << tmpPath.absolutePath();
	
	//// TODO: call save file silent threaded...
	//for (int idx = 1; idx < 10000; idx++) {
	
		QString fileName = name + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh.mm.ss") + fileExt;

		//if (idx < 10)
		//	fileName += "000";
		//else if (idx < 100)
		//	fileName += "00";
		//else if (idx < 1000)
		//	fileName += "0";
		//
		//fileName += QString::number(idx) + fileExt;

		QFileInfo tmpFile = QFileInfo(tmpPath.absolutePath(), fileName);

		if (!tmpFile.exists()) {
			
			saveFileIntern(tmpFile, img);

			//if (threaded)
			//	saveFileSilentThreaded(tmpFile, img);
			//else
				//saveFileSilentIntern(tmpFile, img);	// TODO change to new format
			
			qDebug() << tmpFile.absoluteFilePath() << "saved...";

			return tmpFile;
		}
	//}

	return QFileInfo();
}

/**
 * Saves a file (not threaded!)
 * If the file already exists, it will be replaced.
 * @param file the file name/path
 * @param fileFilter the file extension (e.g. *.jpg)
 * @param saveImg the image to be saved
 * @param compression the compression method (for jpg, tif)
 **/ 
void DkImageLoader::saveFileIntern(QFileInfo file, QImage saveImg, QString fileFilter, int compression) {
	
	QSharedPointer<DkImageContainerT> imgC = (currentImage) ? currentImage : findOrCreateFile(file);
	setCurrentImage(imgC);

	if (saveImg.isNull() && (!currentImage || !currentImage->hasImage()))
		emit errorDialogSignal(tr("Sorry, I cannot save an empty image..."));	// info here?

	QString filePath = file.absoluteFilePath();

	// if the user did not specify the suffix - append the suffix of the file filter
	QString newSuffix = file.suffix();
	if (newSuffix == "") {
		
		newSuffix = fileFilter.remove(0, fileFilter.indexOf("."));
		printf("new suffix: %s\n", newSuffix.toStdString().c_str());

		int endSuffix = -1;
		if (newSuffix.indexOf(")") == -1)
			endSuffix =  newSuffix.indexOf(" ");
		else if (newSuffix.indexOf(" ") == -1)
			endSuffix =  newSuffix.indexOf(")");
		else
			endSuffix = qMin(newSuffix.indexOf(")"), newSuffix.indexOf(" "));

		filePath.append(newSuffix.left(endSuffix));
	}

	emit updateSpinnerSignalDelayed(true);
	QImage sImg = (saveImg.isNull()) ? imgC->image() : saveImg;

	qDebug() << "saving: " << file.absoluteFilePath();

	bool saveStarted = imgC->saveImageThreaded(file, sImg, compression);

	if (!saveStarted)
		imageSaved(QFileInfo(), false);

}

void DkImageLoader::imageSaved(QFileInfo file, bool saved) {

	emit updateSpinnerSignalDelayed(false);

	if (!file.exists() || !file.isFile() || !saved)
		return;

	emit imageLoadedSignal(currentImage, true);
	emit imageUpdatedSignal(currentImage);
	qDebug() << "image updated: " << currentImage->file().fileName();
	
	// TODO: load it again here?


	//QSharedPointer<DkImageContainerT> sImg(new DkImageContainerT(file));
	//setCurrentImage(sImg);

	//emit updateImageSignal();
	//emit sendFileSignal();
}

///**
// * Saves the file (not threaded!).
// * No status information will be displayed if this function is called.
// * @param file the file name/path.
// * @param saveImg the image to be saved.
// **/ 
//void DkImageLoader::saveFileSilentIntern(QFileInfo file, QImage saveImg) {
//
//	QMutexLocker locker(&mutex);
//	
//	this->file.refresh();
//
//	// update watcher
//	if (this->file.exists() && watcher)
//		watcher->removePath(this->file.absoluteFilePath());
//	
//	emit updateInfoSignalDelayed(tr("saving..."), true);
//	QString filePath = file.absoluteFilePath();
//	QImage sImg = (saveImg.isNull()) ? basicLoader.image() : saveImg; 
//	bool saved = basicLoader.save(filePath, sImg);
//	emit updateInfoSignalDelayed(tr("saving..."), false);	// stop the label
//	
//	if (saved)
//		emit updateFileWatcherSignal(file);
//	else 
//		emit updateFileWatcherSignal(this->file);
//
//	if (!saveImg.isNull() && saved) {
//		
//		if (this->file.exists()) {
//			try {
//				// TODO: remove watcher path?!
//				imgMetaData.saveThumbnail(DkThumbsLoader::createThumb(sImg), QFileInfo(filePath));
//				imgMetaData.saveMetaDataToFile(QFileInfo(filePath));
//			} catch (DkException e) {
//
//				qDebug() << "can't write metadata...";
//			} catch (...) {
//				
//				if (!restoreFile(QFileInfo(filePath)))
//					emit newErrorDialog("sorry, I destroyed: " + QFileInfo(filePath).fileName() + "\n remove the numbers after the file extension in order to restore the file...");
//			}
//		}
//
//		// reload my dir (if it was changed...)
//		this->file = QFileInfo(filePath);
//		this->editFile = QFileInfo();
//
//		if (saved)
//			load(filePath, true);
//
//		//this->virtualFile = this->file;
//		//basicLoader.setImage(saveImg, this->file);
//		//loadDir(this->file.absoluteDir());
//
//		//if (cacher) cacher->setCurrentFile(file, basicLoader.image());
//		//sendFileSignal();
//	}
//}

///**
// * Saves the rating to the metadata.
// * This function does nothing if an image format
// * is loaded that does not support metadata.
// * @param rating the rating.
// **/ 
//void DkImageLoader::saveRating(int rating) {
//
//
//
//	file.refresh();
//
//	// file might be edited
//	if (!file.exists())
//		return;
//
//	QMutexLocker locker(&mutex);
//	// update watcher
//	if (this->file.exists() && watcher)
//		watcher->removePath(this->file.absoluteFilePath());
//
//	try {
//		
//		imgMetaData.saveRating(rating);
//	}catch(...) {
//		
//		if (!restoreFile(this->file))
//			emit updateInfoSignal(tr("Sorry, I could not restore: %1").arg(file.fileName()));
//	}
//	emit updateFileWatcher(this->file);
//
//}

//void DkImageLoader::enableWatcher(bool enable) {
//	
//	watcherEnabled = enable;
//}

/**
 * Updates the file history.
 * The file history stores the last 10 folders.
 **/ 
void DkImageLoader::updateHistory() {

	// TODO: 
	QFileInfo file = currentImage->file();
	if (!currentImage || currentImage->hasImage() != DkImageContainer::loaded)
		return;

	// TODO: update the file history here or put file history to the settings (better option)

	DkSettings::global.lastDir = file.absolutePath();

	DkSettings::global.recentFiles.removeAll(file.absoluteFilePath());
	DkSettings::global.recentFolders.removeAll(file.absolutePath());

	DkSettings::global.recentFiles.push_front(file.absoluteFilePath());
	DkSettings::global.recentFolders.push_front(file.absolutePath());

	DkSettings::global.recentFiles.removeDuplicates();
	DkSettings::global.recentFolders.removeDuplicates();

	for (int idx = 0; idx < DkSettings::global.recentFiles.size()-DkSettings::global.numFiles-10; idx++)
		DkSettings::global.recentFiles.pop_back();

	for (int idx = 0; idx < DkSettings::global.recentFolders.size()-DkSettings::global.numFiles-10; idx++)
		DkSettings::global.recentFolders.pop_back();


	DkSettings s = DkSettings();
	s.save();
}

// image manipulation --------------------------------------------------------------------
/**
 * Deletes the currently loaded file.
 **/ 
void DkImageLoader::deleteFile() {
	
	if (currentImage && currentImage->exists()) {

		QFile fileHandle(currentImage->file().absoluteFilePath());
		if (fileHandle.remove())
			emit showInfoSignal(tr("%1 deleted...").arg(currentImage->file().fileName()));
		else
			emit showInfoSignal(tr("Sorry, I could not delete: %1").arg(currentImage->file().fileName()));

		QSharedPointer<DkImageContainerT> imgC = getSkippedImage(1);
		load(imgC);
	}
}

/**
 * Rotates the image.
 * First, we try to set the rotation flag in the metadata
 * (this is the fastest way to rotate an image).
 * If this does not work, the image matrix is rotated.
 * @param angle the rotation angle in degree.
 **/ 
void DkImageLoader::rotateImage(double angle) {

	qDebug() << "rotating image...";

	if (!currentImage || !currentImage->hasImage()) {
		qDebug() << "sorry, loader has no image";
		return;
	}

	currentImage->getLoader()->rotate(qRound(angle));	// TODO: care for saving there

	QSharedPointer<DkMetaDataT> metaData = currentImage->getMetaData();

	if (metaData->hasMetaData() && DkSettings::metaData.saveExifOrientation) {
		QImage thumb = DkImage::createThumb(currentImage->image());
		metaData->setOrientation(qRound(angle));
	}
	else
		setImage(currentImage->image(), currentImage->file());

	emit imageUpdatedSignal(currentImage);

	//	basicLoader.rotate(angle);
	//	emit updateImageSignal();

	//	if (file.exists() && DkSettings::metaData.saveExifOrientation) {
	//		
	//		imgMetaData.saveOrientation((int)angle);

	//		QImage thumb = DkThumbsLoader::createThumb(basicLoader.image());
	//		if (imgMetaData.isJpg()) {
	//			// undo exif orientation
	//			DkBasicLoader loader;
	//			loader.setImage(thumb, QFileInfo());
	//			loader.rotate(-imgMetaData.getOrientation());
	//			thumb = loader.image();
	//		}
	//		imgMetaData.saveThumbnail(thumb, file);
	//	}
	//	else if (file.exists() && !DkSettings::metaData.saveExifOrientation) {
	//		qDebug() << "file: " << file.fileName() << " exists...";
	//		imgMetaData.saveOrientation(0);		// either metadata throws or we force throwing
	//		throw DkException("User forces NO exif orientation", __LINE__, __FILE__);
	//	}
	//	

	//	mutex.unlock();

	//	sendFileSignal();
	//}
	//catch(DkException de) {

	//	mutex.unlock();

	//	// TODO: saveFileSilentThreaded is in the main thread (find out why)
	//	// TODO: in this case the image is reloaded (file watcher seems to be active)
	//	// make a silent save -> if the image is just cached, do not save it
	//	if (file.exists())
	//		saveFileSilentThreaded(file);
	//}
	//catch(...) {	// if file is locked... or permission is missing
	//	mutex.unlock();

	//	// try restoring the file
	//	if (!restoreFile(file))
	//		emit updateInfoSignal(tr("Sorry, I could not restore: %1").arg(file.fileName()));
	//}

	//if (cacher)
	//	cacher->setCurrentFile(file, basicLoader.image());

	//emit updateFileWatcherSignal(this->file);

}

/**
 * Restores files that were destroyed by the Exiv2 lib.
 * If a watch (or some other read lock) is on a file, the
 * Exiv2 lib is known do damage the files on Windows.
 * This function restores these files.
 * @param fileInfo the file to be restored.
 * @return bool true if the file could be restored.
 **/ 
bool DkImageLoader::restoreFile(const QFileInfo& fileInfo) {

	QStringList files = fileInfo.dir().entryList();
	QString fileName = fileInfo.fileName();
	QRegExp filePattern(fileName + "[0-9]+");
	QString backupFileName;

	// if exif crashed it saved a backup file with the format: filename.png1232
	for (int idx = 0; idx < files.size(); idx++) {

		if (filePattern.exactMatch(files[idx])) {
			backupFileName = files[idx];
			break;
		}
	}

	if (backupFileName.isEmpty()) {
		qDebug() << "I could not locate the backup file...";
		return true;
	}

	// delete the destroyed file
	QFile file(fileInfo.absoluteFilePath());
	QFile backupFile(fileInfo.absolutePath() + QDir::separator() + backupFileName);

	if (file.size() == 0 || file.size() <= backupFile.size()) {
		
		if (!file.remove()) {

			// ok I did not destroy the original file - so delete the back-up
			// -> this reverts the file - but otherwise we spam to the disk 
			// actions reverted here include meta data saving
			if (file.size() != 0)
				return backupFile.remove();

			qDebug() << "I could not remove the file...";
			return false;
		}
	}
	else {
		
		qDebug() << "non-empty file: " << fileName << " I won't delete it...";
		qDebug() << "file size: " << file.size() << " back-up file size: " << backupFile.size();
		return false;
	}

	// now 
	return backupFile.rename(fileInfo.absoluteFilePath());
}

void DkImageLoader::updateFileWatcher(QFileInfo filePath) {

	//if (watcher)
	//	delete watcher;

	//watcher = new QFileSystemWatcher(QStringList(filePath.absoluteFilePath()), this);
	//connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

	//qDebug() << "file watcher updated: " << filePath.absoluteFilePath();
}

void DkImageLoader::disableFileWatcher() {

	//if (watcher) {
	//	delete watcher;
	//	watcher = 0;
	//}

}

///**
// * Reloads the currently loaded file if it was edited by another software.
// * @param path the file path of the changed file.
// **/ 
//void DkImageLoader::fileChanged(const QString& path) {
//
//	qDebug() << "file updated: " << path;
//
//	// ignore if watcher was disabled
//	if (path == file.absoluteFilePath()) {
//		QMutexLocker locker(&mutex);
//		load(QFileInfo(path), true, cache_force_load);
//	}
//	else
//		qDebug() << "file watcher is not up-to-date...";
//}

/**
 * Reloads the file index if the directory was edited.
 * @param path the path to the current directory
 **/ 
void DkImageLoader::directoryChanged(const QString& path) {

	if (path.isEmpty() || QDir(path) == dir.absolutePath()) {

		folderUpdated = true;
		
		// guarantee, that only every XX seconds a folder update occurs
		// think of a folder where 100s of files are written to...
		// as this could be pretty fast, the thumbsloader (& whoever) would create a 
		// greater offset and slow down the system
		if ((path.isEmpty() && timerBlockedUpdate) || (!path.isEmpty() && !delayedUpdateTimer.isActive())) {

			loadDir(dir, false);
			timerBlockedUpdate = false;

			if (!path.isEmpty())
				delayedUpdateTimer.start(3000);
		}
		else
			timerBlockedUpdate = true;
	}
	
}

/**
 * Returns true if a file was specified.
 * @return bool true if a file name/path was specified
 **/ 
bool DkImageLoader::hasFile() {

	return currentImage && currentImage->exists();
}

bool DkImageLoader::hasMovie() {

	if (!currentImage || !currentImage->exists())
		return false;

	QString newSuffix = currentImage->file().suffix();
	return newSuffix.contains(QRegExp("(gif|mng)", Qt::CaseInsensitive));

}

/**
 * Returns the currently loaded file information.
 * @return QFileInfo the current file info
 **/ 
QFileInfo DkImageLoader::file() const {

	if (!currentImage)
		return QFileInfo();

	
	return currentImage->file();
}

QSharedPointer<DkImageContainerT> DkImageLoader::getCurrentImage() const {

	return currentImage;
}

/**
 * Returns the currently loaded directory.
 * @return QDir the currently loaded directory.
 **/ 
QDir DkImageLoader::getDir() const {

	return dir;
}

QStringList DkImageLoader::getFoldersRecursive(QDir dir) {

	//DkTimer dt;
	QStringList subFolders;
	//qDebug() << "scanning recursively: " << dir.absolutePath();

	if (DkSettings::global.scanSubFolders) {

		QDirIterator dirs(dir.absolutePath(), QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
	
		int nFolders = 0;
		while (dirs.hasNext()) {
			dirs.next();
			subFolders << dirs.filePath();
			nFolders++;

			if (nFolders > 100)
				break;
			
			//getFoldersRecursive(dirs.filePath(), subFolders);
			//qDebug() << "loop: " << dirs.filePath();
		}
	}	

	subFolders << dir.absolutePath();

	qSort(subFolders.begin(), subFolders.end(), DkUtils::compLogicQString);
	

	qDebug() << dir.absolutePath();
	
	//qDebug() << "scanning folders recursively took me: " << QString::fromStdString(dt.getTotal());
	return subFolders;
}

QFileInfoList DkImageLoader::updateSubFolders(QDir rootDir) {
	
	subFolders = getFoldersRecursive(rootDir);
	QFileInfoList files;
	qDebug() << subFolders;

	// find the first subfolder that has images
	for (int idx = 0; idx < subFolders.size(); idx++) {
		dir = subFolders[idx];
		files = getFilteredFileInfoList(dir, ignoreKeywords, keywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		if (!files.empty())
			break;
	}

	return files;
}

int DkImageLoader::getNextFolderIdx(int folderIdx) {
	
	int nextIdx = -1;

	if (subFolders.empty())
		return nextIdx;

	// find the first sub folder that has images
	for (int idx = 1; idx < subFolders.size(); idx++) {
		
		int tmpNextIdx = folderIdx + idx;

		if (DkSettings::global.loop)
			tmpNextIdx %= subFolders.size();
		else if (tmpNextIdx >= subFolders.size())
			return -1;

		QDir cDir = subFolders[tmpNextIdx];
		QStringList cFiles = getFilteredFileList(cDir, ignoreKeywords, keywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		if (!cFiles.empty()) {
			nextIdx = tmpNextIdx;
			break;
		}
	}

	return nextIdx;
}

int DkImageLoader::getPrevFolderIdx(int folderIdx) {
	
	int prevIdx = -1;

	if (subFolders.empty())
		return prevIdx;

	// find the first sub folder that has images
	for (int idx = 1; idx < subFolders.size(); idx++) {

		int tmpPrevIdx = folderIdx - idx;

		if (DkSettings::global.loop && tmpPrevIdx < 0)
			tmpPrevIdx += subFolders.size();
		else if (tmpPrevIdx < 0)
			return -1;

		QDir cDir = subFolders[tmpPrevIdx];
		QStringList cFiles = getFilteredFileList(cDir, ignoreKeywords, keywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		if (!cFiles.empty()) {
			prevIdx = tmpPrevIdx;
			break;
		}
	}

	return prevIdx;
}

/**
 * Returns the file list of the directory dir.
 * Note: this function might get slow if lots of files (> 10000) are in the
 * directory or if the directory is in the net.
 * Currently the file list is sorted according to the system specification.
 * @param dir the directory to load the file list from.
 * @param ignoreKeywords if one of these keywords is in the file name, the file will be ignored.
 * @param keywords if one of these keywords is not in the file name, the file will be ignored.
 * @return QStringList all filtered files of the current directory.
 **/ 
QStringList DkImageLoader::getFilteredFileList(QDir dir, QStringList ignoreKeywords, QStringList keywords, QStringList folderKeywords) {

	DkTimer dt;

#ifdef WIN32

	QString winPath = QDir::toNativeSeparators(dir.path()) + "\\*.*";

	wchar_t* fnameT = L"C:\\VSProjects\\img\\*.*";
	const wchar_t* fname = reinterpret_cast<const wchar_t *>(winPath.utf16());

	WIN32_FIND_DATAW findFileData;
	HANDLE MyHandle = FindFirstFileW(fname, &findFileData);

	std::vector<std::wstring> fileNameList;
	std::wstring fileName;

	if( MyHandle != INVALID_HANDLE_VALUE) {
		
		do {

			fileName = findFileData.cFileName;
			fileNameList.push_back(fileName);	// TODO: sort correct according to numbers
		} while(FindNextFileW(MyHandle, &findFileData) != 0);
	}

	FindClose(MyHandle);
	
	// slow regexp
	//QString extPattern = ".+((\\" + fileFilters.join("$)|(\\") + "$))";
	//extPattern.replace("*", "");
	//QRegExp exp(extPattern, Qt::CaseInsensitive);

	// remove the * in fileFilters
	QStringList fileFiltersClean = fileFilters;
	for (int idx = 0; idx < fileFilters.size(); idx++)
		fileFiltersClean[idx].replace("*", "");

	//std::sort(fileNameList.begin(), fileNameList.end(), wCompLogic);

	QStringList fileList;
	std::vector<std::wstring>::iterator lIter = fileNameList.begin();

	// convert to QStringList
	for (unsigned int idx = 0; idx < fileNameList.size(); idx++, lIter++) {
		
		QString qFilename = DkUtils::stdWStringToQString(*lIter);

		// believe it or not, but this is 10 times faster than QRegExp
		// drawback: we also get files that contain *.jpg*
		for (int idx = 0; idx < fileFiltersClean.size(); idx++) {

			if (qFilename.contains(fileFiltersClean[idx], Qt::CaseInsensitive)) {
				fileList.append(qFilename);
				break;
			}
		}
	}

	qDebug() << "WinAPI, indexed (" << fileList.size() <<") files in: " << QString::fromStdString(dt.getTotal());
#else

	// true file list
	dir.setSorting(QDir::LocaleAware);
	QStringList fileList = dir.entryList(fileFilters);
	qDebug() << "Qt, sorted file list computed in: " << QString::fromStdString(dt.getIvl());
	qDebug() << fileList;

#endif

	for (int idx = 0; idx < ignoreKeywords.size(); idx++) {
		QRegExp exp = QRegExp("^((?!" + ignoreKeywords[idx] + ").)*$");
		exp.setCaseSensitivity(Qt::CaseInsensitive);
		fileList = fileList.filter(exp);
	}

	for (int idx = 0; idx < keywords.size(); idx++) {
		fileList = fileList.filter(keywords[idx], Qt::CaseInsensitive);
	}

	if (!folderKeywords.empty()) {
		
		QStringList resultList = fileList;
		for (int idx = 0; idx < folderKeywords.size(); idx++) {
			resultList = resultList.filter(folderKeywords[idx], Qt::CaseInsensitive);
		}

		// if string match returns nothing -> try a regexp
		if (resultList.empty())
			resultList = fileList.filter(QRegExp(folderKeywords.join(" ")));

		qDebug() << "filtered file list (get)" << resultList;
		qDebug() << "keywords: " << folderKeywords;
		fileList = resultList;
	}

	if (DkSettings::resources.filterDuplicats) {

		QString preferredExtension = DkSettings::resources.preferredExtension;
		preferredExtension = preferredExtension.replace("*.", "");
		qDebug() << "preferred extension: " << preferredExtension;

		QStringList resultList = fileList;
		fileList.clear();
		
		for (int idx = 0; idx < resultList.size(); idx++) {
			
			QFileInfo cFName = QFileInfo(resultList.at(idx));

			if (preferredExtension.compare(cFName.suffix(), Qt::CaseInsensitive) == 0) {
				fileList.append(resultList.at(idx));
				continue;
			}

			QString cFBase = cFName.baseName();
			bool remove = false;

			for (int cIdx = 0; cIdx < resultList.size(); cIdx++) {

				QString ccBase = QFileInfo(resultList.at(cIdx)).baseName();

				if (cIdx != idx && ccBase == cFBase && resultList.at(cIdx).contains(preferredExtension, Qt::CaseInsensitive)) {
					remove = true;
					break;
				}
			}
			
			if (!remove)
				fileList.append(resultList.at(idx));
		}
	}

	//fileList = sort(fileList, dir);

	return fileList;
}

void DkImageLoader::updateCacher(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC || !DkSettings::resources.cacheMemory)
		return;

	//// no caching? delete all
	//if (!DkSettings::resources.cacheMemory) {
	//	for (int idx = 0; idx < images.size(); idx++) {
	//		images.at(idx)->clear();
	//	}
	//	return;
	//}

	int cIdx = findFileIdx(imgC->file(), images);
	float mem = 0;

	if (cIdx == -1) {
		qDebug() << "WARNING: image not found for caching!";
		return;
	}

	for (int idx = 0; idx < images.size(); idx++) {

		if (idx >= cIdx-1 && idx <= idx+DkSettings::resources.maxImagesCached)
			mem += images.at(idx)->getMemoryUsage();
		else {
			images.at(idx)->clear();
			continue;
		}

		// ignore the last and current one
		if (idx == cIdx-1 || idx == cIdx) {
			continue;
		}
		// fully load the next image
		else if (idx == cIdx+1 && mem < DkSettings::resources.cacheMemory) {
			images.at(idx)->loadImageThreaded();
		}
		else if (idx > cIdx && idx < cIdx+DkSettings::resources.maxImagesCached-2 && mem < DkSettings::resources.cacheMemory) {
			images.at(idx)->fetchFile();
		}
	}

}

/**
 * Returns the file list of the directory dir.
 * Note: this function might get slow if lots of files (> 10000) are in the
 * directory or if the directory is in the net.
 * Currently the file list is sorted according to the system specification.
 * @param dir the directory to load the file list from.
 * @param ignoreKeywords if one of these keywords is in the file name, the file will be ignored.
 * @param keywords if one of these keywords is not in the file name, the file will be ignored.
 * @return QStringList all filtered files of the current directory.
 **/ 
QFileInfoList DkImageLoader::getFilteredFileInfoList(const QDir& dir, QStringList ignoreKeywords, QStringList keywords, QStringList folderKeywords) {

	DkTimer dt;

#ifdef WIN32

	QString winPath = QDir::toNativeSeparators(dir.path()) + "\\*.*";

	wchar_t* fnameT = L"C:\\VSProjects\\img\\*.*";
	const wchar_t* fname = reinterpret_cast<const wchar_t *>(winPath.utf16());

	WIN32_FIND_DATAW findFileData;
	HANDLE MyHandle = FindFirstFileW(fname, &findFileData);

	std::vector<std::wstring> fileNameList;
	std::wstring fileName;

	if( MyHandle != INVALID_HANDLE_VALUE) {
		
		do {

			fileName = findFileData.cFileName;
			fileNameList.push_back(fileName);	// TODO: sort correct according to numbers
		} while(FindNextFileW(MyHandle, &findFileData) != 0);
	}

	FindClose(MyHandle);
	
	// slow regexp
	//QString extPattern = ".+((\\" + fileFilters.join("$)|(\\") + "$))";
	//extPattern.replace("*", "");
	//QRegExp exp(extPattern, Qt::CaseInsensitive);

	// remove the * in fileFilters
	QStringList fileFiltersClean = fileFilters;
	for (int idx = 0; idx < fileFilters.size(); idx++)
		fileFiltersClean[idx].replace("*", "");

	//std::sort(fileNameList.begin(), fileNameList.end(), wCompLogic);

	QStringList fileList;
	std::vector<std::wstring>::iterator lIter = fileNameList.begin();

	// convert to QStringList
	for (unsigned int idx = 0; idx < fileNameList.size(); idx++, lIter++) {
		
		QString qFilename = DkUtils::stdWStringToQString(*lIter);

		// believe it or not, but this is 10 times faster than QRegExp
		// drawback: we also get files that contain *.jpg*
		for (int idx = 0; idx < fileFiltersClean.size(); idx++) {

			if (qFilename.contains(fileFiltersClean[idx], Qt::CaseInsensitive)) {
				fileList.append(qFilename);
				break;
			}
		}
	}

	qDebug() << "WinAPI, indexed (" << fileList.size() <<") files in: " << QString::fromStdString(dt.getTotal());
#else

	// true file list
	dir.setSorting(QDir::LocaleAware);
	QStringList fileList = dir.entryList(fileFilters);
	qDebug() << "Qt, sorted file list computed in: " << QString::fromStdString(dt.getIvl());
	qDebug() << fileList;

#endif

	for (int idx = 0; idx < ignoreKeywords.size(); idx++) {
		QRegExp exp = QRegExp("^((?!" + ignoreKeywords[idx] + ").)*$");
		exp.setCaseSensitivity(Qt::CaseInsensitive);
		fileList = fileList.filter(exp);
	}

	for (int idx = 0; idx < keywords.size(); idx++) {
		fileList = fileList.filter(keywords[idx], Qt::CaseInsensitive);
	}

	if (!folderKeywords.empty()) {
		
		QStringList resultList = fileList;
		for (int idx = 0; idx < folderKeywords.size(); idx++) {
			resultList = resultList.filter(folderKeywords[idx], Qt::CaseInsensitive);
		}

		// if string match returns nothing -> try a regexp
		if (resultList.empty())
			resultList = fileList.filter(QRegExp(folderKeywords.join(" ")));

		qDebug() << "filtered file list (get)" << resultList;
		qDebug() << "keywords: " << folderKeywords;
		fileList = resultList;
	}

	if (DkSettings::resources.filterDuplicats) {

		QString preferredExtension = DkSettings::resources.preferredExtension;
		preferredExtension = preferredExtension.replace("*.", "");
		qDebug() << "preferred extension: " << preferredExtension;

		QStringList resultList = fileList;
		fileList.clear();
		
		for (int idx = 0; idx < resultList.size(); idx++) {
			
			QFileInfo cFName = QFileInfo(resultList.at(idx));

			if (preferredExtension.compare(cFName.suffix(), Qt::CaseInsensitive) == 0) {
				fileList.append(resultList.at(idx));
				continue;
			}

			QString cFBase = cFName.baseName();
			bool remove = false;

			for (int cIdx = 0; cIdx < resultList.size(); cIdx++) {

				QString ccBase = QFileInfo(resultList.at(cIdx)).baseName();

				if (cIdx != idx && ccBase == cFBase && resultList.at(cIdx).contains(preferredExtension, Qt::CaseInsensitive)) {
					remove = true;
					break;
				}
			}
			
			if (!remove)
				fileList.append(resultList.at(idx));
		}
	}

	//fileList = sort(fileList, dir);

	QFileInfoList fileInfoList;
	
	for (int idx = 0; idx < fileList.size(); idx++)
		fileInfoList.append(QFileInfo(dir, fileList.at(idx)));

	return fileInfoList;
}


//QStringList DkImageLoader::sort(const QStringList& files, const QDir& dir) {
//
//	QFileInfoList fList;
//
//	for (int idx = 0; idx < files.size(); idx++)
//		fList.append(QFileInfo(dir, files.at(idx)));
//
//	switch(DkSettings::global.sortMode) {
//
//	case DkSettings::sort_filename:
//		
//		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
//			qSort(fList.begin(), fList.end(), compFilename);
//		else
//			qSort(fList.begin(), fList.end(), compFilenameInv);
//		break;
//
//	case DkSettings::sort_date_created:
//		if (DkSettings::global.sortDir == DkSettings::sort_ascending) {
//			qSort(fList.begin(), fList.end(), compDateCreated);
//			qSort(fList.begin(), fList.end(), compDateCreated);		// sort twice -> in order to guarantee that same entries are sorted correctly (thumbsloader)
//		}
//		else { 
//			qSort(fList.begin(), fList.end(), compDateCreatedInv);
//			qSort(fList.begin(), fList.end(), compDateCreatedInv);
//		}
//		break;
//
//	case DkSettings::sort_date_modified:
//		if (DkSettings::global.sortDir == DkSettings::sort_ascending) {
//			qSort(fList.begin(), fList.end(), compDateModified);
//			qSort(fList.begin(), fList.end(), compDateModified);
//		}
//		else {
//			qSort(fList.begin(), fList.end(), compDateModifiedInv);
//			qSort(fList.begin(), fList.end(), compDateModifiedInv);
//		}
//		break;
//	case DkSettings::sort_random:
//			qSort(fList.begin(), fList.end(), compRandom);
//		break;
//
//	default:
//		// filename
//		qSort(fList.begin(), fList.end(), compFilename);
//
//	}
//
//	QStringList sFiles;
//	for (int idx = 0; idx < fList.size(); idx++)
//		sFiles.append(fList.at(idx).fileName());
//
//	return sFiles;
//}

void DkImageLoader::sort() {
	
	qSort(images.begin(), images.end());
	emit updateDirSignal(images);
}


/**
 * Returns the directory where files are saved to.
 * @return QDir the directory where the user saved the last file to.
 **/ 
QDir DkImageLoader::getSaveDir() {

	qDebug() << "save dir: " << dir;

	if (!saveDir.exists())
		return dir;
	else
		return saveDir;
}

/**
 * Returns the file extension of the current file.
 * @return QString current file extension.
 **/ 
QString DkImageLoader::getCurrentFilter() {

	if (!currentImage || !currentImage->exists())
		return QString();

	QString cSuffix = currentImage->file().suffix();

	for (int idx = 0; idx < saveFilters.size(); idx++) {

		if (saveFilters[idx].contains(cSuffix))
			return saveFilters[idx];
	}

	return QString();
}

/**
	* Returns if an image is loaded currently.
	* @return bool true if an image is loaded.
	**/ 
bool DkImageLoader::hasImage() {
		
	return currentImage && currentImage->hasImage();
};

bool DkImageLoader::isEdited() const {
	return currentImage && currentImage->isEdited();
};

int DkImageLoader::numFiles() const {
	return images.size();
};

/**
	* Returns the currently loaded image.
	* @return QImage the current image
	**/ 
QImage DkImageLoader::getImage() {
		
	if (!currentImage)
		return QImage();

	return currentImage->image();
};

bool DkImageLoader::dirtyTiff() {

	if (!currentImage)
		return false;

	return currentImage->getLoader()->isDirty();
};


/**
 * Returns if a file is supported by nomacs or not.
 * Note: this function only checks for a valid extension.
 * @param fileInfo the file info of the file to be validated.
 * @return bool true if the file format is supported.
 **/ 
bool DkImageLoader::isValid(const QFileInfo& fileInfo) {

	printf("accepting file...\n");

	QFileInfo fInfo = fileInfo;
	if (fInfo.isSymLink())
		fInfo = fileInfo.symLinkTarget();

	if (!fInfo.exists())
		return false;

	QString fileName = fInfo.fileName();
	qDebug() << "filename: " << fileName;
	
	for (int idx = 0; idx < fileFilters.size(); idx++) {

		QRegExp exp = QRegExp(fileFilters.at(idx), Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);
		if (exp.exactMatch(fileName))
			return true;
	}

	printf("I did not accept... honestly...\n");

	return false;

}

///**
// * @deprecated
// * This 
// * @param fileInfo
// * @param dir
// * @return int
// **/ 
//int DkImageLoader::locateFile(QFileInfo& fileInfo, QDir* dir) {
//
//	if (!fileInfo.exists())
//		return -1;
//
//	bool newDir = (dir) ? false : true;
//
//	if (!dir) {
//		dir = new QDir(fileInfo.absoluteDir());
//		dir->setNameFilters(fileFilters);
//		dir->setSorting(QDir::LocaleAware);
//	}
//
//	// locate the current file
//	QStringList files = dir->entryList(fileFilters);
//	QString cFilename = fileInfo.fileName();
//
//	int fileIdx = 0;
//	for ( ; fileIdx < files.size(); fileIdx++) {
//
//		if (files[fileIdx] == cFilename)
//			break;
//	}
//
//	if (fileIdx == files.size()) fileIdx = -1;
//
//	if (newDir)
//		delete dir;
//
//	return fileIdx;
//}

void DkImageLoader::loadLastDir() {

	if (DkSettings::global.recentFolders.empty())
		return;

	QDir lastDir = DkSettings::global.recentFolders[0];
	setDir(lastDir);
}

void DkImageLoader::setFolderFilters(QStringList filters) {

	folderKeywords = filters;
	folderUpdated = true;
	loadDir(dir);	// simulate a folder update operation

	if (!filters.empty() && !images.contains(currentImage))
		loadFileAt(0);

	emit folderFiltersChanged(folderKeywords);
}

QStringList DkImageLoader::getFolderFilters() {
	return folderKeywords;
}

/**
 * Sets the file specified and loads the directory.
 * @param file the file to be set as current file.
 **/ 
void DkImageLoader::setFile(QFileInfo& file) {
	
	setCurrentImage(findFile(file));
}

/**
 * Sets the current directory to dir.
 * @param dir the directory to be loaded.
 **/ 
void DkImageLoader::setDir(QDir& dir) {

	//QDir oldDir = file.absoluteDir();
	
	bool valid = loadDir(dir);

	if (valid)
		firstFile();
}

/**
 * Sets a new save directory.
 * @param dir the new save directory.
 **/ 
void DkImageLoader::setSaveDir(QDir& dir) {
	this->saveDir = dir;
}

/**
 * Sets the current image to img.
 * @param img the loader's new image.
 **/ 
QSharedPointer<DkImageContainerT> DkImageLoader::setImage(QImage img, QFileInfo editFile) {
	
	qDebug() << "edited file: " << editFile.absoluteFilePath();

	QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(editFile);
	newImg->setImage(img, editFile);
	
	setCurrentImage(newImg);
	emit imageUpdatedSignal(currentImage);

	return newImg;
}

/**
 * Returns the current file name.
 * @return QString the file name of the currently loaded file.
 **/ 
QString DkImageLoader::fileName() {
	
	if (!currentImage || !currentImage->exists())
		return QString();
	
	return currentImage->file().fileName();
}

// DkCacher --------------------------------------------------------------------
DkCacher::DkCacher(QDir dir, QStringList files) {

	this->dir = dir;
	this->files = files;
	
	isActive = true;
	somethingTodo = false;
	curFileIdx = -1;
	maxFileSize = 50;	// in MB
	curCache = 0;
	maxNumFiles = 100;

	newDir = true;
	updateFiles = false;

	index();
}

/**
 * Creates cache for a new dir.
 * NOTE: the thread needs to be stopped before calling this function!
 * @param dir the new directory
 * @param files	the sorted file list of this directory
 **/ 
void DkCacher::setNewDir(QDir& dir, QStringList& files) {
	
	//QMutexLocker locker(&mutex);
	this->dir = dir;
	this->files = files;
	
	newDir = true;
	//index();
}

void DkCacher::updateDir(QStringList& files) {

	this->files = files;	// this change is done from another thread!
	updateFiles = true;
	qDebug() << "cacher files num: " << files.size();
	// CRITICAL! he crashes here if you turn cacher on -> and open a folder where lot's of files are written automatically
	// we should redesign the cacher anyway


	//index();
}

void DkCacher::index() {

	//QWriteLocker locker(&lock);

	if (newDir) {
		DkTimer dt;

		curCache = 0;	// clear cache size
		cache.clear();

		for (int idx = 0; idx < files.size(); idx++) {
			QFileInfo cFile = QFileInfo(dir, files[idx]);
			cache.append(DkImageCache(cFile));
		}
		newDir = false;
		somethingTodo = true;

		curFileIdx = -1;

		qDebug() << "cache indexed " << files.size() << " files in: " << QString::fromStdString(dt.getTotal());
	}

	if (updateFiles) {

		QVector<DkImageCache> tmpCache;
		curCache = 0;	// clear cache size

		for (int idx = 0; idx < files.size(); idx++) {
			
			DkImageCache tmp(QFileInfo(dir, files[idx]));
			int cacheIdx = cache.indexOf(tmp);
			
			if (cacheIdx != -1) {
				tmpCache.append(cache[cacheIdx]);
				curCache += cache[cacheIdx].getCacheSize();
			}
			else
				tmpCache.append(tmp);
		}

		qDebug() << "cur cache after updating: " << DkUtils::readableByte(curCache);

		cache = tmpCache;

		//curFileIdx = -1;
		somethingTodo = true;
		updateFiles = false;
	}

}

void DkCacher::run() {

	while (true) {

		msleep(100);

		//mutex.lock();
		//lock.lockForRead();

		if (!isActive) {
			qDebug() << "cacher stopped...";
			//lock.unlock();
			break;
		}

		// re-index folder
		if (newDir || updateFiles) {
			//lock.unlock();
			index();
		}

		//lock.lockForRead();

		if (somethingTodo) {
			//lock.unlock();
			load();		// load locks the mutex on it's own
		}
		//else
			//lock.unlock();
	}

}

/**
* Stops the current loading process.
* This method allows for stopping the thread without killing it.
**/ 
void DkCacher::stop() {

	//QMutexLocker locker(&mutex);
	isActive = false;
	qDebug() << "stopping thread: " << this->thread()->currentThreadId();
}

void DkCacher::start() {
	
	isActive = true;
	QThread::start();
}

void DkCacher::pause() {
	
	somethingTodo = false;
	qDebug() << "[cache] pausing cacher...";
}

void DkCacher::play() {
	
	somethingTodo = true;
	qDebug() << "[cache] restarting cacher...";
}

void DkCacher::setCurrentFile(QFileInfo file, QImage img) {

	//QReadLocker locker(&lock);

	QMutableVectorIterator<DkImageCache> cacheIter(cache);

	for (int idx = 0; idx < cache.size(); idx++) {
		
		cacheIter.next();

		if (cache.at(idx).getFile() == file) {
			curFileIdx = idx;
			if (!img.isNull() && cache.at(idx).getCacheState() != DkImageCache::cache_loaded) {
				curCache -= cacheIter.value().getCacheSize();
				
				// 4* since we are dealing with uncompressed images
				if (DkImage::getBufferSizeFloat(img.size(), img.depth()) + curCache < DkSettings::resources.cacheMemory &&
					DkImage::getBufferSizeFloat(img.size(), img.depth()) < 4*maxFileSize) {
					cacheIter.value().setImage(img, true);	// if we get a new cache image here, we can safely assume, that it is already rotated
					curCache += cacheIter.value().getCacheSize();
					qDebug() << "current file set: " << QSize(img.size());
				}
				else
					cacheIter.value().clearImage();
			}
			else if (!img.isNull() && cache.at(idx).getCacheState() == DkImageCache::cache_loaded) {
				cacheIter.value().setImage(img, true);	// don't question the size - it might change due to editing and cause a complete re-caching
			}
			break;
		}

	}

	somethingTodo = true;
}

void DkCacher::load() {

	//QReadLocker locker(&lock);
	somethingTodo = false;

	// invalid file idx
	if (curFileIdx == -1)
		return;

	QMutableVectorIterator<DkImageCache> cacheIter(cache);

	// it's fair enough if we index about +/- 100 images
	for (int idx = 1; idx < maxNumFiles*0.5; idx++) {

		int nIdx = curFileIdx+idx;
		int pIdx = curFileIdx-idx;

		if (nIdx < (int)cache.size() && cache.at(nIdx).getCacheState() == DkImageCache::cache_not_loaded) {
						
			if (!clean(idx))
				break;	// we're done

			// if you know how to directly access the n-th element of a QMutableVectorIterator, please replace the next 3 lines
			cacheIter.toFront();
			for (int cIterIdx = 0; cIterIdx <= nIdx; cIterIdx++)
				cacheIter.next();
				
			if (cacheImage(cacheIter.value())) {	// that might take time
				somethingTodo = true;
				break;	// go to thread to see if some action is waiting
			}
		}
		if (pIdx > 0 && cache.at(pIdx).getCacheState() == DkImageCache::cache_not_loaded) {

			if (!clean(idx))
				break;	// we're done

			cacheIter.toFront();
			for (int cIterIdx = 0; cIterIdx <= pIdx; cIterIdx++)
				cacheIter.next();

			//!! this is important:
			// currently setting a new dir happens in the thread of DkImageLoader (?! - pretty sure)
			// however, this thread is not synced on the vector... so if we change the vector while caching an image
			// bad things happen...
			if (cacheImage(cacheIter.value())) {	// that might take time // TODO: that might crash
				somethingTodo = true;
				break;	// go to thread to see if some action is waiting
			}
		}
	}


}

bool DkCacher::clean(int curCacheIdx) {

	// nothing todo
	if (curCache < DkSettings::resources.cacheMemory)
		return true;
	
	QMutableVectorIterator<DkImageCache> cacheIter(cache);

	for (int idx = 0; idx < (int)cache.size(); idx++) {

		cacheIter.next();

		// skip the current cache region
		if (idx > curFileIdx-curCacheIdx && idx <= curFileIdx+curCacheIdx)
			continue;

		if (cacheIter.value().getCacheState() == DkImageCache::cache_loaded) {
			
			curCache -= cacheIter.value().getCacheSize();
			cacheIter.value().clearImage();	// clear cached image

			qDebug() << "[cache] I cleared: " << cacheIter.value().getFile().fileName() << " cache volume: " << curCache << " MB";
		}
	}

	qDebug() << "[cache] cache volume: " << curCache << " MB";

	// stop caching
	if (curCache >= DkSettings::resources.cacheMemory)
		return false;
	
	return true;
}

bool DkCacher::cacheImage(DkImageCache& cacheImg) {
	
	QFileInfo file = cacheImg.getFile();
	
	// resolve links
	if (file.isSymLink()) file = QFileInfo(file.symLinkTarget());
	QFile f(file.filePath());

	// jpg files must be smaller than 1/4 of the max file size (as they are way larger when loaded
	// ignore files < 100 KB || larger than maxFileSize
	if (f.size() < 100*1024 || f.size() > 1024*1024*maxFileSize || (f.size() > 1024*1024*maxFileSize*0.25f && file.suffix() == "jpg")) {
		qDebug() << "[cache] I ignored: " << cacheImg.getFile().fileName() << " file size: " << f.size()/(1024.0f*1024.0f) << " MB";
		cacheImg.ignore();
		return false;
	}

	QImage img;
	if (loader.loadGeneral(file)) {
		
		cacheImg.setImage(loader.image());
		curCache += cacheImg.getCacheSize();

		qDebug() << "[cache] I cached: " << cacheImg.getFile().fileName() << " cache volume: " << curCache << " MB/ " 
			<< DkSettings::resources.cacheMemory << " MB";
		return true;
	}
	else
		cacheImg.ignore();		// cannot cache image

	return false;
}

// DkColorLoader --------------------------------------------------------------------
DkColorLoader::DkColorLoader(QVector<QSharedPointer<DkImageContainerT> > images) {

	moveToThread(this);
	init();
}

void DkColorLoader::init() {

	isActive = true;
	maxThumbs = 800;
}

void DkColorLoader::run() {

	int updateIvl = 30;

	// max full HD
	for (int idx = 0; idx <= maxThumbs && idx < images.size(); idx++) {

		//mutex.lock();
		if (!isActive) {
			qDebug() << "color loader stopped...";
			//mutex.unlock();
			break;
		}

		loadColor(idx);

		if ((idx % updateIvl) == 0)
			emit updateSignal(cols, indexes);
	}

	emit updateSignal(cols, indexes);

}

void DkColorLoader::loadColor(int fileIdx) {
	
	if (images.size() > maxThumbs)
		fileIdx = qRound((float)fileIdx/maxThumbs*(images.size()-1));

	QSharedPointer<DkThumbNailT> thumb = images.at(fileIdx)->getThumb();

	QImage thumbImg;
	if (thumb->hasImage() != DkThumbNailT::loaded)
		thumb->compute(DkThumbNailT::force_exif_thumb);
	thumbImg = thumb->getImage();
		
	if (!thumb.isNull()) {
		cols.append(DkImage::getMeanColor(thumbImg));	// TODO: compute most significant color
		indexes.append(fileIdx);
	}

}

void DkColorLoader::stop() {

	isActive = false;
}

}
