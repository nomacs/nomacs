/*******************************************************************************************************
 DkWidgets.cpp
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

#include "DkWidgets.h"

#include "DkActionManager.h"
#include "DkDialog.h"
#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkStatusBar.h"
#include "DkThumbs.h"
#include "DkTimer.h"
#include "DkToolbars.h"
#include "DkUtils.h"
#include <QtGlobal>

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QFuture>
#include <QFutureWatcher>
#include <QGraphicsBlurEffect>
#include <QGraphicsItem>
#include <QGraphicsItemAnimation>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QMovie>
#include <QObject>
#include <QPainter>
#include <QPointer>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStringBuilder>
#include <QSvgRenderer>
#include <QThread>
#include <QTimeLine>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVector2D>
#include <QtConcurrentRun>
#include <qmath.h>
#include <qtconcurrentmap.h>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

// DkFolderScrollBar --------------------------------------------------------------------
DkFolderScrollBar::DkFolderScrollBar(QWidget *parent)
    : DkFadeMixin<QSlider>(parent)
{
    setObjectName("DkFolderScrollBar");
    setOrientation(Qt::Horizontal);
    init();
    mMouseDown = false;
}

DkFolderScrollBar::~DkFolderScrollBar()
{
}

void DkFolderScrollBar::updateDir(QVector<QSharedPointer<DkImageContainerT>> images)
{
    setMaximum(images.size() - 1);
}

void DkFolderScrollBar::updateFile(int idx)
{
    if (mMouseDown)
        return;

    if (isVisible()) {
        blockSignals(true);
        QSlider::setValue(idx);
        blockSignals(false);
    }
}

void DkFolderScrollBar::setValue(int idx)
{
    QSlider::setValue(idx);
}

void DkFolderScrollBar::mousePressEvent(QMouseEvent *event)
{
    blockSignals(true);
    QSlider::mousePressEvent(event);
}

void DkFolderScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
    mMouseDown = false;
    blockSignals(false);
    emit valueChanged(value());
    QSlider::mouseReleaseEvent(event);
}

void DkFolderScrollBar::init()
{
    setMouseTracking(true);

    mBgCol = (DkSettingsManager::param().app().appMode == DkSettings::mode_frameless) ? DkSettingsManager::param().display().bgColorFrameless
                                                                                      : DkSettingsManager::param().display().hudBgColor;
}

// DkThumbsSaver --------------------------------------------------------------------
DkThumbsSaver::DkThumbsSaver(QWidget *parent)
    : DkWidget(parent)
{
    mStop = false;
    mNumSaved = 0;
}

void DkThumbsSaver::processDir(QVector<QSharedPointer<DkImageContainerT>> images, bool forceSave)
{
    if (images.empty())
        return;

    mStop = false;
    mNumSaved = 0;

    mPd = new QProgressDialog(tr("\nCreating thumbnails...\n") + images.first()->filePath(), tr("Cancel"), 0, (int)images.size(), DkUtils::getMainWindow());
    mPd->setWindowTitle(tr("Thumbnails"));

    // pd->setWindowModality(Qt::WindowModal);

    connect(this, &DkThumbsSaver::numFilesSignal, mPd, &QProgressDialog::setValue);
    connect(mPd, &QProgressDialog::canceled, this, &DkThumbsSaver::stopProgress);

    mPd->show();

    this->mForceSave = forceSave;
    this->mImages = images;
    loadNext();
}

void DkThumbsSaver::thumbLoaded(bool)
{
    mNumSaved++;
    emit numFilesSignal(mNumSaved);

    if (mNumSaved == mImages.size() || mStop) {
        if (mPd) {
            mPd->close();
            mPd->deleteLater();
            mPd = 0;
        }
        mStop = true;
    } else
        loadNext();
}

void DkThumbsSaver::loadNext()
{
    if (mStop)
        return;

    auto mode = mForceSave ? DkThumbNail::write_exif_always : DkThumbNail::write_exif;

    for (int idx = 0; idx < mImages.size(); idx++) {
        connect(mImages.at(idx)->getThumb().data(), &DkThumbNailT::thumbLoadedSignal, this, &DkThumbsSaver::thumbLoaded);
        mImages.at(idx)->getThumb()->fetchThumb(mode);
    }
}

void DkThumbsSaver::stopProgress()
{
    mStop = true;
}

// DkFileSystemModel --------------------------------------------------------------------
DkFileSystemModel::DkFileSystemModel(QObject *parent /* = 0 */)
    : QFileSystemModel(parent)
{
    // some custom settings
    setRootPath(QDir::rootPath());
    setNameFilters(DkSettingsManager::param().app().fileFilters);
    setReadOnly(false);

    mIconProvider = new QFileIconProvider();
    setIconProvider(mIconProvider);
}

// DkSortFileProxyModel --------------------------------------------------------------------
DkSortFileProxyModel::DkSortFileProxyModel(QObject *parent /* = 0 */)
    : QSortFilterProxyModel(parent)
{
}

bool DkSortFileProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.data().canConvert(QMetaType::QUrl)) {
        QFileInfo lf = QFileInfo(left.data().toString());
        QFileInfo rf = QFileInfo(right.data().toString());

        // could not find a better way to tell files from dirs appart (isDir() is not what we expect)
        if (lf.suffix().isEmpty() && !rf.suffix().isEmpty())
            return true;
        else if (!lf.suffix().isEmpty() && rf.suffix().isEmpty())
            return false;

        QString ls = (!lf.fileName().isEmpty()) ? lf.fileName() : lf.absoluteFilePath(); // otherwise e.g. C: is empty
        QString rs = (!rf.fileName().isEmpty()) ? rf.fileName() : rf.absoluteFilePath();

        QString ld = ls.section(QRegularExpression("[A-Z]:"), 1, -1, QString::SectionIncludeLeadingSep);

        // sort by drive letter if present
        if (!ld.isEmpty()) {
            ld.truncate(2);
            ls = ld;
        }

        QString rd = rs.section(QRegularExpression("[A-Z]:"), 1, -1, QString::SectionIncludeLeadingSep);

        // sort by drive letter if present
        if (!rd.isEmpty()) {
            rd.truncate(2);
            rs = rd;
        }

        return DkUtils::compLogicQString(ls, rs);
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

// -------------------------------------------------------------------- DkBrowseExplorer
DkBrowseExplorer::DkBrowseExplorer(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : DkExplorer(title, parent, flags)
{
    createLayout();
    readSettings();
}

DkBrowseExplorer::~DkBrowseExplorer()
{
    writeSettings();
}

void DkBrowseExplorer::browseClicked()
{
    QString root = QFileDialog::getExistingDirectory(this, tr("Choose Root Directory"), mRootPath, QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

    if (root != "")
        setRootPath(root);
}

void DkBrowseExplorer::scrollToCurrentClicked()
{
    mFileTree->scrollTo(mFileTree->currentIndex());
}

void DkBrowseExplorer::setRootPath(const QString &root)
{
    mRootPath = root;
    mFileTree->setRootIndex(mSortModel->mapFromSource(mFileModel->index(root)));
    mFileModel->setRootPath(root);
    mRootPathLabel->setText(root);
    mRootPathLabel->setToolTip(root);
}

void DkBrowseExplorer::createLayout()
{
    QWidget *rootPathWidget = new QWidget(this);
    QHBoxLayout *rpLayout = new QHBoxLayout(rootPathWidget);
    mRootPathLabel = new DkElidedLabel(rootPathWidget, "");

    auto browseButton = new QPushButton(tr("Browse"));
    browseButton->setFocusPolicy(Qt::ClickFocus);
    connect(browseButton, &QPushButton::clicked, this, &DkBrowseExplorer::browseClicked);

    auto scrollButton = new QToolButton();
    scrollButton->setIcon(DkImage::loadIcon(":/nomacs/img/scroll-to-current.svg"));
    scrollButton->setToolTip(tr("Scroll to current file"));
    scrollButton->setFocusPolicy(Qt::ClickFocus);
    connect(scrollButton, &QPushButton::clicked, this, &DkBrowseExplorer::scrollToCurrentClicked);

    rpLayout->setContentsMargins(4, 2, 2, 2);
    rpLayout->addWidget(mRootPathLabel, 1);
    rpLayout->addWidget(browseButton);
    rpLayout->addWidget(scrollButton);

    mLayout->insertWidget(0, rootPathWidget);
}

void DkBrowseExplorer::readSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());
    setRootPath(settings.value("RootPath", QDir::homePath()).toString());
    settings.endGroup();
}

void DkBrowseExplorer::writeSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("RootPath", mRootPath);
    settings.endGroup();
}

// DkExplorer --------------------------------------------------------------------
DkExplorer::DkExplorer(const QString &title, QWidget *parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
    : DkDockWidget(title, parent, flags)
{
    setObjectName("DkExplorer");
    createLayout();
    readSettings();

    // open selected images
    QAction *selAction = new QAction(tr("Open Image"), this);
    selAction->setShortcut(Qt::Key_Return);
    selAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(selAction, &QAction::triggered, this, &DkExplorer::openSelected);

    connect(mFileTree, &QTreeView::clicked, this, &DkExplorer::fileClicked);

    addAction(selAction);

    if (mLoadSelected)
        connect(mFileTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &DkExplorer::fileClicked, Qt::UniqueConnection);
}

DkExplorer::~DkExplorer()
{
    // save settings
    writeSettings();
}

void DkExplorer::createLayout()
{
    mFileModel = new DkFileSystemModel(this);

    mSortModel = new DkSortFileProxyModel(this);
    mSortModel->setSourceModel(mFileModel);
    mSortModel->setSortLocaleAware(true);

    mFileTree = new QTreeView(this);
    mFileTree->setSortingEnabled(true);
    mFileTree->setModel(mSortModel);
    mFileTree->setDragEnabled(true);
    mFileTree->setAcceptDrops(true);
    mFileTree->setFocusPolicy(Qt::ClickFocus);

    // by default descendingOrder is set
    mFileTree->header()->setSortIndicator(0, Qt::AscendingOrder);
    mFileTree->header()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    QWidget *widget = new QWidget(this);
    mLayout = new QVBoxLayout(widget);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    mLayout->addWidget(mFileTree);
    setWidget(widget);
}

void DkExplorer::setCurrentImage(QSharedPointer<DkImageContainerT> img)
{
    if (!img)
        return;

    setCurrentPath(img->filePath());
}

void DkExplorer::setCurrentPath(const QString &filePath)
{
    // expand folders
    if (QFileInfo(filePath).isDir())
        mFileTree->expand(mSortModel->mapFromSource(mFileModel->index(filePath)));

    mFileTree->setCurrentIndex(mSortModel->mapFromSource(mFileModel->index(filePath)));
}

void DkExplorer::fileClicked(const QModelIndex &index) const
{
    QFileInfo cFile = mFileModel->fileInfo(mSortModel->mapToSource(index));

    qDebug() << "opening: " << cFile.absoluteFilePath();

    if (DkUtils::isValid(cFile))
        emit openFile(cFile.absoluteFilePath());
    else if (cFile.isDir())
        emit openDir(cFile.absoluteFilePath());
}

void DkExplorer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *cm = new QMenu(this);

    // enable editing
    QAction *editAction = new QAction(tr("Editable"), this);
    editAction->setCheckable(true);
    editAction->setChecked(!mFileModel->isReadOnly());
    connect(editAction, &QAction::triggered, this, &DkExplorer::setEditable);

    // open selected images
    QAction *selAction = new QAction(tr("Open Selected Image"), this);
    selAction->setCheckable(true);
    selAction->setChecked(mLoadSelected);
    connect(selAction, &QAction::triggered, this, &DkExplorer::loadSelectedToggled);

    cm->addAction(editAction);
    cm->addAction(selAction);
    cm->addSeparator();

    // adjust sizes
    QAction *sizeAction = new QAction(tr("Adjust Columns"), this);
    connect(sizeAction, &QAction::triggered, this, &DkExplorer::adjustColumnWidth);

    cm->addAction(sizeAction);
    cm->addSeparator();

    mColumnActions.clear(); // quick&dirty

    for (int idx = 0; idx < mFileModel->columnCount(); idx++) {
        QAction *action = new QAction(mFileModel->headerData(idx, Qt::Horizontal).toString(), this);
        action->setCheckable(true);
        action->setChecked(!mFileTree->isColumnHidden(idx));
        action->setObjectName(QString::number(idx));

        connect(action, &QAction::toggled, this, &DkExplorer::showColumn);
        mColumnActions.push_back(action);

        cm->addAction(action);
    }

    cm->exec(event->globalPos());
}

void DkExplorer::showColumn(bool show)
{
    bool ok = false;
    int idx = QObject::sender()->objectName().toInt(&ok);

    if (!ok)
        return;

    mFileTree->setColumnHidden(idx, !show);
}

void DkExplorer::loadSelectedToggled(bool checked)
{
    mLoadSelected = checked;

    if (mLoadSelected)
        connect(mFileTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &DkExplorer::fileClicked, Qt::UniqueConnection);
    else
        disconnect(mFileTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &DkExplorer::fileClicked);
}

void DkExplorer::openSelected()
{
    auto index = mFileTree->selectionModel()->currentIndex();
    QFileInfo cFile = mFileModel->fileInfo(mSortModel->mapToSource(index));
    qDebug() << "opening: " << cFile.absoluteFilePath();

    if (DkUtils::isValid(cFile))
        emit openFile(cFile.absoluteFilePath());
    else if (cFile.isDir())
        emit openDir(cFile.absoluteFilePath());
}

void DkExplorer::setEditable(bool editable)
{
    mFileModel->setReadOnly(!editable);
}

void DkExplorer::adjustColumnWidth()
{
    for (int idx = 0; idx < mFileTree->model()->columnCount(); idx++)
        mFileTree->resizeColumnToContents(idx);
    qDebug() << "size adjusted...";
}

void DkExplorer::closeEvent(QCloseEvent *event)
{
    writeSettings();
    DkDockWidget::closeEvent(event);
}

void DkExplorer::writeSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());

    for (int idx = 0; idx < mFileModel->columnCount(QModelIndex()); idx++) {
        QString headerVal = mFileModel->headerData(idx, Qt::Horizontal).toString();
        settings.setValue(headerVal + "Size", mFileTree->columnWidth(idx));
        settings.setValue(headerVal + "Hidden", mFileTree->isColumnHidden(idx));
    }

    settings.setValue("LoadSelected", mLoadSelected);
    settings.setValue("ReadOnly", mFileModel->isReadOnly());
    settings.endGroup();
}

void DkExplorer::readSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());

    for (int idx = 0; idx < mFileModel->columnCount(QModelIndex()); idx++) {
        QString headerVal = mFileModel->headerData(idx, Qt::Horizontal).toString();

        int colWidth = settings.value(headerVal + "Size", -1).toInt();
        if (colWidth != -1)
            mFileTree->setColumnWidth(idx, colWidth);

        bool showCol = idx != 0; // by default, show the first column only
        mFileTree->setColumnHidden(idx, settings.value(headerVal + "Hidden", showCol).toBool());
    }

    mLoadSelected = settings.value("LoadSelected", mLoadSelected).toBool();
    mFileModel->setReadOnly(settings.value("ReadOnly", true).toBool());
    settings.endGroup();
}

// DkOverview --------------------------------------------------------------------
DkOverview::DkOverview(QWidget *parent)
    : QLabel(parent)
{
    setObjectName("DkOverview");
    setMinimumSize(0, 0);
    setMaximumSize(200, 200);
    setCursor(Qt::ArrowCursor);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void DkOverview::paintEvent(QPaintEvent *event)
{
    if (mImgT.isNull()) {
        mImgT = resizedImg(mImg);
        mImg = QImage(); // free-up space
    }

    if (!mImgMatrix || !mWorldMatrix)
        return;

    QPainter painter(this);

    int lm = 0;
    int tm = 0;
    int rm = 0;
    int bm = 0;
    if (layout() != nullptr)
        layout()->getContentsMargins(&lm, &tm, &rm, &bm);

    QSize viewSize = QSize(width() - lm - rm, height() - tm - bm); // overview shall take 15% of the mViewport....

    if (viewSize.width() > 2 && viewSize.height() > 2) {
        QTransform overviewImgMatrix = getScaledImageMatrix(); // matrix that always resizes the image to the current mViewport
        QRectF overviewImgRect = getScaledImageMatrix().mapRect(QRectF(QPointF(), mImgSize));

        // now render the current view
        QRectF viewRect = mViewPortRect;
        viewRect = mWorldMatrix->inverted().mapRect(viewRect);
        viewRect = mImgMatrix->inverted().mapRect(viewRect);
        viewRect = overviewImgMatrix.mapRect(viewRect);
        viewRect.moveTopLeft(viewRect.topLeft() + QPointF(lm, tm));

        if (viewRect.topLeft().x() < overviewImgRect.topLeft().x())
            viewRect.setTopLeft(QPointF(overviewImgRect.topLeft().x(), viewRect.topLeft().y()));
        if (viewRect.topLeft().y() < overviewImgRect.topLeft().y())
            viewRect.setTopLeft(QPointF(viewRect.topLeft().x(), overviewImgRect.topLeft().y()));
        if (viewRect.bottomRight().x() > overviewImgRect.bottomRight().x())
            viewRect.setBottomRight(QPointF(overviewImgRect.bottomRight().x() - 1, viewRect.bottomRight().y()));
        if (viewRect.bottomRight().y() > overviewImgRect.bottomRight().y())
            viewRect.setBottomRight(QPointF(viewRect.bottomRight().x(), overviewImgRect.bottomRight().y() - 1));

        // draw the image's location
        painter.setRenderHints(QPainter::SmoothPixmapTransform);
        painter.setBrush(DkSettingsManager::param().display().hudBgColor);
        painter.setPen(QColor(200, 200, 200));
        // painter.drawRect(overviewRect);
        painter.setOpacity(0.8f);
        painter.drawImage(overviewImgRect, mImgT, QRect(0, 0, mImgT.width(), mImgT.height()));

        QColor col = DkSettingsManager::param().display().highlightColor;
        col.setAlpha(255);
        painter.setPen(col);
        col.setAlpha(50);
        painter.setBrush(col);

        if (viewRect.width() + 1 < overviewImgRect.width()
            || viewRect.height() + 1 < overviewImgRect.height()) // draw viewrect if we do not see all parts of the image
            painter.drawRect(viewRect);
    }
    painter.end();

    QWidget::paintEvent(event);
}

void DkOverview::mousePressEvent(QMouseEvent *event)
{
    mEnterPos = event->pos();
    mPosGrab = event->pos();
}

void DkOverview::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF dxy = mEnterPos - QPointF(event->pos());

    if (dxy.manhattanLength() < 4) {
        int lm = 0;
        int tm = 0;
        int rm = 0;
        int bm = 0;
        if (layout() != nullptr)
            layout()->getContentsMargins(&lm, &tm, &rm, &bm);

        // move to the current position
        QRectF viewRect = mViewPortRect;
        viewRect = mWorldMatrix->inverted().mapRect(viewRect);
        viewRect = mImgMatrix->inverted().mapRect(viewRect);
        viewRect = getScaledImageMatrix().mapRect(viewRect);
        QPointF currentViewPoint = viewRect.center();

        float panningSpeed = (float)-(mWorldMatrix->m11() / (getScaledImageMatrix().m11() / mImgMatrix->m11()));

        QPointF cPos = event->pos() - QPointF(lm, tm);
        QPointF lDxy = (cPos - currentViewPoint) / mWorldMatrix->m11() * panningSpeed;
        emit moveViewSignal(lDxy);

        if (event->modifiers() == DkSettingsManager::param().global().altMod)
            emit sendTransformSignal();
    }
}

void DkOverview::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() != Qt::LeftButton)
        return;

    float panningSpeed = (float)-(mWorldMatrix->m11() / (getScaledImageMatrix().m11() / mImgMatrix->m11()));

    QPointF cPos = event->pos();
    QPointF dxy = (cPos - mPosGrab) / mWorldMatrix->m11() * panningSpeed;
    mPosGrab = cPos;
    emit moveViewSignal(dxy);

    if (event->modifiers() == DkSettingsManager::param().global().altMod)
        emit sendTransformSignal();
}

QRectF DkOverview::getImageRect() const
{
    QRectF imgRect = QRectF(QPoint(), size()); // get the overview rect

    if ((float)mImgT.width() / mImgT.height() < (float)imgRect.width() / imgRect.height())
        imgRect.setWidth(width() * (float)height() / (float)mImgT.height());
    else
        imgRect.setHeight(height() * (float)width() / (float)mImgT.width());

    return imgRect;
}

QImage DkOverview::resizedImg(const QImage &src)
{
    if (src.isNull())
        return QImage();

    QTransform overviewImgMatrix = getScaledImageMatrix(); // matrix that always resizes the image to the current mViewport

    // is the overviewImgMatrix empty?
    if (overviewImgMatrix.isIdentity())
        return src;

    // fast downscaling
    QImage sImg = src.scaled(maximumWidth() * 2, maximumHeight() * 2, Qt::KeepAspectRatio, Qt::FastTransformation);
    sImg = sImg.scaled(maximumWidth(), maximumHeight(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return sImg;
}

QTransform DkOverview::getScaledImageMatrix()
{
    if (mImgT.isNull() && mImg.isNull())
        return QTransform();

    int lm = 0;
    int tm = 0;
    int rm = 0;
    int bm = 0;
    if (layout() != nullptr)
        layout()->getContentsMargins(&lm, &tm, &rm, &bm);

    QSize iSize = QSize(width() - lm - rm, height() - tm - bm); // inner size

    if (iSize.width() < 2 || iSize.height() < 2)
        return QTransform();

    // the image resizes as we zoom
    QRectF imgRect = QRectF(QPoint(lm, tm), mImgSize);
    float ratioImg = (float)(imgRect.width() / imgRect.height());
    float ratioWin = (float)(iSize.width()) / (float)(iSize.height());

    QTransform imgMatrix;
    float s;
    if (imgRect.width() == 0 || imgRect.height() == 0)
        s = 1.0f;
    else
        s = (ratioImg > ratioWin) ? (float)(iSize.width() / imgRect.width()) : (float)(iSize.height() / imgRect.height());

    imgMatrix.scale(s, s);

    QRectF imgViewRect = imgMatrix.mapRect(imgRect);
    imgMatrix.translate((iSize.width() - imgViewRect.width()) * 0.5f / s, (iSize.height() - imgViewRect.height()) * 0.5f / s);

    return imgMatrix;
}

// DkZoomWidget --------------------------------------------------------------------
DkZoomWidget::DkZoomWidget(QWidget *parent)
    : DkFadeLabel("", parent)
{
    setObjectName("DkZoomWidget");
    createLayout();

    setMinimumSize(70, 0);
    setMaximumSize(200, 240);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void DkZoomWidget::createLayout()
{
    mOverview = new DkOverview(this);

    mSlZoom = new QSlider(Qt::Horizontal, this);
    mSlZoom->setObjectName("slZoom");
    mSlZoom->setCursor(Qt::ArrowCursor);
    mSlZoom->setMinimum(0); // add a mapping here
    mSlZoom->setMaximum(100);
    connect(mSlZoom, &QSlider::valueChanged, this, &DkZoomWidget::onSlZoomValueChanged);

    mSbZoom = new QDoubleSpinBox(this);
    mSbZoom->setObjectName("sbZoom");
    mSbZoom->setButtonSymbols(QAbstractSpinBox::NoButtons);
    mSbZoom->setSuffix("%");
    mSbZoom->setDecimals(0);
    mSbZoom->setValue(100);
    mSbZoom->setMinimum(0.2);
    mSbZoom->setMaximum(6000);
    connect(mSbZoom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DkZoomWidget::onSbZoomValueChanged);

    QLabel *sliderWidget = new QLabel(this);
    sliderWidget->setObjectName("DkOverviewSliderWidget");
    QHBoxLayout *sliderLayout = new QHBoxLayout(sliderWidget);
    sliderLayout->setContentsMargins(10, 0, 0, 0);
    sliderLayout->setSpacing(10);
    sliderLayout->addWidget(mSlZoom);
    sliderLayout->addWidget(mSbZoom);

    QVBoxLayout *layout = new QVBoxLayout(this);
    // mLayout->setContentsMargins(10,10,10,10);
    layout->setSpacing(0);
    layout->addWidget(mOverview);
    layout->addWidget(sliderWidget);
}

void DkZoomWidget::onSbZoomValueChanged(double zoomLevel)
{
    updateZoom((float)zoomLevel);
    emit zoomSignal(zoomLevel / 100.0);
}

void DkZoomWidget::onSlZoomValueChanged(int zoomLevel)
{
    double level = (zoomLevel > 50) ? (zoomLevel - 50.0) / 50.0 * mSbZoom->maximum() + 200.0 : zoomLevel * 4.0;
    if (level < 0.2)
        level = 0.2;
    updateZoom(level);
    emit zoomSignal(level / 100.0);
}

void DkZoomWidget::updateZoom(double zoomLevel)
{
    mSlZoom->blockSignals(true);
    mSbZoom->blockSignals(true);

    int slVal = (zoomLevel > 200.0) ? qRound(zoomLevel / mSbZoom->maximum() * 50.0 + 50.0) : qRound(zoomLevel * 0.25);
    mSlZoom->setValue(slVal);
    mSbZoom->setValue(zoomLevel);
    mSlZoom->blockSignals(false);
    mSbZoom->blockSignals(false);
}

DkOverview *DkZoomWidget::getOverview() const
{
    return mOverview;
}

// DkButton --------------------------------------------------------------------
DkButton::DkButton(QWidget *parent)
    : QPushButton(parent)
{
}

DkButton::DkButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
}

DkButton::DkButton(const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton(icon, text, parent)
{
    checkedIcon = icon;
    setText(text);

    init();
}

DkButton::DkButton(const QIcon &checkedIcon, const QIcon &uncheckedIcon, const QString &text, QWidget *parent)
    : QPushButton(checkedIcon, text, parent)
{
    this->checkedIcon = checkedIcon;
    this->uncheckedIcon = uncheckedIcon;
    this->setCheckable(true);
    setText(text);

    init();
}

void DkButton::init()
{
    setIcon(checkedIcon);

    if (!checkedIcon.availableSizes().empty())
        this->setMaximumSize(checkedIcon.availableSizes()[0]); // crashes if the image is empty!!

    mouseOver = false;
    keepAspectRatio = true;
}

void DkButton::setFixedSize(QSize size)
{
    mySize = size;
    this->setMaximumSize(size);
}

void DkButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPoint offset;
    QSize s;
    float opacity = 1.0f;

    if (!isEnabled())
        opacity = 0.5f;
    else if (!mouseOver)
        opacity = 0.7f;

    painter.setOpacity(opacity);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!mySize.isEmpty()) {
        offset = QPoint(qRound((float)(size().width() - mySize.width()) * 0.5f), qRound((float)(size().height() - mySize.height()) * 0.5f));
        s = mySize;
    } else
        s = this->size();

    // scale to parent label
    QRect r = (keepAspectRatio) ? QRect(offset, checkedIcon.actualSize(s)) : QRect(offset, s); // actual size preserves the aspect ratio
    QPixmap pm2draw;

    if (isChecked() || !isCheckable())
        pm2draw = checkedIcon.pixmap(s);
    else
        pm2draw = uncheckedIcon.pixmap(s);

    if (this->isDown()) {
        QPixmap effect = createSelectedEffect(&pm2draw);
        painter.drawPixmap(r, effect);
    }

    painter.drawPixmap(r, pm2draw);
    painter.end();
}

QPixmap DkButton::createSelectedEffect(QPixmap *pm)
{
    if (!pm || pm->isNull())
        return QPixmap();

    return DkImage::colorizePixmap(*pm, DkSettingsManager::param().display().highlightColor, 1.0f);
}

void DkButton::focusInEvent(QFocusEvent *)
{
    mouseOver = true;
}

void DkButton::focusOutEvent(QFocusEvent *)
{
    mouseOver = false;
}

void DkButton::enterEvent(DkEnterEvent *)
{
    mouseOver = true;
}

void DkButton::leaveEvent(QEvent *)
{
    mouseOver = false;
}

// star label --------------------------------------------------------------------
DkRatingLabel::DkRatingLabel(int rating, QWidget *parent, Qt::WindowFlags flags)
    : DkWidget(parent, flags)
{
    setObjectName("DkRatingLabel");
    mRating = rating;
    init();

    int iconSize = 16;

    mLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(3);
    mLayout->addStretch();

    for (int idx = 0; idx < mStars.size(); idx++) {
        mStars[idx]->setFixedSize(QSize(iconSize, iconSize));
        mLayout->addWidget(mStars[idx]);
    }

    setLayout(mLayout);
}

void DkRatingLabel::init()
{
    QIcon starDark = DkImage::loadIcon(":/nomacs/img/star-off.svg", QSize(), DkSettingsManager::param().display().hudFgdColor);
    QIcon starWhite = DkImage::loadIcon(":/nomacs/img/star-on.svg", QSize(), DkSettingsManager::param().display().hudFgdColor);

    mStars.resize(5);

    // TODO: make this into a for loop?
    mStars[rating_1] = new DkButton(starWhite, starDark, tr("one star"), this);
    mStars[rating_1]->setCheckable(true);
    connect(mStars[rating_1], &DkButton::released, this, &DkRatingLabel::rating1);

    mStars[rating_2] = new DkButton(starWhite, starDark, tr("two stars"), this);
    mStars[rating_2]->setCheckable(true);
    connect(mStars[rating_2], &DkButton::released, this, &DkRatingLabel::rating2);

    mStars[rating_3] = new DkButton(starWhite, starDark, tr("three star"), this);
    mStars[rating_3]->setCheckable(true);
    connect(mStars[rating_3], &DkButton::released, this, &DkRatingLabel::rating3);

    mStars[rating_4] = new DkButton(starWhite, starDark, tr("four star"), this);
    mStars[rating_4]->setCheckable(true);
    connect(mStars[rating_4], &DkButton::released, this, &DkRatingLabel::rating4);

    mStars[rating_5] = new DkButton(starWhite, starDark, tr("five star"), this);
    mStars[rating_5]->setCheckable(true);
    connect(mStars[rating_5], &DkButton::released, this, &DkRatingLabel::rating5);
}

// title info + star label -------------------------------------------------------
DkFileInfoLabel::DkFileInfoLabel(QWidget *parent)
    : DkFadeLabel("", parent)
{
    setObjectName("DkFileInfoLabel");
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    mTitleLabel = new QLabel(this);
    mTitleLabel->setMouseTracking(true);
    mTitleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mDateLabel = new QLabel(this);
    mDateLabel->setMouseTracking(true);
    mDateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mRatingLabel = new DkRatingLabel(0, this);
    setMinimumWidth(110);
    setCursor(Qt::ArrowCursor);

    createLayout();
}

void DkFileInfoLabel::createLayout()
{
    mLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    mLayout->setSpacing(2);

    mLayout->addWidget(mTitleLabel);
    mLayout->addWidget(mDateLabel);
    mLayout->addWidget(mRatingLabel);
    // mLayout->addStretch();
}

void DkFileInfoLabel::setVisible(bool visible, bool saveSettings)
{
    DkFadeLabel::setVisible(visible, saveSettings);
    if (mSetWidgetVisible)
        return; // prevent recursion via fade()

    // FIXME: this block is disabled because if you answer NO it would just keep recursing
    //        through setVisible(). Also the setting is not saved to file and no UI for it exists.
    //        To fix this it should have a context menu like other panels
    if (0) {
        // nothing to display??
        if (!DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_file_name)
            && !DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_creation_date)
            && !DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_file_rating) && visible) {
            QMessageBox infoDialog(DkUtils::getMainWindow());
            infoDialog.setWindowTitle(tr("Info Box"));
            infoDialog.setText(tr("All information fields are currently hidden.\nDo you want to show them again?"));
            infoDialog.setIcon(QMessageBox::Information);
            infoDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            infoDialog.setDefaultButton(QMessageBox::Yes);
            infoDialog.show();
            int choice = infoDialog.exec();

            if (choice == QMessageBox::No) {
                DkFadeLabel::setVisible(false);
                return;
            } else {
                DkSettingsManager::param().slideShow().display.setBit(DkSettings::display_file_name, true);
                DkSettingsManager::param().slideShow().display.setBit(DkSettings::display_creation_date, true);
                DkSettingsManager::param().slideShow().display.setBit(DkSettings::display_file_rating, true);
            }
        }

        mTitleLabel->setVisible(DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_file_name));
        mDateLabel->setVisible(DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_creation_date));
        mRatingLabel->setVisible(DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_file_rating));
    }

    int height = 32;
    if (mTitleLabel->isVisible())
        height += mTitleLabel->sizeHint().height();
    if (mDateLabel->isVisible())
        height += mDateLabel->sizeHint().height();
    if (mRatingLabel->isVisible())
        height += mRatingLabel->sizeHint().height();

    setMinimumHeight(height);
    updateWidth();
}

void DkFileInfoLabel::setEdited(bool edited)
{
    if (!isVisible() || !edited)
        return;

    QString cFileName = mTitleLabel->text() + "*";
    this->mTitleLabel->setText(cFileName);
}

DkRatingLabel *DkFileInfoLabel::getRatingLabel()
{
    return mRatingLabel;
}

void DkFileInfoLabel::updateInfo(const QString &filePath, const QString &attr, const QString &date, const int rating)
{
    mFilePath = filePath;
    updateTitle(filePath, attr);
    updateDate(date);
    updateRating(rating);

    updateWidth();
}

void DkFileInfoLabel::updateTitle(const QString &filePath, const QString &attr)
{
    updateDate();
    mTitleLabel->setText(QFileInfo(filePath).fileName() + " " + attr);
    mTitleLabel->setAlignment(Qt::AlignRight);

    updateWidth();
}

void DkFileInfoLabel::updateDate(const QString &date)
{
    QString dateConverted = DkUtils::convertDateString(date, QFileInfo(mFilePath));

    mDateLabel->setText(dateConverted);
    mDateLabel->setAlignment(Qt::AlignRight);

    updateWidth();
}

void DkFileInfoLabel::updateRating(const int rating)
{
    mRatingLabel->setRating(rating);
}

void DkFileInfoLabel::updateWidth()
{
    int width = 20; // mar
    width += qMax(qMax(mTitleLabel->sizeHint().width(), mDateLabel->sizeHint().width()), mRatingLabel->sizeHint().width());

    if (width < minimumWidth())
        setMinimumWidth(width);

    setMaximumWidth(width);
}

// player --------------------------------------------------------------------
DkPlayer::DkPlayer(QWidget *parent)
    : DkFadeWidget(parent)
{
    init();
    createLayout();
}

void DkPlayer::createLayout()
{
    int height = 50;
    QSize ih(height - 12, height - 12);

    previousButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/previous.svg", ih, Qt::white), "", this);
    // previousButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previousButton->setIconSize(ih);
    previousButton->setMinimumSize(QSize(qRound(1.5 * height), height));
    previousButton->setToolTip(tr("Show previous image"));
    previousButton->setObjectName("DkPlayerButton");
    previousButton->setFlat(true);
    connect(previousButton, &QPushButton::pressed, this, &DkPlayer::previous);

    QIcon icon;
    icon.addPixmap(DkImage::loadIcon(":/nomacs/img/pause.svg", ih, Qt::white), QIcon::Normal, QIcon::On);
    icon.addPixmap(DkImage::loadIcon(":/nomacs/img/play.svg", ih, Qt::white), QIcon::Normal, QIcon::Off);
    playButton = new QPushButton(icon, "", this);
    // playButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    playButton->setIconSize(ih);
    playButton->setMinimumSize(QSize(qRound(1.5 * height), height));
    playButton->setToolTip(tr("Play/Pause"));
    playButton->setObjectName("DkPlayerButton");
    playButton->setFlat(true);
    playButton->setCheckable(true);
    playButton->setChecked(false);
    playButton->addAction(DkActionManager::instance().action(DkActionManager::menu_view_slideshow));
    connect(playButton, &QPushButton::clicked, this, &DkPlayer::play);

    nextButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/next.svg", ih, Qt::white), "", this);
    // nextButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    nextButton->setIconSize(ih);
    nextButton->setMinimumSize(QSize(qRound(1.5 * height), height));
    nextButton->setToolTip(tr("Show next image"));
    nextButton->setObjectName("DkPlayerButton");
    nextButton->setFlat(true);
    connect(nextButton, &QPushButton::pressed, this, &DkPlayer::next);

    // now add to mLayout
    container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    // layout->setContentsMargins(0,0,0,0);
    layout->addStretch();
    layout->addWidget(previousButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);
    layout->addStretch();

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(container);
    l->addStretch();

    // setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void DkPlayer::init()
{
    setObjectName("DkPlayer");

    // slide show
    int timeToDisplayPlayer = 3000;
    timeToDisplay = qRound(DkSettingsManager::param().slideShow().time * 1000);
    playing = false;
    displayTimer = new QTimer(this);
    displayTimer->setInterval(timeToDisplay);
    displayTimer->setSingleShot(true);
    connect(displayTimer, &QTimer::timeout, this, &DkPlayer::autoNext);

    hideTimer = new QTimer(this);
    hideTimer->setInterval(timeToDisplayPlayer);
    hideTimer->setSingleShot(true);
    connect(hideTimer, &QTimer::timeout, this, [this]() {
        this->hide(false); // do not save setting when showing/hiding temporarily
    });

    connect(DkActionManager::instance().action(DkActionManager::menu_view_slideshow), &QAction::triggered, this, &DkPlayer::togglePlay);
}

void DkPlayer::play(bool play)
{
    if (play != playing) // emulate a click
        playButton->setChecked(play);

    playing = play;

    if (play) {
        displayTimer->start();
        showTemporarily();
    } else
        displayTimer->stop();
}

void DkPlayer::togglePlay()
{
    hideTimer->stop();
    showTemporarily(!playing);
    playing = !playing;
    playButton->click();
}

void DkPlayer::startTimer()
{
    if (playing) {
        displayTimer->setInterval(qRound(DkSettingsManager::param().slideShow().time * 1000)); // if it was updated...
        displayTimer->start();
    }
}

void DkPlayer::autoNext()
{
    emit nextSignal();
}

void DkPlayer::next()
{
    hideTimer->stop();
    emit nextSignal();
}

void DkPlayer::previous()
{
    hideTimer->stop();
    emit previousSignal();
}

bool DkPlayer::isPlaying() const
{
    return playing;
}

void DkPlayer::setTimeToDisplay(int ms)
{
    timeToDisplay = ms;
    displayTimer->setInterval(ms);
}

void DkPlayer::showTemporarily(bool autoHide)
{
    if (autoHide)
        hideTimer->start();

    // automatic showing, don't save visibility setting
    DkFadeWidget::show(false);
}

// -------------------------------------------------------------------- DkHudNavigation
DkHudNavigation::DkHudNavigation(QWidget *parent)
    : DkFadeWidget(parent)
{
    createLayout();
}

void DkHudNavigation::createLayout()
{
    // previous/next buttons
    QSize s(64, 64);
    QColor c(0, 0, 0);
    c.setAlpha(0);

    mPreviousButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/previous-hud.svg", s, c), "", this);
    mPreviousButton->setObjectName("hudNavigationButton");
    mPreviousButton->setToolTip(tr("Show previous image"));
    mPreviousButton->setFlat(true);
    mPreviousButton->setIconSize(s);
    connect(mPreviousButton, &QPushButton::pressed, this, &DkHudNavigation::previousSignal);

    mNextButton = new QPushButton(DkImage::loadIcon(":/nomacs/img/next-hud.svg", s, c), "", this);
    mNextButton->setObjectName("hudNavigationButton");
    mNextButton->setToolTip(tr("Show next image"));
    mNextButton->setFlat(true);
    mNextButton->setIconSize(s);
    connect(mNextButton, &QPushButton::pressed, this, &DkHudNavigation::nextSignal);

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(mPreviousButton);
    l->addStretch();
    l->addWidget(mNextButton);
}

void DkHudNavigation::showNext()
{
    mNextButton->show();
    DkFadeWidget::show();
    mPreviousButton->hide();
}

void DkHudNavigation::showPrevious()
{
    mPreviousButton->show();
    DkFadeWidget::show();
    mNextButton->hide();
}

// DkTransformRectangle --------------------------------------------------------------------
DkTransformRect::DkTransformRect(int idx, DkRotatingRect *rect, QWidget *parent, Qt::WindowFlags f)
    : DkWidget(parent, f)
{
    this->parentIdx = idx;
    this->size = QSize(12, 12);
    this->rect = rect;

    init();

    this->resize(size);
    setCursor(Qt::CrossCursor);
}

void DkTransformRect::init()
{
}

void DkTransformRect::draw(QPainter *painter)
{
    QPen penNoStroke;
    penNoStroke.setWidth(0);
    penNoStroke.setColor(QColor(0, 0, 0, 0));

    QPen pen;
    // pen.setWidth(1);
    pen.setColor(QColor(255, 255, 0, 100));

    QRectF visibleRect(QPointF(), QSizeF(5, 5));
    QRectF whiteRect(QPointF(), QSize(7, 7));
    visibleRect.moveCenter(geometry().center());
    whiteRect.moveCenter(geometry().center());

    // draw the control point
    painter->setWorldMatrixEnabled(false);
    painter->setPen(penNoStroke);
    painter->setBrush(QColor(0, 0, 0, 0));
    painter->drawRect(geometry()); // invisible mRect for mouseevents...
    // painter->setPen(pen);
    painter->setBrush(QColor(255, 255, 255, 100));
    painter->drawRect(whiteRect);
    painter->setBrush(QColor(0, 0, 0));
    painter->drawRect(visibleRect);
    painter->setWorldMatrixEnabled(true);
}

void DkTransformRect::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        posGrab = event->globalPos();
        initialPos = geometry().topLeft();

        emit updateDiagonal(parentIdx);
    }
    qDebug() << "mouse pressed control point";
    QWidget::mousePressEvent(event);
}

void DkTransformRect::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        QPointF pt = initialPos + event->globalPos() - posGrab;
        emit ctrlMovedSignal(parentIdx, pt, event->modifiers(), true);
    }

    QWidget::mouseMoveEvent(event);
}

void DkTransformRect::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void DkTransformRect::enterEvent(DkEnterEvent *)
{
    if (rect)
        setCursor(rect->cpCursor(parentIdx));
}

// DkEditableRectangle --------------------------------------------------------------------
DkEditableRect::DkEditableRect(const QRectF &rect, QWidget *parent, Qt::WindowFlags f)
    : DkFadeWidget(parent, f)
{
    mRect = rect;
    mRotatingCursor = QCursor(DkImage::loadFromSvg(":/nomacs/img/rotating-cursor.svg", QSize(24, 24)));

    setAttribute(Qt::WA_MouseTracking);

    mPen = QPen(QColor(0, 0, 0, 255), 1);
    mPen.setCosmetic(true);
    mBrush = (DkSettingsManager::param().app().appMode == DkSettings::mode_frameless) ? DkSettingsManager::param().display().bgColorFrameless
                                                                                      : DkSettingsManager::param().display().hudBgColor;

    for (int idx = 0; idx < 8; idx++) {
        mCtrlPoints.push_back(new DkTransformRect(idx, &this->mRect, this));
        mCtrlPoints[idx]->hide();
        connect(mCtrlPoints[idx], &DkTransformRect::ctrlMovedSignal, this, &DkEditableRect::updateCorner);
        connect(mCtrlPoints[idx], &DkTransformRect::updateDiagonal, this, &DkEditableRect::updateDiagonal);
    }
}

void DkEditableRect::reset()
{
    mRect = QRectF();
}

QPointF DkEditableRect::map(const QPointF &pos)
{
    QPointF posM = pos;
    if (mWorldTform)
        posM = mWorldTform->inverted().map(posM);
    if (mImgTform)
        posM = mImgTform->inverted().map(posM);

    return posM;
}

QPointF DkEditableRect::clipToImage(const QPointF &pos)
{
    if (!mImgRect)
        return QPointF(pos);

    QRectF imgViewRect(*mImgRect);
    if (mWorldTform)
        imgViewRect = mWorldTform->mapRect(imgViewRect);

    float x = (float)pos.x();
    float y = (float)pos.y();
    float magicMargin = 5; // 5 px for now

    if (qAbs(x - imgViewRect.left()) < magicMargin)
        x = (float)imgViewRect.left();
    if (qAbs(x - imgViewRect.right()) < magicMargin)
        x = (float)imgViewRect.right();

    if (qAbs(y - imgViewRect.top()) < magicMargin)
        y = (float)imgViewRect.top();
    if (qAbs(y - imgViewRect.bottom()) < magicMargin)
        y = (float)imgViewRect.bottom();

    return QPointF(x, y); // round
}

QPointF DkEditableRect::clipToImageForce(const QPointF &pos)
{
    if (!mImgRect)
        return QPointF(pos);

    QRectF imgViewRect(*mImgRect);
    if (mWorldTform)
        imgViewRect = mWorldTform->mapRect(imgViewRect);

    float x = (float)pos.x();
    float y = (float)pos.y();

    if (x < imgViewRect.left())
        x = (float)imgViewRect.left();
    if (x > imgViewRect.right())
        x = (float)imgViewRect.right();

    if (y < imgViewRect.top())
        y = (float)imgViewRect.top();
    if (y > imgViewRect.bottom())
        y = (float)imgViewRect.bottom();

    return QPointF(x, y); // round
}

void DkEditableRect::updateDiagonal(int idx)
{
    // we need to store the old diagonal in order to enable "keep aspect ratio"
    if (mRect.isEmpty())
        mOldDiag = DkVector(-1.0f, -1.0f);
    else
        mOldDiag = mRect.getDiagonal(idx);
}

void DkEditableRect::setFixedDiagonal(const DkVector &diag)
{
    mFixedDiag = diag;

    qDebug() << "after rotating: " << mFixedDiag.toQPointF();

    // don't update in that case
    if (diag.x == 0 || diag.y == 0)
        return;
    else
        mFixedDiag.rotate(-mRect.getAngle());

    QPointF c = mRect.getCenter();

    if (!mRect.getPoly().isEmpty())
        mRect.updateCorner(0, mRect.getPoly().at(0), mFixedDiag);

    mRect.setCenter(c);
    update();
}

void DkEditableRect::setPanning(bool panning)
{
    this->mPanning = panning;
    setCursor(Qt::OpenHandCursor);
    qDebug() << "panning set...";
}

void DkEditableRect::updateCorner(int idx, const QPointF &point, Qt::KeyboardModifiers modifiers, bool changeState)
{
    if (changeState)
        mState = scaling;

    DkVector diag = (modifiers & Qt::ShiftModifier || (mFixedDiag.x != 0 && mFixedDiag.y != 0)) ? mOldDiag : DkVector(); // braces
    QPointF p = point;

    if ((modifiers & Qt::ControlModifier) == 0)
        p = clipToImage(point);

    mRect.updateCorner(idx, map(p), diag);

    // edge control -> remove aspect ratio constraint
    if (idx >= 4 && idx < 8)
        emit aRatioSignal(QPointF(0, 0));

    update();
}

void DkEditableRect::paintEvent(QPaintEvent *event)
{
    // create path
    QPainterPath path;
    QRect canvas(geometry().x() - 1, geometry().y() - 1, geometry().width() + 1, geometry().height() + 1);
    path.addRect(canvas);

    QPolygonF p;
    if (!mRect.isEmpty()) {
        // TODO: directly map the points (it's easier and not slower at all)
        p = mRect.getClosedPoly();
        p = mTtform.map(p);
        p = mRtform.map(p);
        p = mTtform.inverted().map(p);
        if (mImgTform)
            p = mImgTform->map(p);
        if (mWorldTform)
            p = mWorldTform->map(p);
        QPolygon pr = p.toPolygon(); // round coordinates
        path.addPolygon(pr);
    }

    // now draw
    QPainter painter(this);

    painter.setPen(mPen);
    painter.setBrush(mBrush);
    painter.drawPath(path);

    drawGuide(&painter, p, mPaintMode);

    //// debug
    // painter.drawPoint(mRect.getCenter());

    // this changes the painter -> do it at the end
    if (!mRect.isEmpty()) {
        for (int idx = 0; idx < mCtrlPoints.size(); idx++) {
            QPointF cp;

            if (idx < 4) {
                QPointF c = p[idx];
                cp = c - mCtrlPoints[idx]->getCenter();
            }
            // paint control points in the middle of the edge
            else if (idx >= 4) {
                QPointF s = mCtrlPoints[idx]->getCenter();

                QPointF lp = p[idx % 4];
                QPointF rp = p[(idx + 1) % 4];

                QVector2D lv = QVector2D(lp - s);
                QVector2D rv = QVector2D(rp - s);

                cp = (lv + 0.5 * (rv - lv)).toPointF();
            }

            mCtrlPoints[idx]->move(qRound(cp.x()), qRound(cp.y()));
            mCtrlPoints[idx]->draw(&painter);
        }
    }

    painter.end();

    QWidget::paintEvent(event);
}

QRect DkEditableRect::rect() const
{
    QRect r;
    r.setTopLeft(mRect.getTopLeft().toPoint());
    r.setSize(mRect.size());

    return r;
}

void DkEditableRect::drawGuide(QPainter *painter, const QPolygonF &p, int paintMode)
{
    if (p.isEmpty() || paintMode == no_guide)
        return;

    QColor col = painter->pen().color();
    col.setAlpha(150);
    QPen pen = painter->pen();
    QPen cPen = pen;
    cPen.setColor(col);
    painter->setPen(cPen);

    // vertical
    DkVector l9 = p[3] - p[0]; // perpendicular to drawing

    int nLines = (paintMode == rule_of_thirds) ? 3 : qRound(l9.norm() / 20.0f);
    DkVector offset = l9;
    offset.normalize();
    offset *= l9.norm() / nLines;

    DkVector offsetVec = offset;

    for (int idx = 0; idx < (nLines - 1); idx++) {
        // step through & paint
        QLineF l = QLineF(DkVector(p[1] + offsetVec).toQPointF(), DkVector(p[0] + offsetVec).toQPointF());
        painter->drawLine(l);
        offsetVec += offset;
    }

    // horizontal
    l9 = p[1] - p[0]; // perpendicular to drawing

    nLines = (paintMode == rule_of_thirds) ? 3 : qRound(l9.norm() / 20);
    offset = l9;
    offset.normalize();
    offset *= l9.norm() / nLines;

    offsetVec = offset;

    for (int idx = 0; idx < (nLines - 1); idx++) {
        // step through & paint
        QLineF l = QLineF(DkVector(p[3] + offsetVec).toQPointF(), DkVector(p[0] + offsetVec).toQPointF());
        painter->drawLine(l);
        offsetVec += offset;
    }

    painter->setPen(pen); // revert painter
}

// make events callable
void DkEditableRect::mousePressEvent(QMouseEvent *event)
{
    // panning -> redirect to mViewport
    if (event->buttons() == Qt::LeftButton && (event->modifiers() == DkSettingsManager::param().global().altMod || mPanning)) {
        event->setModifiers(Qt::NoModifier); // we want a 'normal' action in the mViewport
        event->ignore();
        return;
    }

    mPosGrab = map(QPointF(event->pos()));
    mClickPos = QPointF(event->pos());

    if (mRect.isEmpty()) {
        mState = initializing;
        setAngle(0);
    } else if (mRect.getPoly().containsPoint(mPosGrab, Qt::OddEvenFill)) {
        mState = moving;
    } else {
        mState = rotating;
    }
}

void DkEditableRect::mouseMoveEvent(QMouseEvent *event)
{
    // panning -> redirect to mViewport
    if (event->modifiers() == DkSettingsManager::param().global().altMod || mPanning) {
        if (event->buttons() != Qt::LeftButton)
            setCursor(Qt::OpenHandCursor);
        else
            setCursor(Qt::ClosedHandCursor);

        event->setModifiers(Qt::NoModifier);
        event->ignore();
        update();
        return;
    }

    QPointF posM = map(QPointF(event->pos()));

    if (event->buttons() != Qt::LeftButton && !mRect.isEmpty()) {
        // show rotating - moving
        if (mRect.getPoly().containsPoint(map(event->pos()), Qt::OddEvenFill))
            setCursor(Qt::SizeAllCursor);
        else
            setCursor(mRotatingCursor);
    } else if (mRect.isEmpty())
        setCursor(Qt::CrossCursor);

    // additionally needed for showToolTip
    double angle = 0;

    if (mState == initializing && event->buttons() == Qt::LeftButton) {
        QPointF clipPos = clipToImageForce(QPointF(event->pos()));

        if (!mImgRect || !mRect.isEmpty() || clipPos == QPointF(event->pos())) {
            if (mRect.isEmpty()) {
                for (int idx = 0; idx < mCtrlPoints.size(); idx++)
                    mCtrlPoints[idx]->show();

                QPointF p = map(clipToImageForce(mClickPos));
                mRect.setAllCorners(p);
            }

            DkVector diag;

            // when initializing shift should make the mRect a square
            if (event->modifiers() == Qt::ShiftModifier)
                diag = DkVector(1.0f, 1.0f);
            else
                diag = mFixedDiag;
            mRect.updateCorner(2, map(clipPos), diag);
            update();
        }

    } else if (mState == moving && event->buttons() == Qt::LeftButton) {
        QPointF dxy = posM - mPosGrab;
        mRtform.translate(dxy.x(), dxy.y());
        mPosGrab = posM;
        update();
    } else if (mState == rotating && event->buttons() == Qt::LeftButton) {
        DkVector c(mRect.getCenter());
        DkVector xt(mPosGrab);
        DkVector xn(posM);

        // compute the direction vector;
        xt = c - xt;
        xn = c - xn;
        angle = xn.angle() - xt.angle();

        // just rotate in CV_PI*0.25 steps if shift is pressed
        if (event->modifiers() == Qt::ShiftModifier) {
            double angleRound = DkMath::normAngleRad(angle + mRect.getAngle(), -CV_PI * 0.125, CV_PI * 0.125);
            angle -= angleRound;
        }

        setAngle(angle, false);
    }

    if (event->buttons() == Qt::LeftButton) {
        QPolygonF p = mRect.getPoly();

        float sAngle = DkMath::getReadableAngle(mRect.getAngle() + angle);
        int height = qRound(DkVector(p[1] - p[0]).norm());
        int width = qRound(DkVector(p[3] - p[0]).norm());

        // show coordinates
        QString info;
        QPoint tl;

        if (sAngle == 0.0f || fabs(sAngle) == 90.0f) {
            tl = mRtform.map(mRect.getTopLeft()).toPoint();
            info += "x: ";
        } else {
            // Because we rotate around the center, there's no need to do rotation.
            // However, we need to check whether the center is tranlated.
            // Not sure why, but the tranlation is stored in mRtform in the above.
            const QTransform transform{1, 0, 0, 1, mRtform.dx(), mRtform.dy()};
            tl = transform.map(mRect.getCenter()).toPoint();
            info += "center x: ";
        }
        info += QString::number(tl.x()) + ", y: ";
        info += QString::number(tl.y()) + " | ";
        info += QString::number(width) + " x ";
        info += QString::number(height) + " px | ";
        info += QString::number(sAngle) + dk_degree_str;

        if (mShowInfo) {
            QToolTip::showText(event->globalPos(), info, this);
        }

        DkStatusBarManager::instance().setMessage(info);
    }

    // QWidget::mouseMoveEvent(event);
}

void DkEditableRect::mouseReleaseEvent(QMouseEvent *event)
{
    // panning -> redirect to mViewport
    if (event->buttons() == Qt::LeftButton && (event->modifiers() == DkSettingsManager::param().global().altMod || mPanning)) {
        setCursor(Qt::OpenHandCursor);
        event->setModifiers(Qt::NoModifier);
        event->ignore();
        return;
    }

    mState = do_nothing;

    applyTransform();
    // QWidget::mouseReleaseEvent(event);
}

void DkEditableRect::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);
    update(); // this is an extra update - however we get rendering errors otherwise?!
}

void DkEditableRect::applyTransform()
{
    // apply transform
    mRect.transform(mTtform, mRtform);

    mRtform.reset();
    mTtform.reset();
    update();

    emit updateRectSignal(rect());
}

void DkEditableRect::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt)
        setCursor(Qt::OpenHandCursor);

    QWidget::keyPressEvent(event);
}

void DkEditableRect::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void DkEditableRect::setPaintHint(int paintMode /* = DkCropToolBar::no_guide */)
{
    this->mPaintMode = paintMode;
    update();
}

void DkEditableRect::setShadingHint(bool)
{
    QColor col = mBrush.color();
    col = QColor(255 - col.red(), 255 - col.green(), 255 - col.blue(), col.alpha());
    mBrush.setColor(col);

    col = mPen.color();
    col = QColor(255 - col.red(), 255 - col.green(), 255 - col.blue(), col.alpha());
    mPen.setColor(col);

    update();
}

void DkEditableRect::setShowInfo(bool showInfo)
{
    this->mShowInfo = showInfo;
}

void DkEditableRect::setRect(const QRect &rect)
{
    mRect.setCenter(rect.center());
    mRect.setSize(rect.size());

    update();
}

void DkEditableRect::setAngle(double angle, bool apply)
{
    DkVector c(mRect.getCenter());

    if (!mTtform.isTranslating())
        mTtform.translate(-c.x, -c.y);

    mRtform.reset();
    if (apply)
        mRtform.rotateRadians(angle - mRect.getAngle());
    else
        mRtform.rotateRadians(angle);

    if (apply)
        applyTransform();
    else {
        emit angleSignal(mRect.getAngle() + angle);
        update();
    }
}

void DkEditableRect::setVisible(bool visible)
{
    DkFadeWidget::setVisible(visible);
    if (mSetWidgetVisible)
        return; // prevent recursion via fade()

    if (!visible) {
        mRect = DkRotatingRect();
        for (int idx = 0; idx < mCtrlPoints.size(); idx++)
            mCtrlPoints[idx]->hide();
    } else {
        // setFocus(Qt::ActiveWindowFocusReason);
        setCursor(Qt::CrossCursor);
    }
}

// DkEditableRect --------------------------------------------------------------------
DkCropWidget::DkCropWidget(QRectF rect /* = QRect */, QWidget *parent /* = 0*/, Qt::WindowFlags f /* = 0*/)
    : DkEditableRect(rect, parent, f)
{
    cropToolbar = 0;
}

void DkCropWidget::createToolbar()
{
    cropToolbar = new DkCropToolBar(tr("Crop Toolbar"), this);

    // crop toolbar will be reparented, which messes up shortcut resolution
    for (QAction *a : cropToolbar->actions())
        this->addAction(a);

    connect(cropToolbar, &DkCropToolBar::updateRectSignal, this, &DkCropWidget::setRect);

    connect(cropToolbar, &DkCropToolBar::cropSignal, this, &DkCropWidget::crop);
    connect(cropToolbar, &DkCropToolBar::cancelSignal, this, &DkCropWidget::hideSignal);
    connect(cropToolbar, &DkCropToolBar::aspectRatio, this, &DkCropWidget::setFixedDiagonal);
    connect(cropToolbar, &DkCropToolBar::angleSignal, this, [this](double angle) {
        this->setAngle(angle);
    });
    connect(cropToolbar, &DkCropToolBar::panSignal, this, &DkCropWidget::setPanning);
    connect(cropToolbar, &DkCropToolBar::paintHint, this, &DkCropWidget::setPaintHint);
    connect(cropToolbar, &DkCropToolBar::shadingHint, this, &DkCropWidget::setShadingHint);
    connect(cropToolbar, &DkCropToolBar::showInfo, this, &DkCropWidget::setShowInfo);
    connect(this, &DkCropWidget::angleSignal, cropToolbar, &DkCropToolBar::angleChanged);
    connect(this, &DkCropWidget::aRatioSignal, cropToolbar, &DkCropToolBar::setAspectRatio);
    connect(this, &DkCropWidget::updateRectSignal, cropToolbar, &DkCropToolBar::setRect);

    cropToolbar->loadSettings(); // need to this manually after connecting the slots
}

void DkCropWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    crop();
    QWidget::mouseDoubleClickEvent(ev);
}

DkCropToolBar *DkCropWidget::getToolbar() const
{
    return cropToolbar;
}

void DkCropWidget::crop(bool cropToMetadata)
{
    if (!cropToolbar)
        return;

    if (!mRect.isEmpty())
        emit cropImageSignal(mRect, cropToolbar->getColor(), cropToMetadata);

    setVisible(false);
    setWindowOpacity(0);
    emit hideSignal();
}

void DkCropWidget::setVisible(bool visible)
{
    DkEditableRect::setVisible(visible);
    if (mSetWidgetVisible)
        return; // prevent recursion via fade()

    if (visible && !cropToolbar)
        createToolbar();

    DkToolBarManager::inst().showToolBar(cropToolbar, visible);
}

// Image histogram  -------------------------------------------------------------------
DkHistogram::DkHistogram(QWidget *parent)
    : DkFadeWidget(parent)
{
    setObjectName("DkHistogram");
    setMinimumWidth(265);
    setMinimumHeight(142);
    setCursor(Qt::ArrowCursor);
    loadSettings();

    // create context menu
    QAction *showStats = new QAction(tr("Show Statistics"), this);
    showStats->setCheckable(true);
    showStats->setChecked(mDisplayMode == DisplayMode::histogram_mode_extended);
    connect(showStats, &QAction::triggered, this, &DkHistogram::onToggleStatsTriggered);

    mContextMenu = new QMenu(tr("Histogram Settings"));
    mContextMenu->addAction(showStats);
}

DkHistogram::~DkHistogram()
{
}

/**
 * Paints the image histogram
 **/
void DkHistogram::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(1, 1, width(), height(), DkSettingsManager::param().display().hudBgColor);

    int numTextLines;
    if (mDisplayMode == DisplayMode::histogram_mode_simple) {
        numTextLines = 0;
    } else {
        // histogram_mode_extended has 2 lines of text
        numTextLines = 2;
    }

    const int margin = 5;
    const int TEXT_SIZE = 12 + 1; // FIXME just guessed
    const int textHeight = numTextLines * TEXT_SIZE + margin;

    int binHeight = height() - margin * 2 - textHeight;
    int binBottom = height() - margin - textHeight;

    // draw Histogram
    if (mIsPainted && mMaxValue > 0) {
        for (int x = 0; x < 256; x++) {
            // get bounded values
            int rLineHeight = qMax(qMin(qRound((float)mHist[0][x] * binHeight * mScaleFactor / mMaxValue), binHeight), 0);
            int gLineHeight = qMax(qMin(qRound((float)mHist[1][x] * binHeight * mScaleFactor / mMaxValue), binHeight), 0);
            int bLineHeight = qMax(qMin(qRound((float)mHist[2][x] * binHeight * mScaleFactor / mMaxValue), binHeight), 0);
            int maxLineHeight = qMax(qMax(rLineHeight, gLineHeight), bLineHeight);

            painter.setCompositionMode(QPainter::CompositionMode_Clear);
            painter.setPen(Qt::black);
            painter.drawLine(QPoint(x + margin, binBottom), QPoint(x + margin, binBottom - maxLineHeight));

            painter.setCompositionMode(QPainter::CompositionMode_Screen);
            painter.setPen(Qt::red);
            painter.drawLine(QPoint(x + margin, binBottom), QPoint(x + margin, binBottom - rLineHeight));
            painter.setPen(Qt::green);
            painter.drawLine(QPoint(x + margin, binBottom), QPoint(x + margin, binBottom - gLineHeight));
            painter.setPen(Qt::blue);
            painter.drawLine(QPoint(x + margin, binBottom), QPoint(x + margin, binBottom - bLineHeight));
        }
    }

    if (mDisplayMode == DisplayMode::histogram_mode_extended) {
        // draw histogram text
        double megaPixels = (double)mNumPixels * 10.0e-7;
        painter.setPen(QColor(255, 255, 255, 200));

        QString histText1("Pixels: %1\tMPix: %2");
        painter.drawText(QPoint(margin, height() - 2 * TEXT_SIZE + margin), histText1.arg(mNumPixels, 10, 10).arg(megaPixels, 10, 'f', 2));

        if (mMinBinValue < 256) {
            // gray image statistics
            QString histText2("Min: %1\tMax: %2\tValue Count: %3");
            painter.drawText(QPoint(margin, height() - 1 * TEXT_SIZE + margin),
                             histText2.arg(mMinBinValue, 5, 10).arg(mMaxBinValue, 5, 10).arg(mNumDistinctValues, 5, 10));
        } else {
            // color image statistics
            double blackPct = 100.0 * (double)mNumZeroPixels / (double)mNumPixels;
            double whitePct = 100.0 * (double)mNumSaturatedPixels / (double)mNumPixels;
            double goodPct = 100.0 * (double)(mNumPixels - mNumZeroPixels - mNumSaturatedPixels) / (double)mNumPixels;

            QString histText2("Black:  %1\tGood: %3\tWhite: %2");
            painter.drawText(QPoint(margin, height() - 1 * TEXT_SIZE + margin),
                             histText2.arg(blackPct, 5, 'f', 1).arg(whitePct, 5, 'f', 1).arg(goodPct, 5, 'f', 1));
        }
    }
}

void DkHistogram::contextMenuEvent(QContextMenuEvent *event)
{
    mContextMenu->exec(event->globalPos());
    event->accept();

    // do not pass it on
    // DkFadeWidget::contextMenuEvent(event);
}

void DkHistogram::onToggleStatsTriggered(bool show)
{
    mDisplayMode = (show) ? DisplayMode::histogram_mode_extended : DisplayMode::histogram_mode_simple;
    DkSettingsManager::param().display().histogramStyle = (int)mDisplayMode;
    update();
}

void DkHistogram::loadSettings()
{
    int styleSetting = DkSettingsManager::param().display().histogramStyle;
    DisplayMode maybeMode = static_cast<DisplayMode>(styleSetting);
    if (maybeMode == DisplayMode::histogram_mode_simple || maybeMode == DisplayMode::histogram_mode_extended) {
        mDisplayMode = maybeMode;
    } else {
        mDisplayMode = DisplayMode::histogram_mode_simple;
    }
}

/**
 * Goes through the image and counts pixels values. They are used to create the image histogram.
 * @param currently displayed image
 **/
void DkHistogram::drawHistogram(QImage imgQt)
{
    if (!isVisible() || imgQt.isNull()) {
        setPainted(false);
        return;
    }

    DkTimer dt;

    // clear histogram values
    mNumZeroPixels = 0;
    mNumSaturatedPixels = 0;
    mMaxBinValue = -1;
    mMinBinValue = 256;
    mMaxValue = 0;
    mNumPixels = imgQt.width() * imgQt.height();

    for (int idx = 0; idx < 256; idx++) {
        mHist[0][idx] = 0;
        mHist[1][idx] = 0;
        mHist[2][idx] = 0;
    }

    // count pixel- and total values, for
    // 8 bit images
    if (imgQt.depth() == 8) {
        qDebug() << "8 bit histogram -------------------";

        for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {
            const unsigned char *pixel = imgQt.constScanLine(rIdx);

            for (int cIdx = 0; cIdx < imgQt.width(); cIdx++, pixel++) {
                mHist[0][*pixel]++;
                mHist[1][*pixel]++;
                mHist[2][*pixel]++;

                if (*pixel == 255)
                    mNumSaturatedPixels++;
                if (*pixel < mMinBinValue)
                    mMinBinValue = *pixel;
                if (*pixel > mMaxBinValue)
                    mMaxBinValue = *pixel;
            }
        }
    }
    // 24 bit images
    else if (imgQt.depth() == 24) {
        for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {
            const unsigned char *pixel = imgQt.constScanLine(rIdx);
            for (int cIdx = 0; cIdx < imgQt.width(); cIdx++) {
                // If I understood the api correctly, the first bits are 0 if we have 24bpp & < 8 bits per channel
                unsigned char pixR = *pixel;
                mHist[0][*pixel]++;
                pixel++;
                unsigned char pixG = *pixel;
                mHist[1][*pixel]++;
                pixel++;
                unsigned char pixB = *pixel;
                mHist[2][*pixel]++;
                pixel++;

                if (pixR == 0 && pixG == 0 && pixB == 0) {
                    mNumZeroPixels++;
                } else if (pixR == 255 && pixG == 255 && pixB == 255) {
                    mNumSaturatedPixels++;
                }
            }
        }
    }
    // 32 bit images
    else if (imgQt.depth() == 32) {
        for (int rIdx = 0; rIdx < imgQt.height(); rIdx++) {
            const QRgb *pixel = (QRgb *)(imgQt.constScanLine(rIdx));

            for (int cIdx = 0; cIdx < imgQt.width(); cIdx++, pixel++) {
                size_t pixR = static_cast<size_t>(qRed(*pixel));
                size_t pixG = static_cast<size_t>(qGreen(*pixel));
                size_t pixB = static_cast<size_t>(qBlue(*pixel));

                mHist[0][pixR]++;
                mHist[1][pixG]++;
                mHist[2][pixB]++;

                if (pixR == 0 && pixG == 0 && pixB == 0) {
                    mNumZeroPixels++;
                } else if (pixR == 255 && pixG == 255 && pixB == 255) {
                    mNumSaturatedPixels++;
                }
            }
        }
    }

    // determine extreme values from the histogram
    mNumDistinctValues = 0;

    for (int idx = 0; idx < 256; idx++) {
        if (mHist[0][idx] > mMaxValue)
            mMaxValue = mHist[0][idx];
        if (mHist[1][idx] > mMaxValue)
            mMaxValue = mHist[1][idx];
        if (mHist[2][idx] > mMaxValue)
            mMaxValue = mHist[2][idx];

        if (mHist[0][idx] || mHist[1][idx] || mHist[2][idx]) {
            mNumDistinctValues++;
        }
    }

    setPainted(true);
    qDebug() << "drawing the histogram took me: " << dt;
    update();
}

/**
 * Clears the histogram panel
 **/
void DkHistogram::clearHistogram()
{
    setPainted(false);
    update();
}

void DkHistogram::setPainted(bool isPainted)
{
    this->mIsPainted = isPainted;
}

void DkHistogram::setMaxHistogramValue(int maxValue)
{
    if (maxValue == 0)
        setPainted(false);

    this->mMaxValue = maxValue;
}

/**
 * Updates histogram values.
 * @param values to be copied
 **/
void DkHistogram::updateHistogramValues(int histValues[][256])
{
    for (int i = 0; i < 256; i++) {
        this->mHist[0][i] = histValues[0][i];
        this->mHist[1][i] = histValues[1][i];
        this->mHist[2][i] = histValues[2][i];
    }
}

/**
 * Mouse events for scaling the histogram - enlarge the histogram between the bottom axis and the cursor position
 **/
void DkHistogram::mousePressEvent(QMouseEvent *event)
{
    // always propagate mouse events
    if (event->buttons() != Qt::LeftButton)
        DkFadeWidget::mousePressEvent(event);
}

void DkHistogram::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        float cp = (float)(height() - event->pos().y());

        if (cp > 0) {
            mScaleFactor = height() / cp;
            update();
        }
    } else
        DkFadeWidget::mouseMoveEvent(event);
}

void DkHistogram::mouseReleaseEvent(QMouseEvent *event)
{
    mScaleFactor = 1;
    update();

    if (event->buttons() != Qt::LeftButton)
        DkFadeWidget::mouseReleaseEvent(event);
}

// DkFileInfo --------------------------------------------------------------------
DkFileInfo::DkFileInfo()
{
    mFileExists = false;
    mUsed = false;
}

DkFileInfo::DkFileInfo(const QFileInfo &fileInfo)
{
    mFileInfo = fileInfo;
}

bool DkFileInfo::exists() const
{
    return mFileExists;
}

void DkFileInfo::setExists(bool fileExists)
{
    mFileExists = fileExists;
}

bool DkFileInfo::inUse() const
{
    return mUsed;
}

void DkFileInfo::setInUse(bool inUse)
{
    mUsed = inUse;
}

QString DkFileInfo::getFilePath() const
{
    return mFileInfo.absoluteFilePath();
}

// DkFileLabel --------------------------------------------------------------------
DkFolderLabel::DkFolderLabel(const DkFileInfo &fileInfo, QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : QLabel(parent, f)
{
    // we don't use the file labels anymore
    // + isDir() might hang - if we try to get an unavailable network resource on windows
    // QFileInfo fInfo(fileInfo.getFilePath());
    // if (fInfo.isDir())
    setText(fileInfo.getFilePath());
    // else
    //	setText(fInfo.fileName());

    this->fileInfo = fileInfo;
    setObjectName("DkFileLabel");
}

void DkFolderLabel::mousePressEvent(QMouseEvent *ev)
{
    emit loadFileSignal(fileInfo.getFilePath());

    QLabel::mousePressEvent(ev);
}

// DkDirectoryEdit --------------------------------------------------------------------
DkDirectoryEdit::DkDirectoryEdit(QWidget *parent /* = 0 */)
    : QLineEdit(parent)
{
    setObjectName("DkWarningEdit");
    connect(this, &DkDirectoryEdit::textChanged, this, &DkDirectoryEdit::lineEditChanged);

    QCompleter *completer = new QCompleter(this);
    QFileSystemModel *model = new QFileSystemModel(completer);
    model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    completer->setModel(model);
    setCompleter(completer);
}

DkDirectoryEdit::DkDirectoryEdit(const QString &content, QWidget *parent /* = 0 */)
    : QLineEdit(parent)
{
    setObjectName("DkWarningEdit");
    connect(this, &DkDirectoryEdit::textChanged, this, &DkDirectoryEdit::lineEditChanged);
    setText(content);

    QCompleter *completer = new QCompleter(this);
    QFileSystemModel *model = new QFileSystemModel(completer);
    model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    completer->setModel(model);
    setCompleter(completer);
}

void DkDirectoryEdit::lineEditChanged(const QString &path)
{
    setProperty("error", !existsDirectory(path));
    style()->unpolish(this);
    style()->polish(this);
    update();

    // converting to QDir since D:/img == D:/img/ then
    if (QDir(path).absolutePath() != QDir(mOldPath).absolutePath() && existsDirectory(path)) {
        mOldPath = path;
        emit directoryChanged(path);
    }
}

bool DkDirectoryEdit::existsDirectory(const QString &path)
{
    return QDir(path).exists();
}

// DkDirectoryChooser --------------------------------------------------------------------
DkDirectoryChooser::DkDirectoryChooser(const QString &dirPath, QWidget *parent)
    : DkWidget(parent)
{
    createLayout(dirPath);
}

void DkDirectoryChooser::createLayout(const QString &dirPath)
{
    mDirEdit = new DkDirectoryEdit(dirPath, this);
    mDirEdit->setObjectName("dirEdit");

    QPushButton *dirButton = new QPushButton(tr("..."), this);
    connect(dirButton, &QPushButton::clicked, this, &DkDirectoryChooser::onDirButtonClicked);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignLeft);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mDirEdit);
    layout->addWidget(dirButton);

    connect(mDirEdit, &DkDirectoryEdit::textChanged, this, &DkDirectoryChooser::directoryChanged);
}

void DkDirectoryChooser::onDirButtonClicked()
{
    QString dirPath =
        QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"), mDirEdit->text(), QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

    if (dirPath.isEmpty())
        return;

    mDirEdit->setText(dirPath);
}

// DkListWidget --------------------------------------------------------------------
DkListWidget::DkListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setAcceptDrops(true);
    setDragEnabled(true);
    setMinimumHeight(100);
    setDropIndicatorShown(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void DkListWidget::startDrag(Qt::DropActions)
{
    QListWidget::startDrag(Qt::MoveAction);
}

bool DkListWidget::isEmpty() const
{
    return count() == 0;
}

void DkListWidget::setEmptyText(const QString &text)
{
    mEmptyText = text;
}

void DkListWidget::dropEvent(QDropEvent *event)
{
    if (event->source() != this)
        QListWidget::dropEvent(event);

    // workaround for the empty rows
    emit dataDroppedSignal();
}

void DkListWidget::paintEvent(QPaintEvent *event)
{
    QListView::paintEvent(event);

    if (model() && model()->rowCount(rootIndex()))
        return;

    // The view is empty.
    QPainter p(viewport());
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor(200, 200, 200), Qt::BDiagPattern));
    p.drawRect(QRect(QPoint(), size()));

    p.setPen(QPen(QColor(100, 100, 100)));
    p.drawText(rect(), Qt::AlignCenter, mEmptyText);
}

// DkProgressBar --------------------------------------------------------------------
DkProgressBar::DkProgressBar(QWidget *parent)
    : QProgressBar(parent)
{
    initPoints();
    mTimer.setInterval(20);
    connect(&mTimer, &QTimer::timeout, this, QOverload<>::of(&DkProgressBar::update));

    mShowTimer.setInterval(3000);
    connect(&mShowTimer, &QTimer::timeout, this, &DkProgressBar::show);
}

void DkProgressBar::setVisible(bool visible)
{
    if (visible)
        mTimer.start();
    else
        mTimer.stop();

    if (visible && !isVisible())
        initPoints();

    QProgressBar::setVisible(visible);
}

void DkProgressBar::setVisibleTimed(bool visible, int time)
{
    // nothing todo?
    if (visible && mShowTimer.isActive())
        return;

    // hide
    if (isVisible() && !visible)
        hide();

    // just stop the timer, if it is active & the process is done already
    if (!visible && mShowTimer.isActive())
        mShowTimer.stop();

    // start the timer
    if (!isVisible() && visible && time > 0) {
        mShowTimer.setInterval(time);
        mShowTimer.start();
    }

    // simply show
    if (visible && !isVisible() && time <= 0)
        show();
}

void DkProgressBar::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.setPen(Qt::NoPen);

    if (parentWidget() && DkUtils::getMainWindow()->isFullScreen())
        p.setBackground(DkSettingsManager::param().slideShow().backgroundColor);

    p.setBrush(DkSettingsManager::param().display().highlightColor);

    if (value() != minimum()) {
        int rv = qRound((value() - minimum()) / (double)(maximum() - minimum()) * width());

        // draw the current status bar
        QRect r(QPoint(), size());
        r.setRight(rv);

        p.drawRect(r);
    }

    bool stillVisible = false;

    // draw points
    for (double &pt : mPoints) {
        animatePoint(pt);

        QRect r(QPoint(), QSize(height(), height()));
        r.moveLeft(qRound(pt * width()));

        p.drawRect(r);

        if (pt < 0.99)
            stillVisible = true;
    }

    if (!stillVisible)
        initPoints();
}

void DkProgressBar::initPoints()
{
    mPoints.clear();

    int m = 7;
    for (int idx = 1; idx < m; idx++) {
        mPoints.append((double)idx / m * 0.1);
    }
}

void DkProgressBar::animatePoint(double &xVal)
{
    double speed = xVal > 0.5 ? std::abs(1.0 - xVal) : std::abs(xVal);
    speed *= .05;

    xVal += speed;
}

// DkGenericProfileWidget --------------------------------------------------------------------
DkGenericProfileWidget::DkGenericProfileWidget(const QString &name, QWidget *parent, const QString &settingsPath)
    : DkNamedWidget(name, parent)
{
    mSettingsPath = settingsPath.isEmpty() ? DkSettingsManager::param().settingsPath() : settingsPath;
}

DkGenericProfileWidget::~DkGenericProfileWidget()
{
    // save settings
    for (int idx = 0; idx < mProfileList->count(); idx++) {
        saveSettings(mProfileList->itemText(idx));
    }
}

void DkGenericProfileWidget::init()
{
    createLayout();

    connect(mSaveButton, &QPushButton::clicked, this, QOverload<>::of(&DkGenericProfileWidget::saveSettings));
    connect(mDeleteButton, &QPushButton::clicked, this, &DkGenericProfileWidget::deleteCurrentSetting);
    // NOTE:This was changed
    connect(mProfileList, &QComboBox::currentTextChanged, this, &DkGenericProfileWidget::loadSettings);

    activate(false); // inits gui states
}

void DkGenericProfileWidget::createLayout()
{
    QPixmap pm = QPixmap(":/nomacs/img/save.svg");
    mSaveButton = new QPushButton(this);
    mSaveButton->setIcon(QIcon(pm));
    // mSaveButton->setIconSize(pm.size());
    mSaveButton->setFlat(true);

    pm = QPixmap(":/nomacs/img/trash.svg");
    mDeleteButton = new QPushButton(this);
    mDeleteButton->setIcon(QIcon(pm));
    // mDeleteButton->setIconSize(pm.size());
    mDeleteButton->setFlat(true);

    mProfileList = new QComboBox(this);
    mProfileList->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    QAction *delGradientAction = new QAction(tr("Set As Default"), mProfileList);
    connect(delGradientAction, &QAction::triggered, this, &DkGenericProfileWidget::setDefaultModel);

    mProfileList->addAction(delGradientAction);
    mProfileList->setContextMenuPolicy(Qt::ActionsContextMenu);

    QStringList modelStrings = loadProfileStrings();

    if (!modelStrings.empty()) {
        mProfileList->addItems(loadProfileStrings());
        mProfileList->setCurrentText(loadDefaultProfileString());
    }

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mProfileList);
    layout->addWidget(mSaveButton);
    layout->addWidget(mDeleteButton);
}

QStringList DkGenericProfileWidget::loadProfileStrings() const
{
    QSettings s(mSettingsPath, QSettings::IniFormat);

    s.beginGroup(mSettingsGroup);
    QStringList modelStrings = s.childGroups();
    s.endGroup();

    qDebug() << "profile settings of group: " << mSettingsGroup;

    return modelStrings;
}

void DkGenericProfileWidget::deleteCurrentSetting()
{
    QString modelName = mProfileList->currentText();

    QSettings s(mSettingsPath, QSettings::IniFormat);

    s.beginGroup(mSettingsGroup);
    s.beginGroup(modelName);
    s.remove(""); // remove all group entries
    s.endGroup();
    s.endGroup();

    // update list
    mProfileList->removeItem(mProfileList->currentIndex());
}

void DkGenericProfileWidget::saveSettings() const
{
    // default mode is overwrite (UI is asking anyway)
    QString dName = mProfileList->currentText().isEmpty() ? "Profile 1" : mProfileList->currentText();

    bool ok;
    QString text = QInputDialog::getText(DkUtils::getMainWindow(), tr("Profile Name"), tr("Profile Name:"), QLineEdit::Normal, dName, &ok);

    if (!ok || text.isEmpty())
        return; // user canceled

    // is the model name unique?
    if (mProfileList->findText(text) != -1) {
        QMessageBox::StandardButton button = QMessageBox::information(DkUtils::getMainWindow(),
                                                                      tr("Profile Already Exists"),
                                                                      tr("Do you want to overwrite %1?").arg(text),
                                                                      QMessageBox::Yes | QMessageBox::No);

        if (button == QMessageBox::No) {
            saveSettings(); // start over
            return;
        }
    }

    saveSettings(text);
}

void DkGenericProfileWidget::saveSettings(const QString &name) const
{
    if (mProfileList->findText(name) == -1)
        mProfileList->addItem(name);

    int idx = mProfileList->findText(name);
    if (idx != -1)
        mProfileList->setCurrentIndex(idx);
}

void DkGenericProfileWidget::activate(bool active)
{
    mProfileList->setVisible(active);
    mSaveButton->setVisible(active);
    mDeleteButton->setVisible(active);
    mEmpty = !active;

    if (active)
        loadSettings(mProfileList->currentText());

    update();
}

void DkGenericProfileWidget::setDefaultModel() const
{
    QSettings s(mSettingsPath, QSettings::IniFormat);
    s.beginGroup(mSettingsGroup);
    s.setValue("DefaultProfileString", mProfileList->currentText());
    s.endGroup();
}

QString DkGenericProfileWidget::loadDefaultProfileString() const
{
    QSettings s(mSettingsPath, QSettings::IniFormat);
    s.beginGroup(mSettingsGroup);
    QString defaultString = s.value("DefaultProfileString", "").toString();
    s.endGroup();

    return defaultString;
}

void DkGenericProfileWidget::paintEvent(QPaintEvent *ev)
{
    if (mEmpty) {
        // The view is empty.
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(200, 200, 200), Qt::BDiagPattern));
        p.drawRect(QRect(QPoint(), size()));

        p.setPen(QPen(QColor(100, 100, 100)));
        p.drawText(rect(), Qt::AlignCenter, tr("No Profiles"));
    } else
        DkNamedWidget::paintEvent(ev);
}

// DkTabEntryWidget --------------------------------------------------------------------
DkTabEntryWidget::DkTabEntryWidget(const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setObjectName("DkTabEntryWidget");

    QPixmap pm = DkImage::colorizePixmap(icon.pixmap(100), QColor(255, 255, 255));
    setIcon(pm);
    setIconSize(pm.size());

    setFlat(true);
    setCheckable(true);
}

void DkTabEntryWidget::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QPushButton::paintEvent(event);
}

// -------------------------------------------------------------------- DkDisplayWidget
DkDisplayWidget::DkDisplayWidget(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
    updateLayout();

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    setMinimumHeight(100);
}

QRect DkDisplayWidget::screenRect() const
{
    int idx = currentIndex();

    if (idx >= 0)
        return mScreens[idx]->availableGeometry();

    return QRect();
}

int DkDisplayWidget::count() const
{
    return mScreens.size();
}

int DkDisplayWidget::currentIndex() const
{
    for (int idx = 0; idx < mScreenButtons.size(); idx++) {
        if (mScreenButtons[idx]->isChecked())
            return idx;
    }

    return 0;
}

void DkDisplayWidget::setCurrentIndex(int index)
{
    mScreenButtons[index]->setChecked(true);
}

void DkDisplayWidget::resizeEvent(QResizeEvent *event)
{
    DkWidget::resizeEvent(event);
    updateLayout();
}

void DkDisplayWidget::createLayout()
{
    mScreens = QGuiApplication::screens();

    QButtonGroup *bg = new QButtonGroup(this);

    for (int idx = 0; idx < mScreens.size(); idx++) {
        QPushButton *sb = new QPushButton(QString::number(idx + 1), this);
        sb->setObjectName("displayButton");
        sb->setCheckable(true);
        sb->setFlat(true);

        bg->addButton(sb);
        mScreenButtons << sb;
    }
}

void DkDisplayWidget::updateLayout()
{
    QRect desktop;
    for (auto s : mScreens) {
        desktop = desktop.united(s->geometry());
    }

    int w = width() - 6;
    int h = height() - 6;

    QTransform tm;
    double sx = (double)w / desktop.width();
    double sy = (double)h / desktop.height();
    double s = qMin(sx, sy);
    tm.scale(s, s);

    QRect md = tm.mapRect(desktop);
    QPoint dxy(qRound((width() - md.width()) * 0.5), qRound((height() - md.height()) * 0.5));

    QScreen *myScreen = screen();

    for (int idx = 0; idx < mScreens.size(); idx++) {
        QRect r = mScreens[idx]->geometry();
        r.moveCenter(r.center() - desktop.topLeft());

        r = tm.mapRect(r);
        r.moveCenter(r.center() + dxy); // center

        if (mScreens[idx] == myScreen)
            mScreenButtons[idx]->setChecked(true);

        mScreenButtons[idx]->setGeometry(r);
    }
}

// void DkDisplayWidget::changeDisplay() {
//
//	QRect sr = screenRect();
//
//	DkNoMacsFrameless* w = dynamic_cast<DkNoMacsFrameless*>(DkUtils::getMainWindow());
//
//	if (w)
//		w->setGeometry(sr);
// }

}
