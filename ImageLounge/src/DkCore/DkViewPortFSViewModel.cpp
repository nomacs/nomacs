// SPDX-License-Identifier: GPL-3.0-or-later

#include "DkViewPortFSViewModel.h"
#include "DkImageLoader.h"
#include <qcontainerfwd.h>
#include <qimage.h>
#include <qobject.h>

#ifdef Q_OS_WIN
#include <QSettings>
#include <windows.h>
#endif

namespace nmc
{

DkViewPortFSViewModel::DkViewPortFSViewModel()
    : mLoader{QSharedPointer<DkImageLoader>(new DkImageLoader())}
{
    connectLoader();
}

void DkViewPortFSViewModel::loadFirst()
{
    mLoader->firstFile();
}

void DkViewPortFSViewModel::loadLast()
{
    mLoader->lastFile();
}

void DkViewPortFSViewModel::loadOffsetFromCurrentFile(int offset)
{
    int sIdx = offset;
    QSharedPointer<DkImageContainerT> lastImg;

    for (int idx = 0; idx < mLoader->getImages().size(); idx++) {
        QSharedPointer<DkImageContainerT> imgC = mLoader->getSkippedImage(sIdx);

        if (!imgC) {
            break;
        }

        mLoader->setCurrentImage(imgC);

        if (imgC && imgC->getLoadState() != DkImageContainer::exists_not) {
            mLoader->load(imgC);
            break;
        }

        if (lastImg == imgC) {
            sIdx += offset; // get me out of endless loops (self referencing shortcuts)
        } else {
            qDebug() << "image does not exist - skipping";
        }

        lastImg = imgC;
    }
}

void DkViewPortFSViewModel::reloadFile()
{
    mLoader->reloadImage();
}

void DkViewPortFSViewModel::loadFile(const QString &path)
{
    DkFileInfo info(path);

    if (info.isDir()) {
        mLoader->setDir(info);
    } else {
        mLoader->load(info);
    }
}

QSharedPointer<DkImageContainerT> DkViewPortFSViewModel::currentImage() const
{
    return mLoader->getCurrentImage();
}

void DkViewPortFSViewModel::deleteCurrentFile()
{
    mLoader->deleteFile();
}

bool DkViewPortFSViewModel::isCurrentFileEdited() const
{
    return mLoader->isEdited();
}

QString DkViewPortFSViewModel::currentFilePath() const
{
    return mLoader->filePath();
}

void DkViewPortFSViewModel::saveUserFile(const QImage &img, bool silent)
{
    mLoader->saveUserFile(img, silent);
}

void DkViewPortFSViewModel::saveFileWeb(const QImage &img)
{
    mLoader->saveFileWeb(img);
}

bool DkViewPortFSViewModel::setCurrentFileAsWallpaper()
{
    // based on code from: http://qtwiki.org/Set_windows_background_using_QT
    auto imgC = mLoader->getCurrentImage();

    if (!imgC || !imgC->hasImage()) {
        qWarning() << "cannot create wallpaper because there is no image loaded...";
        return false;
    }

    QImage img = imgC->image();
    QString tmpPath = mLoader->saveTempFile(img, "wallpaper", "jpg", false);

    // is there a more elegant way to see if saveTempFile returned an empty path
    if (tmpPath.isEmpty()) {
        return false;
    }

#ifdef Q_OS_WIN

    // Read current windows background image path
    QSettings appSettings("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    appSettings.setValue("Wallpaper", tmpPath);

    QByteArray ba = tmpPath.toLatin1();
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void *)ba.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#endif
    // TODO: add functionality for unix based systems
    return true;
}

void DkViewPortFSViewModel::setLoader(QSharedPointer<DkImageLoader> v)
{
    Q_ASSERT(v);
    disconnect(this, nullptr, mLoader.get(), nullptr);
    disconnect(mLoader.get(), nullptr, this, nullptr);
    mLoader = std::move(v);
    connectLoader();
}

void DkViewPortFSViewModel::connectLoader()
{
    auto *loader = mLoader.get();
    connect(loader, &DkImageLoader::imageLoadedSignal, this, &DkViewPortFSViewModel::currentImageLoaded);
    connect(loader,
            QOverload<QSharedPointer<DkImageContainerT>>::of(&DkImageLoader::imageUpdatedSignal),
            this,
            &DkViewPortFSViewModel::currentImageUpdated);
    connect(loader,
            QOverload<int>::of(&DkImageLoader::imageUpdatedSignal),
            this,
            &DkViewPortFSViewModel::imageIndexChanged);
    connect(loader, &DkImageLoader::updateDirSignal, this, &DkViewPortFSViewModel::directoryChanged);
    connect(loader, &DkImageLoader::showInfoSignal, this, &DkViewPortFSViewModel::showInfoRequested);
    connect(loader, &DkImageLoader::setPlayer, this, &DkViewPortFSViewModel::playStateChanged);
}

void DkViewPortFSViewModel::loadFileAt(int idx)
{
    mLoader->loadFileAt(idx);
}
}
