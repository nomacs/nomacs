/*******************************************************************************************************
 DkWidgets.h
 Created on:	17.05.2011
 
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

#pragma once

#include <QApplication>
#include <QRadioButton>
#include <QAction>
#include <QBoxLayout>
#include <QDialog>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QMouseEvent>
#include <QShortcut>
#include <QToolButton>
#include <QComboBox>
#include <QMessageBox>
#include <QStringBuilder>
#include <QPointer>
#include <QTimer>
#include <QMap>
#include <QDesktopServices>
#include <QVector2D>
#include <qmath.h>
#include <QScrollBar>
#include <QFileSystemModel>
#include <QDockWidget>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QToolTip>
#include <QProgressDialog>
#include <QHeaderView>
#include <QMenu>
#include <QScrollArea>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QtConcurrentRun>
#include <QMimeData>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#ifdef WIN32
#pragma warning(disable: 4275)	// there are some weird things happening if qtconcurrentmap.h is included - we ignore this elegantly
#endif

#include <QThread>
#include <QFuture>
#include <QFutureWatcher>
#include <qtconcurrentmap.h>

#if QT_VERSION < 0x050000
#include <QPlastiqueStyle>
#endif

// gif animation label -----
#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
// gif animation label -----

#include "DkImage.h"
#include "DkNetwork.h"
#include "DkSettings.h"
#include "DkMath.h"
#include "DkToolbars.h"
#include "DkBaseViewPort.h"

#ifdef WIN32
#include <ShObjIdl.h>
#include <ShlObj.h>
#endif

namespace nmc {

class DkThumbNail;

class DkWidget : public QWidget {
	Q_OBJECT

public:
	DkWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void registerAction(QAction* action) {
		connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
	};

	void block(bool blocked) {
		this->blocked = blocked;
		setVisible(false);
	};

	void setDisplaySettings(QBitArray* displayBits) {
		displaySettingsBits = displayBits;
	};

	bool getCurrentDisplaySetting() {
		
		if (!displaySettingsBits)
			return false;

		if (DkSettings::app.currentAppMode < 0 || DkSettings::app.currentAppMode >= displaySettingsBits->size()) {
			qDebug() << "[WARNING] illegal app mode: " << DkSettings::app.currentAppMode;
			return false;
		}

		return displaySettingsBits->testBit(DkSettings::app.currentAppMode);
	};

	bool isHiding() const {
		return hiding;
	};


signals:
	void visibleSignal(bool visible);

public slots:
	virtual void show();
	virtual void hide();
	virtual void setVisible(bool visible);

	void animateOpacityUp();
	void animateOpacityDown();

protected:

	QColor bgCol;
	bool blocked;
	bool hiding;
	bool showing;

	QGraphicsOpacityEffect* opacityEffect;
	QBitArray* displaySettingsBits;

	// functions
	void init();
};

class DllExport DkLabel : public QLabel {
	Q_OBJECT

public:
	DkLabel(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkLabel() {
		if (timer) delete timer;
		timer = 0;
	};

	virtual void showTimed(int time = 3000);
	virtual void setText(const QString msg, int time = 3000);
	QString getText();
	void setFontSize(int fontSize);
	void stop();
	
	void block(bool blocked) {
		this->blocked = blocked;
	};
	
	void setFixedWidth(int fixedWidth);
	
	void setMargin(const QPoint& margin) {
		this->margin = margin;
		updateStyleSheet();
	};

public slots:
	virtual void hide();

protected:
	QWidget* parent;
	QTimer* timer;
	QString text;
	QColor textCol;
	int defaultTime;
	int fontSize;
	int time;
	int fixedWidth;
	QPoint margin;
	bool blocked;
	QColor bgCol;

	virtual void init();
	virtual void paintEvent(QPaintEvent *event);
	virtual void draw(QPainter* painter);

	// for my children...
	virtual void drawBackground(QPainter* painter) {};
	virtual void setTextToLabel();
	virtual void updateStyleSheet();
};

class DkLabelBg : public DkLabel {
	Q_OBJECT

public:
	DkLabelBg(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkLabelBg() {};

protected:
	virtual void updateStyleSheet();
};

class DkGradientLabel : public DkLabel {
	Q_OBJECT

public:
	DkGradientLabel(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkGradientLabel() {};

protected:
	void init();
	void drawBackground(QPainter* painter);
	void updateStyleSheet();

	QImage gradient;
	QImage end;

};

/**
 * This label fakes the DkWidget behavior.
 * (allows for registering actions + fades in and out)
 * we need this class too, since we cannot derive from DkLabel & DkWidget
 * at the same time -> both have QObject as common base class.
 **/
class DkFadeLabel : public DkLabel {
	Q_OBJECT

public:
	DkFadeLabel(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkFadeLabel() {};

	void block(bool blocked) {
		this->blocked = blocked;
		setVisible(false);
	};

	void registerAction(QAction* action) {
		connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
	};

	void setDisplaySettings(QBitArray* displayBits) {
		displaySettingsBits = displayBits;
	};

	bool getCurrentDisplaySetting() {

		if (!displaySettingsBits)
			return false;

		if (DkSettings::app.currentAppMode < 0 || DkSettings::app.currentAppMode >= displaySettingsBits->size()) {
			qDebug() << "[WARNING] illegal app mode: " << DkSettings::app.currentAppMode;
			return false;
		}

		return displaySettingsBits->testBit(DkSettings::app.currentAppMode);
	};

signals:
	void visibleSignal(bool visible);

public slots:
	virtual void show();
	virtual void hide();
	virtual void setVisible(bool visible);

protected slots:
	void animateOpacityUp();
	void animateOpacityDown();

protected:

	bool hiding;
	bool showing;
	QBitArray* displaySettingsBits;

	QGraphicsOpacityEffect *opacityEffect;

	// functions
	void init();

};

class DkButton : public QPushButton {
	Q_OBJECT

public:
	DkButton ( QWidget * parent = 0 );
	DkButton ( const QString & text, QWidget * parent = 0 );
	DkButton ( const QIcon & icon, const QString & text, QWidget * parent = 0 );
	DkButton ( const QIcon & checkedIcon, const QIcon & uncheckedIcon, const QString & text, QWidget * parent = 0 );
	~DkButton() {};

	void adjustSize();
	void setFixedSize(QSize size);

	bool keepAspectRatio;

protected:
	QIcon checkedIcon;
	QIcon uncheckedIcon;
	bool mouseOver;
	QSize mySize;
	
	// functions
	void init();

	void paintEvent(QPaintEvent * event);
	void focusInEvent(QFocusEvent * event);
	void focusOutEvent(QFocusEvent * event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	QPixmap createSelectedEffect(QPixmap* pm);

};

class DkRatingLabel : public DkWidget {
	Q_OBJECT

public:
	enum {
		rating_1,
		rating_2,
		rating_3,
		rating_4,
		rating_5,
		rating_0,	// no image for that one
	};

	DkRatingLabel(int rating = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkRatingLabel() {};

	void setRating(int rating) {
		this->rating = rating;
		updateRating();
	};

	virtual void changeRating(int newRating) {
		rating = newRating;
		updateRating();
		emit newRatingSignal(rating);
	};

	int getRating() {
		return rating;
	};

signals:
	void newRatingSignal(int rating = 0);

public slots:
	void rating0() {
		changeRating(0);
	};

	void rating1() {
		changeRating(1);
	};

	void rating2() {
		changeRating(2);
	};

	void rating3() {
		changeRating(3);
	};

	void rating4() {
		changeRating(4);
	};

	void rating5() {
		changeRating(5);
	};

protected:
	QVector<DkButton*> stars;
	QBoxLayout* layout;
	int rating;
	QPoint margin;
	int spacing;

	void updateRating() {
		
		for (int idx = 0; idx < stars.size(); idx++) {
			stars[idx]->setChecked(idx < rating);
		}
	};

	virtual void init();
};

class DkRatingLabelBg : public DkRatingLabel {
	Q_OBJECT

public:
	DkRatingLabelBg(int rating = 0, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkRatingLabelBg() {
		if (hideTimer) delete hideTimer;
		hideTimer = 0;
	};

	void changeRating(int newRating) {
		DkRatingLabel::changeRating(newRating);
		show();
		hideTimer->start();
	}

	QVector<QAction*> getActions() {
		return actions;
	};


protected:
	QVector<QAction*> actions;
	QTimer* hideTimer;
	int timeToDisplay;
	
	virtual void paintEvent(QPaintEvent *event);
};

class DkFileInfoLabel : public DkFadeLabel {
	Q_OBJECT

public:
	DkFileInfoLabel(QWidget* parent = 0);
	~DkFileInfoLabel() {};

	void createLayout();
	void updateInfo(const QFileInfo& file, const QString& attr, const QString& date, const int rating);
	void updateTitle(const QFileInfo& file, const QString& attr);
	void updateDate(const QString& date = QString());
	void updateRating(const int rating);
	void setEdited(bool edited);
	DkRatingLabel* getRatingLabel();

public slots:
	virtual void setVisible(bool visible);

protected:
	QPoint offset;
	QFileInfo file;

	QBoxLayout* layout;
	QWidget* parent;
	QLabel* title;
	QLabel* date;
	DkRatingLabel* rating;
	int minWidth;

	void updateWidth();
};

class DkPlayer : public DkWidget {
	Q_OBJECT

public:
	enum {
		play_action,	// if more actions are to be assigned
	};

	DkPlayer(QWidget* parent = 0);
	~DkPlayer() {};

	void draw(QPainter* painter);
	void setTimeToDisplay(int ms = 1000);

	QVector<QAction*> getActions() {
		return actions;
	};

signals:
	void nextSignal();
	void previousSignal();

public slots:
	void play(bool play) {
		
		if (play != playing)	// emulate a click
			playButton->setChecked(play);

		playing = play;

		if (play) {
			displayTimer->start();
			hideTimer->start();
		}
		else
			displayTimer->stop();
	};

	void togglePlay() {
		
		show();
		playing = !playing;
		playButton->click();
	};

	void startTimer() {
		if (playing) {
			displayTimer->setInterval(DkSettings::slideShow.time*1000);	// if it was updated...
			displayTimer->start();
		}
	};

	void autoNext() {
		emit nextSignal();
	};

	void next() {
		hideTimer->stop();
		emit nextSignal();
	};
	void previous() {
		hideTimer->stop();
		emit previousSignal();
	};

	virtual void show(int ms = 0);

	bool isPlaying() const {
		return playing;
	};

protected:
	void resizeEvent(QResizeEvent *event);
	void init();
	bool playing;

	int timeToDisplay;
	QTimer* displayTimer;
	QTimer* hideTimer;

	DkButton* previousButton;
	DkButton* nextButton;
	DkButton* playButton;
	QWidget* container;

	QVector<QAction*> actions;

};


class DkFilePreview : public DkWidget {
	Q_OBJECT

public:
	DkFilePreview(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	
	~DkFilePreview() {
	};

	void setCurrentDx(float dx) {
		scrollToCurrentImage = false;	// external move events
		this->currentDx = dx;
	};

	QTimer* getMoveImageTimer() {
		return moveImageTimer;
	};

	void setVisible(bool visible);

public slots:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void leaveEvent(QEvent *event);
	void moveImages();
	void updateFileIdx(int fileIdx);
	void updateThumbs(QVector<QSharedPointer<DkImageContainerT> > thumbs);
	void setFileInfo(QSharedPointer<DkImageContainerT> cImage);

signals:
	void loadFileSignal(QFileInfo file);
	//void loadThumbsSignal(int start, int end);
	void changeFileSignal(int idx);

private:
	QVector<QSharedPointer<DkImageContainerT> > thumbs;
	QWidget* parent;
	QTransform worldMatrix;
	
	QPoint lastMousePos;
	QPoint enterPos;
	
	QTimer* moveImageTimer;
	
	QRectF bufferDim;
	QVector<QRectF> thumbRects;
	
	QLinearGradient leftGradient;
	QLinearGradient rightGradient;
	QPixmap selectedImg;
	QPixmap currentImg;
	
	QImage stubImg;
	int mouseTrace;
	QFileInfo currentFile;
	int currentFileIdx;
	int oldFileIdx;
	float currentDx;
	QLabel* wheelButton;

	//DkGradientLabel* fileLabel;

	int selected;
	float winPercent;
	float borderTrigger;

	int xOffset;
	int yOffset;
	int minHeight;

	QRectF newFileRect;
	bool scrollToCurrentImage;
	bool isPainted;
	
	void init();
	//void clearThumbs();
	//void indexDir(int force = DkThumbsLoader::not_forced);
	void drawThumbs(QPainter* painter);
	void drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img);
	void createCurrentImg(const QImage& img);
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

};

class DkFolderScrollBar : public QScrollBar {
	Q_OBJECT

public: 
	DkFolderScrollBar(QWidget* parent = 0);
	~DkFolderScrollBar();

	virtual void setValue(int i);

	// DkWidget stuff
	void registerAction(QAction* action) {
		connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
	};

	void block(bool blocked) {
		this->blocked = blocked;
		setVisible(false);
	};

	void setDisplaySettings(QBitArray* displayBits) {
		displaySettingsBits = displayBits;
	};

	bool getCurrentDisplaySetting() {

		if (!displaySettingsBits)
			return false;

		if (DkSettings::app.currentAppMode < 0 || DkSettings::app.currentAppMode >= displaySettingsBits->size()) {
			qDebug() << "[WARNING] illegal app mode: " << DkSettings::app.currentAppMode;
			return false;
		}

		return displaySettingsBits->testBit(DkSettings::app.currentAppMode);
	};

public slots:
	void updateDir(QVector<QSharedPointer<DkImageContainerT> > images);
	void updateFile(QSharedPointer<DkImageContainerT> imgC);
	void update(const QVector<QColor>& colors, const QVector<int>& indexes);

	// DkWidget
	virtual void show();
	virtual void hide();
	virtual void setVisible(bool visible);

	void animateOpacityUp();
	void animateOpacityDown();

protected slots:
	void emitFileSignal(int i);
	void colorUpdated();

signals:
	void changeFileSignal(int idx);

	// DkWidget
	void visibleSignal(bool visible);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);
	int fileIdx(QSharedPointer<DkImageContainerT> imgC);
	void updateColors();

	QSharedPointer<DkImageContainerT> cImg;
	QVector<QSharedPointer<DkImageContainerT> > images;
	//DkColorLoader* colorLoader;
	QVector<QColor> colors;
	QVector<int> indexes;
	bool sliding;
	bool updateFolder;
	int updatesWaiting;
	QLabel* handle;
	DkWidget* dummyWidget;
	int minHandleWidth;


	// DkWidget
	QColor bgCol;
	bool blocked;
	bool hiding;
	bool showing;

	QGraphicsOpacityEffect* opacityEffect;
	QBitArray* displaySettingsBits;

	void init();

};

// this class is one of the first batch processing classes -> move them to a new file in the (near) future
class DkThumbsSaver : public DkWidget {
	Q_OBJECT

public:
	DkThumbsSaver(QWidget* parent = 0);

	void processDir(QVector<QSharedPointer<DkImageContainerT> > images, bool forceSave);

signals:
	void numFilesSignal(int currentFileIdx);

public slots:
	void stopProgress();
	void thumbLoaded(bool loaded);
	void loadNext();

protected:

	QFileInfo currentDir;
	QProgressDialog* pd;
	int cLoadIdx;
	QVector<QSharedPointer<DkImageContainerT> > images;
	bool stop;
	bool forceSave;
	int numSaved;
};

class DkFileSystemModel : public QFileSystemModel {
	Q_OBJECT

public:
	DkFileSystemModel(QObject* parent = 0);

protected:

};

class DkSortFileProxyModel : public QSortFilterProxyModel {
	Q_OBJECT

public:
	DkSortFileProxyModel(QObject* parent = 0);

protected:
	virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

};

class DkExplorer : public QDockWidget {
	Q_OBJECT

public:
	DkExplorer(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkExplorer();

public slots:
	void setCurrentPath(QFileInfo fileInfo);
	void fileClicked(const QModelIndex &index) const;
	void showColumn(bool show);
	void setEditable(bool editable);

signals:
	void openFile(QFileInfo fileInfo) const;
	void openDir(QFileInfo dir) const;

protected:
	void closeEvent(QCloseEvent *event);
	void contextMenuEvent(QContextMenuEvent* event);

	void createLayout();
	void writeSettings();
	void readSettings();

	DkFileSystemModel* fileModel;
	DkSortFileProxyModel* sortModel;
	QTreeView* fileTree;
	QVector<QAction*> columnActions;
};

class DkOverview : public DkWidget {
	Q_OBJECT

public:
	DkOverview(QWidget * parent = 0, Qt::WindowFlags f = 0);
	~DkOverview() {};

	void resize(int w, int h);
	void resize(const QSize& size);

	void setImage(QImage img) {
		this->img = img;

		if (isVisible())
			resizeImg();
	};

	void setTransforms(QTransform* worldMatrix, QTransform* imgMatrix){
		this->worldMatrix = worldMatrix;
		this->imgMatrix = imgMatrix;
	};

	void setViewPortRect(QRectF viewPortRect) {
		this->viewPortRect = viewPortRect;	
	};

	void setVisible(bool visible) {

		if (visible)
			resizeImg();

		DkWidget::setVisible(visible);
	};

signals:
	void moveViewSignal(QPointF dxy);
	void sendTransformSignal();

protected:
	QImage img;
	QImage imgT;
	QTransform* scaledImgMatrix;
	QTransform* worldMatrix;
	QTransform* imgMatrix;
	QRectF viewPortRect;
	QPointF posGrab;
	QPointF enterPos;
	QWidget* parent;

	void resizeImg();
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent* event);
	QTransform getScaledImageMatrix();
};

class DkMetaDataInfo : public DkWidget {
	Q_OBJECT

public:

	static QString sCamDataTags;
	static QString sDescriptionTags;
		
	DkMetaDataInfo(QWidget* parent = 0);
	~DkMetaDataInfo() {};

	void draw(QPainter* painter);
	void createLabels();
	void resizeEvent(QResizeEvent *resizeW);
	//void getResolution(float &xResolution, float &yResolution);
	QString getGPSCoordinates();
	
	//DkMetaData* getMetaData() {
	//	return &metaData;
	//};
	
signals:
	void enableGpsSignal(bool);
	
public slots:
	void setImageInfo(QSharedPointer<DkImageContainerT> imgC);
	void setRating(int rating);
	//void setResolution(int xRes, int yRes);
	void updateLabels();
	void mouseMoveEvent(QMouseEvent *event);
	void setVisible(bool visible) {

		if (visible)
			createLabels();

		qDebug() << "[DkMetaData] setVisible: " << visible;

		DkWidget::setVisible(visible);
	};

protected:
	void init();
	void readTags();
	void layoutLabels();
	void paintEvent(QPaintEvent *event);
	

	QWidget* parent;
	QPoint lastMousePos;
	QTransform worldMatrix;
	int exifHeight;
	int fontSize;
	int textMargin;
	int numLines;
	int maxCols;
	int numLabels;
	int minWidth;
	int gradientWidth;

	int yMargin;
	int xMargin;

	float currentDx;

	QVector<int> maxLenLabel;

	QVector<DkLabel *> pLabels;
	QVector<DkLabel *> pValues;
	//QSize imgSize;

	QStringList camDTags;
	QStringList camDValues;

	QStringList descTags;
	QStringList descValues;

	QRect leftGradientRect;
	QLinearGradient leftGradient;
	QRect rightGradientRect;
	QLinearGradient rightGradient;

	QMap<int, int> mapIptcExif;
	QStringList exposureModes;
	QMap<int, QString> flashModes;
	QSharedPointer<DkImageContainerT> imgC;

};

// TODO: move to DkMath
class DkRotatingRect {

public:
	DkRotatingRect(QRectF rect = QRect()) {

		if (rect.isEmpty()) {

			for (int idx = 0; idx < 4; idx++)
				this->rect.push_back(QPointF());
		}
		else
			this->rect = rect;

	};
	
	virtual ~DkRotatingRect() {};

		friend std::ostream& operator<<(std::ostream& s, DkRotatingRect& r){

		return r.put(s);
	};

	bool isEmpty() {

		if (rect.size() < 4)
			return true;

		QPointF lp = rect[0]; 
		for (int idx = 1; idx < rect.size(); idx++) {
			
			if (lp != rect[idx]) {
				return false;
			}
			lp = rect[idx];
		}
		return true;
	};

	void setAllCorners(QPointF &p) {
		
		for (int idx = 0; idx < rect.size(); idx++)
			rect[idx] = p;

	};

	DkVector getDiagonal(int cIdx) {

		DkVector c0 = rect[cIdx % 4];
		DkVector c2 = rect[(cIdx+2) % 4];

		return c2 - c0;
	};

	QCursor cpCursor(int idx) {

		double angle = 0;

		if (idx >= 0 && idx < 4) {

			// this seems a bit complicated...
			// however the points are not necessarily stored in clockwise order...
			DkVector e1 = rect[(idx+1) % 4] - rect[idx];
			DkVector e2 = rect[(idx+3) % rect.size()] - rect[idx];
			e1.normalize();
			e2.normalize();
			DkVector rv = e1-e2;
			rv = rv.normalVec();
			angle = rv.angle();
		}
		else {
			DkVector edge = rect[(idx+1) % 4] - rect[idx % 4];
			angle = edge.normalVec().angle();	// the angle of the normal vector
		}

		angle = DkMath::normAngleRad(angle, -CV_PI/8.0, 7.0*CV_PI/8.0);

		if (angle > 5.0*CV_PI/8.0)
			return QCursor(Qt::SizeBDiagCursor);
		else if (angle > 3.0*CV_PI/8.0)
			return QCursor(Qt::SizeVerCursor);
		else if (angle > CV_PI/8.0)
			return QCursor(Qt::SizeFDiagCursor);
		else
			return QCursor(Qt::SizeHorCursor);

	};

	void updateCorner(int cIdx, QPointF nC, DkVector oldDiag = DkVector()) {

		// index does not exist
		if (cIdx < 0 || cIdx >= rect.size()*2)
			return;

		if (rect[(cIdx+1) % 4] == rect[(cIdx+3) % 4]) {
			QPointF oC = rect[(cIdx+2) % 4];	// opposite corner
			rect[cIdx] = nC;
			rect[(cIdx+1) % 4] = QPointF(nC.x(), oC.y());
			rect[(cIdx+3) % 4] = QPointF(oC.x(), nC.y());
		}
		// these indices indicate the control points on edges
		else if (cIdx >= 4 && cIdx < 8) {

			DkVector c0 = rect[cIdx % 4];
			DkVector n = (rect[(cIdx+1) % 4] - c0).normalVec();
			n.normalize();

			// compute the offset vector
			DkVector oV = n * n.scalarProduct(nC-c0);

			rect[cIdx % 4] = (rect[cIdx % 4] + oV).getQPointF();
			rect[(cIdx+1) % 4] = (rect[(cIdx+1) % 4] + oV).getQPointF();
		}
		else {

			// we have to update the n-1 and n+1 corner
			DkVector cN = nC;
			DkVector c0 = rect[cIdx];
			DkVector c1 = rect[(cIdx+1) % 4];
			DkVector c2 = rect[(cIdx+2) % 4];
			DkVector c3 = rect[(cIdx+3) % 4];
			
			if (!oldDiag.isEmpty()) {
				DkVector dN = oldDiag.normalVec();
				dN.normalize();

				float d = dN*(cN-c2);
				cN += (dN*-d);
			}

			// new diagonal
			float diagLength = (c2-cN).norm();
			float diagAngle = (c2-cN).angle();

			// compute the idx-1 corner
			float c1Angle = (c1-c0).angle();
			float newLength = cos(c1Angle - diagAngle)*diagLength;
			DkVector nc1 = DkVector((newLength), 0);
			nc1.rotate(-c1Angle);

			// compute the idx-3 corner
			float c3Angle = (c3-c0).angle();
			newLength = cos(c3Angle - diagAngle)*diagLength;
			DkVector nc3 = DkVector((newLength), 0);
			nc3.rotate(-c3Angle);
			
			rect[(cIdx+1) % 4] = (nc1+cN).getQPointF();			
			rect[(cIdx+3) % 4] = (nc3+cN).getQPointF();
			rect[cIdx] = cN.getQPointF();
		}
	};

	QPolygonF& getPoly() {

		return rect;
	};

	void setPoly(QPolygonF &poly) {
		
		rect = poly;
	};

	QPolygonF getClosedPoly() {

		if (rect.isEmpty())
			return QPolygonF();

		QPolygonF closedPoly = rect;
		closedPoly.push_back(closedPoly[0]);
		
		return closedPoly;
	};

	QPointF getCenter() {

		if (rect.empty())
			return QPointF();

		DkVector c1 = rect[0];
		DkVector c2 = rect[2];
		
		return ((c2-c1)*0.5f + c1).getQPointF();
	};

	void setCenter(const QPointF& center) {

		if (rect.empty())
			return;

		DkVector diff = getCenter() - center;

		for (int idx = 0; idx < rect.size(); idx++) {

			rect[idx] = rect[idx] - diff.getQPointF();
		}


	}

	double getAngle() {
		
		// default upper left corner is 0
		DkVector xV = rect[3] - rect[0];
		return xV.angle();
	};

	void getTransform(QTransform& tForm, QPointF& size) {

		if (rect.size() < 4)
			return;

		// default upper left corner is 0
		DkVector xV = DkVector(rect[3] - rect[0]).round();
		DkVector yV = DkVector(rect[1] - rect[0]).round();

		QPointF ul = QPointF(qRound(rect[0].x()), qRound(rect[0].y()));
		size = QPointF(xV.norm(), yV.norm());

		qDebug() << xV.getQPointF();
		qDebug() << "size: " << size;


		double angle = xV.angle();
		angle = DkMath::normAngleRad(angle, -CV_PI, CV_PI);

		if (abs(angle) > DBL_EPSILON)
			qDebug() << "angle is > eps...";

		// switch width/height for /\ and \/ quadrants
		if (abs(angle) > CV_PI*0.25 && abs(angle) < CV_PI*0.75) {
			float x = size.x();
			size.setX(size.y());
			size.setY(x);
		}

		// invariance -> user does not want to make a difference between an upside down rect
		if (angle > CV_PI*0.25 && angle < CV_PI*0.75) {
			angle -= CV_PI*0.5;
			ul = rect[1];
		}
		else if (angle > -CV_PI*0.75 && angle < -CV_PI*0.25) {
			angle += CV_PI*0.5;
			ul = rect[3];
		}
		else if (angle >= CV_PI*0.75 || angle <= -CV_PI*0.75) {
			angle += CV_PI;
			ul = rect[2];
		}

		tForm.rotateRadians(-angle);
		tForm.translate(qRound(-ul.x()), qRound(-ul.y()));	// round guarantees that pixels are not interpolated

	};

protected:

	virtual std::ostream& put(std::ostream& s) {

		s << "DkRotatingRect: ";
		for (int idx = 0; idx < rect.size(); idx++) {
			DkVector vec = DkVector(rect[idx]);
			s << vec << ", ";
		}

		return s;
	};

	QPolygonF rect;
};

class DkTransformRect : public QWidget {
	Q_OBJECT

public:
	
	DkTransformRect(int idx = -1, DkRotatingRect* rect = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);
	virtual ~DkTransformRect() {};

	void draw(QPainter *painter);
	
	QPointF getCenter() {
		return QPointF(size.width()*0.5f, size.height()*0.5f);
	};

signals:
	void ctrlMovedSignal(int, QPointF, bool, bool);
	void updateDiagonal(int);

protected:

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void init();
	
	DkRotatingRect* rect;
	QPointF initialPos;
	QPointF posGrab;
	int parentIdx;
	QSize size;
};


class DkEditableRect : public DkWidget {
	Q_OBJECT

public:

	enum {
		do_nothing,
		initializing,
		rotating,
		moving,
		scaling
	};

	DkEditableRect(QRectF rect = QRect(), QWidget* parent = 0, Qt::WindowFlags f = 0);
	virtual ~DkEditableRect() {};

	void reset();

	void setWorldTransform(QTransform *worldTform) {
		this->worldTform = worldTform;
	};	

	void setImageTransform(QTransform *imgTform) {

		this->imgTform = imgTform;
	};

	void setImageRect(QRectF* imgRect) {
	
		this->imgRect = imgRect;
	};

	virtual void setVisible(bool visible);

signals:
	void enterPressedSignal(DkRotatingRect cropArea, const QColor& bgCol = QColor(0,0,0,0));
	void angleSignal(double angle);
	void aRatioSignal(const QPointF& aRatio);
	void statusInfoSignal(QString msg);

public slots:
	void updateCorner(int idx, QPointF point, bool isShiftDown, bool changeState = false);
	void updateDiagonal(int idx);
	void setFixedDiagonal(const DkVector& diag);
	void setAngle(double angle, bool apply = true);
	void setPanning(bool panning);
	void setPaintHint(int paintMode = DkCropToolBar::no_guide);
	void setShadingHint(bool invert);
	void setShowInfo(bool showInfo);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent* event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	QPointF clipToImage(const QPointF& pos);
	void applyTransform();
	void drawGuide(QPainter* painter, const QPolygonF& p, int paintMode);
	
	void paintEvent(QPaintEvent *event);

	QPointF map(const QPointF &pos);

	int state;

	QTransform *imgTform;
	QTransform *worldTform;
	QTransform tTform;
	QTransform rTform;
	QPointF posGrab;
	QPointF clickPos;
	DkVector oldDiag;
	DkVector fixedDiag;

	QWidget* parent;
	DkRotatingRect rect;
	QPen pen;
	QBrush brush;
	QVector<DkTransformRect*> ctrlPoints;
	QCursor rotatingCursor;
	QRectF* imgRect;
	bool panning;
	int paintMode;
	bool showInfo;
};

class DkCropWidget : public DkEditableRect {
	Q_OBJECT

public:
	DkCropWidget(QRectF rect = QRect(), QWidget* parent = 0, Qt::WindowFlags f = 0);

	virtual void setVisible(bool visible);
	DkCropToolBar* getToolbar() {
		return cropToolbar;
	}

public slots:
	void crop();

signals:
	void cancelSignal();
	void showToolbar(QToolBar* toolbar, bool show);

protected:
	void createToolbar();

	DkCropToolBar* cropToolbar;
};

/**
 * DkAnimationLabel
 * This code is based on: http://www.developer.nokia.com/Community/Wiki/CS001434_-_Creating_a_loading_animation_with_GIF,_QMovie,_and_QLabel
 *
 * Uses animation from the path
 * to display it in a DkLabel.
 */
class DllExport DkAnimationLabel : public DkLabel {

public:
	DkAnimationLabel(QString animationPath = QString(), QWidget* parent = 0);
	DkAnimationLabel(QString animationPath, QSize size, QWidget* parent);
	virtual ~DkAnimationLabel();

	virtual void showTimed(int time = 3000);
	virtual void hide();
	void halfSize();

private:
	
	QSize margin;
	QPointer<QMovie> animation;

	void init(const QString& animationPath, const QSize& size);

};

class DkColorChooser : public QWidget {
	Q_OBJECT

public:
	DkColorChooser(QColor defaultColor = QColor(), QString text = "Color", QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkColorChooser() {};

	void setColor(QColor color);
	QColor getColor();
	bool isAccept() {
		return accept;
	}

	void enableAlpha(bool enable = true) {
		colorDialog->setOption(QColorDialog::ShowAlphaChannel, enable);
	}

public slots:
	void on_resetButton_clicked();
	void on_colorButton_clicked();
	void on_colorDialog_accepted();

signals:
	void resetClicked();
	void accepted();

protected:
	QColorDialog* colorDialog;
	QPushButton* colorButton;

	QColor defaultColor;
	QString text;
	bool accept;

	void init();

};

// Image histogram display
class DkHistogram : public DkWidget {

	Q_OBJECT
	
public:
	DkHistogram(QWidget *parent);
	~DkHistogram();
	void drawHistogram(QImage img);
	void clearHistogram();
	void setMaxHistogramValue(long maxValue);
	void updateHistogramValues(long histValues[][256]);
	void setPainted(bool isPainted);

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void paintEvent(QPaintEvent* event);

private:
	QWidget* parent;
	long hist[3][256];
	long maxValue;
	bool isPainted;
	float scaleFactor;
				
};

class DkSlider : public QWidget {
	Q_OBJECT

public:
	DkSlider(QString title = "", QWidget* parent = 0);

	QSlider* getSlider() {
		return slider;
	};

	void setMinimum(int minValue) {
		slider->setMinimum(minValue);
		sliderBox->setMinimum(minValue);
		minValLabel->setText(QString::number(minValue));
	};

	void setMaximum(int maxValue) {
		slider->setMaximum(maxValue);
		sliderBox->setMaximum(maxValue);
		maxValLabel->setText(QString::number(maxValue));
	};

	void setTickInterval(int ticValue) {
		slider->setTickInterval(ticValue);
	};

	int value() {
		return slider->value();
	};

	void setFocus(Qt::FocusReason reason) {
		sliderBox->setFocus(reason);
	};


public slots:
void setValue(int value) {
		
	slider->blockSignals(true);
	slider->setValue(value);
	slider->blockSignals(false);

	sliderBox->blockSignals(true);
	sliderBox->setValue(value);
	sliderBox->blockSignals(false);

	emit valueChanged(value);
}

signals:
	void sliderMoved(int value);
	void valueChanged(int value);

protected:
	void createLayout();

	QLabel* titleLabel;
	QLabel* minValLabel;
	QLabel* maxValLabel;
	QSlider* slider;
	QSpinBox* sliderBox;
};

class DkFileInfo {

public:
	DkFileInfo();
	DkFileInfo(const QFileInfo& fileInfo);

	QFileInfo getFileInfo() const;
	bool exists() const;
	void setExists(bool fileExists);

	bool inUse() const;
	void setInUse(bool inUse);

protected:
	QFileInfo fileInfo;
	bool fileExists;
	bool used;
};

class DkFolderLabel : public QLabel {
	Q_OBJECT

public:
	DkFolderLabel(const DkFileInfo& fileInfo, QWidget* parent = 0, Qt::WindowFlags f = 0);

signals:
	void loadFileSignal(QFileInfo);

protected:
	void mousePressEvent(QMouseEvent *ev);

	DkFileInfo fileInfo;
};

class DkImageLabel : public QLabel {
	Q_OBJECT

public:
	DkImageLabel(const QFileInfo& fileInfo, QWidget* parent = 0, Qt::WindowFlags f = 0);

	bool hasFile() const;
	QSharedPointer<DkThumbNailT> getThumb() const;

signals:
	void labelLoaded();
	void loadFileSignal(QFileInfo);

public slots:
	void thumbLoaded();
	void removeFileFromList();

protected:
	void mousePressEvent(QMouseEvent *ev);
	void enterEvent(QEvent *ev);
	void leaveEvent(QEvent *ev);
	void createLayout();

	QLabel* imageLabel;
	QLabel* highLightLabel;
	QPushButton* removeFileButton;
	QSharedPointer<DkThumbNailT> thumb;
};

class DkRecentFilesWidget : public DkWidget {
	Q_OBJECT

public:
	DkRecentFilesWidget(QWidget* parent = 0);
	~DkRecentFilesWidget();
	static void mappedFileExists(DkFileInfo& fileInfo);
	void setCustomStyle(bool imgLoadedStyle = false);

signals:
	void loadFileSignal(QFileInfo fileInfo);

public slots:
	void updateFiles();
	void updateFolders();
	virtual void setVisible(bool visible);
	//void clearFileHistory();
	//void clearFolderHistory();

protected:
	void createLayout();
	void updateFileList();

	QVector<QFileInfo> recentFiles;
	QVector<DkFileInfo> recentFolders;
	//QFutureWatcher<void> fileWatcher;
	QFutureWatcher<void> folderWatcher;
	QVector<DkImageLabel*> fileLabels;
	QVector<DkFolderLabel*> folderLabels;

	QGridLayout* filesLayout;
	QVBoxLayout* folderLayout;

	QWidget* filesWidget;
	QWidget* folderWidget;

	QLabel* folderTitle;
	QLabel* filesTitle;
	QLabel* bgLabel;

	QLabel* clearFiles;
	QLabel* clearFolders;

	int rFileIdx;
	int numActiveLabels;
};



};
