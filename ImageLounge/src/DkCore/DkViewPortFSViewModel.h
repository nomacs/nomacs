// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <QMimeData>
#include <QObject>
#include <memory>
#include <optional>

#include "DkImageLoader.h"
#include "DkManipulators.h"
#include "nmc_config.h"

namespace nmc
{
class DllCoreExport DkViewPortFSViewModel : public QObject
{
    Q_OBJECT

public:
    explicit DkViewPortFSViewModel();
    ~DkViewPortFSViewModel() override;

    void setLoader(QSharedPointer<DkImageLoader> v);

    void loadFirst();
    void loadLast();

    void loadOffsetFromCurrentFile(int offset);
    void reloadFile();

    void loadFile(const QString &path);
    [[nodiscard]] QSharedPointer<DkImageContainerT> currentImage() const;

    void deleteCurrentFile();

    [[nodiscard]] bool isCurrentFileEdited() const;
    [[nodiscard]] QString currentFilePath() const;

    void saveUserFile(const QImage &img, bool silent);
    void saveFileWeb(const QImage &img);
    bool setCurrentFileAsWallpaper();
    void loadFileAt(int idx);

    using RenderedImageProvider = std::function<QImage()>;

    void applyManipulator(QSharedPointer<DkBaseManipulator> manipulator, const RenderedImageProvider &imp);
    void cancelManipulator();
    [[nodiscard]] bool isManipulatorRunning();

    void rotateImage(double angle);

    std::optional<QByteArray> uneditedSVGData() const;

    struct MovieData {
        QByteArray data;
        QByteArray format;
        QString filename;
    };
    std::optional<MovieData> uneditedMovieData() const;
    void setEditedImage(QSharedPointer<DkImageContainerT> img);
    void loadImage(const QImage &img);
    void saveCurrentEdits();
    void discardCurrentEdits();
    std::unique_ptr<QMimeData> createMimeData(std::optional<QImage> renderedImg = std::nullopt) const;
signals:
    void imageLoaded(QSharedPointer<DkImageContainerT> img);
    void imageLoadFailed();
    void currentImageUpdated(QSharedPointer<DkImageContainerT> img);
    void imageIndexChanged(int idx);

    // TODO: can we make this a pure signal?
    void directoryChanged(const QVector<QSharedPointer<DkImageContainerT>> &imgs);

    // TODO: this belongs in the view
    void showInfoRequested(const QString &msg, int time, int location);

    // TODO: this probably also not belong here
    void playStateChanged(bool play);

    void manipulatorStarted(bool isExtended);
    void manipulatorBusyAborted();
    void manipulatorSucceeded(QSharedPointer<DkImageContainerT> img);
    void manipulatorErrored(const QString &msg);

private:
    QSharedPointer<DkImageLoader> mLoader;
    // image manipulators
    QFutureWatcher<QImage> mManipulatorWatcher;
    QSharedPointer<DkBaseManipulator> mActiveManipulator;

    void connectLoader();
    void finishManipulator();
};
}
