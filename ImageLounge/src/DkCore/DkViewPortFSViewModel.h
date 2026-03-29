// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <qcontainerfwd.h>
#include <qimage.h>
#include <qobject.h>
#include <utility>

#include "DkImageLoader.h"
#include "nmc_config.h"

namespace nmc
{
class DllCoreExport DkViewPortFSViewModel : public QObject
{
    Q_OBJECT

public:
    explicit DkViewPortFSViewModel();

    [[nodiscard]] QSharedPointer<DkImageLoader> loader() const
    {
        return mLoader;
    }

    void setLoader(QSharedPointer<DkImageLoader> v);

    [[nodiscard]] QString prevFilePath() const
    {
        return mPrevFilePath;
    }

    void setPrevFilePath(const QString &v)
    {
        mPrevFilePath = v;
    }

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
signals:
    // TODO: remove the loaded flag
    void currentImageLoaded(QSharedPointer<DkImageContainerT> img, bool loaded);
    void currentImageUpdated(QSharedPointer<DkImageContainerT> img);
    void imageIndexChanged(int idx);

    // TODO: can we make this a pure signal?
    void directoryChanged(const QVector<QSharedPointer<DkImageContainerT>> &imgs);

    // TODO: this belongs in the view
    void showInfoRequested(const QString &msg, int time, int location);

    // TODO: this probably also not belong here
    void playStateChanged(bool play);

private:
    QString mPrevFilePath;
    QSharedPointer<DkImageLoader> mLoader;

    void connectLoader();
};
}
