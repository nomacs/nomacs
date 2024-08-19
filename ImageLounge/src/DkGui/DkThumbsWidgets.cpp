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
#include "DkImageContainer.h"
#include "DkImageLoader.h"
#include "DkImageStorage.h"
#include "DkMessageBox.h"
#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkStatusBar.h"
#include "DkThumbs.h"
#include "DkTimer.h"
#include "DkUtils.h"

#include "DkBasicLoader.h"
#include "DkDialog.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QApplication>
#include <QClipboard>
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
#include <QStyleOptionGraphicsItem>
#include <QThreadPool>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QtGlobal>
#include <qmath.h>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

// DkFilePreview --------------------------------------------------------------------
DkFilePreview::DkFilePreview(QWidget *parent, Qt::WindowFlags flags)
    : DkFadeWidget(parent, flags)
{
    orientation = Qt::Horizontal;
    windowPosition = pos_north;

    init();
    // setStyleSheet("QToolTip{border: 0px; border-radius: 21px; color: white; background-color: red;}"); //" + DkUtils::colorToString(mBgCol) + ";}");

    loadSettings();
    initOrientations();

    createContextMenu();
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
    leftGradient =
        (orientation == Qt::Horizontal) ? QLinearGradient(QPoint(0, 0), QPoint(borderTriggerI, 0)) : QLinearGradient(QPoint(0, 0), QPoint(0, borderTriggerI));
    rightGradient = (orientation == Qt::Horizontal) ? QLinearGradient(QPoint(width() - borderTriggerI, 0), QPoint(width(), 0))
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
    if (mThumbs.isEmpty())
        return;

    if (minHeight != DkSettingsManager::param().effectiveThumbSize(this) + yOffset && windowPosition != pos_dock_hor && windowPosition != pos_dock_ver) {
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

    if (mThumbs.empty()) {
        thumbRects.clear();
        return;
    }

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
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

    bufferDim = (orientation == Qt::Horizontal) ? QRectF(QPointF(0, yOffset / 2), QSize(xOffset, 0)) : QRectF(QPointF(yOffset / 2, 0), QSize(0, xOffset));
    thumbRects.clear();

    DkTimer dt;

    // mouse over effect
    QPoint p = worldMatrix.inverted().map(mapFromGlobal(QCursor::pos()));

    for (int idx = 0; idx < mThumbs.size(); idx++) {
        QSharedPointer<DkThumbNailT> thumb = mThumbs.at(idx)->getThumb();
        QImage img;

        // if the image is loaded draw that (it might be edited)
        if (mThumbs.at(idx)->hasImage()) {
            img = mThumbs.at(idx)->imageScaledToHeight(DkSettingsManager::param().effectiveThumbSize(this));
        } else {
            if (thumb->hasImage() == DkThumbNail::exists_not) {
                thumbRects.push_back(QRectF());
                continue;
            }

            if (thumb->hasImage() == DkThumbNail::loaded)
                img = thumb->getImage();
        }

        // if (img.width() > max_thumb_size * DkSettingsManager::param().dpiScaleFactor())
        //	qDebug() << thumb->getFilePath() << "size:" << img.size();

        QPointF anchor = orientation == Qt::Horizontal ? bufferDim.topRight() : bufferDim.bottomLeft();
        QRectF r = !img.isNull()
            ? QRectF(anchor, img.size())
            : QRectF(anchor, QSize(DkSettingsManager::param().effectiveThumbSize(this), DkSettingsManager::param().effectiveThumbSize(this)));
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
        if ((orientation == Qt::Horizontal && imgWorldRect.right() < 0) || (orientation == Qt::Vertical && imgWorldRect.bottom() < 0)) {
            continue;
        }

        if (((orientation == Qt::Horizontal && imgWorldRect.left() > width()) || (orientation == Qt::Vertical && imgWorldRect.top() > height()))
            && scrollToCurrentImage) {
            continue;
        } else if ((orientation == Qt::Horizontal && imgWorldRect.left() > width()) || (orientation == Qt::Vertical && imgWorldRect.top() > height())) {
            break;
        }

        // only fetch thumbs if we are not moving too fast...
        if (thumb->hasImage() == DkThumbNail::not_loaded && fabs(currentDx) < 40) {
            thumb->fetchThumb();
            connect(thumb.data(), &DkThumbNailT::thumbLoadedSignal, this, QOverload<>::of(&DkFilePreview::update));
        }

        bool isLeftGradient = (orientation == Qt::Horizontal && worldMatrix.dx() < 0 && imgWorldRect.left() < leftGradient.finalStop().x())
            || (orientation == Qt::Vertical && worldMatrix.dy() < 0 && imgWorldRect.top() < leftGradient.finalStop().y());
        bool isRightGradient = (orientation == Qt::Horizontal && imgWorldRect.right() > rightGradient.start().x())
            || (orientation == Qt::Vertical && imgWorldRect.bottom() > rightGradient.start().y());
        // show that there are more images...
        if (isLeftGradient && !img.isNull())
            drawFadeOut(leftGradient, imgWorldRect, &img);
        if (isRightGradient && !img.isNull())
            drawFadeOut(rightGradient, imgWorldRect, &img);

        if (!img.isNull())
            painter->drawImage(r, img, QRect(QPoint(), img.size()));
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
    float oldOp = (float)painter->opacity();

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
    float oldOp = (float)painter->opacity();

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

void DkFilePreview::drawFadeOut(QLinearGradient gradient, QRectF imgRect, QImage *img)
{
    if (img && img->format() == QImage::Format_Indexed8)
        return;

    // compute current scaling
    QPointF scale(img->width() / imgRect.width(), img->height() / imgRect.height());
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

    QImage mask = *img;
    QPainter painter(&mask);
    painter.fillRect(img->rect(), Qt::black);
    painter.fillRect(img->rect(), imgGradient);
    painter.end();

    img->setAlphaChannel(mask);
}

void DkFilePreview::resizeEvent(QResizeEvent *event)
{
    QWidget *pw = qobject_cast<QWidget *>(parent());

    if (event->size() == event->oldSize()
        && ((orientation == Qt::Horizontal && pw && this->width() == pw->width()) || (orientation == Qt::Vertical && pw && this->height() == pw->height()))) {
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
    rightGradient.setStart((orientation == Qt::Horizontal) ? QPoint(width() - borderTriggerI, 0) : QPoint(0, height() - borderTriggerI));
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

                if (selected <= mThumbs.size() && selected >= 0) {
                    QSharedPointer<DkThumbNailT> thumb = mThumbs.at(selected)->getThumb();
                    // selectedImg = DkImage::colorizePixmap(QPixmap::fromImage(thumb->getImage()), DkSettingsManager::param().display().highlightColor, 0.3f);

                    // important: setText shows the label - if you then hide it here again you'll get a stack overflow
                    // if (fileLabel->height() < height())
                    //	fileLabel->setText(thumbs.at(selected).getFile().fileName(), -1);
                    QFileInfo fileInfo(thumb->getFilePath());
                    QString toolTipInfo = tr("Name: ") + fileInfo.fileName() + "\n" + tr("Size: ") + DkUtils::readableByte((float)fileInfo.size()) + "\n"
                        + tr("Created: ") + fileInfo.birthTime().toString();
                    setToolTip(toolTipInfo);
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
            if (idx < mThumbs.size() && worldMatrix.mapRect(thumbRects.at(idx)).contains(event->pos())) {
                if (mThumbs.at(idx)->isFromZip())
                    emit changeFileSignal(idx - currentFileIdx);
                else
                    emit loadFileSignal(mThumbs.at(idx)->filePath() /*, event->modifiers() == Qt::ControlModifier*/);
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
    QAction *sender = static_cast<QAction *>(QObject::sender());

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

        if (mThumbs.size() < 2000) {
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
    if ((translation >= limit * 0.5 && currentDx > 0) || (translation <= -(bufferPos - limit * 0.5 + xOffset) && currentDx < 0))
        return;

    // set the last step to match the center of the screen...	(nicer if user scrolls very fast)
    if (translation < limit * 0.5 && currentDx > 0 && translation + currentDx > limit * 0.5 && currentDx > 0)
        currentDx = limit * 0.5f - translation;
    else if (translation > -(bufferPos - limit * 0.5 + xOffset) && translation + currentDx <= -(bufferPos - limit * 0.5 + xOffset) && currentDx < 0)
        currentDx = -(bufferPos - limit * 0.5f + xOffset + (float)worldMatrix.dx());

    // qDebug() << "currentDx: " << currentDx;
    if (orientation == Qt::Horizontal)
        worldMatrix.translate(currentDx, 0);
    else
        worldMatrix.translate(0, currentDx);
    // qDebug() << "dx: " << worldMatrix.dx();
    update();
}

void DkFilePreview::updateFileIdx(int idx)
{
    if (idx == currentFileIdx)
        return;

    currentFileIdx = idx;
    if (currentFileIdx >= 0)
        scrollToCurrentImage = true;
    update();
}

void DkFilePreview::setFileInfo(QSharedPointer<DkImageContainerT> cImage)
{
    if (!cImage)
        return;

    int tIdx = -1;

    for (int idx = 0; idx < mThumbs.size(); idx++) {
        if (mThumbs.at(idx)->filePath() == cImage->filePath()) {
            tIdx = idx;
            break;
        }
    }

    currentFileIdx = tIdx;
    if (currentFileIdx >= 0)
        scrollToCurrentImage = true;
    update();
}

void DkFilePreview::updateThumbs(QVector<QSharedPointer<DkImageContainerT>> thumbs)
{
    mThumbs = thumbs;

    for (int idx = 0; idx < thumbs.size(); idx++) {
        if (thumbs.at(idx)->isSelected()) {
            currentFileIdx = idx;
            break;
        }
    }

    update();
}

void DkFilePreview::setVisible(bool visible, bool saveSettings)
{
    emit showThumbsDockSignal(visible);

    DkFadeWidget::setVisible(visible, saveSettings);
}

// DkThumbLabel --------------------------------------------------------------------
DkThumbLabel::DkThumbLabel(QSharedPointer<DkThumbNailT> thumb, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mText(this)
{
    mThumbInitialized = false;
    mFetchingThumb = false;
    mIsHovered = false;

    setThumb(thumb);
    setFlag(ItemIsSelectable, true);

    setAcceptHoverEvents(true);
}

DkThumbLabel::~DkThumbLabel()
{
}

void DkThumbLabel::setThumb(QSharedPointer<DkThumbNailT> thumb)
{
    this->mThumb = thumb;

    if (thumb.isNull())
        return;

    connect(thumb.data(), &DkThumbNailT::thumbLoadedSignal, this, &DkThumbLabel::updateLabel);
    QFileInfo fileInfo(thumb->getFilePath());
    QString toolTipInfo = tr("Name: ") + fileInfo.fileName() + "\n" + tr("Size: ") + DkUtils::readableByte((float)fileInfo.size()) + "\n" + tr("Created: ")
        + fileInfo.birthTime().toString();

    setToolTip(toolTipInfo);

    // style dummy
    mNoImagePen.setColor(QColor(150, 150, 150));
    mNoImageBrush = QColor(100, 100, 100, 50);

    QColor col = DkSettingsManager::param().display().highlightColor;
    col.setAlpha(90);
    mSelectBrush = col;
    mSelectPen.setColor(DkSettingsManager::param().display().highlightColor);
}

QPixmap DkThumbLabel::pixmap() const
{
    return mIcon.pixmap();
}

void DkThumbLabel::cancelLoading()
{
    mFetchingThumb = false;
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

void DkThumbLabel::updateLabel()
{
    if (mThumb.isNull())
        return;

    QPixmap pm;

    if (!mThumb->getImage().isNull()) {
        pm = QPixmap::fromImage(mThumb->getImage());

        if (DkSettingsManager::param().display().displaySquaredThumbs) {
            pm = DkImage::makeSquare(pm);
        }
    } else
        qDebug() << "update called on empty thumb label!";

    if (!pm.isNull()) {
        mIcon.setTransformationMode(Qt::SmoothTransformation);
        mIcon.setPixmap(pm);
        mIcon.setFlag(ItemIsSelectable, true);
    }
    if (pm.isNull())
        setFlag(ItemIsSelectable, false); // if we cannot load it -> disable selection

    // update label
    mText.setPos(0, pm.height());
    mText.setDefaultTextColor(QColor(255, 255, 255));

    QFont font;
    font.setBold(false);
    font.setPointSize(8);
    mText.setFont(font);
    mText.setPlainText(QFileInfo(mThumb->getFilePath()).fileName());
    mText.hide();

    prepareGeometryChange();
    updateSize();
}

void DkThumbLabel::updateSize()
{
    if (mIcon.pixmap().isNull())
        return;

    prepareGeometryChange();

    // resize pixmap label
    int maxSize = qMax(mIcon.pixmap().width(), mIcon.pixmap().height());
    int ps = DkSettingsManager::param().effectiveThumbPreviewSize();

    if ((float)ps / maxSize != mIcon.scale()) {
        mIcon.setScale(1.0f);
        mIcon.setPos(0, 0);

        mIcon.setScale((float)ps / maxSize);
        mIcon.moveBy((ps - mIcon.pixmap().width() * mIcon.scale()) * 0.5f, (ps - mIcon.pixmap().height() * mIcon.scale()) * 0.5);
    }

    // update();
}

void DkThumbLabel::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (mThumb.isNull())
        return;

    emit loadFileSignal(mThumb->getFilePath(), event->modifiers() == Qt::ControlModifier);
}

void DkThumbLabel::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    mIsHovered = true;
    emit showFileSignal(mThumb->getFilePath());
    update();
}

void DkThumbLabel::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    mIsHovered = false;
    emit showFileSignal();
    update();
}

void DkThumbLabel::setVisible(bool visible)
{
    mIcon.setVisible(visible);
    mText.setVisible(visible);
}

void DkThumbLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (!mFetchingThumb && mThumb->hasImage() == DkThumbNail::not_loaded) {
        mThumb->fetchThumb();
        mFetchingThumb = true;
    } else if (!mThumbInitialized && (mThumb->hasImage() == DkThumbNail::loaded || mThumb->hasImage() == DkThumbNail::exists_not)) {
        updateLabel();
        mThumbInitialized = true;
        return; // exit - otherwise we get paint errors
    }

    if (mIcon.pixmap().isNull() && mThumb->hasImage() == DkThumbNail::exists_not) {
        painter->setPen(mNoImagePen);
        painter->setBrush(mNoImageBrush);
        painter->drawRect(boundingRect());
    } else if (mIcon.pixmap().isNull()) {
        QColor c = DkSettingsManager::param().display().highlightColor;
        c.setAlpha(30);
        painter->setPen(mNoImagePen);
        painter->setBrush(c);

        QRectF r = boundingRect();
        painter->drawRect(r);
    }

    // this is the Qt idea of how to fix the dashed border:
    // http://www.qtcentre.org/threads/23087-How-to-hide-the-dashed-frame-outside-the-QGraphicsItem
    // I don't think it's beautiful...
    QStyleOptionGraphicsItem noSelOption;
    if (option) {
        noSelOption = *option;
        noSelOption.state &= ~QStyle::State_Selected;
    }

    // painter->setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

    QTransform mt = painter->worldTransform();
    QTransform it = mt;
    it.translate(mIcon.pos().x(), mIcon.pos().y());
    it.scale(mIcon.scale(), mIcon.scale());

    painter->setTransform(it);
    mIcon.paint(painter, &noSelOption, widget);
    painter->setTransform(mt);

    // draw text
    if (boundingRect().width() > 50 && DkSettingsManager::param().display().showThumbLabel) {
        QTransform tt = mt;
        tt.translate(0, boundingRect().height() - mText.boundingRect().height());

        QRectF r = mText.boundingRect();
        r.setWidth(boundingRect().width());
        painter->setPen(Qt::NoPen);
        painter->setWorldTransform(tt);
        painter->setBrush(DkSettingsManager::param().display().hudBgColor);
        painter->drawRect(r);
        mText.paint(painter, &noSelOption, widget);
        painter->setWorldTransform(mt);
    }

    // render hovered
    if (mIsHovered) {
        painter->setBrush(QColor(255, 255, 255, 60));
        painter->setPen(mNoImagePen);
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

// DkThumbWidget --------------------------------------------------------------------
DkThumbScene::DkThumbScene(QWidget *parent /* = 0 */)
    : QGraphicsScene(parent)
{
    setObjectName("DkThumbWidget");
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

    DkTimer dt;
    int cYOffset = mXOffset;

    for (int rIdx = 0; rIdx < mNumRows; rIdx++) {
        int cXOffset = mXOffset;

        for (int cIdx = 0; cIdx < mNumCols; cIdx++) {
            int tIdx = rIdx * mNumCols + cIdx;

            if (tIdx < 0 || tIdx >= mThumbLabels.size())
                break;

            DkThumbLabel *cLabel = mThumbLabels.at(tIdx);
            cLabel->setPos(cXOffset, cYOffset);
            cLabel->updateSize();

            cXOffset += psz + mXOffset;
        }

        // update ypos
        cYOffset += psz + mXOffset; // 20 for label
    }

    for (int idx = 0; idx < mThumbLabels.size(); idx++) {
        if (mThumbLabels.at(idx)->isSelected())
            mThumbLabels.at(idx)->ensureVisible();
    }

    mFirstLayout = false;
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

    this->mThumbs = thumbs;
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

    for (int idx = 0; idx < mThumbs.size(); idx++) {
        DkThumbLabel *thumb = new DkThumbLabel(mThumbs.at(idx)->getThumb());
        connect(thumb, &DkThumbLabel::loadFileSignal, this, &DkThumbScene::loadFileSignal);
        connect(thumb, &DkThumbLabel::showFileSignal, this, &DkThumbScene::showFile);
        connect(mThumbs.at(idx).data(), &DkImageContainerT::thumbLoadedSignal, this, &DkThumbScene::thumbLoadedSignal);

        // thumb->show();
        addItem(thumb);
        mThumbLabels.append(thumb);
        // thumbsNotLoaded.append(thumb);
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
}

void DkThumbScene::connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals)
{
    if (!loader)
        return;

    if (connectSignals) {
        connect(loader.data(), &DkImageLoader::updateDirSignal, this, &DkThumbScene::updateThumbs, Qt::UniqueConnection);
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
        && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down))
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

void DkThumbScene::showFile(const QString &filePath)
{
    if (filePath == QDir::currentPath() || filePath.isEmpty()) {
        int sf = getSelectedFiles().size();

        QString info;

        if (sf > 1)
            info = QString::number(sf) + tr(" selected");
        else
            info = QString::number(mThumbLabels.size()) + tr(" images");

        DkStatusBarManager::instance().setMessage(tr("%1 | %2").arg(info, currentDir()));
    } else
        DkStatusBarManager::instance().setMessage(QFileInfo(filePath).fileName());
}

void DkThumbScene::ensureVisible(QSharedPointer<DkImageContainerT> img) const
{
    if (!img)
        return;

    for (DkThumbLabel *label : mThumbLabels) {
        if (label->getThumb()->getFilePath() == img->filePath()) {
            label->ensureVisible();
            break;
        }
    }
}

QString DkThumbScene::currentDir() const
{
    if (mThumbs.empty() || !mThumbs[0])
        return "";

    return mThumbs[0]->fileInfo().absolutePath();
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

    for (const auto t : mThumbLabels)
        t->updateLabel();

    // well, that's not too beautiful
    if (DkSettingsManager::param().display().displaySquaredThumbs)
        updateLayout();
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
    ensureVisible(mThumbs[idx]);
}

void DkThumbScene::copySelected() const
{
    QStringList fileList = getSelectedFiles();

    if (fileList.empty())
        return;

    QMimeData *mimeData = new QMimeData();

    if (!fileList.empty()) {
        QList<QUrl> urls;
        for (QString cStr : fileList)
            urls.append(QUrl::fromLocalFile(cStr));
        mimeData->setUrls(urls);
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setMimeData(mimeData);
    }
}

void DkThumbScene::pasteImages() const
{
    copyImages(QApplication::clipboard()->mimeData());
}

void DkThumbScene::copyImages(const QMimeData *mimeData, const Qt::DropAction &da) const
{
    if (!mimeData || !mimeData->hasUrls() || !mLoader)
        return;

    QDir dir = mLoader->getDirPath();

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

    DkMessageBox *msgBox = new DkMessageBox(QMessageBox::Question,
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

            const QString filePath = thumb->getThumb()->getFilePath();
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
    QString newFileName = QInputDialog::getText(DkUtils::getMainWindow(), tr("Rename File(s)"), tr("New Filename:"), QLineEdit::Normal, "", &ok);

    if (!ok || newFileName.isEmpty()) {
        return;
    }

    QString pattern = (fileList.size() == 1) ? newFileName + ".<old>" : newFileName + "<d:3>.<old>"; // no index if just 1 file was added
    DkFileNameConverter converter(pattern);

    for (int idx = 0; idx < fileList.size(); idx++) {
        QFileInfo fileInfo = QFileInfo(fileList.at(idx));
        QFile file(fileInfo.absoluteFilePath());
        QFileInfo newFileInfo(fileInfo.dir(), converter.convert(fileInfo.fileName(), idx));
        if (!file.rename(newFileInfo.absoluteFilePath())) {
            int answer = QMessageBox::critical(DkUtils::getMainWindow(),
                                               tr("Error"),
                                               tr("Sorry, I cannot rename: %1 to %2").arg(fileInfo.fileName(), newFileInfo.fileName()),
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
            fileList.append(mThumbLabels.at(idx)->getThumb()->getFilePath());
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
{
    setObjectName("DkThumbsView");
    this->scene = scene;
    connect(scene, &DkThumbScene::thumbLoadedSignal, this, &DkThumbsView::fetchThumbs);

    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setAcceptDrops(true);

    lastShiftIdx = -1;
}

void DkThumbsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier) {
        scene->resizeThumbs(event->angleDelta().y() / 100.0f);
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
        mousePos = event->pos();
    }

    qDebug() << "mouse pressed";

    DkThumbLabel *itemClicked = static_cast<DkThumbLabel *>(scene->itemAt(mapToScene(event->pos()), QTransform()));

    // this is a bit of a hack
    // what we want to achieve: if the user is selecting with e.g. shift or ctrl
    // and he clicks (unintentionally) into the background - the selection would be lost
    // otherwise so we just don't propagate this event
    if (itemClicked || event->modifiers() == Qt::NoModifier)
        QGraphicsView::mousePressEvent(event);
}

void DkThumbsView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        int dist = qRound(QPointF(event->pos() - mousePos).manhattanLength());

        if (dist > QApplication::startDragDistance()) {
            QStringList fileList = scene->getSelectedFiles();

            QMimeData *mimeData = new QMimeData;

            if (!fileList.empty()) {
                QList<QUrl> urls;
                for (QString fStr : fileList)
                    urls.append(QUrl::fromLocalFile(fStr));

                mimeData->setUrls(urls);

                // create thumb image
                QVector<DkThumbLabel *> tl = scene->getSelectedThumbs();
                QVector<QImage> imgs;

                for (int idx = 0; idx < tl.size() && idx < 3; idx++) {
                    imgs << tl[idx]->getThumb()->getImage();
                }

                QPixmap pm = DkImage::merge(imgs).scaledToHeight(73); // 73: see https://www.youtube.com/watch?v=TIYMmbHik08

                QDrag *drag = new QDrag(this);
                drag->setMimeData(mimeData);
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

    DkThumbLabel *itemClicked = static_cast<DkThumbLabel *>(scene->itemAt(mapToScene(event->pos()), QTransform()));

    if (lastShiftIdx != -1 && event->modifiers() & Qt::ShiftModifier && itemClicked != 0) {
        scene->selectThumbs(true, lastShiftIdx, scene->findThumb(itemClicked));
    } else if (itemClicked != 0) {
        lastShiftIdx = scene->findThumb(itemClicked);
    } else
        lastShiftIdx = -1;
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

        QFileInfo file = QFileInfo(url.toString());

        // just accept image files
        if (DkUtils::isValid(file))
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

        QFileInfo file = QFileInfo(url.toString());

        // just accept image files
        if (DkUtils::isValid(file))
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
            scene->copyImages(event->mimeData(), event->proposedAction());
        }
    }

    QGraphicsView::dropEvent(event);
}

void DkThumbsView::fetchThumbs()
{
    QList<QGraphicsItem *> items = scene->items(mapToScene(viewport()->rect()).boundingRect(), Qt::IntersectsItemShape);

    for (int idx = 0; idx < items.size(); idx++) {
        DkThumbLabel *th = dynamic_cast<DkThumbLabel *>(items.at(idx));

        if (!th) {
            qWarning() << "could not cast to thumb label...";
            continue;
        }

        if (th->pixmap().isNull()) {
            th->update();
        }
    }
}

// DkThumbScrollWidget --------------------------------------------------------------------
DkThumbScrollWidget::DkThumbScrollWidget(QWidget *parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
    : DkFadeWidget(parent, flags)
{
    // TODO: is this name required elsewhere?
    setObjectName("DkThumbScrollWidget");
    setContentsMargins(0, 0, 0, 0);

    mThumbsScene = new DkThumbScene(this);
    // thumbsView->setContentsMargins(0,0,0,0);

    mView = new DkThumbsView(mThumbsScene, this);
    mView->setFocusPolicy(Qt::StrongFocus);

    createActions();
    createToolbar();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mToolbar);
    layout->addWidget(mView);
    setLayout(layout);

    enableSelectionActions();
}

void DkThumbScrollWidget::createToolbar()
{
    mToolbar = new QToolBar(tr("Thumb Preview Toolbar"), this);
    mToolbar->setIconSize(QSize(DkSettingsManager::param().effectiveIconSize(this), DkSettingsManager::param().effectiveIconSize(this)));

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

    QToolButton *toolButton = new QToolButton(this);
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
    QWidget *spacer = new QWidget(this);
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
    QAction *loadFile = new QAction(tr("Open Image"), this);
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

    DkPrintPreviewDialog *printPreviewDialog = new DkPrintPreviewDialog(DkUtils::getMainWindow());

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

    if (thumb && thumb->getThumb())
        mThumbsScene->loadFileSignal(thumb->getThumb()->getFilePath(), false);
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

    DkFadeWidget::setVisible(visible);

    if (visible) {
        mThumbsScene->updateThumbLabels();
        mFilterEdit->setText("");
    } else
        mThumbsScene->cancelLoading();
}

void DkThumbScrollWidget::connectToActions(bool activate)
{
    DkActionManager &am = DkActionManager::instance();
    for (QAction *a : am.previewActions())
        a->setEnabled(activate);

    if (activate) {
        connect(am.action(DkActionManager::preview_select_all), &QAction::triggered, mThumbsScene, &DkThumbScene::selectAllThumbs);
        connect(am.action(DkActionManager::preview_zoom_in), &QAction::triggered, mThumbsScene, &DkThumbScene::increaseThumbs);
        connect(am.action(DkActionManager::preview_zoom_out), &QAction::triggered, mThumbsScene, &DkThumbScene::decreaseThumbs);
        connect(am.action(DkActionManager::preview_display_squares), &QAction::triggered, mThumbsScene, &DkThumbScene::toggleSquaredThumbs);
        connect(am.action(DkActionManager::preview_show_labels), &QAction::triggered, mThumbsScene, &DkThumbScene::toggleThumbLabels);
        connect(am.action(DkActionManager::preview_filter), &QAction::triggered, this, &DkThumbScrollWidget::setFilterFocus);
        connect(am.action(DkActionManager::preview_delete), &QAction::triggered, mThumbsScene, &DkThumbScene::deleteSelected);
        connect(am.action(DkActionManager::preview_copy), &QAction::triggered, mThumbsScene, &DkThumbScene::copySelected);
        connect(am.action(DkActionManager::preview_paste), &QAction::triggered, mThumbsScene, &DkThumbScene::pasteImages);
        connect(am.action(DkActionManager::preview_rename), &QAction::triggered, mThumbsScene, &DkThumbScene::renameSelected);
        connect(am.action(DkActionManager::preview_batch), &QAction::triggered, this, &DkThumbScrollWidget::batchProcessFiles);
        connect(am.action(DkActionManager::preview_print), &QAction::triggered, this, &DkThumbScrollWidget::batchPrint);

        connect(mFilterEdit, &QLineEdit::textChanged, this, &DkThumbScrollWidget::filterChangedSignal);
        connect(mView, &DkThumbsView::updateDirSignal, this, &DkThumbScrollWidget::updateDirSignal);
        connect(mThumbsScene, &DkThumbScene::selectionChanged, this, &DkThumbScrollWidget::enableSelectionActions);
    } else {
        disconnect(am.action(DkActionManager::preview_select_all), &QAction::triggered, mThumbsScene, &DkThumbScene::selectAllThumbs);
        disconnect(am.action(DkActionManager::preview_zoom_in), &QAction::triggered, mThumbsScene, &DkThumbScene::increaseThumbs);
        disconnect(am.action(DkActionManager::preview_zoom_out), &QAction::triggered, mThumbsScene, &DkThumbScene::decreaseThumbs);
        disconnect(am.action(DkActionManager::preview_display_squares), &QAction::triggered, mThumbsScene, &DkThumbScene::toggleSquaredThumbs);
        disconnect(am.action(DkActionManager::preview_show_labels), &QAction::triggered, mThumbsScene, &DkThumbScene::toggleThumbLabels);
        disconnect(am.action(DkActionManager::preview_filter), &QAction::triggered, this, &DkThumbScrollWidget::setFilterFocus);
        disconnect(am.action(DkActionManager::preview_delete), &QAction::triggered, mThumbsScene, &DkThumbScene::deleteSelected);
        disconnect(am.action(DkActionManager::preview_copy), &QAction::triggered, mThumbsScene, &DkThumbScene::copySelected);
        disconnect(am.action(DkActionManager::preview_paste), &QAction::triggered, mThumbsScene, &DkThumbScene::pasteImages);
        disconnect(am.action(DkActionManager::preview_rename), &QAction::triggered, mThumbsScene, &DkThumbScene::renameSelected);
        disconnect(am.action(DkActionManager::preview_batch), &QAction::triggered, this, &DkThumbScrollWidget::batchProcessFiles);
        disconnect(am.action(DkActionManager::preview_print), &QAction::triggered, this, &DkThumbScrollWidget::batchPrint);

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

    DkFadeWidget::resizeEvent(event);
}

void DkThumbScrollWidget::contextMenuEvent(QContextMenuEvent *event)
{
    mContextMenu->exec(event->globalPos());
    event->accept();

    // QGraphicsView::contextMenuEvent(event);
}

void DkThumbScrollWidget::enableSelectionActions()
{
    bool enable = !mThumbsScene->getSelectedFiles().isEmpty();

    DkActionManager &am = DkActionManager::instance();
    am.action(DkActionManager::preview_copy)->setEnabled(enable);
    am.action(DkActionManager::preview_rename)->setEnabled(enable);
    am.action(DkActionManager::preview_delete)->setEnabled(enable);
    am.action(DkActionManager::preview_batch)->setEnabled(enable);

    am.action(DkActionManager::preview_select_all)->setChecked(mThumbsScene->allThumbsSelected());
}

// DkThumbPreviewLabel --------------------------------------------------------------------
DkThumbPreviewLabel::DkThumbPreviewLabel(const QString &filePath, int thumbSize, QWidget *parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : QLabel(parent, f)
{
    mThumbSize = thumbSize;

    mThumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(filePath));
    connect(mThumb.data(), &DkThumbNailT::thumbLoadedSignal, this, &DkThumbPreviewLabel::thumbLoaded);

    setFixedSize(mThumbSize, mThumbSize);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setStatusTip(filePath);

    QFileInfo fInfo(filePath);
    setToolTip(fInfo.fileName());

    mThumb->fetchThumb(DkThumbNail::force_exif_thumb);
}

void DkThumbPreviewLabel::thumbLoaded()
{
    if (mThumb->getImage().isNull()) {
        setProperty("empty", true); // apply empty style
        style()->unpolish(this);
        style()->polish(this);
        update();

        return;
    }

    QPixmap pm = QPixmap::fromImage(mThumb->getImage());
    pm = DkImage::makeSquare(pm);

    if (pm.width() > width())
        pm = pm.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    setPixmap(pm);
}

void DkThumbPreviewLabel::mousePressEvent(QMouseEvent *ev)
{
    emit loadFileSignal(mThumb->getFilePath(), ev->modifiers() == Qt::ControlModifier);

    // do not propagate
    // QLabel::mousePressEvent(ev);
}

// -------------------------------------------------------------------- DkRecentFilesEntry
DkRecentDirWidget::DkRecentDirWidget(const DkRecentDir &rde, QWidget *parent)
    : DkFadeWidget(parent)
{
    mRecentDir = rde;

    createLayout();
}

void DkRecentDirWidget::createLayout()
{
    QLabel *dirNameLabel = new QLabel(mRecentDir.dirName(), this);
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

    QIcon pIcon;
    pIcon.addPixmap(DkImage::loadIcon(":/nomacs/img/pin-checked.svg"), QIcon::Normal, QIcon::On);
    pIcon.addPixmap(DkImage::loadIcon(":/nomacs/img/pin.svg"), QIcon::Normal, QIcon::Off);

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

    // check if the folder exists (in the current context)
    // this should fix issues with disconnected samba drives on windows
    if (DkUtils::exists(QFileInfo(mRecentDir.firstFilePath()), 30)) {
        for (auto tp : mRecentDir.filePaths(4)) {
            auto tpl = new DkThumbPreviewLabel(tp, 42, this);
            connect(tpl, &DkThumbPreviewLabel::loadFileSignal, this, &DkRecentDirWidget::loadFileSignal);
            tls << tpl;
        }
    } else {
        qInfo() << mRecentDir.firstFilePath() << "does not exist - according to a fast check";
    }

    QLabel *pathLabel = new QLabel(mRecentDir.dirPath(), this);
    pathLabel->setAlignment(Qt::AlignLeft);
    pathLabel->setObjectName("recentFilesPath");

    QGridLayout *layout = new QGridLayout(this);
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
    mRecentDir.remove();
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

    DkFadeWidget::mousePressEvent(event);
}

void DkRecentDirWidget::enterEvent(DkEnterEvent *event)
{
    for (auto b : mButtons)
        b->show();

    DkFadeWidget::enterEvent(event);
}

void DkRecentDirWidget::leaveEvent(QEvent *event)
{
    for (auto b : mButtons)
        b->hide();

    DkFadeWidget::leaveEvent(event);
}

// -------------------------------------------------------------------- DkRecentFilesEntry
DkRecentFilesWidget::DkRecentFilesWidget(QWidget *parent)
    : DkFadeWidget(parent)
{
    createLayout();
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void DkRecentFilesWidget::setVisible(bool visible)
{
    if (visible)
        updateList();

    DkFadeWidget::setVisible(visible);
}

void DkRecentFilesWidget::createLayout()
{
    mScrollArea = new QScrollArea(this);
    QVBoxLayout *sl = new QVBoxLayout(this);
    sl->addWidget(mScrollArea);
    sl->setContentsMargins(0, 0, 0, 0);

    mScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // updateList();
}

void DkRecentFilesWidget::updateList()
{
    DkTimer dt;

    DkRecentDirManager fm;

    QWidget *dummy = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout(dummy);

    QVector<DkRecentDirWidget *> recentFiles;
    int idx = 0;

    for (auto rd : fm.recentDirs()) {
        DkRecentDirWidget *rf = new DkRecentDirWidget(rd, dummy);
        rf->setMaximumWidth(500);
        connect(rf, &DkRecentDirWidget::loadFileSignal, this, &DkRecentFilesWidget::loadFileSignal);
        connect(rf, &DkRecentDirWidget::loadDirSignal, this, &DkRecentFilesWidget::loadDirSignal);
        connect(rf, &DkRecentDirWidget::removeSignal, this, &DkRecentFilesWidget::entryRemoved);

        recentFiles << rf;
        l->addWidget(rf);
        idx++;
    }

    qInfo() << "list updated in" << dt;

    mScrollArea->setWidget(dummy);
}

void DkRecentFilesWidget::entryRemoved()
{
    updateList();
}

// -------------------------------------------------------------------- DkRecentDir
DkRecentDir::DkRecentDir(const QStringList &filePaths, bool pinned)
{
    mFilePaths = filePaths;
    mIsPinned = pinned;
}

bool DkRecentDir::operator==(const DkRecentDir &o) const
{
    return dirPath() == o.dirPath();
}

void DkRecentDir::update(const DkRecentDir &o)
{
    for (const QString &cp : o.filePaths())
        mFilePaths.push_front(cp);

    mFilePaths.removeDuplicates();
}

QStringList DkRecentDir::filePaths(int max) const
{
    if (max <= 0)
        return mFilePaths;

    QStringList fps = mFilePaths;

    while (fps.size() > max)
        fps.pop_back();

    return fps;
}

bool DkRecentDir::isEmpty() const
{
    return mFilePaths.isEmpty();
}

bool DkRecentDir::isPinned() const
{
    return mIsPinned;
}

QString DkRecentDir::dirName() const
{
    QDir d(dirPath());
    return d.dirName();
}

QString DkRecentDir::dirPath() const
{
    if (mFilePaths.empty())
        return QString("");

    return QFileInfo(mFilePaths[0]).absolutePath();
}

QString DkRecentDir::firstFilePath() const
{
    if (!mFilePaths.isEmpty())
        return mFilePaths[0];

    return QString();
}

void DkRecentDir::remove() const
{
    QStringList &pf = DkSettingsManager::param().global().pinnedFiles;
    QStringList &rf = DkSettingsManager::param().global().recentFiles;

    // remove from history
    for (const QString &fp : mFilePaths) {
        pf.removeAll(fp);
        rf.removeAll(fp);
    }
}

// -------------------------------------------------------------------- DkRecentDirManager
DkRecentDirManager::DkRecentDirManager()
{
    // update pinned files
    mDirs = genFileLists(DkSettingsManager::param().global().pinnedFiles, true);
    auto recentDirs = genFileLists(DkSettingsManager::param().global().recentFiles);

    for (auto rde : recentDirs) {
        if (!mDirs.contains(rde))
            mDirs << rde;
        else {
            int idx = mDirs.indexOf(rde);
            if (idx != -1)
                mDirs[idx].update(rde);
        }
    }
}

QList<DkRecentDir> DkRecentDirManager::recentDirs() const
{
    return mDirs;
}

QList<DkRecentDir> DkRecentDirManager::genFileLists(const QStringList &filePaths, bool pinned)
{
    QMap<QString, QStringList> gPaths;

    for (const QString &cp : filePaths) {
        QFileInfo fi(cp);

        // this if is needed if there are errors in our data
        // however, it incredibly slows down the process if samba mounts are lost
        // if (!fi.isFile())
        //	continue;

        // get folder
        QString dp = fi.absolutePath();

        auto dir = gPaths.find(dp);

        // ok, create a new entry
        if (dir == gPaths.end()) {
            QStringList cpl;
            cpl << cp;
            gPaths.insert(dp, cpl);
        } else {
            // append the filename
            dir.value() << cp;
        }
    }

    // TODO: here is the issue reported in #279
    // the map re-sorts the entries w.r.t to the path
    // create recent directories
    QList<DkRecentDir> rdes;
    for (const QStringList &fps : gPaths.values())
        rdes << DkRecentDir(fps, pinned);

    return rdes;
}

}
