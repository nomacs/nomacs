/*******************************************************************************************************
 DkThumbsWidgets.cpp
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

#include "DkThumbsWidgets.h"

#include "DkActionManager.h"
#include "DkBasicLoader.h"
#include "DkDialog.h"
#include "DkImageContainer.h"
#include "DkImageLoader.h"
#include "DkImageStorage.h"
#include "DkMessageBox.h"
#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkShortcuts.h"
#include "DkStatusBar.h"
#include "DkThumbs.h"
#include "DkTimer.h"
#include "DkUtils.h"

#include <qpixmap.h>
#include <qpixmapcache.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStringBuilder>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>

namespace nmc
{

// DkFilePreview --------------------------------------------------------------------
DkFilePreview::DkFilePreview(DkThumbLoader *loader, QWidget *parent, Qt::WindowFlags flags)
    : DkFadeWidget(parent, flags)
    , mThumbLoader{loader}
{
    orientation = Qt::Horizontal;
    windowPosition = pos_north;

    init();
    // setStyleSheet("QToolTip{border: 0px; border-radius: 21px; color: white; background-color: red;}"); //" +
    // DkUtils::colorToString(mBgCol) + ";}");

    loadSettings();
    initOrientations();

    createContextMenu();

    connect(mThumbLoader,
            &DkThumbLoader::thumbnailLoaded,
            this,
            [this](const QString &filePath, const QImage &thumb, const bool fromExif) {
                if (!mThumbs.contains(filePath)) {
                    return;
                }
                mThumbs[filePath].image = thumb;
                mThumbs[filePath].fromExif = fromExif;
                mThumbs[filePath].loading = false;
                update();
            });

    connect(mThumbLoader, &DkThumbLoader::thumbnailLoadFailed, this, [this](const QString &filePath) {
        if (!mThumbs.contains(filePath)) {
            return;
        }
        mThumbs[filePath].notExist = true;
        mThumbs[filePath].loading = false;
        update();
    });
}

void DkFilePreview::init()
{
    setObjectName("DkFilePreview");
    setMouseTracking(true); // receive mouse event everytime

    // thumbsLoader = 0;

    xOffset = qRound(DkSettingsManager::param().effectiveThumbSize(this) * 0.1f);
    yOffset = qRound(DkSettingsManager::param().effectiveThumbSize(this) * 0.1f);

    currentDx = 0;
    currentFileIdx = -1;
    oldFileIdx = -1;
    mouseTrace = 0;
    scrollToCurrentImage = false;
    isPainted = false;

    winPercent = 0.1f;
    borderTrigger = (orientation == Qt::Horizontal) ? (float)width() * winPercent : (float)height() * winPercent;

    worldMatrix = QTransform();

    moveImageTimer = new QTimer(this);
    moveImageTimer->setInterval(5); // reduce cpu utilization
    connect(moveImageTimer, &QTimer::timeout, this, &DkFilePreview::moveImages);

    int borderTriggerI = qRound(borderTrigger);
    leftGradient = (orientation == Qt::Horizontal) ? QLinearGradient(QPoint(0, 0), QPoint(borderTriggerI, 0))
                                                   : QLinearGradient(QPoint(0, 0), QPoint(0, borderTriggerI));
    rightGradient = (orientation == Qt::Horizontal)
        ? QLinearGradient(QPoint(width() - borderTriggerI, 0), QPoint(width(), 0))
        : QLinearGradient(QPoint(0, height() - borderTriggerI), QPoint(0, height()));
    leftGradient.setColorAt(1, Qt::white);
    leftGradient.setColorAt(0, Qt::black);
    rightGradient.setColorAt(1, Qt::black);
    rightGradient.setColorAt(0, Qt::white);

    minHeight = DkSettingsManager::param().effectiveThumbSize(this) + yOffset;
    // resize(parent->width(), minHeight);

    selected = -1;

    // wheel label
    QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.svg");
    wheelButton = new QLabel(this);
    wheelButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    wheelButton->setPixmap(wp);
    wheelButton->hide();

    auto nomacs = dynamic_cast<DkNoMacs *>(DkUtils::getMainWindow());
    if (nomacs != nullptr) {
        connect(this, &DkFilePreview::showThumbsDockSignal, nomacs, &DkNoMacs::showThumbsDock);
    }
}

void DkFilePreview::initOrientations()
{
    if (windowPosition == pos_north || windowPosition == pos_south || windowPosition == pos_dock_hor)
        orientation = Qt::Horizontal;
    else if (windowPosition == pos_east || windowPosition == pos_west || windowPosition == pos_dock_ver)
        orientation = Qt::Vertical;

    if (windowPosition == pos_dock_ver || windowPosition == pos_dock_hor)
        minHeight = max_thumb_size;
    else
        minHeight = DkSettingsManager::param().effectiveThumbSize(this);

    if (orientation == Qt::Horizontal) {
        setMinimumSize(20, 20);
        setMaximumSize(QWIDGETSIZE_MAX, minHeight);
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        borderTrigger = (float)width() * winPercent;
        int borderTriggerI = qRound(borderTrigger);
        leftGradient = QLinearGradient(QPoint(0, 0), QPoint(borderTriggerI, 0));
        rightGradient = QLinearGradient(QPoint(width() - borderTriggerI, 0), QPoint(width(), 0));
    } else {
        setMinimumSize(20, 20);
        setMaximumSize(minHeight, QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        borderTrigger = (float)height() * winPercent;
        int borderTriggerI = qRound(borderTrigger);
        leftGradient = QLinearGradient(QPoint(0, 0), QPoint(0, borderTriggerI));
        rightGradient = QLinearGradient(QPoint(0, height() - borderTriggerI), QPoint(0, height()));
    }

    leftGradient.setColorAt(1, Qt::white);
    leftGradient.setColorAt(0, Qt::black);
    rightGradient.setColorAt(1, Qt::black);
    rightGradient.setColorAt(0, Qt::white);

    worldMatrix.reset();
    currentDx = 0;
    scrollToCurrentImage = true;
    update();
}

void DkFilePreview::loadSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());
    windowPosition = settings.value("windowPosition", windowPosition).toInt();
    settings.endGroup();
}

void DkFilePreview::saveSettings()
{
    DefaultSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("windowPosition", windowPosition);
    settings.endGroup();
}

void DkFilePreview::createContextMenu()
{
    contextMenuActions.resize(cm_end - 1); // -1 because we just need to know of one dock widget

    contextMenuActions[cm_pos_west] = new QAction(tr("Show Left"), this);
    contextMenuActions[cm_pos_west]->setStatusTip(tr("Shows the Thumbnail Bar on the Left"));
    connect(contextMenuActions[cm_pos_west], &QAction::triggered, this, &DkFilePreview::newPosition);

    contextMenuActions[cm_pos_north] = new QAction(tr("Show Top"), this);
    contextMenuActions[cm_pos_north]->setStatusTip(tr("Shows the Thumbnail Bar at the Top"));
    connect(contextMenuActions[cm_pos_north], &QAction::triggered, this, &DkFilePreview::newPosition);

    contextMenuActions[cm_pos_east] = new QAction(tr("Show Right"), this);
    contextMenuActions[cm_pos_east]->setStatusTip(tr("Shows the Thumbnail Bar on the Right"));
    connect(contextMenuActions[cm_pos_east], &QAction::triggered, this, &DkFilePreview::newPosition);

    contextMenuActions[cm_pos_south] = new QAction(tr("Show Bottom"), this);
    contextMenuActions[cm_pos_south]->setStatusTip(tr("Shows the Thumbnail Bar at the Bottom"));
    connect(contextMenuActions[cm_pos_south], &QAction::triggered, this, &DkFilePreview::newPosition);

    contextMenuActions[cm_pos_dock_hor] = new QAction(tr("Undock"), this);
    contextMenuActions[cm_pos_dock_hor]->setStatusTip(tr("Undock the thumbnails"));
    connect(contextMenuActions[cm_pos_dock_hor], &QAction::triggered, this, &DkFilePreview::newPosition);

    contextMenu = new QMenu(tr("File Preview Menu"), this);
    contextMenu->addActions(contextMenuActions.toList());
}

void DkFilePreview::paintEvent(QPaintEvent *)
{
    // render nothing if there are no thumbs
    if (mFiles.empty())
        return;

    if (minHeight != DkSettingsManager::param().effectiveThumbSize(this) + yOffset && windowPosition != pos_dock_hor
        && windowPosition != pos_dock_ver) {
        xOffset = qCeil(DkSettingsManager::param().effectiveThumbSize(this) * 0.1f);
        yOffset = qCeil(DkSettingsManager::param().effectiveThumbSize(this) * 0.1f);

        minHeight = DkSettingsManager::param().effectiveThumbSize(this) + yOffset;

        if (orientation == Qt::Horizontal)
            setMaximumSize(QWIDGETSIZE_MAX, minHeight);
        else
            setMaximumSize(minHeight, QWIDGETSIZE_MAX);
    }

    QPainter painter(this);
    painter.setBackground(DkSettingsManager::param().display().hudBgColor);

    painter.setPen(Qt::NoPen);
    painter.setBrush(DkSettingsManager::param().display().hudBgColor);

    if (windowPosition != pos_dock_hor && windowPosition != pos_dock_ver) {
        QRect r = QRect(QPoint(), this->size());
        painter.drawRect(r);
    }

    painter.setWorldTransform(worldMatrix);
    painter.setWorldMatrixEnabled(true);

    drawThumbs(&painter);

    if (currentFileIdx != oldFileIdx && currentFileIdx >= 0) {
        oldFileIdx = currentFileIdx;
        scrollToCurrentImage = true;
        moveImageTimer->start();
    }
    isPainted = true;
}

void DkFilePreview::drawThumbs(QPainter *painter)
{
    // qDebug() << "drawing thumbs: " << worldMatrix.dx();

    bufferDim = (orientation == Qt::Horizontal) ? QRectF(QPointF(0, yOffset / 2), QSize(xOffset, 0))
                                                : QRectF(QPointF(yOffset / 2, 0), QSize(0, xOffset));
    thumbRects.clear();

    DkTimer dt;

    // mouse over effect
    QPoint p = worldMatrix.inverted().map(mapFromGlobal(QCursor::pos()));

    for (int idx = 0; static_cast<unsigned int>(idx) < mFiles.size(); idx++) {
        const QString &filePath = mFiles[idx].path();

        const auto thumb = mThumbs.constFind(filePath);
        bool existsInTable = thumb != mThumbs.constEnd();

        if (existsInTable && thumb->notExist) {
            thumbRects.push_back(QRectF());
            continue;
        }

        QImage img;
        if (existsInTable && !thumb->image.isNull()) {
            img = thumb->image;
        }

        // if (img.width() > max_thumb_size * DkSettingsManager::param().dpiScaleFactor())
        //	qDebug() << thumb->getFilePath() << "size:" << img.size();

        QPointF anchor = orientation == Qt::Horizontal ? bufferDim.topRight() : bufferDim.bottomLeft();
        QRectF r = !img.isNull() ? QRectF(anchor, img.size() / devicePixelRatio())
                                 : QRectF(anchor,
                                          QSize(DkSettingsManager::param().effectiveThumbSize(this),
                                                DkSettingsManager::param().effectiveThumbSize(this)));
        if (orientation == Qt::Horizontal && height() - yOffset < r.height() * 2)
            r.setSize(QSizeF(qFloor(r.width() * (float)(height() - yOffset) / r.height()), height() - yOffset));
        else if (orientation == Qt::Vertical && width() - yOffset < r.width() * 2)
            r.setSize(QSizeF(width() - yOffset, qFloor(r.height() * (float)(width() - yOffset) / r.width())));

        // check if the size is still valid
        if (r.width() < 1 || r.height() < 1)
            continue; // this brings us in serious problems with the selection

        // center vertically
        if (orientation == Qt::Horizontal)
            r.moveCenter(QPoint(qFloor(r.center().x()), height() / 2));
        else
            r.moveCenter(QPoint(width() / 2, qFloor(r.center().y())));

        // update the buffer dim
        if (orientation == Qt::Horizontal)
            bufferDim.setRight(qFloor(bufferDim.right() + r.width()) + qCeil(xOffset / 2.0f));
        else
            bufferDim.setBottom(qFloor(bufferDim.bottom() + r.height()) + qCeil(xOffset / 2.0f));
        thumbRects.push_back(r);

        QRectF imgWorldRect = worldMatrix.mapRect(r);

        // update file rect for move to current file timer
        if (scrollToCurrentImage && idx == currentFileIdx)
            newFileRect = imgWorldRect;

        // is the current image within the canvas?
        const bool oobStart = (orientation == Qt::Horizontal && imgWorldRect.right() < 0)
            || (orientation == Qt::Vertical && imgWorldRect.bottom() < 0);
        const bool oobEnd = (orientation == Qt::Horizontal && imgWorldRect.left() > width())
            || (orientation == Qt::Vertical && imgWorldRect.top() > height());

        if (oobStart || oobEnd) {
            if (existsInTable && thumb->loading) {
                mThumbLoader->cancelThumbnailRequest(mThumbs[filePath].request);
                mThumbs.remove(filePath);
            }

            if (oobEnd && !scrollToCurrentImage) {
                break;
            }
            continue;
        }

        // only fetch thumbs if we are not moving too fast...
        if (!existsInTable) {
            int size = max_thumb_size;
            ScaleConstraint constraint = orientation == Qt::Horizontal ? ScaleConstraint::height
                                                                       : ScaleConstraint::width;
            LoadThumbnailOption option = LoadThumbnailOption::none;
            if (DkSettingsManager::param().display().highQualityThumbs) {
                size = r.height() * this->devicePixelRatio();
                option = LoadThumbnailOption::force_size;
            }

            Thumb newThumb;
            newThumb.loading = true;
            newThumb.request = LoadThumbnailRequest{filePath, option, size, constraint};

            mThumbs.insert(filePath, newThumb);
            mThumbLoader->requestThumbnail(newThumb.request);
        }

        bool isLeftGradient = (orientation == Qt::Horizontal && worldMatrix.dx() < 0
                               && imgWorldRect.left() < leftGradient.finalStop().x())
            || (orientation == Qt::Vertical && worldMatrix.dy() < 0
                && imgWorldRect.top() < leftGradient.finalStop().y());
        bool isRightGradient = (orientation == Qt::Horizontal && imgWorldRect.right() > rightGradient.start().x())
            || (orientation == Qt::Vertical && imgWorldRect.bottom() > rightGradient.start().y());
        // show that there are more images...
        if (isLeftGradient && !img.isNull())
            img = applyFadeOut(leftGradient, imgWorldRect, img);
        if (isRightGradient && !img.isNull())
            img = applyFadeOut(rightGradient, imgWorldRect, img);

        bool unscaled = std::abs(r.height() * devicePixelRatio() - img.height()) < 1.0;
        painter->setRenderHint(QPainter::SmoothPixmapTransform, !unscaled);
        if (unscaled) {
            qreal adj = img.height() / devicePixelRatio() - r.height();
            adj /= 2.0;
            r = r.adjusted(-adj, -adj, adj, adj);
        }

        if (!img.isNull())
            painter->drawImage(r, img);
        else
            drawNoImgEffect(painter, r);

        if (idx == currentFileIdx)
            drawCurrentImgEffect(painter, r);
        else if (idx == selected && r.contains(p))
            drawSelectedEffect(painter, r);

        // painter->fillRect(QRect(0,0,200, 110), leftGradient);
    }
}

void DkFilePreview::drawNoImgEffect(QPainter *painter, const QRectF &r)
{
    QBrush oldBrush = painter->brush();
    QPen oldPen = painter->pen();

    QPen noImgPen(DkSettingsManager::param().display().bgColor);
    painter->setPen(noImgPen);
    painter->setBrush(QColor(0, 0, 0, 0));
    painter->drawRect(r);
    painter->setPen(oldPen);
    painter->setBrush(oldBrush);
}

void DkFilePreview::drawSelectedEffect(QPainter *painter, const QRectF &r)
{
    QBrush oldBrush = painter->brush();
    qreal oldOp = painter->opacity();

    // drawing
    painter->setOpacity(0.4);
    painter->setBrush(DkSettingsManager::param().display().highlightColor);
    painter->drawRect(r);

    // reset painter
    painter->setOpacity(oldOp);
    painter->setBrush(oldBrush);
}

void DkFilePreview::drawCurrentImgEffect(QPainter *painter, const QRectF &r)
{
    QPen oldPen = painter->pen();
    QBrush oldBrush = painter->brush();
    auto oldOp = (float)painter->opacity();

    // draw
    QRectF cr = r;
    cr.setSize(QSize((int)cr.width() + 1, (int)cr.height() + 1));
    cr.moveCenter(cr.center() + QPointF(-1, -1));

    QPen cPen(DkSettingsManager::param().display().highlightColor, 1);
    painter->setBrush(QColor(0, 0, 0, 0));
    painter->setOpacity(1.0);
    painter->setPen(cPen);
    painter->drawRect(cr);

    painter->setOpacity(0.5);
    cr.setSize(QSize((int)cr.width() + 2, (int)cr.height() + 2));
    cr.moveCenter(cr.center() + QPointF(-1, -1));
    painter->drawRect(cr);

    painter->setBrush(oldBrush);
    painter->setOpacity(oldOp);
    painter->setPen(oldPen);
}

QImage DkFilePreview::applyFadeOut(const QLinearGradient &gradient, const QRectF &imgRect, const QImage &img)
{
    if (img.format() == QImage::Format_Indexed8)
        return img;

    // compute current scaling
    QPointF scale(img.width() / imgRect.width(), img.height() / imgRect.height());
    QTransform wm;
    wm.scale(scale.x(), scale.y());

    if (orientation == Qt::Horizontal)
        wm.translate(-imgRect.left(), 0);
    else
        wm.translate(0, -imgRect.top());

    QLinearGradient imgGradient = gradient;

    if (orientation == Qt::Horizontal) {
        imgGradient.setStart(wm.map(gradient.start()).x(), 0);
        imgGradient.setFinalStop(wm.map(gradient.finalStop()).x(), 0);
    } else {
        imgGradient.setStart(0, wm.map(gradient.start()).y());
        imgGradient.setFinalStop(0, wm.map(gradient.finalStop()).y());
    }

    QImage mask = QImage(img.size(), QImage::Format_Grayscale8);
    QPainter painter(&mask);
    painter.fillRect(mask.rect(), Qt::black);
    painter.fillRect(mask.rect(), imgGradient);
    painter.end();

    QImage newImage = img;
    newImage.setAlphaChannel(mask);
    return newImage;
}

void DkFilePreview::resizeEvent(QResizeEvent *event)
{
    QWidget *pw = qobject_cast<QWidget *>(parent());

    if (event->size() == event->oldSize()
        && ((orientation == Qt::Horizontal && pw && this->width() == pw->width())
            || (orientation == Qt::Vertical && pw && this->height() == pw->height()))) {
        return;
    }

    if (currentFileIdx >= 0 && isVisible()) {
        scrollToCurrentImage = true;
        moveImageTimer->start();
    }

    // now update...
    borderTrigger = (orientation == Qt::Horizontal) ? (float)width() * winPercent : (float)height() * winPercent;
    int borderTriggerI = qRound(borderTrigger);
    leftGradient.setFinalStop((orientation == Qt::Horizontal) ? QPoint(borderTriggerI, 0) : QPoint(0, borderTriggerI));
    rightGradient.setStart((orientation == Qt::Horizontal) ? QPoint(width() - borderTriggerI, 0)
                                                           : QPoint(0, height() - borderTriggerI));
    rightGradient.setFinalStop((orientation == Qt::Horizontal) ? QPoint(width(), 0) : QPoint(0, height()));

    // update();
    QWidget::resizeEvent(event);
}

void DkFilePreview::mouseMoveEvent(QMouseEvent *event)
{
    if (lastMousePos.isNull()) {
        lastMousePos = event->pos();
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (mouseTrace < 21) {
        mouseTrace += qRound(fabs(QPointF(lastMousePos - event->pos()).manhattanLength()));
        return;
    }

    float eventPos = orientation == Qt::Horizontal ? (float)event->pos().x() : (float)event->pos().y();
    float lastMousePosC = orientation == Qt::Horizontal ? (float)lastMousePos.x() : (float)lastMousePos.y();
    int limit = orientation == Qt::Horizontal ? width() : height();

    if (event->buttons() == Qt::MiddleButton) {
        float enterPosC = orientation == Qt::Horizontal ? (float)enterPos.x() : (float)enterPos.y();
        float dx = std::fabs((float)(enterPosC - eventPos)) * 0.015f;
        dx = std::exp(dx);

        if (enterPosC - eventPos < 0)
            dx = -dx;

        currentDx = dx; // update dx
        return;
    }

    int mouseDir = qRound(eventPos - lastMousePosC);

    if (event->buttons() == Qt::LeftButton) {
        currentDx = (float)mouseDir;
        lastMousePos = event->pos();
        selected = -1;
        setCursor(Qt::ClosedHandCursor);
        scrollToCurrentImage = false;
        moveImages();
        return;
    }

    if (cursor().shape() != Qt::ArrowCursor)
        setCursor(Qt::ArrowCursor);

    int ndx = limit - qRound(eventPos);
    int pdx = qRound(eventPos);

    bool left = pdx < ndx;
    float dx = (left) ? (float)pdx : (float)ndx;

    if (dx < borderTrigger && ((mouseDir < 0 && left) || (mouseDir > 0 && !left))) {
        dx = std::exp((borderTrigger - dx) / borderTrigger * 3);
        currentDx = (left) ? dx : -dx;

        scrollToCurrentImage = false;
        moveImageTimer->start();
    } else if (dx > borderTrigger && !scrollToCurrentImage)
        moveImageTimer->stop();

    // select the current thumbnail
    if (dx > borderTrigger * 0.5) {
        int oldSelection = selected;
        selected = -1;

        // find out where the mouse is
        for (int idx = 0; idx < thumbRects.size(); idx++) {
            if (worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
                selected = idx;

                if (selected >= 0 && static_cast<unsigned int>(selected) < mFiles.size()) {
                    // selectedImg = DkImage::colorizePixmap(QPixmap::fromImage(thumb->getImage()),
                    // DkSettingsManager::param().display().highlightColor, 0.3f);

                    // important: setText shows the label - if you then hide it here again you'll get a stack overflow
                    // if (fileLabel->height() < height())
                    //	fileLabel->setText(thumbs.at(selected).getFile().fileName(), -1);
                    const DkFileInfo &fileInfo = mFiles[selected];
                    auto thumb = mThumbs.constFind(fileInfo.path());
                    Q_ASSERT(thumb != mThumbs.constEnd());

                    QString str = QObject::tr("Name: ") % fileInfo.fileName() % "\n" % QObject::tr("Size: ")
                        % DkUtils::readableByte((float)fileInfo.size()) % "\n" % QObject::tr("Created: ")
                        % fileInfo.birthTime().toString();
                    if (!thumb->notExist) {
                        const QImage &img = thumb->image;
                        str = str % "\n" % QObject::tr("Thumb: ") % QString::number(img.size().width()) % "x"
                            % QString::number(img.size().height()) % " "
                            % (thumb->fromExif ? QObject::tr("Embedded ") : "");
                    }
                    setToolTip(str);
                    setStatusTip(fileInfo.fileName());
                }
                break;
            }
        }

        if (selected != -1 || selected != oldSelection)
            update();
        // else if (selected == -1)
        //	fileLabel->hide();
    } else
        selected = -1;

    if (selected == -1)
        setToolTip(tr("CTRL+Zoom resizes the thumbnails"));

    lastMousePos = event->pos();

    // QWidget::mouseMoveEvent(event);
}

void DkFilePreview::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        mouseTrace = 0;
    } else if (event->buttons() == Qt::MiddleButton) {
        enterPos = event->pos();
        qDebug() << "stop scrolling (middle button)";
        scrollToCurrentImage = false;
        moveImageTimer->start();

        // show icon
        wheelButton->move(event->pos().x() - 16, event->pos().y() - 16);
        wheelButton->show();
    }
}

void DkFilePreview::mouseReleaseEvent(QMouseEvent *event)
{
    currentDx = 0;
    moveImageTimer->stop();
    wheelButton->hide();

    if (mouseTrace < 20) {
        // find out where the mouse did click
        for (int idx = 0; idx < thumbRects.size(); idx++) {
            if (worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
                emit changeFileSignal(idx - currentFileIdx);
                return;
            }
        }
    } else
        unsetCursor();
}

void DkFilePreview::wheelEvent(QWheelEvent *event)
{
    auto delta = event->angleDelta().y();
    if (event->modifiers() == Qt::CTRL && windowPosition != pos_dock_hor && windowPosition != pos_dock_ver) {
        int newSize = DkSettingsManager::param().display().thumbSize;
        newSize += qRound(delta * 0.05f);

        // make sure it is even
        if (qRound(newSize * 0.5f) != newSize * 0.5f)
            newSize++;

        if (newSize < 8)
            newSize = 8;
        else if (newSize > max_thumb_size)
            newSize = max_thumb_size;

        if (newSize != DkSettingsManager::param().display().thumbSize) {
            DkSettingsManager::param().display().thumbSize = newSize;
            update();
        }
    } else if (delta != 0) {
        int fc = (delta > 0) ? -1 : 1;

        if (!DkSettingsManager::param().resources().waitForLastImg) {
            currentFileIdx += fc;
            scrollToCurrentImage = true;
        }
        emit changeFileSignal(fc);
    }

    // accept the event here - so it won't be propagated to the viewport
    event->accept();
}

void DkFilePreview::leaveEvent(QEvent *)
{
    selected = -1;
    if (!scrollToCurrentImage) {
        moveImageTimer->stop();
    }
    update();
}

void DkFilePreview::contextMenuEvent(QContextMenuEvent *event)
{
    contextMenu->exec(event->globalPos());
    event->accept();

    DkFadeWidget::contextMenuEvent(event);
}

void DkFilePreview::newPosition()
{
    const auto *sender = static_cast<QAction *>(QObject::sender());
    if (!sender)
        return;

    int pos = 0;
    Qt::Orientation orient = Qt::Horizontal;

    if (sender == contextMenuActions[cm_pos_west]) {
        pos = pos_west;
        orient = Qt::Vertical;
    } else if (sender == contextMenuActions[cm_pos_east]) {
        pos = pos_east;
        orient = Qt::Vertical;
    } else if (sender == contextMenuActions[cm_pos_north]) {
        pos = pos_north;
        orient = Qt::Horizontal;
    } else if (sender == contextMenuActions[cm_pos_south]) {
        pos = pos_south;
        orient = Qt::Horizontal;
    } else if (sender == contextMenuActions[cm_pos_dock_hor]) {
        pos = pos_dock_hor;
        orient = Qt::Horizontal;
    }

    // don't apply twice
    if (windowPosition == pos || (pos == pos_dock_hor && windowPosition == pos_dock_ver))
        return;

    windowPosition = pos;
    orientation = orient;
    initOrientations();
    emit positionChangeSignal(windowPosition);

    hide();
    show();

    // emit showThumbsDockSignal(true);
}

void DkFilePreview::moveImages()
{
    if (!isVisible()) {
        moveImageTimer->stop();
        return;
    }

    int limit = orientation == Qt::Horizontal ? width() : height();
    float center = orientation == Qt::Horizontal ? (float)newFileRect.center().x() : (float)newFileRect.center().y();

    if (scrollToCurrentImage) {
        float cDist = limit / 2.0f - center;

        if (mFiles.size() < 2000) {
            if (fabs(cDist) < limit) {
                currentDx = sqrt(fabs(cDist)) / 1.3f;
                if (cDist < 0)
                    currentDx *= -1.0f;
            } else
                currentDx = cDist / 4.0f;
        }
        // this is not too beautiful - but fast
        else
            currentDx = cDist / 2.0f;

        if (fabs(currentDx) < 2)
            currentDx = (currentDx < 0) ? -2.0f : 2.0f;

        // end position
        if (fabs(cDist) <= 2) {
            currentDx = limit / 2.0f - center;
            moveImageTimer->stop();
            scrollToCurrentImage = false;
        } else
            isPainted = false;
    }

    float translation = orientation == Qt::Horizontal ? (float)worldMatrix.dx() : (float)worldMatrix.dy();
    float bufferPos = orientation == Qt::Horizontal ? (float)bufferDim.right() : (float)bufferDim.bottom();

    // do not scroll out of the thumbs
    if ((translation >= limit * 0.5 && currentDx > 0)
        || (translation <= -(bufferPos - limit * 0.5 + xOffset) && currentDx < 0))
        return;

    // set the last step to match the center of the screen...	(nicer if user scrolls very fast)
    if (translation < limit * 0.5 && currentDx > 0 && translation + currentDx > limit * 0.5 && currentDx > 0)
        currentDx = limit * 0.5f - translation;
    else if (translation > -(bufferPos - limit * 0.5 + xOffset)
             && translation + currentDx <= -(bufferPos - limit * 0.5 + xOffset) && currentDx < 0)
        currentDx = -(bufferPos - limit * 0.5f + xOffset + (float)worldMatrix.dx());

    // qDebug() << "currentDx: " << currentDx;
    if (orientation == Qt::Horizontal)
        worldMatrix.translate(currentDx, 0);
    else
        worldMatrix.translate(0, currentDx);
    // qDebug() << "dx: " << worldMatrix.dx();
    update();
}

void DkFilePreview::setFileInfo(QSharedPointer<DkImageContainerT> cImage)
{
    if (!cImage)
        return;

    int tIdx = -1;

    for (int idx = 0; static_cast<unsigned int>(idx) < mFiles.size(); idx++) {
        if (mFiles[idx] == cImage->originalFileInfo()) {
            tIdx = idx;
            break;
        }
    }

    currentFileIdx = tIdx;
    if (currentFileIdx >= 0)
        scrollToCurrentImage = true;
    update();
}

void DkFilePreview::updateThumbs(QVector<QSharedPointer<DkImageContainerT>> images)
{
    mThumbs.clear();
    mFiles.resize(images.size());
    for (int idx = 0; idx < images.size(); idx++) {
        const auto &imgC = images[idx];
        mFiles[idx] = imgC->originalFileInfo();
        if (imgC->isSelected()) {
            currentFileIdx = idx;
        }
    }

    update();
}

void DkFilePreview::setVisible(bool visible, bool saveSettings)
{
    DkFadeWidget::setVisible(visible, saveSettings);
    if (mSetWidgetVisible)
        return; // prevent recursion via fade()

    emit showThumbsDockSignal(visible);
}

// DkThumbLabel --------------------------------------------------------------------
DkThumbLabel::DkThumbLabel(DkThumbLoader *thumbLoader,
                           const DkFileInfo &fileInfo,
                           bool fillSquare,
                           QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mText(this)
    , mFilePath{fileInfo.path()}
    , mThumbLoader{thumbLoader}
    , mFillSquare{fillSquare}

{
    // clang-format off
    mTooltip =
        QObject::tr("Name: ") % fileInfo.fileName() % "\n" %
        QObject::tr("Size: ") % DkUtils::readableByte((float)fileInfo.size()) % "\n" %
        QObject::tr("Created: ") % fileInfo.birthTime().toString();
    // clang-format on
    setToolTip(mTooltip);

    QColor col = DkSettingsManager::param().display().highlightColor;
    col.setAlpha(90);
    mSelectBrush = col;
    mSelectPen.setColor(DkSettingsManager::param().display().highlightColor);

    QFont font;
    font.setBold(false);
    font.setPointSize(9); // two sizes smaller than default font see:stylesheet.css
    mText.setFont(font);
    mText.setPlainText(fileInfo.fileName());
    mText.setDefaultTextColor(QColor(255, 255, 255));
    mText.hide();

    // Not loaded -> disable selection
    setFlag(ItemIsSelectable, false);

    setAcceptHoverEvents(true);
    connect(mThumbLoader, &DkThumbLoader::thumbnailLoaded, this, &DkThumbLabel::onThumbnailLoaded);
    connect(mThumbLoader, &DkThumbLoader::thumbnailLoadFailed, this, &DkThumbLabel::onThumbnailLoadFailed);
}

void DkThumbLabel::onThumbnailLoaded(const QString &filePath, const QImage &thumb, bool fromExif)
{
    if (filePath != mFilePath) {
        return;
    }

    mFetchingThumb = false;
    mThumbRequest = {};

    // update label
    mText.setPos(0, DkSettingsManager::param().effectiveThumbPreviewSize());

    prepareGeometryChange();
    generatePixmap(thumb);
    updateTooltip(thumb, fromExif);
    update();
    setFlag(ItemIsSelectable, true);
}

void DkThumbLabel::onThumbnailLoadFailed(const QString &filePath)
{
    if (filePath != mFilePath) {
        return;
    }
    mThumbNotExist = true;
    mFetchingThumb = false;
    mThumbRequest = {};
    update();
}

DkThumbLabel::~DkThumbLabel() = default;

void DkThumbLabel::updateTooltip(const QImage &thumb, bool fromExif)
{
    // clang-format off
    const QString str = mTooltip % "\n" %
            QObject::tr("Thumb: ") %
            QString::number(thumb.size().width()) % "x" % QString::number(thumb.size().height()) % " " %
            (fromExif ? QObject::tr("Embedded ") : "");
    // clang-format on

    setToolTip(str);
}

void DkThumbLabel::cancelLoading()
{
    if (!mFetchingThumb) {
        return;
    }
    mThumbLoader->cancelThumbnailRequest(mThumbRequest);
    mFetchingThumb = false;
    mThumbRequest = {};
}

QRectF DkThumbLabel::boundingRect() const
{
    int sz = DkSettingsManager::param().effectiveThumbPreviewSize();
    return QRectF(QPoint(0, 0), QSize(sz, sz));
}

QPainterPath DkThumbLabel::shape() const
{
    QPainterPath path;

    path.addRect(boundingRect());
    return path;
}

void DkThumbLabel::generatePixmap(const QImage &thumb)
{
    QRectF br = boundingRect();
    QSizeF imgSize = thumb.size();

    QRectF srcRect;
    QRectF targetRect;

    if (mFillSquare) {
        srcRect = QRectF(QPoint(), br.size().scaled(imgSize, Qt::KeepAspectRatio));
        targetRect = br;
        srcRect.moveCenter(thumb.rect().center());
    } else {
        srcRect = thumb.rect();
        targetRect = QRectF(QPoint(), imgSize.scaled(br.size(), Qt::KeepAspectRatio));
        targetRect.moveCenter(br.center());
    }

    bool unscaled = std::abs(targetRect.height() * mDevicePixelRatio - srcRect.height()) < 1.0;
    if (unscaled) {
        // we have a device-scaled thumbnail, but rounding will result in interpolation
        // adjust to get closer to ideal scale factor and reduce this
        qreal scaleFactor = srcRect.height() / targetRect.height();
        qreal scaleDiff = mDevicePixelRatio - scaleFactor;
        qreal dx = srcRect.width() * scaleDiff / 2.0;
        qreal dy = srcRect.height() * scaleDiff / 2.0;
        srcRect = srcRect.adjusted(-dx, -dy, dx, dy);

        // qreal adjFactorH = mDevicePixelRatio - (srcRect.height() / targetRect.height());
        // qreal adjFactorW = mDevicePixelRatio - (srcRect.width() / targetRect.width());
        // qDebug() << "SF" << scaleDiff << adjFactorH << adjFactorW;
    }

    const QSize pixmapSize = (br.size() * mDevicePixelRatio).toSize();
    QPixmap pm(pixmapSize);
    pm.setDevicePixelRatio(mDevicePixelRatio);
    pm.fill(Qt::transparent);

    QPainter painter(&pm);
    if (!unscaled) {
        painter.setRenderHints(QPainter::SmoothPixmapTransform);
    }
    painter.drawImage(targetRect, thumb, srcRect);

    if (mPixmapKey) {
        QPixmapCache::remove(mPixmapKey.value());
    }
    mPixmapKey = QPixmapCache::insert(pm);
}

void DkThumbLabel::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit loadFileSignal(QString(mFilePath), event->modifiers() == Qt::ControlModifier);
    // FIXME: `this` could already be destroyed after this line due to the slot connected to
    // loadFileSignal updates the directory.
}

void DkThumbLabel::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    mIsHovered = true;
    emit showFileSignal(mFilePath);
    update();
}

void DkThumbLabel::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    mIsHovered = false;
    emit showFileSignal();
    update();
}

std::optional<QPixmap> DkThumbLabel::pixmap() const
{
    if (!mPixmapKey) {
        return std::nullopt;
    }
    QPixmap pm;
    bool found = QPixmapCache::find(mPixmapKey.value(), &pm);
    if (!found) {
        return std::nullopt;
    }
    return pm;
}

void DkThumbLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    std::optional<QPixmap> pm = pixmap();

    mThumbOption = DkSettingsManager::param().display().highQualityThumbs ? LoadThumbnailOption::force_size
                                                                          : LoadThumbnailOption::none;
    mDevicePixelRatio = painter->device()->devicePixelRatio();
    const QSize pixmapSize = (boundingRect().size() * mDevicePixelRatio).toSize();

    if (!mFetchingThumb && (!pm || pm->size() != pixmapSize) && !mThumbNotExist) {
        // setting fetching flag first, because requestThumbnail might return thumbnail immediately
        // This avoids stucking infinitely in fetching thumb state
        mFetchingThumb = true;
        int maxSize = mThumbOption == LoadThumbnailOption::force_size ? pixmapSize.height() : max_thumb_size;
        auto constraint = DkSettingsManager::param().display().displaySquaredThumbs ? ScaleConstraint::shortest_side
                                                                                    : ScaleConstraint::longest_side;
        mThumbRequest = LoadThumbnailRequest{mFilePath, mThumbOption, maxSize, constraint};
        mThumbLoader->requestThumbnail(mThumbRequest);

        // It is possible we have pixmap now (from cache), check again.
        pm = pixmap();
    }

    if (mThumbNotExist) {
        painter->setPen(sNoImagePen);
        painter->setBrush(sNoImageBrush);
        painter->drawRect(boundingRect());
    } else if (!pm) {
        QColor c = DkSettingsManager::param().display().highlightColor;
        c.setAlpha(30);
        painter->setPen(sNoImagePen);
        painter->setBrush(c);

        QRectF r = boundingRect();
        painter->drawRect(r);
    } else {
        painter->drawPixmap(boundingRect(), pm.value(), pm->rect());
    }

    // draw text
    if (boundingRect().width() > 50 && DkSettingsManager::param().display().showThumbLabel) {
        // this is the Qt idea of how to fix the dashed border:
        // http://www.qtcentre.org/threads/23087-How-to-hide-the-dashed-frame-outside-the-QGraphicsItem
        // I don't think it's beautiful...
        QStyleOptionGraphicsItem noSelOption;
        if (option) {
            noSelOption = *option;
            noSelOption.state &= ~QStyle::State_Selected;
        }
        painter->save();

        painter->translate(0, boundingRect().height() - mText.boundingRect().height());

        QRectF r = mText.boundingRect();
        r.setWidth(boundingRect().width());
        painter->setPen(Qt::NoPen);
        painter->setBrush(DkSettingsManager::param().display().hudBgColor);
        painter->drawRect(r);
        mText.paint(painter, &noSelOption, widget);
        painter->restore();
    }

    // render hovered
    if (mIsHovered) {
        painter->setBrush(QColor(255, 255, 255, 60));
        painter->setPen(sNoImagePen);
        // painter->setPen(Qt::NoPen);
        painter->drawRect(boundingRect());
    }

    // render selected
    if (isSelected()) {
        painter->setBrush(mSelectBrush);
        painter->setPen(mSelectPen);
        painter->drawRect(boundingRect());
    }
}

QString DkThumbLabel::filePath() const
{
    return mFilePath;
}

QImage DkThumbLabel::image() const
{
    const std::optional<QPixmap> pm = pixmap();
    if (!pm) {
        return {};
    }
    return pm->toImage();
}

void DkThumbLabel::setFillSquare(bool value)
{
    mFillSquare = value;
    // Instead of requesting thumbnail right away,
    // we reset the pixmap and call update,
    // so only the visible ones get loaded.
    if (mPixmapKey) {
        QPixmapCache::remove(mPixmapKey.value());
    }
    mPixmapKey = std::nullopt;
    update();
}

// DkThumbWidget --------------------------------------------------------------------
DkThumbScene::DkThumbScene(DkThumbLoader *thumbLoader, QWidget *parent /* = 0 */)
    : QGraphicsScene(parent)
    , mThumbLoader{thumbLoader}
{
    setObjectName("DkThumbWidget");

    DkShortcutEventFilter::reserveKeys(parent,
                                       {
                                           Qt::Key_Up,
                                           Qt::Key_Down,
                                           Qt::Key_Left,
                                           Qt::Key_Right,
                                           Qt::SHIFT | Qt::Key_Up,
                                           Qt::SHIFT | Qt::Key_Down,
                                           Qt::SHIFT | Qt::Key_Left,
                                           Qt::SHIFT | Qt::Key_Right,
                                       });
}

void DkThumbScene::updateLayout()
{
    if (mThumbLabels.empty())
        return;

    QSize pSize;

    if (!views().empty())
        pSize = QSize(views().first()->viewport()->size());

    int psz = DkSettingsManager::param().effectiveThumbPreviewSize();
    mXOffset = 2; // qCeil(psz*0.1f);
    mNumCols = qMax(qFloor(((float)pSize.width() - mXOffset) / (psz + mXOffset)), 1);
    mNumCols = qMin(mThumbLabels.size(), mNumCols);
    mNumRows = qCeil((float)mThumbLabels.size() / mNumCols);

    int tso = psz + mXOffset;
    setSceneRect(0, 0, mNumCols * tso + mXOffset, mNumRows * tso + mXOffset);

    int cYOffset = mXOffset;

    for (int rIdx = 0; rIdx < mNumRows; rIdx++) {
        int cXOffset = mXOffset;

        for (int cIdx = 0; cIdx < mNumCols; cIdx++) {
            int tIdx = rIdx * mNumCols + cIdx;

            if (tIdx < 0 || tIdx >= mThumbLabels.size())
                break;

            DkThumbLabel *cLabel = mThumbLabels.at(tIdx);
            cLabel->setPos(cXOffset, cYOffset);
            cXOffset += psz + mXOffset;
        }

        // update ypos
        cYOffset += psz + mXOffset; // 20 for label
    }

    DkThumbLabel *lastSelected = nullptr;
    for (DkThumbLabel *thumb : std::as_const(mThumbLabels)) {
        thumb->cancelLoading(); // visibility and/or size may have changed
        if (thumb->isSelected())
            lastSelected = thumb;
    }
    if (lastSelected)
        lastSelected->ensureVisible();
    update();
}

void DkThumbScene::updateThumbs(QVector<QSharedPointer<DkImageContainerT>> thumbs)
{
    int selectedIdx = mLastSelectedIdx;
    mLastSelectedIdx = -1;

    if (selectedIdx < 0) {
        for (int idx = 0; idx < mThumbLabels.size(); idx++) {
            if (mThumbLabels.at(idx)->isSelected()) {
                selectedIdx = idx;
                break;
            }
        }
    }

    mThumbs.clear();
    mThumbs.reserve(thumbs.size());
    for (const auto &img : thumbs) {
        mThumbs.push_back(img->originalFileInfo());
    }
    updateThumbLabels();

    if (selectedIdx >= 0) {
        selectedIdx = qMax(0, qMin(selectedIdx, mThumbLabels.size() - 1));
        selectThumb(selectedIdx);
    }
}

void DkThumbScene::updateThumbLabels()
{
    blockSignals(true); // do not emit selection changed while clearing!
    clear(); // deletes the thumbLabels
    blockSignals(false);

    mThumbLabels.clear();

    for (const auto &fileInfo : std::as_const(mThumbs)) {
        auto *thumb = new DkThumbLabel(mThumbLoader,
                                       fileInfo,
                                       DkSettingsManager::param().display().displaySquaredThumbs);
        connect(thumb, &DkThumbLabel::loadFileSignal, this, &DkThumbScene::loadFileSignal);
        connect(thumb, &DkThumbLabel::showFileSignal, this, &DkThumbScene::showFile);

        addItem(thumb);
        mThumbLabels.append(thumb);
    }

    showFile();

    if (!mThumbs.empty())
        updateLayout();

    emit selectionChanged();
}

void DkThumbScene::setImageLoader(QSharedPointer<DkImageLoader> loader)
{
    connectLoader(mLoader, false); // disconnect
    mLoader = loader;
    connectLoader(loader);
    showFile(""); // if we don't do this, then if the FIRST directory opened after startup is empty, no directory will
                  // be displayed in the status bar
}

void DkThumbScene::connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals)
{
    if (!loader)
        return;

    if (connectSignals) {
        connect(loader.data(),
                &DkImageLoader::updateDirSignal,
                this,
                &DkThumbScene::updateThumbs,
                Qt::UniqueConnection);
    } else {
        disconnect(loader.data(), &DkImageLoader::updateDirSignal, this, &DkThumbScene::updateThumbs);
    }
}

void DkThumbScene::keyPressEvent(QKeyEvent *event)
{
    int idx = selectedThumbIndex((event->key() != Qt::Key_Right && event->key() != Qt::Key_Down));

    if (idx == -1)
        return;

    if (event->modifiers() != Qt::ShiftModifier
        && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right || event->key() == Qt::Key_Up
            || event->key() == Qt::Key_Down))
        selectThumbs(false);

    switch (event->key()) {
    case Qt::Key_Left: {
        selectThumb(qMax(idx - 1, 0));
        break;
    }
    case Qt::Key_Right: {
        selectThumb(qMin(idx + 1, mThumbLabels.size() - 1));
        break;
    }
    case Qt::Key_Up: {
        selectThumb(qMax(idx - mNumCols, 0));
        break;
    }
    case Qt::Key_Down: {
        selectThumb(qMin(idx + mNumCols, mThumbLabels.size() - 1));
        break;
    }
    }
}

void displayFileInfoInStatusbar(const QString &filePath)
{
    QFileInfo fInfo(filePath);
    DkStatusBar *bar = DkStatusBarManager::instance().statusbar();
    if (fInfo.exists()) {
        bar->setMessage(DkUtils::readableByte(fInfo.size()), DkStatusBar::status_filesize_info);
        bar->setMessage(filePath);
    } else {
        bar->setMessage("", DkStatusBar::status_filesize_info);
    }
}

void DkThumbScene::showFile(const QString &filePath)
{
    int sf = getSelectedFiles().size();
    DkStatusBar *bar = DkStatusBarManager::instance().statusbar();
    if (filePath == QDir::currentPath() || filePath.isEmpty()) { // i.e. user is NO LONGER hovering over a file
        if (sf == 0) {
            QString info = QString::number(mThumbLabels.size()) + tr(" images");
            bar->setMessage(tr("%1 | %2").arg(info, currentDir()));
            bar->setMessage("", DkStatusBar::status_filesize_info);
        } else if (sf == 1) {
            displayFileInfoInStatusbar(getSelectedFiles()[0]);
        } else {
            QString info = QString::number(sf) + tr(" selected");
            bar->setMessage(tr("%1 | %2").arg(info, currentDir()));
            bar->setMessage("", DkStatusBar::status_filesize_info);
        }
    } else { // i.e. user STARTED hovering over a file
        displayFileInfoInStatusbar(filePath);
    }
}

void DkThumbScene::ensureVisible(const QString &path) const
{
    for (DkThumbLabel *label : mThumbLabels) {
        if (label->filePath() == path) {
            label->ensureVisible();
            break;
        }
    }
}

QString DkThumbScene::currentDir() const
{
    if (mThumbs.empty()) {
        if (mLoader) {
            return mLoader->getDirPath();
        } else { // should never happen
            return "";
        }
    }

    return mThumbs[0].dirPath();
}

int DkThumbScene::selectedThumbIndex(bool first)
{
    int selIdx = -1;
    for (int idx = 0; idx < mThumbLabels.size(); idx++) {
        if (first && mThumbLabels[idx]->isSelected())
            return idx;
        else if (mThumbLabels[idx]->isSelected())
            selIdx = idx;
    }

    return selIdx;
}

void DkThumbScene::toggleThumbLabels(bool show)
{
    DkSettingsManager::param().display().showThumbLabel = show;

    for (const auto t : mThumbLabels)
        t->update();
}

void DkThumbScene::toggleSquaredThumbs(bool squares)
{
    DkSettingsManager::param().display().displaySquaredThumbs = squares;

    for (const auto t : mThumbLabels) {
        t->setFillSquare(squares);
    }
}

void DkThumbScene::increaseThumbs()
{
    resizeThumbs(1.2f);
}

void DkThumbScene::decreaseThumbs()
{
    resizeThumbs(0.8f);
}

void DkThumbScene::resizeThumbs(float dx)
{
    if (dx < 0)
        dx += 2.0f;

    int newSize = qRound(DkSettingsManager::param().display().thumbPreviewSize * dx);

    if (newSize > 6 && newSize <= max_thumb_size) {
        DkSettingsManager::param().display().thumbPreviewSize = newSize;
        updateLayout();
        // TODO: cancel anything no longer visible
    }
}

void DkThumbScene::cancelLoading()
{
    DkThumbsThreadPool::clear();

    for (auto t : mThumbLabels)
        t->cancelLoading();
}

void DkThumbScene::selectAllThumbs(bool selected)
{
    selectThumbs(selected);
}

void DkThumbScene::selectThumbs(bool selected /* = true */, int from /* = 0 */, int to /* = -1 */)
{
    if (mThumbLabels.empty())
        return;

    if (to == -1)
        to = mThumbLabels.size() - 1;

    if (from > to) {
        int tmp = to;
        to = from;
        from = tmp;
    }

    blockSignals(true);
    for (int idx = from; idx <= to && idx < mThumbLabels.size(); idx++) {
        mThumbLabels.at(idx)->setSelected(selected);
    }
    blockSignals(false);
    emit selectionChanged();
    showFile(); // update selection label
}

void DkThumbScene::selectThumb(int idx, bool select)
{
    if (mThumbLabels.empty())
        return;

    if (idx < 0 || idx >= mThumbLabels.size()) {
        qWarning() << "index out of bounds in selectThumbs()" << idx;
        return;
    }

    blockSignals(true);
    mThumbLabels[idx]->setSelected(select);
    blockSignals(false);

    emit selectionChanged();
    showFile(); // update selection label

    Q_ASSERT(mThumbLabels.size() > idx && mThumbLabels[idx]);
    mThumbLabels[idx]->ensureVisible();
}

void DkThumbScene::copySelected() const
{
    QStringList fileList = getSelectedFiles();
    if (fileList.empty())
        return;

    QList<QUrl> urls;
    for (QString cStr : fileList)
        urls.append(QUrl::fromLocalFile(cStr));

    auto *mimeData = new QMimeData();
    mimeData->setUrls(urls);

    QApplication::clipboard()->setMimeData(mimeData);
}

void DkThumbScene::pasteImages() const
{
    copyImages(QApplication::clipboard()->mimeData());
}

void DkThumbScene::copyImages(const QMimeData *mimeData, const Qt::DropAction &da) const
{
    if (!mimeData || !mimeData->hasUrls() || !mLoader)
        return;

    DkFileInfo dirInfo(mLoader->getDirPath());
    if (!dirInfo.isDir()) {
        QMessageBox::information(DkUtils::getMainWindow(),
                                 tr("Copy Images"),
                                 tr("The current directory is invalid or no longer exists."));
        return;
    }

    if (dirInfo.isZipFile()) {
        QMessageBox::information(DkUtils::getMainWindow(),
                                 tr("Copy Images"),
                                 tr("Copying to an archive is unsupported."));
        return;
    }

    QDir dir = dirInfo.path();

    for (QUrl url : mimeData->urls()) {
        QFileInfo fileInfo = DkUtils::urlToLocalFile(url);
        QFile file(fileInfo.absoluteFilePath());
        QString newFilePath = QFileInfo(dir, fileInfo.fileName()).absoluteFilePath();

        // ignore existing silently
        if (QFileInfo(newFilePath).exists())
            continue;

        auto askUser = [&](const QString &aMsg) {
            int answer = QMessageBox::critical(DkUtils::getMainWindow(),
                                               tr("Error"),
                                               tr("Sorry, I cannot %1 %2").arg(aMsg, newFilePath),
                                               QMessageBox::Ok | QMessageBox::Cancel);

            return answer == QMessageBox::Cancel;
        };

        bool userCanceled = false;

        switch (da) {
        // move files -> a move is actually a rename
        case Qt::MoveAction: {
            if (!file.rename(newFilePath))
                userCanceled = askUser(tr("move"));
            break;
        }
        // create links
        case Qt::LinkAction: {
#ifdef Q_OS_WIN
            newFilePath += ".lnk";
#endif

            if (!file.link(newFilePath)) {
                userCanceled = askUser(tr("create link"));
            }
            break;
        }

        default: {
            // our default action is copying (usually done without modifiers)
            if (!file.copy(newFilePath)) {
                userCanceled = askUser(tr("copy"));
            }
            break;
        }
        }

        if (userCanceled)
            break;
    }
}

void DkThumbScene::deleteSelected()
{
    const int numFiles = getSelectedThumbs().size();

    if (numFiles <= 0)
        return;

    auto *msgBox = new DkMessageBox(QMessageBox::Question,
                                    tr("Delete File"),
                                    tr("Shall I move %1 file(s) to trash?").arg(numFiles),
                                    (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                    DkUtils::getMainWindow());

    msgBox->setDefaultButton(QMessageBox::Yes);
    msgBox->setObjectName("deleteThumbFileDialog");

    int answer = msgBox->exec();

    if (answer == QMessageBox::Yes || answer == QMessageBox::Accepted) {
        blockSignals(true); // setSelected may emit
        mLoader->blockSignals(true); // use-after-free if loader emits in the loop

        mLastSelectedIdx = -1;

        for (int i = 0; i < mThumbLabels.size(); i++) {
            DkThumbLabel *thumb = mThumbLabels.at(i);
            if (!thumb->isSelected())
                continue;

            if (mLastSelectedIdx < 0)
                mLastSelectedIdx = i;

            const QString filePath = thumb->filePath();
            const QString fileName = QFileInfo(filePath).fileName();

            if (!DkUtils::moveToTrash(filePath)) {
                QMessageBox::critical(DkUtils::getMainWindow(),
                                      tr("Error"),
                                      tr("Sorry, I cannot delete:\n%1").arg(fileName),
                                      QMessageBox::Ok | QMessageBox::Cancel);

                if (answer == QMessageBox::Cancel) {
                    break;
                }
            }

            // we might try to delete it twice because directoryChanged() can defer the update
            thumb->setSelected(false);
        }

        mLoader->blockSignals(false);
        blockSignals(false);

        if (mLoader)
            mLoader->directoryChanged(mLoader->getDirPath());
    }
}

void DkThumbScene::renameSelected() const
{
    QStringList fileList = getSelectedFiles();

    if (fileList.empty())
        return;

    bool ok;
    QString newFileName = QInputDialog::getText(DkUtils::getMainWindow(),
                                                tr("Rename File(s)"),
                                                tr("New Filename:"),
                                                QLineEdit::Normal,
                                                "",
                                                &ok);

    if (!ok || newFileName.isEmpty()) {
        return;
    }

    QString pattern = (fileList.size() == 1) ? newFileName + ".<old>"
                                             : newFileName + "<d:3>.<old>"; // no index if just 1 file was added
    DkFileNameConverter converter(pattern);

    for (int idx = 0; idx < fileList.size(); idx++) {
        QFileInfo fileInfo = QFileInfo(fileList.at(idx));
        QFile file(fileInfo.absoluteFilePath());
        QFileInfo newFileInfo(fileInfo.dir(), converter.convert(fileInfo.fileName(), idx));
        if (!file.rename(newFileInfo.absoluteFilePath())) {
            int answer = QMessageBox::critical(DkUtils::getMainWindow(),
                                               tr("Error"),
                                               tr("Sorry, I cannot rename: %1 to %2")
                                                   .arg(fileInfo.fileName(), newFileInfo.fileName()),
                                               QMessageBox::Ok | QMessageBox::Cancel);

            if (answer == QMessageBox::Cancel) {
                break;
            }
        }
    }
}

QStringList DkThumbScene::getSelectedFiles() const
{
    QStringList fileList;

    for (int idx = 0; idx < mThumbLabels.size(); idx++) {
        if (mThumbLabels.at(idx) && mThumbLabels.at(idx)->isSelected()) {
            fileList.append(mThumbLabels.at(idx)->filePath());
        }
    }

    return fileList;
}

QVector<DkThumbLabel *> DkThumbScene::getSelectedThumbs() const
{
    QVector<DkThumbLabel *> selected;

    for (DkThumbLabel *label : mThumbLabels) {
        if (label->isSelected())
            selected << label;
    }

    return selected;
}

int DkThumbScene::findThumb(DkThumbLabel *thumb) const
{
    int thumbIdx = -1;

    for (int idx = 0; idx < mThumbLabels.size(); idx++) {
        if (thumb == mThumbLabels.at(idx)) {
            thumbIdx = idx;
            break;
        }
    }

    return thumbIdx;
}

bool DkThumbScene::allThumbsSelected() const
{
    for (DkThumbLabel *label : mThumbLabels)
        if (label->flags() & QGraphicsItem::ItemIsSelectable && !label->isSelected())
            return false;

    return true;
}

// DkThumbView --------------------------------------------------------------------
DkThumbsView::DkThumbsView(DkThumbScene *scene, QWidget *parent /* = 0 */)
    : QGraphicsView(scene, parent)
    , mThumbScene(scene)
{
    setObjectName("DkThumbsView");

    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setAcceptDrops(true);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &DkThumbsView::onScroll);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DkThumbsView::onScroll);
}

void DkThumbsView::onScroll()
{
    const QRectF portRect = mapToScene(viewport()->rect()).boundingRect();

    for (const auto item : items()) {
        auto it = dynamic_cast<DkThumbLabel *>(item);
        if (it == nullptr) {
            continue;
        }

        const bool intesects = portRect.intersects(it->sceneBoundingRect());
        if (!intesects) {
            it->cancelLoading();
        }
    }
}

void DkThumbsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier) {
        mThumbScene->resizeThumbs(event->angleDelta().y() / 100.0f);
    } else if (event->modifiers() == Qt::NoModifier) {
        if (verticalScrollBar()->isVisible()) {
            verticalScrollBar()->setValue(verticalScrollBar()->value() - event->angleDelta().y());
        }
    }

    // QWidget::wheelEvent(event);
}

void DkThumbsView::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        mMouseDownPos = event->pos();
    }

    qDebug() << "mouse pressed";

    DkThumbLabel *itemClicked = static_cast<DkThumbLabel *>(
        mThumbScene->itemAt(mapToScene(event->pos()), QTransform()));

    // this is a bit of a hack
    // what we want to achieve: if the user is selecting with e.g. shift or ctrl
    // and he clicks (unintentionally) into the background - the selection would be lost
    // otherwise so we just don't propagate this event
    if (itemClicked || event->modifiers() == Qt::NoModifier)
        QGraphicsView::mousePressEvent(event);

    if (!itemClicked) {
        mThumbScene->showFile("");
    }
}

void DkThumbsView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        int dist = qRound(QPointF(event->pos() - mMouseDownPos).manhattanLength());

        if (dist > QApplication::startDragDistance()) {
            QStringList fileList = mThumbScene->getSelectedFiles();

            if (!fileList.empty()) {
                QList<QUrl> urls;
                for (QString fStr : fileList)
                    urls.append(QUrl::fromLocalFile(fStr));

                auto *mimeData = new QMimeData;
                mimeData->setUrls(urls);

                // create thumb image
                QVector<DkThumbLabel *> tl = mThumbScene->getSelectedThumbs();
                QVector<QImage> imgs;

                for (int idx = 0; idx < tl.size() && idx < 3; idx++) {
                    imgs << tl[idx]->image();
                }

                QPixmap pm = DkImage::merge(imgs).scaledToHeight(
                    73); // 73: see https://www.youtube.com/watch?v=TIYMmbHik08

                auto *drag = new QDrag(this);
                drag->setMimeData(mimeData); // noleak: takes ownership
                drag->setPixmap(pm);

                drag->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, Qt::CopyAction);
            }
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void DkThumbsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    DkThumbLabel *itemClicked = static_cast<DkThumbLabel *>(
        mThumbScene->itemAt(mapToScene(event->pos()), QTransform()));

    if (mLastShiftIdx != -1 && event->modifiers() & Qt::ShiftModifier && itemClicked != nullptr) {
        mThumbScene->selectThumbs(true, mLastShiftIdx, mThumbScene->findThumb(itemClicked));
    } else if (itemClicked != nullptr) {
        mLastShiftIdx = mThumbScene->findThumb(itemClicked);
    } else
        mLastShiftIdx = -1;
}

void DkThumbsView::dragEnterEvent(QDragEnterEvent *event)
{
    QGraphicsView::dragEnterEvent(event);

    qDebug() << event->source() << " I am: " << this;

    if (event->source() == this)
        event->accept();
    else if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();

        DkFileInfo file(url.toString());

        // just accept image files
        if (DkUtils::isLoadable(file))
            event->acceptProposedAction();
        else if (file.isDir())
            event->acceptProposedAction();
    }
}

void DkThumbsView::dragMoveEvent(QDragMoveEvent *event)
{
    QGraphicsView::dragMoveEvent(event);

    if (event->source() == this) {
        event->accept();
    } else if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();

        DkFileInfo file(url.toString());

        // just accept image files
        if (DkUtils::isLoadable(file))
            event->acceptProposedAction();
        else if (file.isDir())
            event->acceptProposedAction();
    }
}

void DkThumbsView::dropEvent(QDropEvent *event)
{
    if (event->source() == this) {
        event->accept();
        return;
    }

    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        QUrl url = event->mimeData()->urls().at(0);
        url = url.toLocalFile();

        QFileInfo file = QFileInfo(url.toString());

        // if a folder is dropped -> open it
        if (file.isDir()) {
            emit updateDirSignal(file.absoluteFilePath());
        } else {
            mThumbScene->copyImages(event->mimeData(), event->proposedAction());
        }
    }

    QGraphicsView::dropEvent(event);
}

// DkThumbScrollWidget --------------------------------------------------------------------
DkThumbScrollWidget::DkThumbScrollWidget(DkThumbLoader *thumbLoader,
                                         QWidget *parent /* = 0 */,
                                         Qt::WindowFlags flags /* = 0 */)
    : DkWidget(parent, flags)
{
    // TODO: is this name required elsewhere?
    setObjectName("DkThumbScrollWidget");
    setContentsMargins(0, 0, 0, 0);

    mThumbsScene = new DkThumbScene(thumbLoader, this);
    // thumbsView->setContentsMargins(0,0,0,0);

    mView = new DkThumbsView(mThumbsScene, this);
    mView->setFocusPolicy(Qt::StrongFocus);

    createActions();
    createToolbar();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mToolbar);
    layout->addWidget(mView);
    setLayout(layout);

    enableSelectionActions();
}

DkThumbScrollWidget::~DkThumbScrollWidget()
{
    // ~QGraphicsScene will emit selectionChanged() while being destructed.
    // this gives a segfault or assertion failure (debug builds) and seems
    // to be a Qt bug IMO.
    mThumbsScene->disconnect();
}

void DkThumbScrollWidget::createToolbar()
{
    mToolbar = new QToolBar(tr("Thumb Preview Toolbar"), this);
    mToolbar->setIconSize(
        QSize(DkSettingsManager::param().effectiveIconSize(this), DkSettingsManager::param().effectiveIconSize(this)));

    DkActionManager &am = DkActionManager::instance();
    mToolbar->addAction(am.action(DkActionManager::preview_zoom_in));
    mToolbar->addAction(am.action(DkActionManager::preview_zoom_out));
    mToolbar->addAction(am.action(DkActionManager::preview_display_squares));
    mToolbar->addAction(am.action(DkActionManager::preview_show_labels));
    mToolbar->addSeparator();
    mToolbar->addAction(am.action(DkActionManager::preview_copy));
    mToolbar->addAction(am.action(DkActionManager::preview_paste));
    mToolbar->addAction(am.action(DkActionManager::preview_rename));
    mToolbar->addAction(am.action(DkActionManager::preview_delete));
    mToolbar->addSeparator();
    mToolbar->addAction(am.action(DkActionManager::preview_batch));
    mToolbar->addAction(am.action(DkActionManager::preview_print));

    // add sorting
    QString menuTitle = tr("&Sort");
    QMenu *m = mContextMenu->addMenu(menuTitle);
    m->addActions(am.sortActions().toList());

    auto *toolButton = new QToolButton(this);
    toolButton->setObjectName("DkThumbToolButton");
    toolButton->setMenu(m);
    toolButton->setAccessibleName(menuTitle);
    toolButton->setText(menuTitle);
    toolButton->setIcon(DkImage::loadIcon(":/nomacs/img/sort.svg"));
    toolButton->setPopupMode(QToolButton::InstantPopup);
    mToolbar->addWidget(toolButton);

    // filter edit
    mFilterEdit = new QLineEdit("", this);
    mFilterEdit->setPlaceholderText(tr("Filter Files (Ctrl + F)"));
    mFilterEdit->setMaximumWidth(250);

    // right align search filters
    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mToolbar->addWidget(spacer);
    mToolbar->addWidget(mFilterEdit);
}

void DkThumbScrollWidget::createActions()
{
    // create context menu
    mContextMenu = new QMenu(tr("Thumb"), this);
    QVector<QAction *> actions = DkActionManager::instance().previewActions();
    for (int idx = 0; idx < actions.size(); idx++) {
        mContextMenu->addAction(actions.at(idx));

        if (idx == DkActionManager::preview_show_labels)
            mContextMenu->addSeparator();
    }

    DkActionManager &am = DkActionManager::instance();
    // addActions(am.allActions().toList());
    addActions(am.previewActions().toList());

    // add a shortcut to open the selected image
    auto *loadFile = new QAction(tr("Open Image"), this);
    loadFile->setShortcut(Qt::Key_Return);
    connect(loadFile, &QAction::triggered, this, &DkThumbScrollWidget::onLoadFileTriggered);

    addAction(loadFile);
}

void DkThumbScrollWidget::batchProcessFiles() const
{
    QStringList fileList = mThumbsScene->getSelectedFiles();
    emit batchProcessFilesSignal(fileList);
}

void DkThumbScrollWidget::batchPrint() const
{
    QStringList fileList = mThumbsScene->getSelectedFiles();

    QVector<QImage> imgs;
    DkBasicLoader bl;

    for (const QString &f : fileList) {
        bl.loadGeneral(f);

        if (!bl.image().isNull())
            imgs << bl.image();
    }

    auto *printPreviewDialog = new DkPrintPreviewDialog(DkUtils::getMainWindow());

    for (const QImage &img : imgs)
        printPreviewDialog->addImage(img);

    printPreviewDialog->exec();
    printPreviewDialog->deleteLater();
}

void DkThumbScrollWidget::onLoadFileTriggered()
{
    auto selected = mThumbsScene->selectedItems();

    if (selected.isEmpty())
        return;

    auto thumb = dynamic_cast<DkThumbLabel *>(selected.first());

    if (thumb)
        mThumbsScene->loadFileSignal(thumb->filePath(), false);
}

void DkThumbScrollWidget::updateThumbs(QVector<QSharedPointer<DkImageContainerT>> thumbs)
{
    mThumbsScene->updateThumbs(thumbs);
}

void DkThumbScrollWidget::clear()
{
    mThumbsScene->updateThumbs(QVector<QSharedPointer<DkImageContainerT>>());
}

void DkThumbScrollWidget::setDir(const QString &dirPath)
{
    if (isVisible())
        emit updateDirSignal(dirPath);
}

void DkThumbScrollWidget::setVisible(bool visible)
{
    connectToActions(visible);

    if (visible) {
        mThumbsScene->updateThumbLabels();
        mFilterEdit->setText("");
    } else
        mThumbsScene->cancelLoading();

    if (mAction) {
        mAction->blockSignals(true);
        mAction->setChecked(visible);
        mAction->blockSignals(false);
    }

    DkWidget::setVisible(visible);
}

void DkThumbScrollWidget::connectToActions(bool activate)
{
    DkActionManager &am = DkActionManager::instance();
    for (QAction *a : am.previewActions())
        a->setEnabled(activate);

    if (activate) {
        connect(am.action(DkActionManager::preview_select_all),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::selectAllThumbs);
        connect(am.action(DkActionManager::preview_zoom_in),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::increaseThumbs);
        connect(am.action(DkActionManager::preview_zoom_out),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::decreaseThumbs);
        connect(am.action(DkActionManager::preview_display_squares),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::toggleSquaredThumbs);
        connect(am.action(DkActionManager::preview_show_labels),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::toggleThumbLabels);
        connect(am.action(DkActionManager::preview_filter),
                &QAction::triggered,
                this,
                &DkThumbScrollWidget::setFilterFocus);
        connect(am.action(DkActionManager::preview_delete),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::deleteSelected);
        connect(am.action(DkActionManager::preview_copy),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::copySelected);
        connect(am.action(DkActionManager::preview_paste),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::pasteImages);
        connect(am.action(DkActionManager::preview_rename),
                &QAction::triggered,
                mThumbsScene,
                &DkThumbScene::renameSelected);
        connect(am.action(DkActionManager::preview_batch),
                &QAction::triggered,
                this,
                &DkThumbScrollWidget::batchProcessFiles);
        connect(am.action(DkActionManager::preview_print), &QAction::triggered, this, &DkThumbScrollWidget::batchPrint);

        connect(mFilterEdit, &QLineEdit::textChanged, this, &DkThumbScrollWidget::filterChangedSignal);
        connect(mView, &DkThumbsView::updateDirSignal, this, &DkThumbScrollWidget::updateDirSignal);
        connect(mThumbsScene, &DkThumbScene::selectionChanged, this, &DkThumbScrollWidget::enableSelectionActions);
    } else {
        disconnect(am.action(DkActionManager::preview_select_all),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::selectAllThumbs);
        disconnect(am.action(DkActionManager::preview_zoom_in),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::increaseThumbs);
        disconnect(am.action(DkActionManager::preview_zoom_out),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::decreaseThumbs);
        disconnect(am.action(DkActionManager::preview_display_squares),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::toggleSquaredThumbs);
        disconnect(am.action(DkActionManager::preview_show_labels),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::toggleThumbLabels);
        disconnect(am.action(DkActionManager::preview_filter),
                   &QAction::triggered,
                   this,
                   &DkThumbScrollWidget::setFilterFocus);
        disconnect(am.action(DkActionManager::preview_delete),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::deleteSelected);
        disconnect(am.action(DkActionManager::preview_copy),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::copySelected);
        disconnect(am.action(DkActionManager::preview_paste),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::pasteImages);
        disconnect(am.action(DkActionManager::preview_rename),
                   &QAction::triggered,
                   mThumbsScene,
                   &DkThumbScene::renameSelected);
        disconnect(am.action(DkActionManager::preview_batch),
                   &QAction::triggered,
                   this,
                   &DkThumbScrollWidget::batchProcessFiles);
        disconnect(am.action(DkActionManager::preview_print),
                   &QAction::triggered,
                   this,
                   &DkThumbScrollWidget::batchPrint);

        disconnect(mFilterEdit, &QLineEdit::textChanged, this, &DkThumbScrollWidget::filterChangedSignal);
        disconnect(mView, &DkThumbsView::updateDirSignal, this, &DkThumbScrollWidget::updateDirSignal);
        disconnect(mThumbsScene, &DkThumbScene::selectionChanged, this, &DkThumbScrollWidget::enableSelectionActions);
    }
}

void DkThumbScrollWidget::setFilterFocus() const
{
    mFilterEdit->setFocus(Qt::MouseFocusReason);
}

void DkThumbScrollWidget::resizeEvent(QResizeEvent *event)
{
    if (event->oldSize().width() != event->size().width() && isVisible())
        mThumbsScene->updateLayout();

    DkWidget::resizeEvent(event);
}

void DkThumbScrollWidget::contextMenuEvent(QContextMenuEvent *event)
{
    mContextMenu->exec(event->globalPos());
    event->accept();

    // QGraphicsView::contextMenuEvent(event);
}

void DkThumbScrollWidget::enableSelectionActions()
{
    const QStringList files = mThumbsScene->getSelectedFiles();
    bool hasSelection = !files.isEmpty();
    bool isFromZip = hasSelection && DkFileInfo(files[0]).isFromZip();

    DkActionManager &am = DkActionManager::instance();
    am.action(DkActionManager::preview_copy)->setEnabled(hasSelection && !isFromZip);

    am.action(DkActionManager::preview_rename)->setEnabled(hasSelection && !isFromZip);
    am.action(DkActionManager::preview_delete)->setEnabled(hasSelection && !isFromZip);
    am.action(DkActionManager::preview_batch)->setEnabled(hasSelection);

    am.action(DkActionManager::preview_select_all)->setChecked(mThumbsScene->allThumbsSelected());
}

// DkThumbPreviewLabel --------------------------------------------------------------------
DkThumbPreviewLabel::DkThumbPreviewLabel(const QString &filePath,
                                         DkThumbLoader *thumbLoader,
                                         int thumbSize,
                                         QWidget *parent /* = 0 */,
                                         Qt::WindowFlags f /* = 0 */)
    : QLabel(parent, f)
    , mThumbSize{thumbSize}
    , mLoader{thumbLoader}
    , mFilePath{filePath}

{
    connect(mLoader, &DkThumbLoader::thumbnailLoaded, this, &DkThumbPreviewLabel::thumbLoaded);
    connect(mLoader, &DkThumbLoader::thumbnailLoadFailed, this, [this](const QString &path) {
        if (path != mFilePath) {
            return;
        }
        setProperty("empty", true); // apply empty style
        style()->unpolish(this);
        style()->polish(this);
        update();
    });

    setFixedSize(mThumbSize, mThumbSize);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setStatusTip(filePath);

    QFileInfo fInfo(filePath);
    setToolTip(fInfo.fileName());

    mLoader->requestThumbnail(LoadThumbnailRequest{filePath});
}

void DkThumbPreviewLabel::thumbLoaded(const QString &filePath, const QImage &img)
{
    if (filePath != mFilePath) {
        return;
    }

    QPixmap pm = QPixmap::fromImage(img);
    pm = DkImage::makeSquare(pm);

    if (pm.width() > width())
        pm = pm.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    setPixmap(pm);
}

void DkThumbPreviewLabel::mousePressEvent(QMouseEvent *ev)
{
    emit loadFileSignal(mFilePath, ev->modifiers() == Qt::ControlModifier);

    // do not propagate
    // QLabel::mousePressEvent(ev);
}

// -------------------------------------------------------------------- DkRecentFilesEntry
DkRecentDirWidget::DkRecentDirWidget(const DkRecentDir &rde, DkThumbLoader *thumbLoader, QWidget *parent)
    : DkWidget(parent)
    , mRecentDir(rde)
{
    createLayout(thumbLoader);
}

void DkRecentDirWidget::createLayout(DkThumbLoader *thumbLoader)
{
    auto *dirNameLabel = new QLabel(mRecentDir.dirName(), this);
    dirNameLabel->setAlignment(Qt::AlignBottom);
    dirNameLabel->setObjectName("recentFilesTitle");

    // create buttons
    mButtons.resize(button_end);

    mButtons[button_load_dir] = new QPushButton(DkImage::loadIcon(":/nomacs/img/dir.svg"), "", this);
    mButtons[button_load_dir]->setToolTip(tr("Load the directory"));
    mButtons[button_load_dir]->setObjectName("load_dir");
    mButtons[button_load_dir]->setFlat(true);
    mButtons[button_load_dir]->hide();
    connect(mButtons[button_load_dir], &QPushButton::clicked, this, &DkRecentDirWidget::onLoadDirClicked);

    QIcon pIcon = DkImage::loadIcon(":/nomacs/img/pin.svg");
    pIcon.addFile(":/nomacs/img/pin-checked.svg", QSize(), QIcon::Normal, QIcon::On);

    mButtons[button_pin] = new QPushButton(pIcon, "", this);
    mButtons[button_pin]->setToolTip(tr("Pin this directory"));
    mButtons[button_pin]->setObjectName("pin");
    mButtons[button_pin]->setCheckable(true);
    mButtons[button_pin]->setChecked(mRecentDir.isPinned());
    mButtons[button_pin]->setFlat(true);
    mButtons[button_pin]->hide();
    connect(mButtons[button_pin], &QPushButton::clicked, this, &DkRecentDirWidget::onPinClicked);

    mButtons[button_remove] = new QPushButton(DkImage::loadIcon(":/nomacs/img/close.svg"), "", this);
    mButtons[button_remove]->setToolTip(tr("Remove this directory"));
    mButtons[button_remove]->setObjectName("remove");
    mButtons[button_remove]->setFlat(true);
    mButtons[button_remove]->hide();
    connect(mButtons[button_remove], &QPushButton::clicked, this, &DkRecentDirWidget::onRemoveClicked);

    QVector<DkThumbPreviewLabel *> tls;

    DkFileInfo firstFile = mRecentDir.firstFile();
    if (DkUtils::tryExists(firstFile, 30)) {
        const DkFileInfoList files = mRecentDir.files(4);
        for (auto &tp : files) {
            auto tpl = new DkThumbPreviewLabel(tp.path(), thumbLoader, 42, this);
            connect(tpl, &DkThumbPreviewLabel::loadFileSignal, this, &DkRecentDirWidget::loadFileSignal);
            tls << tpl;
        }
    } else {
        qInfo() << firstFile.path() << "does not exist - according to a fast check";
    }

    auto *pathLabel = new QLabel(mRecentDir.dirPath(), this);
    pathLabel->setAlignment(Qt::AlignLeft);
    pathLabel->setObjectName("recentFilesPath");

    auto *layout = new QGridLayout(this);
    layout->setAlignment(Qt::AlignLeft);
    layout->addWidget(dirNameLabel, 1, 0, 1, tls.size() + 1);
    layout->setColumnStretch(tls.size() + 2, 1);
    layout->addWidget(mButtons[button_load_dir], 1, tls.size() + 3);
    layout->addWidget(mButtons[button_pin], 1, tls.size() + 4);
    layout->addWidget(mButtons[button_remove], 1, tls.size() + 5);
    layout->addWidget(pathLabel, 2, tls.size(), 1, 6);

    for (int idx = 0; idx < tls.size(); idx++)
        layout->addWidget(tls[idx], 2, idx, Qt::AlignTop);

    show();
    setCursor(Qt::PointingHandCursor);

    setToolTip(mRecentDir.dirPath());
    setStatusTip(mRecentDir.dirPath());
}

void DkRecentDirWidget::onPinClicked(bool checked)
{
    if (checked) {
        DkSettingsManager::param().global().pinnedFiles << mRecentDir.filePaths();
    } else {
        for (const QString &fp : mRecentDir.filePaths())
            DkSettingsManager::param().global().pinnedFiles.removeAll(fp);
    }
}

void DkRecentDirWidget::onRemoveClicked()
{
    mRecentDir.removeFromHistory();
    emit removeSignal();
}

void DkRecentDirWidget::onLoadDirClicked()
{
    emit loadDirSignal(mRecentDir.dirPath());
}

void DkRecentDirWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !mRecentDir.isEmpty()) {
        emit loadFileSignal(mRecentDir.firstFilePath(), event->modifiers() == Qt::ControlModifier);
    }

    DkWidget::mousePressEvent(event);
}

void DkRecentDirWidget::enterEvent(DkEnterEvent *event)
{
    for (auto b : mButtons)
        b->show();

    DkWidget::enterEvent(event);
}

void DkRecentDirWidget::leaveEvent(QEvent *event)
{
    for (auto b : mButtons)
        b->hide();

    DkWidget::leaveEvent(event);
}

// -------------------------------------------------------------------- DkRecentFilesEntry
DkRecentFilesWidget::DkRecentFilesWidget(DkThumbLoader *thumbLoader, QWidget *parent)
    : DkWidget(parent)
    , mThumbLoader{thumbLoader}
{
    createLayout();
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void DkRecentFilesWidget::setVisible(bool visible)
{
    if (visible)
        updateList();
    if (mAction) {
        mAction->blockSignals(true);
        mAction->setChecked(visible);
        mAction->blockSignals(false);
    }

    DkWidget::setVisible(visible);
}

void DkRecentFilesWidget::createLayout()
{
    mScrollArea = new QScrollArea(this);
    auto *sl = new QVBoxLayout(this);
    sl->addWidget(mScrollArea);
    sl->setContentsMargins(0, 0, 0, 0);

    mScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // updateList();
}

void DkRecentFilesWidget::updateList()
{
    DkTimer dt;

    DkRecentDirManager fm;

    auto *dummy = new QWidget(this);
    auto *l = new QVBoxLayout(dummy);

    for (auto rd : fm.recentDirs()) {
        auto *rf = new DkRecentDirWidget(rd, mThumbLoader, dummy);
        rf->setMaximumWidth(500);
        connect(rf, &DkRecentDirWidget::loadFileSignal, this, &DkRecentFilesWidget::loadFileSignal);
        connect(rf, &DkRecentDirWidget::loadDirSignal, this, &DkRecentFilesWidget::loadDirSignal);
        connect(rf, &DkRecentDirWidget::removeSignal, this, &DkRecentFilesWidget::entryRemoved);

        l->addWidget(rf);
    }

    qInfo() << "list updated in" << dt; // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks) false positive on layout

    mScrollArea->setWidget(dummy);
}

void DkRecentFilesWidget::entryRemoved()
{
    updateList();
}

// -------------------------------------------------------------------- DkRecentDir
DkRecentDir::DkRecentDir(const DkFileInfo &dir, const DkFileInfoList &files, bool pinned)
{
    mDir = dir;
    mFiles = files;
    mIsPinned = pinned;
}

void DkRecentDir::update(const DkRecentDir &dir)
{
    Q_ASSERT(dir.mDir.path() == mDir.path());

    for (auto &info : dir.mFiles) {
        mFiles.removeAll(info);
        mFiles.append(info);
    }
}

void DkRecentDir::append(const DkFileInfo &file)
{
    Q_ASSERT(file.dirPath() == mDir.path());
    Q_ASSERT(!mFiles.contains(file));
    mFiles.append(file);
}

DkFileInfoList DkRecentDir::files(int max) const
{
    DkFileInfoList list = mFiles;
    if (max > 0)
        list = mFiles.mid(0, max);
    return list;
}

QStringList DkRecentDir::filePaths(int max) const
{
    DkFileInfoList list = mFiles;
    if (max > 0)
        list = mFiles.mid(0, max);

    QStringList paths;
    for (auto &info : std::as_const(list))
        paths.append(info.path());

    return paths;
}

QString DkRecentDir::dirName() const
{
    return mDir.fileName();
}

QString DkRecentDir::dirPath() const
{
    if (mFiles.empty())
        return {};

    return mDir.path();
}

void DkRecentDir::removeFromHistory() const
{
    QStringList &pinnedFiles = DkSettingsManager::param().global().pinnedFiles;
    QStringList &recentFiles = DkSettingsManager::param().global().recentFiles;

    for (const auto &info : mFiles) {
        QString filePath = info.path();
        pinnedFiles.removeAll(filePath);
        recentFiles.removeAll(filePath);
    }
}

// -------------------------------------------------------------------- DkRecentDirManager
DkRecentDirManager::DkRecentDirManager()
{
    // pinned dirs appear first, followed by dirs of recent files
    mDirs = genFileLists(DkSettingsManager::param().global().pinnedFiles, true);
    QList<DkRecentDir> recentDirs = genFileLists(DkSettingsManager::param().global().recentFiles);

    // merge pinned dirs with recent dirs
    for (const DkRecentDir &recentDir : recentDirs) {
        int idx = mDirs.indexOf(recentDir);
        if (idx < 0)
            mDirs.append(recentDir);
        else
            mDirs[idx].update(recentDir);
    }
}

QList<DkRecentDir> DkRecentDirManager::genFileLists(const QStringList &filePaths, bool pinned)
{
    QList<DkRecentDir> recentDirs; // keeps the order of dirs the same as filePaths (fixes #279)
    QHash<QString, int> dirIndex; // use a map to avoid unecessary filesystem syscalls
    for (const QString &recentFilePath : filePaths) {
        // We do not require the file/dir to exist here
        // - The check can be incredibly slow for disconnected networked resources
        // - We do not want to hide history and have it potentially come back later
        DkFileInfo fileInfo(recentFilePath);
        QString dirPath = fileInfo.dirPath();

        auto it = dirIndex.find(dirPath);
        if (it == dirIndex.end()) {
            recentDirs.append(DkRecentDir(DkFileInfo{dirPath}, DkFileInfoList{fileInfo}, pinned));
            dirIndex[dirPath] = recentDirs.count() - 1;
        } else {
            recentDirs[it.value()].append(fileInfo);
        }
    }

    return recentDirs;
}
}
