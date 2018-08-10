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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QProcess>
#include <QDrag>
#include <QSharedPointer>
#include <QFileInfo>
#include <QGraphicsObject>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsView>
#pragma warning(pop)		// no warnings from includes - end

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"

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

namespace nmc {

// nomacs defines
class DkImageLoader;

class DkFilePreview : public DkWidget {
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

	DkFilePreview(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	~DkFilePreview() {
		saveSettings();
	};

	void setCurrentDx(float dx) {
		scrollToCurrentImage = false;	// external move events
		currentDx = dx;
	};

	QTimer* getMoveImageTimer() {
		return moveImageTimer;
	};

	void setVisible(bool visible, bool saveSettings = true);

	void setWindowPosition(int position) {

		windowPosition = position;
		initOrientations();
	};

	int getWindowPosition() {

		return windowPosition;
	};

public slots:
	void moveImages();
	void updateFileIdx(int fileIdx);
	void updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs);
	void setFileInfo(QSharedPointer<DkImageContainerT> cImage);
	void newPosition();

signals:
	void loadFileSignal(const QString& filePath) const;
	void changeFileSignal(int idx) const;
	void positionChangeSignal(int pos) const;
	void showThumbsDockSignal(bool show) const;

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void leaveEvent(QEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
	void loadSettings();
	void saveSettings();

private:
	QVector<QSharedPointer<DkImageContainerT> > mThumbs;
	QTransform worldMatrix;

	QPoint lastMousePos;
	QPoint enterPos;
	Qt::Orientation orientation;
	QTimer* moveImageTimer;

	QRectF bufferDim;
	QVector<QRectF> thumbRects;

	QLinearGradient leftGradient;
	QLinearGradient rightGradient;
	//QPixmap selectedImg;
	//QPixmap currentImg;

	int mouseTrace;
	QFileInfo currentFile;
	int currentFileIdx;
	int oldFileIdx;
	float currentDx;
	QLabel* wheelButton;

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

	QMenu* contextMenu;
	QVector<QAction*> contextMenuActions;

	void init();
	void initOrientations();
	void drawThumbs(QPainter* painter);
	void drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img);
	void drawSelectedEffect(QPainter* painter, const QRectF& r);
	void drawCurrentImgEffect(QPainter* painter, const QRectF& r);
	void drawNoImgEffect(QPainter* painter, const QRectF& r);
	void createContextMenu();

};

class DkThumbLabel : public QGraphicsObject {
	Q_OBJECT

public:
	DkThumbLabel(QSharedPointer<DkThumbNailT> thumb = QSharedPointer<DkThumbNailT>(), QGraphicsItem* parent = 0);
	~DkThumbLabel();

	void setThumb(QSharedPointer<DkThumbNailT> thumb);
	QSharedPointer<DkThumbNailT> getThumb() {return mThumb;};
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void updateSize();
	void setVisible(bool visible);
	QPixmap pixmap() const;
	void cancelLoading();

public slots:
	void updateLabel();

signals:
	void loadFileSignal(const QString& filePath) const;
	void showFileSignal(const QString& filePath = QString()) const;

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget * widget = 0);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	QSharedPointer<DkThumbNailT> mThumb;
	QGraphicsPixmapItem mIcon;
	QGraphicsTextItem mText;
	bool mThumbInitialized = false;
	bool mFetchingThumb = false;
	QPen mNoImagePen;
	QBrush mNoImageBrush;
	QPen mSelectPen;
	QBrush mSelectBrush;
	bool mIsHovered = false;
	QPointF mLastMove;
};

class DllCoreExport DkThumbScene : public QGraphicsScene {
	Q_OBJECT

public:
	DkThumbScene(QWidget* parent = 0);

	void updateLayout();
	QStringList getSelectedFiles() const;
	QVector<DkThumbLabel*> getSelectedThumbs() const;
	void setImageLoader(QSharedPointer<DkImageLoader> loader);
	void copyImages(const QMimeData* mimeData, const Qt::DropAction& da = Qt::CopyAction) const;
	int findThumb(DkThumbLabel* thumb) const;
	bool allThumbsSelected() const;
	void ensureVisible(QSharedPointer<DkImageContainerT> img) const;
	QString currentDir() const;

public slots:
	void updateThumbLabels();
	void cancelLoading();
	void loadFile(const QString& filePath) const;
	void increaseThumbs();
	void decreaseThumbs();
	void toggleSquaredThumbs(bool squares);
	void toggleThumbLabels(bool show);
	void resizeThumbs(float dx);
	void showFile(const QString& filePath = QString());
	void selectThumbs(bool select = true, int from = 0, int to = -1);
	void selectAllThumbs(bool select = true);
	void updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs);
	void deleteSelected() const;
	void copySelected() const;
	void pasteImages() const;
	void renameSelected() const;

signals:
	void loadFileSignal(const QString& filePath) const;
	void statusInfoSignal(const QString& msg, int pos = 0) const;
	void thumbLoadedSignal() const;

protected:
	void connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals = true);
	
	int mXOffset = 0;
	int mNumRows = 0;
	int mNumCols = 0;
	bool mFirstLayout = true;

	QVector<DkThumbLabel* > mThumbLabels;
	QSharedPointer<DkImageLoader> mLoader;
	QVector<QSharedPointer<DkImageContainerT> > mThumbs;
};

class DkThumbsView : public QGraphicsView {
	Q_OBJECT

public:
	DkThumbsView(DkThumbScene* scene, QWidget* parent = 0);

signals:
	void updateDirSignal(const QString& dir) const;

public slots:
	void fetchThumbs();

protected:
	void wheelEvent(QWheelEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	DkThumbScene* scene;
	QPointF mousePos;
	int lastShiftIdx;

};

class DllCoreExport DkThumbScrollWidget : public DkWidget {
	Q_OBJECT

public:
	DkThumbScrollWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	DkThumbScene* getThumbWidget() {
		return mThumbsScene;
	};

	void clear();

public slots:
	virtual void setVisible(bool visible);
	void updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs);
	void setDir(const QString& dirPath);
	void enableSelectionActions();
	void setFilterFocus() const;
	void batchProcessFiles() const;
	void batchPrint() const;

signals:
	void updateDirSignal(const QString& dir);
	void filterChangedSignal(const QString& filters);
	void batchProcessFilesSignal(const QStringList& fileList) const;

protected:
	void createActions();
	void createToolbar();
	void resizeEvent(QResizeEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
	void connectToActions(bool activate = true);

	DkThumbScene* mThumbsScene = 0;
	DkThumbsView* mView = 0;

	QMenu* mContextMenu = 0;
	QToolBar* mToolbar = 0;
	QLineEdit* mFilterEdit = 0;
};


}
