/*******************************************************************************************************
 DkThumbsWidgets.h
 Created on:	18.09.2014

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

#pragma once

#include "DkQt5Compat.h"
#include <vector>

#pragma warning(push, 0) // no warnings from includes - begin
#include <QDrag>
#include <QFileInfo>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPen>
#include <QProcess>
#include <QSharedPointer>
#pragma warning(pop) // no warnings from includes - end

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"
#include "DkThumbs.h"

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QMenu;
class QToolBar;
class QLineEdit;

namespace nmc
{

// nomacs defines
class DkImageLoader;

class DkFilePreview : public DkFadeWidget
{
    Q_OBJECT

public:
    enum {
        cm_pos_west,
        cm_pos_north,
        cm_pos_east,
        cm_pos_south,
        cm_pos_dock_hor,
        cm_pos_dock_ver,

        cm_end,
    };

    DkFilePreview(DkThumbLoader *loader, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    ~DkFilePreview()
    {
        saveSettings();
    };

    void setCurrentDx(float dx)
    {
        scrollToCurrentImage = false; // external move events
        currentDx = dx;
    };

    QTimer *getMoveImageTimer()
    {
        return moveImageTimer;
    };

    void setVisible(bool visible, bool saveSettings = true) override;

    void setWindowPosition(int position)
    {
        windowPosition = position;
        initOrientations();
    };

    int getWindowPosition()
    {
        return windowPosition;
    };

public slots:
    void moveImages();
    void updateThumbs(QVector<QSharedPointer<DkImageContainerT>> thumbs);
    void setFileInfo(QSharedPointer<DkImageContainerT> cImage);
    void newPosition();

signals:
    void loadFileSignal(const QString &filePath) const;
    void changeFileSignal(int idx) const;
    void positionChangeSignal(int pos) const;
    void showThumbsDockSignal(bool show) const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void loadSettings();
    void saveSettings();

private:
    QTransform worldMatrix;

    QPoint lastMousePos;
    QPoint enterPos;
    Qt::Orientation orientation;
    QTimer *moveImageTimer;

    QRectF bufferDim;
    QVector<QRectF> thumbRects;

    QLinearGradient leftGradient;
    QLinearGradient rightGradient;

    int mouseTrace;
    int currentFileIdx;
    int oldFileIdx;
    float currentDx;
    QLabel *wheelButton;

    int selected;
    float winPercent;
    float borderTrigger;

    int xOffset;
    int yOffset;
    int minHeight;

    int windowPosition;

    QRectF newFileRect;
    bool scrollToCurrentImage;
    bool isPainted;

    QMenu *contextMenu;
    QVector<QAction *> contextMenuActions;

    std::vector<QString> mFilePaths{};

    struct Thumb {
        QImage image;
        bool notExist;
        bool fromExif;
    };

    QHash<QString, Thumb> mThumbs;
    DkThumbLoader *mThumbLoader;

    void init();
    void initOrientations();
    void drawThumbs(QPainter *painter);
    void drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img);
    void drawSelectedEffect(QPainter *painter, const QRectF &r);
    void drawCurrentImgEffect(QPainter *painter, const QRectF &r);
    void drawNoImgEffect(QPainter *painter, const QRectF &r);
    void createContextMenu();
};

class DkThumbLabel : public QGraphicsObject
{
    Q_OBJECT

public:
    DkThumbLabel(DkThumbLoader *thumbLoader, const QString &path, QGraphicsItem *parent = 0);
    ~DkThumbLabel();

    void setThumb();
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void updateSize();
    void setVisible(bool visible);
    QPixmap pixmap() const;
    void cancelLoading();
    QString filePath() const;
    QImage image() const;

public slots:
    void updateLabel();

signals:
    void loadFileSignal(const QString &filePath, bool newTab) const;
    void showFileSignal(const QString &filePath = QString()) const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    QGraphicsPixmapItem mIcon;
    QGraphicsTextItem mText;
    bool mFetchingThumb = false;
    QPen mNoImagePen;
    QBrush mNoImageBrush;
    QPen mSelectPen;
    QBrush mSelectBrush;
    bool mIsHovered = false;
    QPointF mLastMove;
    QString mFilePath;
    QImage mThumbImage;
    bool mThumbNotExist;
    bool mThumbFromExif;
    DkThumbLoader *mThumbLoader;

private:
    void updateTooltip();
};

class DllCoreExport DkThumbScene : public QGraphicsScene
{
    Q_OBJECT

public:
    DkThumbScene(DkThumbLoader *thumbLoader, QWidget *parent = 0);

    void updateLayout();
    QStringList getSelectedFiles() const;
    QVector<DkThumbLabel *> getSelectedThumbs() const;

    void setImageLoader(QSharedPointer<DkImageLoader> loader);
    void copyImages(const QMimeData *mimeData, const Qt::DropAction &da = Qt::CopyAction) const;
    int findThumb(DkThumbLabel *thumb) const;
    bool allThumbsSelected() const;
    void ensureVisible(const QString &path) const;

public slots:
    void updateThumbLabels();
    void cancelLoading();
    void increaseThumbs();
    void decreaseThumbs();
    void toggleSquaredThumbs(bool squares);
    void toggleThumbLabels(bool show);
    void resizeThumbs(float dx);
    void showFile(const QString &filePath = QString());
    void selectThumbs(bool select = true, int from = 0, int to = -1);
    void selectThumb(int idx, bool select = true);
    void selectAllThumbs(bool select = true);
    void updateThumbs(QVector<QSharedPointer<DkImageContainerT>> thumbs);
    void deleteSelected();
    void copySelected() const;
    void pasteImages() const;
    void renameSelected() const;

signals:
    void loadFileSignal(const QString &filePath, bool newTab) const;
    void thumbLoadedSignal() const;

private:
    void connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals = true);
    void keyPressEvent(QKeyEvent *event) override;
    QString currentDir() const;
    int selectedThumbIndex(bool first = true);

    int mXOffset = 0;
    int mNumRows = 0;
    int mNumCols = 0;
    int mLastSelectedIdx = -1; // last selected item to restore on updateThumbs()

    QVector<DkThumbLabel *> mThumbLabels;
    QSharedPointer<DkImageLoader> mLoader;
    QVector<QString> mThumbs;
    DkThumbLoader *mThumbLoader;
};

class DkThumbsView : public QGraphicsView
{
    Q_OBJECT

public:
    DkThumbsView(DkThumbScene *scene, QWidget *parent = 0);

signals:
    void updateDirSignal(const QString &dir) const;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    DkThumbScene *scene;
    QPointF mousePos;
    int lastShiftIdx;

private:
    void onScroll();
};

class DllCoreExport DkThumbScrollWidget : public DkWidget
{
    Q_OBJECT

public:
    DkThumbScrollWidget(DkThumbLoader *thumbLoader, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkThumbScrollWidget();

    DkThumbScene *getThumbWidget()
    {
        return mThumbsScene;
    };

    void clear();
    void registerAction(QAction *action)
    {
        mAction = action;
    }

public slots:
    virtual void setVisible(bool visible) override;
    void updateThumbs(QVector<QSharedPointer<DkImageContainerT>> thumbs);
    void setDir(const QString &dirPath);
    void enableSelectionActions();
    void setFilterFocus() const;
    void batchProcessFiles() const;
    void batchPrint() const;
    void onLoadFileTriggered();

signals:
    void updateDirSignal(const QString &dir);
    void filterChangedSignal(const QString &filters);
    void batchProcessFilesSignal(const QStringList &fileList) const;

protected:
    void createActions();
    void createToolbar();
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void connectToActions(bool activate = true);

    DkThumbScene *mThumbsScene = 0;
    DkThumbsView *mView = 0;

    QMenu *mContextMenu = 0;
    QToolBar *mToolbar = 0;
    QLineEdit *mFilterEdit = 0;
    QAction *mAction = 0;
};

class DkRecentDir
{
public:
    DkRecentDir(const QStringList &filePaths = QStringList(), bool pinned = false);

    bool operator==(const DkRecentDir &o) const;
    void update(const DkRecentDir &o);

    bool isEmpty() const;
    bool isPinned() const;

    QString dirName() const;
    QString dirPath() const;
    QString firstFilePath() const;
    QStringList filePaths(int max = -1) const;

    void remove() const;

private:
    QStringList mFilePaths;
    bool mIsPinned = false;
};

class DkRecentDirManager
{
public:
    DkRecentDirManager();

    QList<DkRecentDir> recentDirs() const;

private:
    QList<DkRecentDir> genFileLists(const QStringList &filePaths, bool pinned = false);

    QList<DkRecentDir> mDirs;
};

class DkThumbPreviewLabel : public QLabel
{
    Q_OBJECT

public:
    DkThumbPreviewLabel(const QString &filePath, DkThumbLoader *thumbLoader, int thumbSize = 100, QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void loadFileSignal(const QString &filePath, bool newTab);

public slots:
    void thumbLoaded(const QString &filePath, const QImage &img);

protected:
    void mousePressEvent(QMouseEvent *ev) override;

    int mThumbSize = 100;
    DkThumbLoader *mLoader;
    QString mFilePath;
};

class DllCoreExport DkRecentDirWidget : public DkWidget
{
    Q_OBJECT

public:
    DkRecentDirWidget(const DkRecentDir &rde, DkThumbLoader *thumbLoader, QWidget *parent = 0);

signals:
    void loadFileSignal(const QString &filePath, bool newTab);
    void loadDirSignal(const QString &filePath);
    void removeSignal();

public slots:
    void onPinClicked(bool checked);
    void onRemoveClicked();
    void onLoadDirClicked();

protected:
    DkRecentDir mRecentDir;
    QVector<DkThumbLabel> mThumbs;

    enum {
        button_load_dir = 0,
        button_pin,
        button_remove,

        button_end
    };

    QVector<QPushButton *> mButtons;

    void createLayout(DkThumbLoader *);
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(DkEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

class DllCoreExport DkRecentFilesWidget : public DkWidget
{
    Q_OBJECT

public:
    DkRecentFilesWidget(DkThumbLoader *thumbLoader, QWidget *parent = 0);

    void registerAction(QAction *action)
    {
        mAction = action;
    }

signals:
    void loadFileSignal(const QString &filePath, bool newTab);
    void loadDirSignal(const QString &filePath);

public slots:
    void entryRemoved();
    void setVisible(bool visible) override;

protected:
    void createLayout();
    void updateList();

    QScrollArea *mScrollArea = nullptr;

private:
    QAction *mAction = nullptr;
    DkThumbLoader *mThumbLoader;
};

}
