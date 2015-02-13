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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QPushButton>
#include <QScrollBar>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QDockWidget>
#include <QPointer>
#include <QPen>
#include <QFutureWatcher>
#include <QLineEdit>
#pragma warning(pop)		// no warnings from includes - end

#pragma warning(disable: 4251)	// TODO: remove

#include "DkMath.h"
#include "DkBaseWidgets.h"

// Qt defines
class QColorDialog;
class QSpinBox;
class QDoubleSpinBox;
class QToolBar;
class QBoxLayout;
class QProgressDialog;
class QMovie;
class QTreeView;
class QSlider;
class QGridLayout;
class QVBoxLayout;

namespace nmc {

// nomacs defines
class DkThumbNail;
class DkThumbNailT;
class DkImageContainerT;
class DkCropToolBar;

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
	~DkRatingLabelBg();

	void changeRating(int newRating);
	QVector<QAction*> getActions() const;

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
	void play(bool play);
	void togglePlay();
	void startTimer();
	void autoNext();
	void next();
	void previous();
	virtual void show(int ms = 0);
	bool isPlaying() const;

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


class DkFolderScrollBar : public QScrollBar {
	Q_OBJECT

public: 
	DkFolderScrollBar(QWidget* parent = 0);
	~DkFolderScrollBar();

	virtual void setValue(int i);

	void registerAction(QAction* action);
	void block(bool blocked);
	void setDisplaySettings(QBitArray* displayBits);
	bool getCurrentDisplaySetting();

public slots:
	void updateDir(QVector<QSharedPointer<DkImageContainerT> > images);
	void updateFile(QSharedPointer<DkImageContainerT> imgC);
	void update(const QVector<QColor>& colors, const QVector<int>& indexes);

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

	DkFileSystemModel* getModel() { return fileModel; };

public slots:
	void setCurrentPath(QFileInfo fileInfo);
	void fileClicked(const QModelIndex &index) const;
	void showColumn(bool show);
	void setEditable(bool editable);

signals:
	void openFile(QFileInfo fileInfo) const;
	void openDir(QDir dir) const;

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

class DkOverview : public QLabel {
	Q_OBJECT

public:
	DkOverview(QWidget * parent = 0);
	~DkOverview() {};

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
		updateVirtualViewport();
	};

	void setVisible(bool visible) {

		if (visible)
			resizeImg();

		QWidget::setVisible(visible);
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
	QSizeF virtualVPSize;

	void resizeImg();
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent* event);
	QRectF getImageRect() const;
	void updateVirtualViewport();
	QTransform getScaledImageMatrix();
};

class DkZoomWidget : public DkFadeLabel {
	Q_OBJECT

public:
	DkZoomWidget(QWidget* parent = 0);

	DkOverview* getOverview() const;

	bool isAutoHide() const;

signals:
	void zoomSignal(float zoomLevel);

public slots:
	virtual void setVisible(bool visible, bool autoHide = false);

	void updateZoom(float zoomLevel);
	void on_sbZoom_valueChanged(double zoomLevel);
	void on_slZoom_valueChanged(int zoomLevel);

protected:
	DkOverview* overview;
	QSlider* slZoom;
	QDoubleSpinBox* sbZoom;
	bool autoHide;

	void createLayout();
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
		no_guide = 0,
		rule_of_thirds,
		grid,

		mode_end,
	};

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
	void setPaintHint(int paintMode = no_guide);
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

	DkCropToolBar* getToolbar() const;

public slots:
	void crop();
	virtual void setVisible(bool visible);

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
	bool isAccept() const;
	void enableAlpha(bool enable = true);

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

	QSlider* getSlider() const;
	void setMinimum(int minValue);
	void setMaximum(int maxValue);
	void setTickInterval(int ticValue);
	int value() const;
	void setFocus(Qt::FocusReason reason);

public slots:
	void setValue(int value);

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
	virtual void hide();
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


class DkDirectoryEdit : public QLineEdit {
	Q_OBJECT

	public:	
		DkDirectoryEdit(QWidget* parent = 0);
		DkDirectoryEdit(QString content, QWidget* parent = 0);
		
		bool existsDirectory() { return existsDirectory(text());};

	signals:
		bool directoryChanged(QDir path);

	public slots:
		void lineEditChanged(QString path);

	private:
		bool existsDirectory(QString path);
		QString oldPath;
};

};
