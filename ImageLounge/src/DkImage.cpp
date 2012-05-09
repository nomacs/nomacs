/*******************************************************************************************************
 DkImage.cpp
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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

namespace nmc {

// well this is pretty shitty... but we need the filter without description too
QStringList DkImageLoader::fileFilters = QString("*.png *.jpg *.tif *.bmp *.ppm *.xbm *.xpm *.gif *.pbm *.pgm *.jpeg *.tiff *.ico *.nef *.crw *.cr2 *.arw *.roh *.jps *.pns *.mpo *.lnk").split(' ');

// formats we can save
QString DkImageLoader::saveFilter = QString("PNG (*.png);;JPEG (*.jpg *.jpeg);;") %
	QString("TIFF (*.tif *.tiff);;") %
	QString("Windows Bitmap (*.bmp);;") %
	QString("Portable Pixmap (*.ppm);;") %
	QString("X11 Bitmap (*.xbm);;") %
	QString("X11 Pixmap (*.xpm)");

// formats we can save
QStringList DkImageLoader::saveFilters = saveFilter.split(QString(";;"));

QString DkImageLoader::openFilter = QString("Image Files (*.jpg *.png *.tif *.bmp *.gif *.pbm *.pgm *.xbm *.xpm *.ppm *.jpeg *.tiff *.ico *.nef *.crw *.cr2 *.arw *.roh *.jps *.pns *.mpo *.lnk);;") %
	QString(saveFilter) %
	QString(";;Graphic Interchange Format (*.gif);;") %
	QString("Portable Bitmap (*.pbm);;") %
	QString("Portable Graymap (*.pgm);;") %
	QString("Icon Files (*.ico);;") %
	QString("Nikon Raw (*.nef);;") %
	QString("Canon Raw (*.crw *.cr2);;") %
	QString("Sony Raw (*.arw);;") %
	QString("JPEG Stereo (*.jps);;") %
	QString("PNG Stereo (*.pns);;") %
	QString("Multi Picture Object (*.mpo);;") %
	QString("Rohkost (*.roh);;");
	

// formats we can load
QStringList DkImageLoader::openFilters = openFilter.split(QString(";;"));

DkMetaData DkImageLoader::imgMetaData = DkMetaData();

/**
 * Default constructor.
 * Creates a DkImageLoader instance with a given file.
 * @param file the file to be loaded.
 **/ 
DkImageLoader::DkImageLoader(QFileInfo file) {

	qRegisterMetaType<QFileInfo>("QFileInfo");
	loaderThread = new QThread;
	loaderThread->start();
	moveToThread(loaderThread);

	//watcher = 0;
	// init the watcher
	watcher = new QFileSystemWatcher();
	connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));
	
	dirWatcher = new QFileSystemWatcher();
	connect(dirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));

	folderUpdated = false;
	silent = false;
	 
	this->file = file;
	this->virtualFile = file;

	saveDir = DkSettings::GlobalSettings::lastSaveDir;	// loading save dir is obsolete ?!

	if (file.exists())
		loadDir(file.absoluteDir());
	else
		dir = DkSettings::GlobalSettings::lastDir;

}

/**
 * Default destructor.
 **/ 
DkImageLoader::~DkImageLoader() {

	loaderThread->exit(0);
	loaderThread->wait();
	delete loaderThread;

	qDebug() << "dir open: " << dir.absolutePath();
	qDebug() << "filepath: " << saveDir.absolutePath();
}

/**
 * Clears the path.
 * Calling this method makes the loader forget
 * about the current directory. It also destroys
 * the currently loaded image.
 **/ 
void DkImageLoader::clearPath() {
	
	QMutexLocker locker(&mutex);
	img = QImage();
	lastFileLoaded = file;
	file = QFileInfo();
	imgMetaData.setFileName(file);	// unload exif too
	//dir = QDir();
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

/**
 * Loads a given directory and the first image in this directory.
 * @param newDir the directory to be loaded.
 **/ 
void DkImageLoader::loadDir(QDir newDir) {

	// folder changed signal was emitted
	if (folderUpdated && newDir.absolutePath() == dir.absolutePath()) {
		
		files = getFilteredFileList(dir, ignoreKeywords, keywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		
		//emit updateDirSignal(file, true);		// if the signal is set to true thumbs are updated if images are added to the folder (however this may be nesty)
		emit updateDirSignal(file);
		folderUpdated = false;
		qDebug() << "getting file list.....";
	}
	// new folder is loaded
	else if ((newDir.absolutePath() != dir.absolutePath() || files.empty()) && newDir.exists()) {

		// update save directory
		if (saveDir == dir) saveDir = dir;
		dir = newDir;
		dir.setNameFilters(fileFilters);
		dir.setSorting(QDir::LocaleAware);		// TODO: extend
		folderUpdated = false;
		
		files = getFilteredFileList(dir, ignoreKeywords, keywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		qDebug() << "getting file list.....";
	
		if (dirWatcher) {
			if (!dirWatcher->directories().isEmpty())
				dirWatcher->removePaths(dirWatcher->directories());
			dirWatcher->addPath(dir.absolutePath());
		}

		qDebug() << "dir watcher: " << dirWatcher->directories();
	}
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
void DkImageLoader::changeFile(int skipIdx, bool silent) {

	//if (!img.isNull() && !file.exists())
	//	return;
	if (!file.exists() && !virtualFile.exists()) {
		qDebug() << virtualFile.absoluteFilePath() << "does not exist...";
		return;
	}

	mutex.lock();
	QFileInfo loadFile = getChangedFileInfo(skipIdx);
	qDebug() << "loading: " << file.absoluteFilePath();
	mutex.unlock();

	if (loadFile.exists())
		load(loadFile, silent);
}

/**
 * Returns the file info of the ancesting/subsequent file + skipIdx.
 * @param skipIdx the number of files to be skipped from the current file.
 * @param silent if true, no status information will be displayed.
 * @return QFileInfo the file info of the demanded file
 **/ 
QFileInfo DkImageLoader::getChangedFileInfo(int skipIdx, bool silent) {

	DkTimer dt;
	QDir newDir = (virtualFile.exists()) ? virtualFile.absoluteDir() : file.absoluteDir();
	loadDir(newDir);

	// locate the current file
	QString cFilename = (virtualFile.exists()) ? virtualFile.fileName() : file.fileName();
	int cFileIdx = 0;
	int newFileIdx = 0;

	//qDebug() << "virtual file " << virtualFile.absoluteFilePath();
	//qDebug() << "file" << file.absoluteFilePath();

	if (file.exists() || virtualFile.exists()) {

		for ( ; cFileIdx < files.size(); cFileIdx++) {

			if (files[cFileIdx] == cFilename)
				break;
		}

		qDebug() << "my idx " << cFileIdx;
		newFileIdx = cFileIdx + skipIdx;

		if (DkSettings::GlobalSettings::loop) {
			newFileIdx %= files.size();

			while (newFileIdx < 0)
				newFileIdx = files.size() + newFileIdx;

		}
		else if (cFileIdx > 0 && newFileIdx < 0) {
			newFileIdx = 0;
		}
		else if (cFileIdx < files.size()-1 && newFileIdx >= files.size()) {
			newFileIdx = files.size()-1;
		}
		else if (newFileIdx < 0) {
			QString msg = tr("You have reached the beginning");
			if (!silent)
				updateInfoSignal(msg, 1000);
			return QFileInfo();
		}
		else if (newFileIdx >= files.size()) {
			QString msg = tr("You have reached the end");
			
			if (!silent)
				updateInfoSignal(msg, 1000);
			return QFileInfo();
		}
		qDebug() << "idx: " << newFileIdx;
	}

	qDebug() << "file idx changed in: " << QString::fromStdString(dt.getTotal());

	// file requested becomes current file
	return (files.isEmpty()) ? QFileInfo() : QFileInfo(dir, files[newFileIdx]);


}

/**
 * Returns all files of the current directory.
 * @return QStringList empty list if no directory is set.
 **/ 
QStringList DkImageLoader::getFiles() {

	// guarantee that the file list is up-to-date
	loadDir(dir);
	
	return files;
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

/**
 * Loads the file at index idx.
 * @param idx the file index of the file which should be loaded.
 **/ 
void DkImageLoader::loadFileAt(int idx) {

	if (!img.isNull() && !file.exists())
		return;

	mutex.lock();

	if (!dir.exists()) {
		QDir newDir = (virtualFile.exists()) ? virtualFile.absoluteDir() : file.absolutePath();	
		loadDir(newDir);
	}

	qDebug() << "virtual file: " << virtualFile.absoluteFilePath();
	qDebug() << "real file " << file.absoluteFilePath();

	if (dir.exists()) {

		if (idx == -1) {
			idx = files.size()-1;
		}
		else if (DkSettings::GlobalSettings::loop) {
			idx %= files.size();

			while (idx < 0)
				idx = files.size() + idx;

		}
		else if (idx < 0 && !DkSettings::GlobalSettings::loop) {
			QString msg = tr("You have reached the beginning");
			updateInfoSignal(msg, 1000);
			mutex.unlock();
			return;
		}
		else if (idx >= files.size()) {
			QString msg = tr("You have reached the end");
			updateInfoSignal(msg, 1000);
			mutex.unlock();
			return;
		}

	}

	// file requested becomes current file
	QFileInfo loadFile = QFileInfo(dir, files[idx]);
	qDebug() << "loading: " << loadFile.absoluteFilePath();

	mutex.unlock();
	load(loadFile);

}


/**
 * Loads the current file in a thread.
 **/ 
void DkImageLoader::load() {

	load(file);
}

/**
 * Loads the file specified in a thread.
 * @param file the file to be loaded.
 * @param silent if true, no status will be displayed.
 **/ 
void DkImageLoader::load(QFileInfo file, bool silent) {

	this->silent = silent;
	QMetaObject::invokeMethod(this, "loadFile", Qt::QueuedConnection, Q_ARG(QFileInfo, file));
}

/**
 * Loads the file specified (not threaded!)
 * @param file the file to be loaded.
 * @return bool true if the file could be loaded.
 **/ 
bool DkImageLoader::loadFile(QFileInfo file) {
	
	DkTimer dtt;
	qDebug() << "loading...";
	QMutexLocker locker(&mutex);

	if (!file.exists()) {
		
		if (!silent) {
			QString msg = tr("Sorry, the file: %1 does not exist... ").arg(file.fileName());
			updateInfoSignal(msg);
		}
		return false;
	}
	else if (!file.permission(QFile::ReadUser)) {
		
		if (!silent) {
			QString msg = tr("Sorry, you are not allowed to read: %1").arg(file.fileName());
			updateInfoSignal(msg);
		}
		return false;
	}

	DkTimer dt;

	//test exif
	//DkExif dataExif = DkExif(file);
	//int orientation = dataExif.getOrientation();
	//dataExif.getExifKeys();
	//dataExif.getExifValues();
	//dataExif.getIptcKeys();
	//dataExif.getIptcValues();
	//dataExif.saveOrientation(90);

	if (!silent)
		emit updateInfoSignalDelayed(tr("loading..."), true);

	qDebug() << "loading: " << file.absoluteFilePath();

	bool imgLoaded;
	try {
		if (!file.isSymLink())
			imgLoaded = loadGeneral(file);
		else 			
			imgLoaded = loadGeneral(file.symLinkTarget());
	
	} catch(...) {
		imgLoaded = false;
	}

	if (!silent)
		emit updateInfoSignalDelayed(tr("loading..."), false);	// stop showing

	qDebug() << "image loaded in: " << QString::fromStdString(dt.getTotal());
	this->virtualFile = file;

	if (imgLoaded) {
				
		DkMetaData imgMetaData(file);		
		int orientation = imgMetaData.getOrientation();

		if (orientation != -1 && !imgMetaData.isTiff() && orientation != 0) {
			QTransform rotationMatrix;
			rotationMatrix.rotate((double)orientation);
			img = img.transformed(rotationMatrix);
		}
		qDebug() << "exif loaded in: " << QString::fromStdString(dt.getIvl());

		// update watcher
		if (watcher) {
			
			if (!watcher->files().isEmpty())
				watcher->removePaths(watcher->files());	// remove all files previously watched
			watcher->addPath(file.absoluteFilePath());
		}
		
		qDebug() << "watcher files: " << watcher->files();
		
		emit updateImageSignal();
		emit updateFileSignal(file, img.size());
		emit updateDirSignal(file);	// this should be called updateFileSignal too

		this->file = file;
		loadDir(file.absoluteDir());

		// update history
		updateHistory();

	}
	else {
		if (!silent) {
			QString msg = tr("Sorry, I could not load: %1").arg(file.fileName());
			updateInfoSignal(msg);
			this->file = lastFileLoaded;	// revert to last file
			loadDir(this->file.absoluteDir());
		}
		fileNotLoadedSignal(file);

		qDebug() << "I did load it silent: " << silent;
		return false;
	}

	qDebug() << "total loading time: " << QString::fromStdString(dtt.getTotal());

	return true;
}


/**
 * Loads special RAW files that are generated by the Hamamatsu camera.
 * @param fileName the filename of the file to be loaded.
 * @return bool true if the file could be loaded.
 **/ 
bool DkImageLoader::loadRohFile(QString fileName){

	bool imgLoaded = true;

	void *pData;
	FILE * pFile;
	unsigned char test[2];
	unsigned char *buf;
	int rohW = 4000;
	int rohH = 2672;
	pData = malloc(sizeof(unsigned char) * (rohW*rohH*2));
	buf = (unsigned char *)malloc(sizeof(unsigned char) * (rohW*rohH));

	try{
		pFile = fopen (fileName.toStdString().c_str(), "rb" );

		fread(pData, 2, rohW*rohH, pFile);

		fclose(pFile);

		for (long i=0; i < (rohW*rohH); i++){
		
			test[0] = ((unsigned char*)pData)[i*2];
			test[1] = ((unsigned char*)pData)[i*2+1];
			test[0] = test[0] >> 4;
			test[0] = test[0] & 15;
			test[1] = test[1] << 4;
			test[1] = test[1] & 240;

			buf[i] = (test[0] | test[1]);
		
		}

		img = QImage((const uchar*) buf, rohW, rohH, QImage::Format_Indexed8);

		if (img.isNull())
			throw DkFileException("sorry, the roh file is empty...", __LINE__, __FILE__);

		//img = img.copy();
		QVector<QRgb> colorTable;

		for (int i = 0; i < 256; i++)
			colorTable.push_back(QColor(i, i, i).rgb());
		img.setColorTable(colorTable);

	} catch(...) {
		imgLoaded = false;
	}
	
	free(buf);
	free(pData);


	return imgLoaded;

}

bool DkImageLoader::loadGeneral(QFileInfo file) {

	bool imgLoaded = false;

	QString newSuffix = file.suffix();

	
	if (newSuffix.contains(QRegExp("(roh)", Qt::CaseInsensitive))) {

		imgLoaded = loadRohFile(file.absoluteFilePath());

	} else if (!newSuffix.contains(QRegExp("(nef|crw|cr2|arw)", Qt::CaseInsensitive))) {

		// if image has Indexed8 + alpha channel -> we crash... sorry for that
		imgLoaded = img.load(file.absoluteFilePath());
	
	} else {

		// load raw files
		imgLoaded = loadRawFile(file);
	}
	return imgLoaded;
}


/**
 * Loads the RAW file specified.
 * Note: nomacs needs to be compiled with OpenCV and LibRaw in
 * order to enable RAW file loading.
 * @param file the file to be loaded.
 * @return bool true if the file could be loaded.
 **/ 
bool DkImageLoader::loadRawFile(QFileInfo file) {

	bool imgLoaded = false;

	try {

#ifdef WITH_OPENCV

		LibRaw iProcessor;
		QImage image;
		int orientation = 0;

		iProcessor.open_file(file.absoluteFilePath().toStdString().c_str());

		//// (-w) Use camera white balance, if possible (otherwise, fallback to auto_wb)
		//iProcessor.imgdata.params.use_camera_wb = 1;
		//// (-a) Use automatic white balance obtained after averaging over the entire image
		//iProcessor.imgdata.params.use_auto_wb = 1;
		//// (-q 3) Adaptive homogeneity-directed de-mosaicing algorithm (AHD)
		//iProcessor.imgdata.params.user_qual = 3;
		//iProcessor.imgdata.params.output_tiff = 1;
		////iProcessor.imgdata.params.four_color_rgb = 1;
		////iProcessor.imgdata.params.output_color = 1; //sRGB  (0...raw)
		//// RAW data filtration mode during data unpacking and post-processing
		//iProcessor.imgdata.params.filtering_mode = LIBRAW_FILTERING_AUTOMATIC;

		iProcessor.unpack();
		//iProcessor.dcraw_process();
		//iProcessor.dcraw_ppm_tiff_writer("test.tiff");

		unsigned short cols = iProcessor.imgdata.sizes.width,//.raw_width,
			rows = iProcessor.imgdata.sizes.height;//.raw_height;

		Mat rawMat, rgbImg;

		if (iProcessor.imgdata.idata.filters == 0)
		{
			//iProcessor.dcraw_process();
			rawMat = Mat(rows, cols, CV_32FC3);
			rawMat.setTo(0);
			std::vector<Mat> rawCh;
			split(rawMat, rawCh);

			for (unsigned int row = 0; row < rows; row++)
			{
				float *ptrR = rawCh[0].ptr<float>(row);
				float *ptrG = rawCh[1].ptr<float>(row);
				float *ptrB = rawCh[2].ptr<float>(row);
				//float *ptrE = rawCh[3].ptr<float>(row);

				for (unsigned int col = 0; col < cols; col++)
				{
					ptrR[col] = (float)iProcessor.imgdata.image[cols*row + col][0]/(float)iProcessor.imgdata.color.maximum;
					ptrG[col] = (float)iProcessor.imgdata.image[cols*row + col][1]/(float)iProcessor.imgdata.color.maximum;
					ptrB[col] = (float)iProcessor.imgdata.image[cols*row + col][2]/(float)iProcessor.imgdata.color.maximum;
					//ptrE[colIdx] = (float)iProcessor.imgdata.image[iProcessor.imgdata.sizes.width*row + col][3]/(float)iProcessor.imgdata.color.maximum;
				}
			}
			merge(rawCh, rawMat);
			rawMat.convertTo(rgbImg, CV_8U, 255);

			image = QImage(rgbImg.data, rgbImg.cols, rgbImg.rows, rgbImg.step, QImage::Format_RGB888);

		}
		else
		{

			qDebug() << "----------------";
			qDebug() << "Bayer Pattern: " << QString::fromStdString(iProcessor.imgdata.idata.cdesc);
			qDebug() << "Camera manufacturer: " << QString::fromStdString(iProcessor.imgdata.idata.make);
			qDebug() << "Camera model: " << QString::fromStdString(iProcessor.imgdata.idata.model);
			qDebug() << "canon_ev " << (float)iProcessor.imgdata.color.canon_ev;

			//qDebug() << "white: [%.3f %.3f %.3f %.3f]\n", iProcessor.imgdata.color.cam_mul[0],
			//	iProcessor.imgdata.color.cam_mul[1], iProcessor.imgdata.color.cam_mul[2],
			//	iProcessor.imgdata.color.cam_mul[3]);
			//qDebug() << "black: %i\n", iProcessor.imgdata.color.black);
			//qDebug() << "maximum: %.i %i\n", iProcessor.imgdata.color.maximum,
			//	iProcessor.imgdata.params.adjust_maximum_thr);
			//qDebug() << "gamma: %.3f %.3f %.3f %.3f %.3f %.3f\n",
			//	iProcessor.imgdata.params.gamm[0],
			//	iProcessor.imgdata.params.gamm[1],
			//	iProcessor.imgdata.params.gamm[2],
			//	iProcessor.imgdata.params.gamm[3],
			//	iProcessor.imgdata.params.gamm[4],
			//	iProcessor.imgdata.params.gamm[5]);

			qDebug() << "----------------";

			if (strcmp(iProcessor.imgdata.idata.cdesc, "RGBG")) throw DkException("Wrong Bayer Pattern (not RGBG)\n", __LINE__, __FILE__);

			rawMat = Mat(rows, cols, CV_32FC1);
			//rawMat.setTo(0);
			float mulWhite[4];
			//mulWhite[0] = iProcessor.imgdata.color.cam_mul[0] > 10 ? iProcessor.imgdata.color.cam_mul[0]/255.0f : iProcessor.imgdata.color.cam_mul[0];
			//mulWhite[1] = iProcessor.imgdata.color.cam_mul[1] > 10 ? iProcessor.imgdata.color.cam_mul[1]/255.0f : iProcessor.imgdata.color.cam_mul[1];
			//mulWhite[2] = iProcessor.imgdata.color.cam_mul[2] > 10 ? iProcessor.imgdata.color.cam_mul[2]/255.0f : iProcessor.imgdata.color.cam_mul[2];
			//mulWhite[3] = iProcessor.imgdata.color.cam_mul[3] > 10 ? iProcessor.imgdata.color.cam_mul[3]/255.0f : iProcessor.imgdata.color.cam_mul[3];
			//if (mulWhite[3] == 0)
			//	mulWhite[3] = mulWhite[1];

			mulWhite[0] = iProcessor.imgdata.color.cam_mul[0];
			mulWhite[1] = iProcessor.imgdata.color.cam_mul[1];
			mulWhite[2] = iProcessor.imgdata.color.cam_mul[2];
			mulWhite[3] = iProcessor.imgdata.color.cam_mul[3];

			float dynamicRange = iProcessor.imgdata.color.maximum-iProcessor.imgdata.color.black;

			float w = (mulWhite[0] + mulWhite[1] + mulWhite[2] + mulWhite[3])/4.0f;
			float maxW = 1.0f;//mulWhite[0];

			if (w > 2.0f)
				maxW = 256.0f;
			if (w > 2.0f && QString(iProcessor.imgdata.idata.make).compare("Canon", Qt::CaseInsensitive) == 0)
				maxW = 512.0f;	// some cameras would even need ~800 - why?

			//if (maxW < mulWhite[1])
			//	maxW = mulWhite[1];
			//if (maxW < mulWhite[2])
			//	maxW = mulWhite[2];
			//if (maxW < mulWhite[3])
			//	maxW = mulWhite[3];

			mulWhite[0] /= maxW;
			mulWhite[1] /= maxW;
			mulWhite[2] /= maxW;
			mulWhite[3] /= maxW;

			//if (iProcessor.imgdata.color.cmatrix[0][0] != 0) {
			//	mulWhite[0] = iProcessor.imgdata.color.cmatrix[0][0];
			//	mulWhite[1] = iProcessor.imgdata.color.cmatrix[0][1];
			//	mulWhite[2] = iProcessor.imgdata.color.cmatrix[0][2];
			//	mulWhite[3] = iProcessor.imgdata.color.cmatrix[0][3];
			//}

			//if (iProcessor.imgdata.color.rgb_cam[0][0] != 0) {
			//	mulWhite[0] = iProcessor.imgdata.color.rgb_cam[0][0];
			//	mulWhite[1] = iProcessor.imgdata.color.rgb_cam[1][1];
			//	mulWhite[2] = iProcessor.imgdata.color.rgb_cam[2][2];
			//	mulWhite[3] = iProcessor.imgdata.color.rgb_cam[1][1];
			//}
			//if (iProcessor.imgdata.color.cam_xyz[0][0] != 0) {
			//	mulWhite[0] = iProcessor.imgdata.color.cam_xyz[0][0];
			//	mulWhite[1] = iProcessor.imgdata.color.cam_xyz[1][1];
			//	mulWhite[2] = iProcessor.imgdata.color.cam_xyz[2][2];
			//	mulWhite[3] = iProcessor.imgdata.color.cam_xyz[1][1];
			//}


			if (mulWhite[3] == 0)
				mulWhite[3] = mulWhite[1];



			////DkUtils::printDebug(DK_MODULE, "----------------\n", (float)iProcessor.imgdata.color.maximum);
			////DkUtils::printDebug(DK_MODULE, "Bayer Pattern: %s\n", iProcessor.imgdata.idata.cdesc);
			////DkUtils::printDebug(DK_MODULE, "Camera manufacturer: %s\n", iProcessor.imgdata.idata.make);
			////DkUtils::printDebug(DK_MODULE, "Camera model: %s\n", iProcessor.imgdata.idata.model);
			////DkUtils::printDebug(DK_MODULE, "canon_ev %f\n", (float)iProcessor.imgdata.color.canon_ev);

			////DkUtils::printDebug(DK_MODULE, "white: [%.3f %.3f %.3f %.3f]\n", iProcessor.imgdata.color.cam_mul[0],
			////	iProcessor.imgdata.color.cam_mul[1], iProcessor.imgdata.color.cam_mul[2],
			////	iProcessor.imgdata.color.cam_mul[3]);
			////DkUtils::printDebug(DK_MODULE, "white (processing): [%.3f %.3f %.3f %.3f]\n", mulWhite[0],
			////	mulWhite[1], mulWhite[2],
			////	mulWhite[3]);
			////DkUtils::printDebug(DK_MODULE, "black: %i\n", iProcessor.imgdata.color.black);
			////DkUtils::printDebug(DK_MODULE, "maximum: %.i %i\n", iProcessor.imgdata.color.maximum,
			////	iProcessor.imgdata.params.adjust_maximum_thr);
			////DkUtils::printDebug(DK_MODULE, "----------------\n", (float)iProcessor.imgdata.color.maximum);



			float gamma = (float)iProcessor.imgdata.params.gamm[0];///(float)iProcessor.imgdata.params.gamm[1];
			float gammaTable[65536];
			for (int i = 0; i < 65536; i++) {
				gammaTable[i] = (float)(1.099f*pow((float)i/65535.0f, gamma)-0.099f);
			}


			for (uint row = 0; row < rows; row++)
			{
				float *ptrRaw = rawMat.ptr<float>(row);

				for (uint col = 0; col < cols; col++)
				{
					int colorIdx = iProcessor.COLOR(row, col);
					ptrRaw[col] = (float)(iProcessor.imgdata.image[cols*(row) + col][colorIdx]);
					//ptrRaw[col] = (float)iProcessor.imgdata.color.curve[(int)ptrRaw[col]];

					ptrRaw[col] -= iProcessor.imgdata.color.black;
					ptrRaw[col] /= dynamicRange;

					//// clip
					//if (ptrRaw[col] > 1.0f) ptrRaw[col] = 1.0f;
					//if (ptrRaw[col] < 0.0f) ptrRaw[col] = 0.0f;


					//if (ptrRaw[col] <= 1.0f)
					ptrRaw[col] *= mulWhite[colorIdx];
					ptrRaw[col] = ptrRaw[col] > 1.0f ? 1.0f : ptrRaw[col]; 
					//ptrRaw[col] = (float)(pow((float)ptrRaw[col], gamma));
					//ptrRaw[col] *= 255.0f;		

					ptrRaw[col] = ptrRaw[col] <= 0.018f ? (ptrRaw[col]*(float)iProcessor.imgdata.params.gamm[1]) *255.0f :
						gammaTable[(int)(ptrRaw[col]*65535.0f)]*255;
					//									(1.099f*(float)(pow((float)ptrRaw[col], gamma))-0.099f)*255.0f;
					//ptrRaw[col] *= mulWhite[colorIdx];
					//ptrRaw[col] = ptrRaw[col] > 255.0f ? 255.0f : ptrRaw[col];
					//ptrRaw[col] *=255.0f;
				}
			}

			//Mat cropMat(rawMat, Rect(1, 1, rawMat.cols-1, rawMat.rows-1));
			//rawMat.release();
			//rawMat = cropMat;
			//rawMat.setTo(0);

			//normalize(rawMat, rawMat, 255, 0, NORM_MINMAX);

			rawMat.convertTo(rawMat,CV_8U);
			//cvtColor(rawMat, rgbImg, CV_BayerBG2RGB);
			unsigned long type = (unsigned long)iProcessor.imgdata.idata.filters;
			type = type & 255;

			if (type == 180) cvtColor(rawMat, rgbImg, CV_BayerBG2RGB);      //bitmask  10 11 01 00  -> 3(G) 2(B) 1(G) 0(R) -> RG RG RG
			//												                                                                  GB GB GB
			else if (type == 30) cvtColor(rawMat, rgbImg, CV_BayerRG2RGB);		//bitmask  00 01 11 10	-> 0 1 3 2
			else if (type == 225) cvtColor(rawMat, rgbImg, CV_BayerGB2RGB);		//bitmask  11 10 00 01
			else if (type == 75) cvtColor(rawMat, rgbImg, CV_BayerGR2RGB);		//bitmask  01 00 10 11
			else throw DkException("Wrong Bayer Pattern (not BG, RG, GB, GR)\n", __LINE__, __FILE__);

			if (iProcessor.imgdata.sizes.pixel_aspect != 1.0f) {
				resize(rgbImg, rawMat, Size(), (double)iProcessor.imgdata.sizes.pixel_aspect, 1.0f);
				rgbImg = rawMat;
			}
			image = QImage(rgbImg.data, rgbImg.cols, rgbImg.rows, rgbImg.step/*rgbImg.cols*3*/, QImage::Format_RGB888);

			//orientation is done in loadGeneral with libExiv
			//orientation = iProcessor.imgdata.sizes.flip;
			//switch (orientation) {
			//case 0: orientation = 0; break;
			//case 3: orientation = 180; break;
			//case 5:	orientation = -90; break;
			//case 6: orientation = 90; break;
			//}
		}

		img = image.copy();
		//if (orientation!=0) {
		//	QTransform rotationMatrix;
		//	rotationMatrix.rotate((double)orientation);
		//	img = img.transformed(rotationMatrix);
		//}
		imgLoaded = true;

		iProcessor.recycle();

#else
		throw DkException("Not compiled using OpenCV - could not load any RAW image", __LINE__, __FILE__);
		qDebug() << "Not compiled using OpenCV - could not load any RAW image";
#endif
	} catch (...) {
		qWarning() << "failed to load raw image...";
	}

	return imgLoaded;
}

/**
 * Saves the file specified in a thread.
 * If the file already exists, it will be replaced.
 * @param file the (new) filename.
 * @param fileFilter the file extenstion (e.g. *.jpg)
 * @param saveImg the image to be saved
 * @param compression the compression method (for *.jpg or *.tif images)
 **/ 
void DkImageLoader::saveFile(QFileInfo file, QString fileFilter, QImage saveImg, int compression) {

		QMetaObject::invokeMethod(this, "saveFileIntern", 
			Qt::QueuedConnection, 
			Q_ARG(QFileInfo, file), 
			Q_ARG(QString, fileFilter), 
			Q_ARG(QImage, saveImg),
			Q_ARG(int, compression));
}

/**
 * Saves a temporary file to the folder specified in Settings.
 * @param img the image (which was in most cases pasted to nomacs)
 **/ 
void DkImageLoader::saveTempFile(QImage img) {

	QFileInfo tmpPath = QFileInfo(DkSettings::GlobalSettings::tmpPath + "\\");
	
	if (!DkSettings::GlobalSettings::useTmpPath || !tmpPath.exists()) {
		qDebug() << tmpPath.absolutePath() << "does not exist";
		return;
	}

	qDebug() << "tmpPath: " << tmpPath.absolutePath();
	
	// TODO: let user set filename + extension?
	QString fileExt = ".png";

	// TODO: call save file silent threaded...
	for (int idx = 1; idx < 10000; idx++) {
	
		QString fileName = "img";

		if (idx < 10)
			fileName += "000";
		else if (idx < 100)
			fileName += "00";
		else if (idx < 1000)
			fileName += "0";
		
		fileName += QString::number(idx) + fileExt;

		QFileInfo tmpFile = QFileInfo(tmpPath.absolutePath(), fileName);

		if (!tmpFile.exists()) {
			saveFileSilentThreaded(tmpFile, img);

			//this->virtualFile = tmpFile;	// why doesn't it work out -> file does not exist (locked?)
			//setImage(img);

			//emit updateFileSignal(tmpFile, img.size());
			
			//if (updateFolder) {
			//	emit updateDirSignal(tmpFile);
			//	this->file = tmpFile;
			//	loadDir(tmpFile.absoluteDir());
			//}
			
			qDebug() << tmpFile.absoluteFilePath() << "saved...";
			break;
		}
	}

}

/**
 * Saves a file (not threaded!)
 * If the file already exists, it will be replaced.
 * @param file the file name/path
 * @param fileFilter the file extension (e.g. *.jpg)
 * @param saveImg the image to be saved
 * @param compression the compression method (for jpg, tif)
 **/ 
void DkImageLoader::saveFileIntern(QFileInfo file, QString fileFilter, QImage saveImg, int compression) {
	
	QMutexLocker locker(&mutex);
	
	if (img.isNull() && saveImg.isNull()) {
		QString msg = tr("I can't save an empty file, sorry...\n");
		emit newErrorDialog(msg);
		return;
	}
	if (!file.absoluteDir().exists()) {
		QString msg = tr("Sorry, the directory: %1  does not exist\n").arg(file.absolutePath());
		emit newErrorDialog(msg);
		return;
	}
	if (file.exists() && !file.isWritable()) {
		QString msg = tr("Sorry, I can't write to the file: %1").arg(file.fileName());
		emit newErrorDialog(msg);
		return;
	}

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
	
	// update watcher
	if (this->file.exists() && watcher)
		watcher->removePath(this->file.absoluteFilePath());
	
	QImage sImg = (saveImg.isNull()) ? img : saveImg;
		
	emit updateInfoSignalDelayed(tr("saving..."), true);
	QImageWriter* imgWriter = new QImageWriter(filePath);
	imgWriter->setCompression(compression);
	imgWriter->setQuality(compression);
	bool saved = imgWriter->write(sImg);
	//imgWriter->setFileName(QFileInfo().absoluteFilePath());
	delete imgWriter;
	//bool saved = sImg.save(filePath, 0, compression);
	emit updateInfoSignalDelayed(tr("saving..."), false);
	//qDebug() << "jpg compression: " << compression;

	if ( QFileInfo(filePath).exists())
		qDebug() << QFileInfo(filePath).absoluteFilePath() << " (before exif) exists...";
	else
		qDebug() << QFileInfo(filePath).absoluteFilePath() << " (before exif) does NOT exists...";

	if (saved) {
		
		try {
			imgMetaData.saveMetaDataToFile(QFileInfo(filePath)/*, dataExif.getOrientation()*/);
		} catch (...) {

			if (!restoreFile(QFileInfo(filePath)))
				emit newErrorDialog("sorry, I destroyed: " + QFileInfo(filePath).fileName() + "\n remove the numbers after the file extension in order to restore the file...");
			qDebug() << "could not copy meta-data to file" << filePath;
		}

		// assign the new save directory
		saveDir = QDir(file.absoluteDir());
		DkSettings::GlobalSettings::lastSaveDir = file.absolutePath();
				
		// reload my dir (if it was changed...)
		this->file = QFileInfo(filePath);

		if (this->file.exists())
			qDebug() << this->file.absoluteFilePath() << " (refreshed) exists...";
		else
			qDebug() << this->file.absoluteFilePath() << " (refreshed) does NOT exist...";

		this->virtualFile = this->file;
		this->img = sImg;
		loadDir(file.absoluteDir());

		emit updateImageSignal();
		emit updateFileSignal(this->file, img.size());

		printf("I could save the image...\n");
	}
	else {
		QString msg = tr("Sorry, I can't write to the file: %1").arg(file.fileName());
		emit newErrorDialog(msg);
	}

	if (watcher) watcher->addPath(this->file.absoluteFilePath());

}

/**
 * Saves a file in a thread with no status information.
 * @param file the file name/path
 * @param img the image to be saved
 **/ 
void DkImageLoader::saveFileSilentThreaded(QFileInfo file, QImage img) {

	QMetaObject::invokeMethod(this, "saveFileSilentIntern", Qt::QueuedConnection, Q_ARG(QFileInfo, file), Q_ARG(QImage, img));
}

/**
 * Saves the file (not threaded!).
 * No status information will be displayed if this function is called.
 * @param file the file name/path.
 * @param saveImg the image to be saved.
 **/ 
void DkImageLoader::saveFileSilentIntern(QFileInfo file, QImage saveImg) {

	QMutexLocker locker(&mutex);
	
	// update watcher
	if (this->file.exists() && watcher)
		watcher->removePath(this->file.absoluteFilePath());
	
	emit updateInfoSignalDelayed(tr("saving..."), true);
	QString filePath = file.absoluteFilePath();
	bool saved = (saveImg.isNull()) ? img.save(filePath) : saveImg.save(filePath);
	emit updateInfoSignalDelayed(tr("saving..."), false);	// stop the label
	
	if (saved && watcher)
		watcher->addPath(file.absoluteFilePath());
	else if (watcher)
		watcher->addPath(this->file.absoluteFilePath());

	if (!saveImg.isNull() && saved) {
		
		if (this->file.exists()) {
			try {
				imgMetaData.saveMetaDataToFile(QFileInfo(filePath));
			} catch (...) {
				if (!restoreFile(QFileInfo(filePath)))
					emit newErrorDialog("sorry, I destroyed: " + QFileInfo(filePath).fileName() + "\n remove the numbers after the file extension in order to restore the file...");
			}
		}

		// reload my dir (if it was changed...)
		this->file = QFileInfo(filePath);
		
		this->virtualFile = this->file;
		this->img = saveImg;
		loadDir(this->file.absoluteDir());

		emit updateFileSignal(this->file, saveImg.size());
	}

}

/**
 * Saves the rating to the metadata.
 * This function does nothing if an image format
 * is loaded that does not support metadata.
 * @param rating the rating.
 **/ 
void DkImageLoader::saveRating(int rating) {

	QMutexLocker locker(&mutex);

	try {
		imgMetaData.setRating(rating);
	}catch(...) {
		
		if (!restoreFile(this->file))
			emit updateInfoSignal(tr("Sorry, I could not restore: %1").arg(file.fileName()));
	}
}

//void DkImageLoader::enableWatcher(bool enable) {
//	
//	watcherEnabled = enable;
//}

/**
 * Updates the file history.
 * The file history stores the last 10 folders.
 **/ 
void DkImageLoader::updateHistory() {

	DkSettings::GlobalSettings::lastDir = file.absolutePath();

	DkSettings::GlobalSettings::recentFiles.removeAll(file.absoluteFilePath());
	DkSettings::GlobalSettings::recentFolders.removeAll(file.absolutePath());

	DkSettings::GlobalSettings::recentFiles.push_front(file.absoluteFilePath());
	DkSettings::GlobalSettings::recentFolders.push_front(file.absolutePath());

	DkSettings::GlobalSettings::recentFiles.removeDuplicates();
	DkSettings::GlobalSettings::recentFolders.removeDuplicates();

	for (int idx = 0; idx < DkSettings::GlobalSettings::recentFiles.size()-20; idx++)
		DkSettings::GlobalSettings::recentFiles.pop_back();

	for (int idx = 0; idx < DkSettings::GlobalSettings::recentFiles.size()-20; idx++)
		DkSettings::GlobalSettings::recentFiles.pop_back();


	// TODO: shouldn't we delete that -> it's saved when nomacs is closed anyway
	//DkSettings s = DkSettings();
	//s.save();
}

// image manipulation --------------------------------------------------------------------
/**
 * Deletes the currently loaded file.
 **/ 
void DkImageLoader::deleteFile() {

	if (file.exists()) {

		QFile* fileHandle = new QFile(file.absoluteFilePath());

		//if (fileHandle->permissions() != QFile::WriteUser) {		// on unix this may lead to troubles (see qt doc)
		//	emit updateInfoSignal("You don't have permissions to delete: \n" % file.fileName());
		//	return;
		//}

		QFileInfo fileToDelete = file;

		// load the next file
		QFileInfo loadFile = getChangedFileInfo(1, true);

		if (loadFile.exists())
			load(loadFile, true);
		else {
			clearPath();
			emit updateImageSignal();
		}
		
		if (fileHandle->remove())
			emit updateInfoSignal(tr("%1 deleted...").arg(fileToDelete.fileName()));
		else
			emit updateInfoSignal(tr("Sorry, I could not delete: %1").arg(fileToDelete.fileName()));
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

	if (img.isNull())
		return;

	if (file.exists() && watcher)
		watcher->removePath(this->file.absoluteFilePath());

	//updateInfoSignal("test", 5000);

	try {
		
		mutex.lock();
		QTransform rotationMatrix;
		rotationMatrix.rotate(angle);

		img = img.transformed(rotationMatrix);
		mutex.unlock();

		updateImageSignal();
		updateFileSignal(file, img.size());
		
		mutex.lock();
		
		updateInfoSignalDelayed(tr("saving..."), true);
		imgMetaData.saveOrientation((int)angle);
		updateInfoSignalDelayed(tr("saving..."), false);
		qDebug() << "exif data saved (rotation)?";
		mutex.unlock();

	}
	catch(DkException de) {

		mutex.unlock();
		updateInfoSignalDelayed(tr("saving..."), false);

		// make a silent save -> if the image is just cached, do not save it
		if (file.exists())
			saveFileSilentThreaded(file);
	}
	catch(...) {	// if file is locked... or permission is missing
		mutex.unlock();
		
		// try restoring the file
		if (!restoreFile(file))
			emit updateInfoSignal(tr("Sorry, I could not restore: %1").arg(file.fileName()));
		
	}

	if (watcher) watcher->addPath(this->file.absoluteFilePath());

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
		return false;
	}

	// delete the destroyed file
	QFile file(fileInfo.absoluteFilePath());
	if (file.size() == 0) {
		
		if (!file.remove()) {
			qDebug() << "I could not remove the file...";
			return false;
		}
	}
	else {
		qDebug() << "non-empty file: " << fileName << " I won't delete it...";
		return false;
	}

	// now 
	QFile backupFile(fileInfo.absolutePath() + QDir::separator() + backupFileName);
	return backupFile.rename(fileInfo.absoluteFilePath());
}

/**
 * Reloads the currently loaded file if it was edited by another software.
 * @param path the file path of the changed file.
 **/ 
void DkImageLoader::fileChanged(const QString& path) {

	// ignore if watcher was disabled
	if (path == file.absoluteFilePath()) {
		QMutexLocker locker(&mutex);
		load(QFileInfo(path), true);
	}
}

/**
 * Reloads the file index if the directory was edited.
 * @param path the path to the current directory
 **/ 
void DkImageLoader::directoryChanged(const QString& path) {

	if (QDir(path) == dir.absolutePath()) {

		qDebug() << "folder updated";
		folderUpdated = true;
		// TODO: emit update folder signal
	}
	
}

/**
 * Returns true if a file was specified.
 * @return bool true if a file name/path was specified
 **/ 
bool DkImageLoader::hasFile() {

	return file.exists();
}

/**
 * Returns the currently loaded file information.
 * @return QFileInfo the current file info
 **/ 
QFileInfo DkImageLoader::getFile() {

	QMutexLocker locker(&mutex);
	return file;
}

/**
 * Returns the currently loaded directory.
 * @return QDir the currently loaded directory.
 **/ 
QDir DkImageLoader::getDir() {

	QMutexLocker locker(&mutex);
	return dir;
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
QStringList DkImageLoader::getFilteredFileList(QDir dir, QStringList ignoreKeywords, QStringList keywords) {

	DkTimer dt;
	dir.setSorting(QDir::LocaleAware);
	QStringList fileList = dir.entryList(fileFilters);

	for (int idx = 0; idx < ignoreKeywords.size(); idx++) {
		QRegExp exp = QRegExp("^((?!" + ignoreKeywords[idx] + ").)*$");
		exp.setCaseSensitivity(Qt::CaseInsensitive);
		fileList = fileList.filter(exp);
	}

	for (int idx = 0; idx < keywords.size(); idx++) {
		fileList = fileList.filter(keywords[idx], Qt::CaseInsensitive);
	}
	qDebug() << "filtered file list: " << QString::fromStdString(dt.getTotal());

	return fileList;
}

/**
 * Returns the directory where files are saved to.
 * @return QDir the directory where the user saved the last file to.
 **/ 
QDir DkImageLoader::getSaveDir() {

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

	QString cSuffix = file.suffix();

	for (int idx = 0; idx < saveFilters.size(); idx++) {

		if (saveFilters[idx].contains(cSuffix))
			return saveFilters[idx];
	}

	return "";
}

/**
 * Returns if a file is supported by nomacs or not.
 * Note: this function only checks for a valid extension.
 * @param fileInfo the file info of the file to be validated.
 * @return bool true if the file format is supported.
 **/ 
bool DkImageLoader::isValid(QFileInfo& fileInfo) {

	if (!fileInfo.exists())
		return false;

	printf("accepting file...\n");

	QString fileName = fileInfo.fileName();
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

/**
 * Sets the file specified and loads the directory.
 * @param file the file to be set as current file.
 **/ 
void DkImageLoader::setFile(QFileInfo& file) {
	
	this->file = file;
	this->virtualFile = file;
	loadDir(file.absoluteDir());

}

/**
 * Sets the current directory to dir.
 * @param dir the directory to be loaded.
 **/ 
void DkImageLoader::setDir(QDir& dir) {

	QDir oldDir = file.absoluteDir();

	// locate the current file
	QStringList files = getFilteredFileList(dir, ignoreKeywords, keywords);
	
	if (files.empty()) {
		emit updateInfoSignal(tr("%1 \n does not contain any image").arg(dir.absolutePath()), 4000);	// stop showing
		return;
	}

	loadDir(dir);

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
void DkImageLoader::setImage(QImage& img) {
	
	this->img = img;
}

/**
 * Returns the current file name.
 * @return QString the file name of the currently loaded file.
 **/ 
QString DkImageLoader::fileName() {
	return file.fileName();
}

// DkThumbsLoader --------------------------------------------------------------------

/**
 * Default constructor of the thumbnail loader.
 * Note: currently the init calls the getFilteredFileList which might be slow.
 * @param thumbs a pointer to an array holding the thumbnails. while
 * loading, the thumbsloader will add all images to this array. however, the
 * caller must destroy the thumbs vector.
 * @param dir the directory where thumbnails should be loaded from.
 **/ 
DkThumbsLoader::DkThumbsLoader(std::vector<DkThumbNail>* thumbs, QDir dir) {

	this->thumbs = thumbs;
	this->dir = dir;
	this->isActive = true;
	this->maxThumbSize = DkSettings::DisplaySettings::thumbSize;
	init();
}

/**
 * Initializes the thumbs loader.
 * Note: getFilteredFileList might be slow.
 **/ 
void DkThumbsLoader::init() {

	QStringList files = DkImageLoader::getFilteredFileList(dir);
	startIdx = -1;
	endIdx = -1;
	somethingTodo = false;
	
	DkTimer dt;
	for (int idx = 0; idx < files.size(); idx++) {
		QFileInfo cFile = QFileInfo(dir, files[idx]);
		thumbs->push_back(DkThumbNail(cFile));
	}
	qDebug() << "thumb stubs loaded in: " << QString::fromStdString(dt.getTotal());
}

/**
 * Returns the file idx of the file specified.
 * @param file the file to be querried.
 * @return int the index of the file.
 **/ 
int DkThumbsLoader::getFileIdx(QFileInfo& file) {

	//mutex.lock();

	if (!file.exists() || !thumbs)
		return -1;

	QString cFilePath = file.absoluteFilePath();
	unsigned int fileIdx = 0;
	for ( ; fileIdx < thumbs->size(); fileIdx++) {

		if (thumbs->at(fileIdx).getFile().absoluteFilePath() == cFilePath)
			break;
	}

	if (fileIdx == thumbs->size()) fileIdx = -1;

	//mutex.unlock();

	return fileIdx;

}

/**
 * Thread routine.
 * Only loads thumbs if somethingTodo is true.
 **/ 
void DkThumbsLoader::run() {

	if (!thumbs)
		return;

	while (true) {

		mutex.lock();
		DkTimer dt;
		usleep(10000);

		//QMutexLocker(&this->mutex);
		if (!isActive) {
			qDebug() << "thumbs loader stopped...";
			mutex.unlock();
			break;
		}
		mutex.unlock();

		if (somethingTodo)
			loadThumbs();
	}

	//// locate the current file
	//QStringList files = dir.entryList(DkImageLoader::fileFilters);

	//DkTimer dtt;

	//for (int idx = 0; idx < files.size(); idx++) {

	//	QMutexLocker(&this->mutex);
	//	if (!isActive) {
	//		break;
	//	}

	//	QFileInfo cFile = QFileInfo(dir, files[idx]);

	//	if (!cFile.exists() || !cFile.isReadable())
	//		continue;

	//	QImage img = getThumbNailQt(cFile);
	//	//QImage img = getThumbNailWin(cFile);
	//	thumbs->push_back(DkThumbNail(cFile, img));
	//}

}

/**
 * Loads thumbnails from the metadata.
 **/ 
void DkThumbsLoader::loadThumbs() {


	std::vector<DkThumbNail>::iterator thumbIter = thumbs->begin()+startIdx;

	for (int idx = startIdx; idx < endIdx; idx++, thumbIter++) {

		mutex.lock();
		
		// does somebody want me to stop?
		if (!isActive) {
			mutex.unlock();
			return;
		}
		
		DkThumbNail* thumb = &(*thumbIter);
		if (!thumb->hasImage()) {
			thumb->setImage(getThumbNailQt(thumb->getFile()));
			if (thumb->hasImage()) {	// could I load the thumb?
				emit updateSignal();
				qDebug() << "image exists...";
			}
			else {
				thumb->setImgExists(false);
				qDebug() << "image does NOT exist...";
			}
		}
		mutex.unlock();
	}

	somethingTodo = false;
}

/**
 * Here you can specify which thumbnails to load.
 * Note: it is not a good idea to load all thumbnails
 * of a folder (might be a lot : )
 * @param start the start index
 * @param end the end index
 **/ 
void DkThumbsLoader::setLoadLimits(int start, int end) {

	//QMutexLocker(&this->mutex);
	//if (start < startIdx || startIdx == -1)	startIdx = (start >= 0 && start < thumbs->size()) ? start : 0;
	//if (end > endIdx || endIdx == -1)		endIdx = (end > 0 && end < thumbs->size()) ? end : thumbs->size();
	startIdx = (start >= 0 && (unsigned int) start < thumbs->size()) ? start : 0;
	endIdx = (end > 0 && (unsigned int) end < thumbs->size()) ? end : thumbs->size();

	somethingTodo = true;
}

//QImage DkThumbsLoader::getThumbNailWin(QFileInfo file) {
//
//	CoInitialize(NULL);
//
//	DkTimer dt;
//
//	QImage thumb;
//
//	// allocate some unmanaged memory for our strings and divide the file name
//	// into a folder path and file name.
//	//String* fileName = file.absoluteFilePath();
//	//IntPtr dirPtr = Marshal::StringToHGlobalUni(Path::GetDirectoryName(fileName));
//	//IntPtr filePtr = Marshal::StringToHGlobalUni(Path::GetFileName(fileName));
//
//	QString winPath = QDir::toNativeSeparators(file.absolutePath());
//	QString winFile = QDir::toNativeSeparators(file.fileName());
//	winPath.append("\\");	
//
//	WCHAR* wDirName = new WCHAR[winPath.length()];
//	WCHAR* wFileName = new WCHAR[winFile.length()];
//
//	int dirLength = winPath.toWCharArray(wDirName);
//	int fileLength = winFile.toWCharArray(wFileName);
//
//	wDirName[dirLength] = L'\0';
//	wFileName[fileLength] = L'\0';
//
//	IShellFolder* pDesktop = NULL;
//	IShellFolder* pSub = NULL;
//	IExtractImage* pIExtract = NULL;
//	LPITEMIDLIST pList = NULL;
//
//	// get the desktop directory
//	if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
//	{   
//		// get the pidl for the directory
//		HRESULT hr = pDesktop->ParseDisplayName(NULL, NULL, wDirName, NULL, &pList, NULL);
//		if (FAILED(hr)) {
//			//throw new Exception(S"Failed to parse the directory name");
//
//			return thumb;
//		}
//
//		// get the directory IShellFolder interface
//		hr = pDesktop->BindToObject(pList, NULL, IID_IShellFolder, (void**)&pSub);
//		if (FAILED(hr))	{
//			//throw new Exception(S"Failed to bind to the directory");
//			return thumb;
//		}
//
//		// get the file's pidl
//		hr = pSub->ParseDisplayName(NULL, NULL, wFileName, NULL, &pList, NULL);
//		if (FAILED(hr))	{
//			//throw new Exception(S"Failed to parse the file name");
//			return thumb;
//		}
//
//		// get the IExtractImage interface
//		LPCITEMIDLIST pidl = pList;
//		hr = pSub->GetUIObjectOf(NULL, 1, &pidl, IID_IExtractImage,
//			NULL, (void**)&pIExtract);
//
//		// set our desired image size
//		SIZE size;
//		size.cx = maxThumbSize;
//		size.cy = maxThumbSize;      
//
//		if(pIExtract == NULL) {
//			return thumb;
//		}         
//
//		HBITMAP hBmp = NULL;
//
//		// The IEIFLAG_ORIGSIZE flag tells it to use the original aspect
//		// ratio for the image size. The IEIFLAG_QUALITY flag tells the 
//		// interface we want the image to be the best possible quality.
//		DWORD dwFlags = IEIFLAG_ORIGSIZE | IEIFLAG_QUALITY;      
//
//		OLECHAR pathBuffer[MAX_PATH];
//		hr = pIExtract->GetLocation(pathBuffer, MAX_PATH, NULL, &size, 4, &dwFlags);         // TODO: color depth!! (1)
//		if (FAILED(hr)) {
//			//throw new Exception(S"The call to GetLocation failed");
//			return thumb;
//		}
//
//		hr = pIExtract->Extract(&hBmp);
//
//		// It is possible for Extract to fail if there is no thumbnail image
//		// so we won't check for success here
//
//		pIExtract->Release();
//
//		if (hBmp != NULL) {
//			thumb = QPixmap::fromWinHBITMAP(hBmp, QPixmap::Alpha).toImage();
//		}      
//	}
//
//	// Release the COM objects we have a reference to.
//	pDesktop->Release();
//	pSub->Release(); 
//
//	// delete the unmanaged memory we allocated
//	//Marshal::FreeCoTaskMem(dirPtr);
//	//Marshal::FreeCoTaskMem(filePtr);
//	//delete[] wDirName;
//	//delete[] wFileName;
//
//
//	return thumb;
//}

/**
 * Loads the thumbnail from the metadata.
 * If no thumbnail is embeded, the whole image
 * is loaded and downsampled in a fast manner.
 * @param file the file to be loaded
 * @return QImage the loaded image. Null if no image
 * could be loaded at all.
 **/ 
QImage DkThumbsLoader::getThumbNailQt(QFileInfo file) {

	
	DkTimer dt;

	//// see if we can read the thumbnail from the exif data
	DkMetaData dataExif(file);
	QImage thumb = dataExif.getThumbnail();
	int orientation = dataExif.getOrientation();
	int imgW = thumb.width();
	int imgH = thumb.height();
	int tS = DkSettings::DisplaySettings::thumbSize;

	// as found at: http://olliwang.com/2010/01/30/creating-thumbnail-images-in-qt/
	QString filePath = (file.isSymLink()) ? file.symLinkTarget() : file.absoluteFilePath();
	QImageReader imageReader(filePath);

	if (thumb.isNull() || thumb.width() < tS && thumb.height() < tS) {

		imgW = imageReader.size().width();
		imgH = imageReader.size().height();	// locks the file!
	}
	else if (!thumb.isNull())
		qDebug() << "EXIV thumb loaded: " << thumb.width() << " x " << thumb.height();
	
	if (imgW > maxThumbSize || imgH > maxThumbSize) {
		if (imgW > imgH) {
			imgH = (float)maxThumbSize / imgW * imgH;
			imgW = maxThumbSize;
		} 
		else if (imgW < imgH) {
			imgW = (float)maxThumbSize / imgH * imgW;
			imgH = maxThumbSize;
		}
		else {
			imgW = maxThumbSize;
			imgH = maxThumbSize;
		}
	}

	if (thumb.isNull() || thumb.width() < tS && thumb.height() < tS) {
		// flip size if the image is rotated by 90°
		if (dataExif.isTiff() && abs(orientation) == 90) {
			int tmpW = imgW;
			imgW = imgH;
			imgH = tmpW;
		}

		QSize initialSize = imageReader.size();

		imageReader.setScaledSize(QSize(imgW, imgH));
		thumb = imageReader.read();

		// is there a nice solution to do so??
		imageReader.setFileName("josef");	// image reader locks the file -> but there should not be one so we just set it to another file...

		// there seems to be a bug in exiv2
		if ((initialSize.width() > 400 || initialSize.height() > 400) && DkSettings::DisplaySettings::saveThumb)	// TODO settings
			dataExif.saveThumbnail(thumb);
	}
	else {
		thumb = thumb.scaled(QSize(imgW, imgH), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		qDebug() << "thumb loaded from exif...";
	}

	if (orientation != -1 && !dataExif.isTiff()) {
		QTransform rotationMatrix;
		rotationMatrix.rotate((double)orientation);
		thumb = thumb.transformed(rotationMatrix);
	}

	qDebug() << "[thumb] " << file.fileName() << " loaded in: " << QString::fromStdString(dt.getTotal());

	return thumb;
}

/**
 * Stops the current loading process.
 * This method allows for stopping the thread without killing it.
 **/ 
void DkThumbsLoader::stop() {
	
	//QMutexLocker(&this->mutex);
	isActive = false;
	qDebug() << "stopping thread: " << this->thread()->currentThreadId();
}

DkMetaData::DkMetaData(const DkMetaData& metaData) {

	//const Exiv2::Image::AutoPtr exifImg((metaData.exifImg));
	this->file = metaData.file;
	this->mdata = false;
	// TODO: not too cool...

}

int DkMetaData::getOrientation() {
	readMetaData();

	if (!mdata)
		return -1;

	int orientation;
		
	Exiv2::ExifData &exifData = exifImg->exifData();


	if (exifData.empty()) {
		orientation = -1;
	} else {

		Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Orientation");
		Exiv2::ExifData::iterator pos = exifData.findKey(key);

		 if (pos == exifData.end() || pos->count() == 0) {
			 qDebug() << "Orientation is not set in the Exif Data";
			 orientation = -1;
		 } else {
			Exiv2::Value::AutoPtr v = pos->getValue();

			orientation = (int)pos->toFloat();

			//Exiv2::UShortValue* prv = dynamic_cast<Exiv2::UShortValue*>(v.release());
			//Exiv2::UShortValue::AutoPtr rv = Exiv2::UShortValue::AutoPtr(prv);
			//orientation = (int)rv->value_[0];

			switch (orientation) {
			case 6: orientation = 90;
				break;
			case 7: orientation = 90;
				break;
			case 3: orientation = 180;
				break;
			case 4: orientation = 180;
				break;
			case 8: orientation = -90;
				break;
			case 5: orientation = -90;
				break;
			default: orientation = 0;
				break;
			}	
		}
	}

	 return orientation;
}

QImage DkMetaData::getThumbnail() {

	readMetaData();

	if (!mdata)
		return QImage();

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (exifData.empty())
		return QImage();

	QImage qThumb;
	try {

		Exiv2::ExifThumb thumb(exifData);
		Exiv2::DataBuf buffer = thumb.copy();
		// ok, get the buffer...
		std::pair<Exiv2::byte*, long> stdBuf = buffer.release();
		QByteArray ba = QByteArray((char*)stdBuf.first, (int)stdBuf.second);
		qThumb.loadFromData(ba);
		//qDebug() << "thumbs size: " << qThumb.size();
	}
	catch (...) {
		qDebug() << "Sorry, I could not load the thumb from the exif data...";
	}
		
	return qThumb;
}

void DkMetaData::saveThumbnail(QImage thumb) {

	readMetaData();	
	
	if (!mdata)
		return;

	Exiv2::ExifData exifData = exifImg->exifData();

	if (exifData.empty())
		exifData = Exiv2::ExifData();

	// ok, let's try to save the thumbnail...
	try {
		//Exiv2::ExifThumb eThumb(exifData);
		//eThumb.setJpegThumbnail((byte*)thumb.bits(), (long)thumb.bitPlaneCount());

		Exiv2::ExifThumb eThumb(exifData);

		//if (isTiff()) {
		//	eThumb.erase();

		//	Exiv2::ExifData::const_iterator pos = exifData.findKey(Exiv2::ExifKey("Exif.Image.NewSubfileType"));
		//	if (pos == exifData.end() || pos->count() != 1 || pos->toLong() != 0) {
		//		 throw DkException("Exif.Image.NewSubfileType missing or not set as main image", __LINE__, __FILE__);
		//	}
		//	 // Remove sub-IFD tags
		//	 std::string subImage1("SubImage1");
		//	 for (Exiv2::ExifData::iterator md = exifData.begin(); md != exifData.end();)
		//	 {
		//		 if (md->groupName() == subImage1)
		//			 md = exifData.erase(md);
		//		 else
		//			 ++md;
		//	 }
		//}

		QByteArray data;
		QBuffer buffer(&data);
		buffer.open(QIODevice::WriteOnly);
		thumb.save(&buffer, "JPEG");

		//if (isTiff()) {
		//	Exiv2::DataBuf buf((Exiv2::byte *)data.data(), data.size());
		//	Exiv2::ULongValue val;
		//	val.read("0");
		//	val.setDataArea(buf.pData_, buf.size_);
		//	exifData["Exif.SubImage1.JPEGInterchangeFormat"] = val;
		//	exifData["Exif.SubImage1.JPEGInterchangeFormatLength"] = uint32_t(buf.size_);
		//	exifData["Exif.SubImage1.Compression"] = uint16_t(6); // JPEG (old-style)
		//	exifData["Exif.SubImage1.NewSubfileType"] = uint32_t(1); // Thumbnail image
		//	qDebug() << "As you told me to, I am writing the tiff thumbs...";

		//} else {
			eThumb.setJpegThumbnail((Exiv2::byte *)data.data(), data.size());
			qDebug() << "As you told me to, I am writing the thumbs...";
		//}

		exifImg->setExifData(exifData);
		exifImg->writeMetadata();
		qDebug() << "thumbnail saved...";

		//Exiv2::Image::AutoPtr exifImgN;
		//
		//exifImgN = Exiv2::ImageFactory::open(QFileInfo("C:/img.tif").absoluteFilePath().toStdString());
		//exifImgN->readMetadata();
		//exifImgN->setExifData(exifData);
		//exifImgN->writeMetadata();



	} catch (...) {

		qDebug() << "I could not save the thumbnail...\n";
	}
}

QStringList DkMetaData::getExifKeys() {
		
	QStringList exifKeys;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::ExifData::const_iterator end = exifData.end();

	if (exifData.empty()) {
		return exifKeys;
		
	} else {
	
		for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {

			std::string tmp = i->key();
			exifKeys << QString(tmp.c_str());

			qDebug() << QString::fromStdString(tmp);

		}
	}


	return exifKeys;
}

QStringList DkMetaData::getExifValues() {

	QStringList exifValues;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::ExifData::const_iterator end = exifData.end();

	if (exifData.empty()) {
		return exifValues;

	} else {

		for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {

			std::string tmp = i->value().toString();
			exifValues << QString(tmp.c_str());
		}
	}


	return exifValues;
}

QStringList DkMetaData::getIptcKeys() {

	QStringList iptcKeys;

	Exiv2::IptcData &iptcData = exifImg->iptcData();
	Exiv2::IptcData::iterator endI = iptcData.end();

	if (iptcData.empty()) {
		qDebug() << "iptc data is empty";

		return iptcKeys;

	} else {
		for (Exiv2::IptcData::iterator md = iptcData.begin(); md != endI; ++md) {
			
			std::string tmp = md->key();
			iptcKeys << QString(tmp.c_str());

			qDebug() << QString::fromStdString(tmp);
		}
	}

	return iptcKeys;
}

QStringList DkMetaData::getIptcValues() {
	QStringList iptcValues;

	Exiv2::IptcData &iptcData = exifImg->iptcData();
	Exiv2::IptcData::iterator endI = iptcData.end();

	if (iptcData.empty()) {
		return iptcValues;
	} else {
		for (Exiv2::IptcData::iterator md = iptcData.begin(); md != endI; ++md) {

			std::string tmp = md->value().toString();
			iptcValues << QString(tmp.c_str());

		}
	}

	return iptcValues;
}

std::string DkMetaData::getNativeExifValue(std::string key) {
	std::string info = "";

	readMetaData();
	if (!mdata)
		return info;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (!exifData.empty()) {

		Exiv2::ExifData::iterator pos;

		try {
			Exiv2::ExifKey ekey = Exiv2::ExifKey(key);
			pos = exifData.findKey(ekey);


		} catch(...) {
			return "";
		}

		if (pos == exifData.end() || pos->count() == 0) {
			//qDebug() << "Information is not set in the Exif Data";
		} else {
			Exiv2::Value::AutoPtr v = pos->getValue();
			info = pos->toString();
		}
	}

	return info;
}

std::string DkMetaData::getExifValue(std::string key) {
	
	std::string info = "";

	readMetaData();
	if (!mdata)
		return info;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (!exifData.empty()) {

		Exiv2::ExifData::iterator pos;

		try {
			Exiv2::ExifKey ekey = Exiv2::ExifKey("Exif.Image." + key);
			pos = exifData.findKey(ekey);

			if (pos == exifData.end() || pos->count() == 0) {
				Exiv2::ExifKey ekey = Exiv2::ExifKey("Exif.Photo." + key);	
				pos = exifData.findKey(ekey);
			}
		} catch(...) {
			try {
			key = "Exif.Photo." + key;
			Exiv2::ExifKey ekey = Exiv2::ExifKey(key);	
			pos = exifData.findKey(ekey);
			} catch (... ) {
				return "";
			}
		}

		if (pos == exifData.end() || pos->count() == 0) {
			//qDebug() << "Information is not set in the Exif Data";
		} else {
			Exiv2::Value::AutoPtr v = pos->getValue();
			//Exiv2::StringValue* prv = dynamic_cast<Exiv2::StringValue*>(v.release());
			//Exiv2::StringValue::AutoPtr rv = Exiv2::StringValue::AutoPtr(prv);

			//info = rv->toString();
			info = pos->toString();
		}
	}

	return info;

}

std::string DkMetaData::getIptcValue(std::string key) {
	std::string info = "";

	readMetaData();
	if (!mdata)
		return info;

	Exiv2::IptcData &iptcData = exifImg->iptcData();

	if (!iptcData.empty()) {

		Exiv2::IptcData::iterator pos;

		try {
			Exiv2::IptcKey ekey = Exiv2::IptcKey(key);
			pos = iptcData.findKey(ekey);
		} catch (...) {
			return "";
		}

		if (pos == iptcData.end() || pos->count() == 0) {
			qDebug() << "Orientation is not set in the Exif Data";
		} else {
			Exiv2::Value::AutoPtr v = pos->getValue();
			//Exiv2::StringValue* prv = dynamic_cast<Exiv2::StringValue*>(v.release());
			//Exiv2::StringValue::AutoPtr rv = Exiv2::StringValue::AutoPtr(prv);

			//info = rv->toString();
			info = pos->toString();
		}
	}

	return info;
}

bool DkMetaData::setExifValue(std::string key, std::string taginfo) {

	readMetaData();
	if (!mdata)
		return false;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (!exifData.empty()) {
		
		Exiv2::Exifdatum& tag = exifData[key];
		
		if (!tag.setValue(taginfo)) {
			exifImg->setExifData(exifData);
			exifImg->writeMetadata();
			return true;
		} else
			qDebug() << "could not write Exif Data";
			return false;
	}

	return false;

	//Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::asciiString);
	//// Set the value to a string
	//v->read("1999:12:31 23:59:59");
	//// Add the value together with its key to the Exif data container
	//Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
	//exifData.add(key, v.get());
}

void DkMetaData::saveOrientation(int o) {

	readMetaData();
	if (!mdata) {
		throw DkFileException(QString(QObject::tr("could not read exif data\n")).toStdString(), __LINE__, __FILE__);
	}
	if (o!=90 && o!=-90 && o!=180 && o!=0 && o!=270) {
		qDebug() << "wrong rotation parameter";
		throw DkIllegalArgumentException(QString(QObject::tr("wrong rotation parameter\n")).toStdString(), __LINE__, __FILE__);
	}
	if (o==-180) o=180;
	if (o== 270) o=-90;

	int orientation;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Orientation");


	////----------
	//print all Exif values
	//Exiv2::ExifData::const_iterator end = exifData.end();
	//for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
	//	const char* tn = i->typeName();
	//	std::cout << std::setw(44) << std::setfill(' ') << std::left
	//		<< i->key() << " "
	//		<< "0x" << std::setw(4) << std::setfill('0') << std::right
	//		<< std::hex << i->tag() << " "
	//		<< std::setw(9) << std::setfill(' ') << std::left
	//		<< (tn ? tn : "Unknown") << " "
	//		<< std::dec << std::setw(3)
	//		<< std::setfill(' ') << std::right
	//		<< i->count() << "  "
	//		<< std::dec << i->value()
	//		<< "\n";

	//}
	////----------

	if (exifData.empty()) {
		exifData["Exif.Image.Orientation"] = uint16_t(1);
		qDebug() << "Orientation added to Exif Data";
	}
		
	Exiv2::ExifData::iterator pos = exifData.findKey(key);

	if (pos == exifData.end() || pos->count() == 0) {
		exifData["Exif.Image.Orientation"] = uint16_t(1);

		pos = exifData.findKey(key);
		qDebug() << "Orientation added to Exif Data";
	}

	Exiv2::Value::AutoPtr v = pos->getValue();
	Exiv2::UShortValue* prv = dynamic_cast<Exiv2::UShortValue*>(v.release());
	Exiv2::UShortValue::AutoPtr rv = Exiv2::UShortValue::AutoPtr(prv);
	orientation = (int) rv->value_[0];
	if (orientation <= 0 || orientation > 8) orientation = 1;

	switch (orientation) {
		case 1: if (o!=0) orientation = (o == -90) ? 8 : (o==90 ? 6 : 3);
			break;
		case 2: if (o!=0) orientation = (o == -90) ? 5 : (o==90 ? 7 : 4);
			break;
		case 3: if (o!=0) orientation = (o == -90) ? 6 : (o==90 ? 8 : 1);
			break;
		case 4: if (o!=0) orientation = (o == -90) ? 7 : (o==90 ? 5 : 2);
			break;
		case 5: if (o!=0) orientation = (o == -90) ? 4 : (o==90 ? 2 : 7);
			break;
		case 6: if (o!=0) orientation = (o == -90) ? 1 : (o==90 ? 3 : 8);
			break;
		case 7: if (o!=0) orientation = (o == -90) ? 2 : (o==90 ? 4 : 5);
			break;
		case 8: if (o!=0) orientation = (o == -90) ? 3 : (o==90 ? 1 : 6);
			break;
	}
	rv->value_[0] = (unsigned short) orientation;

	//////----------
	////print all Exif values
	///*Exiv2::ExifData::const_iterator */end = exifData.end();
	//for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
	//	const char* tn = i->typeName();
	//	std::cout << std::setw(44) << std::setfill(' ') << std::left
	//		<< i->key() << " "
	//		<< "0x" << std::setw(4) << std::setfill('0') << std::right
	//		<< std::hex << i->tag() << " "
	//		<< std::setw(9) << std::setfill(' ') << std::left
	//		<< (tn ? tn : "Unknown") << " "
	//		<< std::dec << std::setw(3)
	//		<< std::setfill(' ') << std::right
	//		<< i->count() << "  "
	//		<< std::dec << i->value()
	//		<< "\n";
	//}
	//////----------

	pos->setValue(rv.get());
	//metadaten schreiben
	exifImg->setExifData(exifData);
	exifImg->writeMetadata();
	
}

int DkMetaData::getHorizontalFlipped() {
	
	readMetaData();
	if (!mdata)
		return -1;

	int flipped;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (exifData.empty()) {
		flipped = -1;
	} else {

		Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Orientation");
		Exiv2::ExifData::iterator pos = exifData.findKey(key);

		if (pos == exifData.end() || pos->count() == 0) {
			qDebug() << "Orientation is not set in the Exif Data";
			flipped = -1;
		} else {
			Exiv2::Value::AutoPtr v = pos->getValue();
			Exiv2::UShortValue* prv = dynamic_cast<Exiv2::UShortValue*>(v.release());
			Exiv2::UShortValue::AutoPtr rv = Exiv2::UShortValue::AutoPtr(prv);
			flipped = (int)rv->value_[0];

			switch (flipped) {
			case 2: flipped = 1;
				break;
			case 7: flipped = 1;
				break;
			case 4: flipped = 1;
				break;
			case 5: flipped = 1;
				break;
			default: flipped = 0;
				break;
			}
		}
	}

	return flipped;
}

void DkMetaData::saveHorizontalFlipped(int f) {
	
	readMetaData();
	if (!mdata)
		return;

	int flipped;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Orientation");

	if (exifData.empty()) {
		exifData["Exif.Image.Orientation"] = uint16_t(1);
		qDebug() << "Orientation added to Exif Data";
	}

	Exiv2::ExifData::iterator pos = exifData.findKey(key);

	if (pos == exifData.end() || pos->count() == 0) {
		exifData["Exif.Image.Orientation"] = uint16_t(1);
		pos = exifData.findKey(key);
		qDebug() << "Orientation added to Exif Data";
	}

	Exiv2::Value::AutoPtr v = pos->getValue();
	Exiv2::UShortValue* prv = dynamic_cast<Exiv2::UShortValue*>(v.release());
	Exiv2::UShortValue::AutoPtr rv = Exiv2::UShortValue::AutoPtr(prv);
	flipped = (int)rv->value_[0];
		
	if (flipped <= 0 || flipped > 8) flipped = 1;

	switch (flipped) {
		case 1: flipped = f != 0 ? 2 : flipped ;
			break;
		case 2: flipped = f != 0 ? 1 : flipped ;
			break;
		case 3: flipped = f != 0 ? 4 : flipped ;
			break;
		case 4: flipped = f != 0 ? 3 : flipped ;
			break;
		case 5: flipped = f != 0 ? 8 : flipped ;
			break;
		case 6: flipped = f != 0 ? 7 : flipped ;
			break;
		case 7: flipped = f != 0 ? 6 : flipped;
			break;
		case 8: flipped = f != 0 ? 5 : flipped ;
			break;
	}

	rv->value_[0] = (unsigned short) flipped;

	pos->setValue(rv.get());
	//metadaten schreiben
	exifImg->setExifData(exifData);

	exifImg->writeMetadata();
		
	
}

//only for debug
void DkMetaData::printMetaData() {
	
	readMetaData();
	if (!mdata)
		return;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::IptcData &iptcData = exifImg->iptcData();
	Exiv2::XmpData &xmpData = exifImg->xmpData();

	qDebug() << "Exif------------------------------------------------------------------";

	Exiv2::ExifData::const_iterator end = exifData.end();
	for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
		const char* tn = i->typeName();
		std::cout << std::setw(44) << std::setfill(' ') << std::left
			<< i->key() << " "
			<< "0x" << std::setw(4) << std::setfill('0') << std::right
			<< std::hex << i->tag() << " "
			<< std::setw(9) << std::setfill(' ') << std::left
			<< (tn ? tn : "Unknown") << " "
			<< std::dec << std::setw(3)
			<< std::setfill(' ') << std::right
			<< i->count() << "  "
			<< std::dec << i->value()
			<< "\n";
	}

	qDebug() << "IPTC------------------------------------------------------------------";

	Exiv2::IptcData::iterator endI2 = iptcData.end();
	for (Exiv2::IptcData::iterator md = iptcData.begin(); md != endI2; ++md) {
		std::cout << std::setw(44) << std::setfill(' ') << std::left
			<< md->key() << " "
			<< "0x" << std::setw(4) << std::setfill('0') << std::right
			<< std::hex << md->tag() << " "
			<< std::setw(9) << std::setfill(' ') << std::left
			<< md->typeName() << " "
			<< std::dec << std::setw(3)
			<< std::setfill(' ') << std::right
			<< md->count() << "  "
			<< std::dec << md->value()
			<< std::endl;
	}

	qDebug() << "XMP------------------------------------------------------------------";

	Exiv2::XmpData::iterator endI3 = xmpData.end();
	for (Exiv2::XmpData::iterator md = xmpData.begin(); md != endI3; ++md) {
		std::cout << std::setw(44) << std::setfill(' ') << std::left
			<< md->key() << " "
			<< "0x" << std::setw(4) << std::setfill('0') << std::right
			<< std::hex << md->tag() << " "
			<< std::setw(9) << std::setfill(' ') << std::left
			<< md->typeName() << " "
			<< std::dec << std::setw(3)
			<< std::setfill(' ') << std::right
			<< md->count() << "  "
			<< std::dec << md->value()
			<< std::endl;
	}
}

float DkMetaData::getRating() {

	readMetaData();
	if (!mdata)
		return -1.0f;

	float exifRating = -1;
	float xmpRating = -1;
	float fRating = 0;


	Exiv2::ExifData &exifData = exifImg->exifData();		//Exif.Image.Rating  - short
															//Exif.Image.RatingPercent - short
	Exiv2::XmpData &xmpData = exifImg->xmpData();			//Xmp.xmp.Rating - text
															//Xmp.MicrosoftPhoto.Rating -text


	//get Rating of Exif Tag
	if (!exifData.empty()) {
		Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Rating");
		Exiv2::ExifData::iterator pos = exifData.findKey(key);

		if (pos != exifData.end() && pos->count() != 0) {
			Exiv2::Value::AutoPtr v = pos->getValue();
			exifRating = v->toFloat();
		}
	}

	//get Rating of Xmp Tag
	if (!xmpData.empty()) {
		Exiv2::XmpKey key = Exiv2::XmpKey("Xmp.xmp.Rating");
		Exiv2::XmpData::iterator pos = xmpData.findKey(key);

		//xmp Rating tag
		if (pos != xmpData.end() && pos->count() != 0) {
			Exiv2::Value::AutoPtr v = pos->getValue();
			xmpRating = v->toFloat();
		}

		//if xmpRating not found, try to find MicrosoftPhoto Rating tag
		if (xmpRating == -1) {
			key = Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating");
			pos = xmpData.findKey(key);
			if (pos != xmpData.end() && pos->count() != 0) {
				Exiv2::Value::AutoPtr v = pos->getValue();
				xmpRating = v->toFloat();
			}
		}
	}

	if (xmpRating == -1.0f && exifRating != -1.0f)
		fRating = exifRating;
	else if (xmpRating != -1.0f && exifRating == -1.0f)
		fRating = xmpRating;
	else
		fRating = exifRating;

	return fRating;
}

void DkMetaData::setRating(int r) {
	
	readMetaData();	
	if (!mdata)
		return;

	unsigned short percentRating = 0;
	std::string sRating, sRatingPercent;

	if (r == 5)  { percentRating = 99; sRating = "5"; sRatingPercent = "99";}
	else if (r==4) { percentRating = 75; sRating = "4"; sRatingPercent = "75";}
	else if (r==3) { percentRating = 50; sRating = "3"; sRatingPercent = "50";}
	else if (r==2) { percentRating = 25; sRating = "2"; sRatingPercent = "25";}
	else if (r==1) {percentRating = 1; sRating = "1"; sRatingPercent = "1";}
	else {r=0;}

	Exiv2::ExifData &exifData = exifImg->exifData();		//Exif.Image.Rating  - short
															//Exif.Image.RatingPercent - short
	Exiv2::XmpData &xmpData = exifImg->xmpData();			//Xmp.xmp.Rating - text
															//Xmp.MicrosoftPhoto.Rating -text

	if (r>0) {
		exifData["Exif.Image.Rating"] = uint16_t(r);
		exifData["Exif.Image.RatingPercent"] = uint16_t(r);
		//xmpData["Xmp.xmp.Rating"] = Exiv2::xmpText(sRating);
	
		Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpText);
		v->read(sRating);
		xmpData.add(Exiv2::XmpKey("Xmp.xmp.Rating"), v.get());
		v->read(sRatingPercent);
		xmpData.add(Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating"), v.get());
	} else {

		Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Rating");
		Exiv2::ExifData::iterator pos = exifData.findKey(key);
		if (pos != exifData.end()) exifData.erase(pos);

		key = Exiv2::ExifKey("Exif.Image.RatingPercent");
		pos = exifData.findKey(key);
		if (pos != exifData.end()) exifData.erase(pos);

		Exiv2::XmpKey key2 = Exiv2::XmpKey("Xmp.xmp.Rating");
		Exiv2::XmpData::iterator pos2 = xmpData.findKey(key2);
		if (pos2 != xmpData.end()) xmpData.erase(pos2);

		key2 = Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating");
		pos2 = xmpData.findKey(key2);
		if (pos2 != xmpData.end()) xmpData.erase(pos2);
	}

	exifImg->setExifData(exifData);
	exifImg->setXmpData(xmpData);
	exifImg->writeMetadata();

}


void DkMetaData::saveMetaDataToFile(QFileInfo fileN, int orientation) {

	qDebug() << "saving metadata...";
	readMetaData();	
	if (!mdata)
		return;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::XmpData &xmpData = exifImg->xmpData();
	Exiv2::IptcData &iptcData = exifImg->iptcData();

	Exiv2::Image::AutoPtr exifImgN;
	
	try {

		exifImgN = Exiv2::ImageFactory::open(fileN.absoluteFilePath().toStdString());

	} catch (...) {

		qDebug() << "could not open image for exif data";
		return;
	}

	if (exifImgN.get() == 0) {
		qDebug() << "image could not be opened for exif data extraction";
		return;
	}

	exifImgN->readMetadata();

	exifData["Exif.Image.Orientation"] = uint16_t(orientation);

	exifImgN->setExifData(exifData);
	exifImgN->setXmpData(xmpData);
	exifImgN->setIptcData(iptcData);

	exifImgN->writeMetadata();

}

bool DkMetaData::isTiff() {
	//Exiv2::ImageType::tiff has the same key as nef, ...
	//int type;
	//type = Exiv2::ImageFactory::getType(file.absoluteFilePath().toStdString());
	//return (type==Exiv2::ImageType::tiff);
	QString newSuffix = file.suffix();

	return newSuffix.contains(QRegExp("(tif|tiff)", Qt::CaseInsensitive));
}

bool DkMetaData::isJpg() {

	QString newSuffix = file.suffix();

	return newSuffix.contains(QRegExp("(jpg|jpeg)", Qt::CaseInsensitive));
}

bool DkMetaData::isRaw() {

	QString newSuffix = file.suffix();

	return newSuffix.contains(QRegExp("(nef|crw|cr2|arw)", Qt::CaseInsensitive));
}

void DkMetaData::readMetaData() {
	
	DkTimer dt;
	if (!mdata) {
	
		try {

			std::string filePath = (file.isSymLink()) ? file.symLinkTarget().toStdString() : file.absoluteFilePath().toStdString();
			exifImg = Exiv2::ImageFactory::open(filePath);
			qDebug() << "open exif file" << QString::fromStdString(dt.getIvl());
		} catch (...) {
			mdata = false;
			qDebug() << "could not open image for exif data";
			return;
		}

		if (exifImg.get() == 0) {
			qDebug() << "image could not be opened for exif data extraction";
			mdata = false;
			return;
		}

		try {
			exifImg->readMetadata();
			qDebug() << "readMetaData: " << QString::fromStdString(dt.getIvl());
			if (!exifImg->good()) {
				qDebug() << "metadata could not be read";
				mdata = false;
				return;
			}

		}catch (...) {
			mdata = false;
			return;
		}


		mdata = true;

	}

	if (dt.getTotalTime() != 0) {
		qDebug() << "reading metadata: " << QString::fromStdString(dt.getTotal());
	}

	
}

void DkMetaData::reloadImg() {

	try {

		exifImg = Exiv2::ImageFactory::open(file.absoluteFilePath().toStdString());

	} catch (...) {
		mdata = false;
		qDebug() << "could not open image for exif data";
		return;
	}

	if (exifImg.get() == 0) {
		qDebug() << "image could not be opened for exif data extraction";
		mdata = false;
		return;
	}

	exifImg->readMetadata();

	if (!exifImg->good()) {
		qDebug() << "metadata could not be read";
		mdata = false;
		return;
	}

	mdata = true;
}

}
