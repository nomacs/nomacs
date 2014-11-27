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

#include <QProcess>
#include <QDrag>

#include "DkWidgets.h"

namespace nmc {

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
		this->currentDx = dx;
	};

	QTimer* getMoveImageTimer() {
		return moveImageTimer;
	};

	void setVisible(bool visible);

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
	void loadFileSignal(QFileInfo file);
	//void loadThumbsSignal(int start, int end);
	void changeFileSignal(int idx);
	void positionChangeSignal(int pos);
	void showThumbsDockSignal(bool show);

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
	QVector<QSharedPointer<DkImageContainerT> > thumbs;
	QWidget* parent;
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
	QSharedPointer<DkThumbNailT> getThumb() {return thumb;};
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void updateSize();
	void setVisible(bool visible);
	QPixmap pixmap() const;

	public slots:
		void updateLabel();

signals:
		void loadFileSignal(QFileInfo& file);
		void showFileSignal(const QFileInfo& file);

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void resizeEvent(QResizeEvent *event);
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget * widget = 0);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	QSharedPointer<DkThumbNailT> thumb;
	QGraphicsPixmapItem icon;
	QGraphicsTextItem text;
	QLabel* imgLabel;
	bool thumbInitialized;
	bool fetchingThumb;
	QPen noImagePen;
	QBrush noImageBrush;
	QPen selectPen;
	QBrush selectBrush;
	bool isHovered;
	QPointF lastMove;
};

class DkThumbScene : public QGraphicsScene {
	Q_OBJECT

public:
	DkThumbScene(QWidget* parent = 0);

	void updateLayout();
	QList<QUrl> getSelectedUrls() const;

	public slots:
		void updateThumbLabels();
		void loadFile(QFileInfo& file);
		void increaseThumbs();
		void decreaseThumbs();
		void toggleSquaredThumbs(bool squares);
		void resizeThumbs(float dx);
		void showFile(const QFileInfo& file);
		void selectThumbs(bool select = true, int from = 0, int to = -1);
		void selectAllThumbs(bool select = true);
		void updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs);

signals:
		void loadFileSignal(QFileInfo file);
		void statusInfoSignal(QString msg, int pos = 0);
		void thumbLoadedSignal();

protected:
	QVector<QSharedPointer<DkImageContainerT> > thumbs;
	//void wheelEvent(QWheelEvent *event);

	int xOffset;
	int numRows;
	int numCols;
	bool firstLayout;
	bool itemClicked;

	QVector<DkThumbLabel* > thumbLabels;
	QList<DkThumbLabel* > thumbsNotLoaded;
};

class DkThumbsView : public QGraphicsView {
	Q_OBJECT

public:
	DkThumbsView(DkThumbScene* scene, QWidget* parent = 0);

signals:
	void updateDirSignal(QFileInfo file);

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

};

class DkThumbScrollWidget : public DkWidget {
	Q_OBJECT

	public:
	enum {
		select_all,
		zoom_in,
		zoom_out,
		display_squares,

		actions_end
	};

	DkThumbScrollWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	DkThumbScene* getThumbWidget() {
		return thumbsScene;
	};

	void addContextMenuActions(const QVector<QAction*>& actions, QString menuTitle = "");


	public slots:
		virtual void setVisible(bool visible);
		void updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs);
		void setDir(QFileInfo file);

	signals:
		void updateDirSignal(QFileInfo file);

	protected:
		void createActions();
		void resizeEvent(QResizeEvent *event);
		void contextMenuEvent(QContextMenuEvent *event);

		DkThumbScene* thumbsScene;
		DkThumbsView* view;

		QMenu* contextMenu;
		QVector<QAction*> actions;
		QVector<QAction*> parentActions;

};


}