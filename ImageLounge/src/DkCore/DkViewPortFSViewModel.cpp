// SPDX-License-Identifier: GPL-3.0-or-later
#include <QImage>
#include <QtConcurrentRun>
#include <QtGlobal>
#include <memory>
#include <optional>

#include "DkBasicLoader.h"
#include "DkImageContainer.h"
#include "DkImageLoader.h"
#include "DkManipulators.h"
#include "DkUtils.h"
#include "DkViewPortFSViewModel.h"

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
    connect(&mManipulatorWatcher, &QFutureWatcher<QImage>::finished, this, &DkViewPortFSViewModel::finishManipulator);
}

DkViewPortFSViewModel::~DkViewPortFSViewModel()
{
    // TODO: verify that these actually works and is needed.
    // cancel() is likely a no-op because we use QtConcurrent::run().
    // Signals should be disconnected automatically.
    mManipulatorWatcher.cancel();
    mManipulatorWatcher.blockSignals(true);
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

    // The image loader can have a previous directory,
    // so need to get the states from it.
    emit directoryChanged(mLoader->getImages());
    mLoader->activate();
}

void DkViewPortFSViewModel::connectLoader()
{
    auto *loader = mLoader.get();
    connect(loader, &DkImageLoader::imageLoaded, this, &DkViewPortFSViewModel::imageLoaded);
    connect(loader, &DkImageLoader::imageLoadFailed, this, &DkViewPortFSViewModel::imageLoadFailed);
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

void DkViewPortFSViewModel::finishManipulator()
{
    if (mManipulatorWatcher.isCanceled() || !mActiveManipulator) {
        qDebug() << "manipulator applied - but it's canceled";
        return;
    }

    // trigger again if it's dirty
    QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mActiveManipulator);

    // set the edited image
    QImage img = mManipulatorWatcher.result();

    if (!img.isNull()) {
        const QSharedPointer<DkImageContainerT> currImg = currentImage();
        if (currImg) {
            currImg->setImage(img, mActiveManipulator->name());
        }
        emit manipulatorSucceeded(currImg);
    } else {
        emit manipulatorErrored(mActiveManipulator->errorMessage());
    }

    if (mplExt && mplExt->isDirty()) {
        mplExt->setDirty(false);
        mplExt->action()->trigger();
        qDebug() << "triggering manipulator - it's dirty";
    }
}

void DkViewPortFSViewModel::applyManipulator(QSharedPointer<DkBaseManipulator> manipulator,
                                             const RenderedImageProvider &imp)
{
    Q_ASSERT(imp);

    // try to cast up
    QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(manipulator);

    // mark dirty
    if (mManipulatorWatcher.isRunning() && mplExt && mActiveManipulator == manipulator) {
        mplExt->setDirty(true);
        return;
    }

    if (mManipulatorWatcher.isRunning()) {
        emit manipulatorBusyAborted();
        return;
    }

    // undo last if it is an extended manipulator
    QImage img;
    const QSharedPointer<DkImageContainerT> currImg = currentImage();
    if (mplExt && currImg) {
        auto l = currImg->getLoader();
        l->setMinHistorySize(3); // increase the min history size to 3 for correctly popping back
        if (!l->history()->isEmpty() && l->lastEdit().editName() == mplExt->name()) {
            // This undo is only to merge the operations and is not meant to
            // update the view.
            // Directly call undo on the loader instead of the container
            // so the imageUpdated signal does not fire.
            l->undo();

            // TODO: The design of the undo here is weird.
            // This merges the two same operations, which might be beneficial for things like rotation.
            // However, the next undo will be wrong.
        }

        img = currImg->image();
    } else if (imp) {
        img = imp();
    }

    mManipulatorWatcher.setFuture(QtConcurrent::run([manipulator, img] {
        return manipulator.data()->apply(img);
    }));

    mActiveManipulator = manipulator;
    emit manipulatorStarted(!mplExt.isNull());
}

void DkViewPortFSViewModel::cancelManipulator()
{
    // TODO: verify that these actually works and is needed.
    // cancel() is likely a no-op because we use QtConcurrent::run().
    if (mManipulatorWatcher.isRunning()) {
        mManipulatorWatcher.cancel();
    }
}

void DkViewPortFSViewModel::rotateImage(double angle)
{
    mLoader->rotateImage(angle);
}

std::optional<QByteArray> DkViewPortFSViewModel::uneditedSVGData() const
{
    if (!mLoader->hasSvg() || isCurrentFileEdited()) {
        return std::nullopt;
    }
    return *currentImage()->getFileBuffer();
}

std::optional<DkViewPortFSViewModel::MovieData> DkViewPortFSViewModel::uneditedMovieData() const
{
    if (!mLoader->hasMovie() || isCurrentFileEdited()) {
        return std::nullopt;
    }

    DkFileInfo fileInfo = currentImage()->fileInfo();
    if (fileInfo.isSymLink() && !fileInfo.resolveSymLink()) {
        return std::nullopt;
    }

    std::unique_ptr<QIODevice> io = fileInfo.getIODevice();
    if (!io) {
        return std::nullopt;
    }

    const QByteArray format = fileInfo.suffix().toLower().toLatin1();

    return {{io->readAll(), format, fileInfo.fileName()}};
}

void DkViewPortFSViewModel::setEditedImage(QSharedPointer<DkImageContainerT> img)
{
    cancelManipulator();
    mLoader->setImage(std::move(img));
}

void DkViewPortFSViewModel::loadImage(const QImage &img)
{
    mLoader->setImage(img, tr("Original Image"));
    // save to temp folder
    mLoader->saveTempFile(img);
}

void DkViewPortFSViewModel::saveCurrentEdits()
{
    QSharedPointer<DkImageContainerT> img = currentImage();
    if (!img) {
        return;
    }

    // Save image if pixmap edited (lastImageEdit); otherwise save only metadata if metadata edited
    const bool imgEdited = img->getLoader()->isImageEdited();
    const bool metaEdited = img->getLoader()->isMetaDataEdited();

    if (DkUtils::isSavable(img->fileInfo().fileName())) {
        if (imgEdited) {
            img->saveImageThreaded(img->filePath());
        } else if (metaEdited) {
            img->saveMetaData();
        }
    } else {
        saveUserFile(img->image(), false); // we loose all metadata here - right?
    }

    // Clear the image container to force reload so we get correct state.
    img->clear();
}

void DkViewPortFSViewModel::discardCurrentEdits()
{
    QSharedPointer<DkImageContainerT> img = currentImage();
    if (!img) {
        return;
    }

    // Clear the image container to discard all edited changes.
    img->clear();
}

std::unique_ptr<QMimeData> DkViewPortFSViewModel::createMimeData(std::optional<QImage> renderedImg) const
{
    auto mimeData = std::make_unique<QMimeData>();

    const QString filePath = currentFilePath();

    if (renderedImg) {
        // Provide image buffer if file edited or cannot be opened by receiver
        const DkFileInfo fileInfo(filePath);
        const QImage &img = renderedImg.value();
        if (!img.isNull() && (isCurrentFileEdited() || !fileInfo.exists() || fileInfo.isFromZip())) {
            mimeData->setImageData(img);
            return mimeData;
        }
    }

    const QUrl fileUrl = QUrl::fromLocalFile(filePath);
    mimeData->setUrls({fileUrl});
    mimeData->setText(fileUrl.toLocalFile());
    return mimeData;
}
}
