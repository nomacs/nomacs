/*******************************************************************************************************
 DkPaintPlugin.cpp
 Created on:	14.07.2013

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

#include "DkPaintPlugin.h"

#include "DkBaseViewPort.h"
#include "DkToolbars.h"
#include "DkUtils.h"

#include <QActionGroup>
#include <QColorDialog>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpinBox>

namespace nmp
{

/*-----------------------------------DkPaintPlugin ---------------------------------------------*/

/**
 *	Constructor
 **/
DkPaintPlugin::DkPaintPlugin()
{
    viewport = nullptr;
}

/**
 *	Destructor
 **/
DkPaintPlugin::~DkPaintPlugin() = default;

/**
 * Returns descriptive image
 **/
QImage DkPaintPlugin::image() const
{
    return QImage(":/nomacsPluginPaint/img/description.png");
}

bool DkPaintPlugin::hideHUD() const
{
    return true;
}

/**
 * Main function: runs plugin based on its ID
 * @param run ID
 * @param current image in the Nomacs viewport
 **/
const int ArrowWidth = 10;
const int ArrowHeight = 18;
const int TextEnlarge = 15;
QPainterPath getArrowHead(QPainterPath line, const int thickness)
{
    QPointF p1 = line.pointAtPercent(0.0);
    QPointF p2 = line.pointAtPercent(1.0);
    QLineF base(p1, p2);
    // Create the vector for the position of the base  of the arrowhead
    QLineF temp(QPoint(0, 0), p2 - p1);
    int val = ArrowHeight + thickness * 4;
    if (base.length() < val) {
        val = (base.length() + thickness * 2);
    }
    temp.setLength(base.length() + thickness * 2 - val);
    // Move across the line up to the head
    QPointF bottonTranslation(temp.p2());

    // Rotate base of the arrowhead
    base.setLength(ArrowWidth + thickness * 2);
    base.setAngle(base.angle() + 90);
    // Move to the correct point
    QPointF temp2 = p1 - base.p2();
    // Center it
    QPointF centerTranslation((temp2.x() / 2), (temp2.y() / 2));

    base.translate(bottonTranslation);
    base.translate(centerTranslation);

    QPainterPath path;
    path.moveTo(p2);
    path.lineTo(base.p1());
    path.lineTo(base.p2());
    path.lineTo(p2);
    path.closeSubpath();
    return path;
}

// gets a shorter line to prevent overlap in the point of the arrow
QLineF getShorterLine(QPainterPath line, const int thickness)
{
    QPointF p1 = line.pointAtPercent(0.0);
    QPointF p2 = line.pointAtPercent(1.0);
    QLineF l(p1, p2);
    int val = ArrowHeight + thickness * 4;
    if (l.length() < val) {
        val = (l.length() + thickness * 2);
    }
    l.setLength(l.length() + thickness * 2 - val);
    return l.toLine();
}

// blur selected rectangle region
void getBlur(const QRectF &selection, QPainter *painter, QImage &img, int radius)
{
    if (selection.isEmpty())
        return;

    QRect selectionScaled = QRect(selection.topLeft().toPoint(), selection.bottomRight().toPoint());

    auto *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(radius);
    auto *item = new QGraphicsPixmapItem(QPixmap::fromImage(img).copy(selectionScaled));
    item->setGraphicsEffect(blur);

    QGraphicsScene scene;
    scene.addItem(item);

    scene.render(painter, selection, QRectF());
    blur->setBlurRadius(radius + 2);
    scene.render(painter, selection, QRectF());
    scene.render(painter, selection, QRectF());
}

QSharedPointer<nmc::DkImageContainer> DkPaintPlugin::runPlugin(const QString &runID,
                                                               QSharedPointer<nmc::DkImageContainer> image) const
{
    Q_UNUSED(runID);

    if (!image)
        return image;

    // for a viewport plugin runID and image are null
    if (viewport) {
        auto *paintViewport = dynamic_cast<DkPaintViewPort *>(viewport);

        if (!paintViewport->isCanceled())
            image->setImage(paintViewport->getPaintedImage(), tr("Drawings Added"));

        viewport->setVisible(false);
    }

    return image;
};

/**
 * returns paintViewPort
 **/
nmc::DkPluginViewPort *DkPaintPlugin::getViewPort()
{
    return viewport;
}

DkPaintViewPort *DkPaintPlugin::getPaintViewPort()
{
    return dynamic_cast<DkPaintViewPort *>(viewport);
}

bool DkPaintPlugin::createViewPort(QWidget *parent)
{
    viewport = new DkPaintViewPort(parent);

    return true;
}

void DkPaintPlugin::setVisible(bool visible)
{
    if (!viewport)
        return;

    viewport->setVisible(visible);
    if (!visible)
        getPaintViewPort()->clear();
}

/*-----------------------------------DkPaintViewPort ---------------------------------------------*/

DkPaintViewPort::DkPaintViewPort(QWidget *parent, Qt::WindowFlags flags)
    : DkPluginViewPort(parent, flags)
{
    setObjectName("DkPaintViewPort");
    init();
    setMouseTracking(true);
}

DkPaintViewPort::~DkPaintViewPort()
{
    saveSettings();

    // active deletion since the MainWindow takes ownership...
    // if we have issues with this, we could disconnect all signals between viewport and toolbar too
    // however, then we have lot's of toolbars in memory if the user opens the plugin again and again
    if (paintToolbar) {
        delete paintToolbar;
        paintToolbar = nullptr;
    }
}

void DkPaintViewPort::saveSettings() const
{
    nmc::DefaultSettings settings;

    settings.beginGroup(objectName());
    settings.setValue("penColor", mPen.color().rgba());
    settings.setValue("penWidth", mPen.width());
    settings.endGroup();
}

void DkPaintViewPort::loadSettings()
{
    nmc::DefaultSettings settings;

    settings.beginGroup(objectName());
    mPen.setColor(QColor::fromRgba(settings.value("penColor", mPen.color().rgba()).toInt()));
    mPen.setWidth(settings.value("penWidth", 15).toInt());
    settings.endGroup();
}

void DkPaintViewPort::init()
{
    panning = false;
    cancelTriggered = false;
    isOutside = false;
    defaultCursor = Qt::CrossCursor;
    setCursor(defaultCursor);
    mPen = QColor(0, 0, 0);
    mPen.setCapStyle(Qt::RoundCap);
    mPen.setJoinStyle(Qt::RoundJoin);
    mPen.setWidth(1);

    paintToolbar = new DkPaintToolBar(tr("Paint Toolbar"), this);

    connect(paintToolbar, &DkPaintToolBar::colorSignal, this, &DkPaintViewPort::setPenColor);
    connect(paintToolbar, &DkPaintToolBar::widthSignal, this, &DkPaintViewPort::setPenWidth);
    connect(paintToolbar, &DkPaintToolBar::panSignal, this, &DkPaintViewPort::setPanning);
    connect(paintToolbar, &DkPaintToolBar::cancelSignal, this, &DkPaintViewPort::discardChangesAndClose);
    connect(paintToolbar, &DkPaintToolBar::undoSignal, this, &DkPaintViewPort::undoLastPaint);
    connect(paintToolbar, &DkPaintToolBar::modeChangeSignal, this, &DkPaintViewPort::setMode);
    connect(paintToolbar, &DkPaintToolBar::applySignal, this, &DkPaintViewPort::applyChangesAndClose);
    connect(paintToolbar, &DkPaintToolBar::textChangeSignal, this, &DkPaintViewPort::textChange);
    connect(paintToolbar, &DkPaintToolBar::editFinishSignal, this, &DkPaintViewPort::textEditFinsh);
    connect(this, &DkPaintViewPort::editShowSignal, paintToolbar, &DkPaintToolBar::showLineEdit);

    loadSettings();
    paintToolbar->setPenColor(mPen.color());
    paintToolbar->setPenWidth(mPen.width());
    textinputenable = false;
}

void DkPaintViewPort::undoLastPaint()
{
    if (paths.empty())
        return; // nothing to undo

    paths.pop_back();
    pathsPen.pop_back();
    pathsMode.pop_back();
    update();
}

void DkPaintViewPort::mousePressEvent(QMouseEvent *event)
{
    // panning -> redirect to viewport
    if (event->buttons() == Qt::LeftButton
        && (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning)) {
        setCursor(Qt::ClosedHandCursor);
        event->setModifiers(Qt::NoModifier); // we want a 'normal' action in the viewport
        event->ignore();
        return;
    }

    if (event->buttons() == Qt::LeftButton && parent()) {
        auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (viewport) {
            if (QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->pos()))) {
                isOutside = false;

                // roll back the empty painterpath generated by click mouse
                if (!paths.empty())
                    if (paths.last().isEmpty())
                        undoLastPaint();

                // create new painterpath
                paths.append(QPainterPath());
                paths.last().moveTo(mapToImage(event->pos()));
                // paths.last().lineTo(mapToImage(event->pos())+QPointF(0.1,0));
                begin = mapToImage(event->pos());
                pathsPen.append(mPen);
                pathsMode.append(selectedMode);
                if (selectedMode == mode_text) {
                    textinputenable = true;
                    // lineedit show only when in text mode and mouse click
                    emit editShowSignal(true);
                }
                update();
            } else
                isOutside = true;
        }
    }

    // no propagation
}

void DkPaintViewPort::mouseMoveEvent(QMouseEvent *event)
{
    // qDebug() << "paint viewport...";

    // panning -> redirect to viewport
    if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning) {
        event->setModifiers(Qt::NoModifier);
        event->ignore();
        update();
        return;
    }

    if (parent()) {
        auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());

        if (viewport) {
            viewport->unsetCursor();

            if (event->buttons() == Qt::LeftButton && parent()) {
                if (QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->pos()))) {
                    if (isOutside) {
                        paths.append(QPainterPath());
                        paths.last().moveTo(mapToImage(event->pos()));
                        pathsPen.append(mPen);
                        pathsMode.append(selectedMode);
                    } else {
                        QPointF point = mapToImage(event->pos());
                        switch (selectedMode) {
                        case mode_pencil:
                            paths.last().lineTo(point);
                            break;

                        case mode_line:
                        case mode_arrow:
                            // paths.last().clear();
                            paths.last() = QPainterPath();
                            paths.last().moveTo(begin);
                            paths.last().lineTo(point);
                            break;

                        case mode_circle:
                            // paths.last().clear();
                            paths.last() = QPainterPath();
                            paths.last().addEllipse(QRectF(begin, point));
                            break;

                        case mode_square:
                        case mode_square_fill:
                        case mode_blur:
                            // paths.last().clear();
                            paths.last() = QPainterPath();
                            paths.last().addRect(QRectF(begin, point));
                            break;

                        case mode_text:
                            break;

                        default:
                            paths.last().lineTo(point);
                            break;
                        }
                        update();
                    }
                    isOutside = false;
                } else
                    isOutside = true;
            }
        }
    }
    // QWidget::mouseMoveEvent(event);	// do not propagate mouse event
}

void DkPaintViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    // panning -> redirect to viewport
    if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning) {
        setCursor(defaultCursor);
        event->setModifiers(Qt::NoModifier);
        event->ignore();
        return;
    }
}

void DkPaintViewPort::drawPaths(QPainter &painter, nmc::DkBaseViewPort *viewport, bool toImage) const
{
    for (int idx = 0; idx < paths.size(); idx++) {
        painter.setPen(pathsPen.at(idx));
        if (pathsMode.at(idx) == mode_arrow) {
            painter.fillPath(getArrowHead(paths.at(idx), pathsPen.at(idx).width()), QBrush(pathsPen.at(idx).color()));
            painter.drawLine(getShorterLine(paths.at(idx), pathsPen.at(idx).width()));
        } else if (pathsMode.at(idx) == mode_square_fill)
            painter.fillPath(paths.at(idx), QBrush(pathsPen.at(idx).color()));
        else if (pathsMode.at(idx) == mode_text) {
            painter.fillPath(paths.at(idx), QBrush(pathsPen.at(idx).color()));
            if (!toImage && (idx == paths.size() - 1) && (textinputenable)) {
                QPointF p = paths.at(idx).boundingRect().bottomRight();
                painter.setPen(QPen(QBrush(QColor(0, 0, 0, 180)), pathsPen.at(idx).width(), Qt::DotLine));
                if (sbuffer.isEmpty())
                    painter.drawLine(QLineF(begin, begin - QPoint(0, pathsPen.at(idx).width() * 10)));
                else
                    painter.drawLine(QLineF(p, p - QPoint(0, pathsPen.at(idx).width() * 10)));
            }
        } else if (pathsMode.at(idx) == mode_blur) {
            QImage img = viewport->getImage();
            qreal dpr = viewport->devicePixelRatioF();
            QTransform tx = QTransform::fromScale(dpr, dpr);
            QRectF rect = tx.map(paths.at(idx)).boundingRect();
            painter.save();
            painter.scale(1.0 / dpr, 1.0 / dpr);
            getBlur(rect, &painter, img, pathsPen.at(idx).width());
            painter.restore();
        } else {
            painter.drawPath(paths.at(idx));
        }
    }
}

void DkPaintViewPort::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
    if (!viewport)
        return;

    QPainter painter(this);

    // path coordinates are in logical image pixels, so use the Image->Viewport transform
    if (mWorldMatrix && mImgMatrix)
        painter.setWorldTransform((*mImgMatrix) * (*mWorldMatrix));

    drawPaths(painter, viewport, false);
}

QImage DkPaintViewPort::getPaintedImage()
{
    auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
    if (!viewport)
        return {};
    if (paths.empty())
        return {};

    QImage img = viewport->getImage();

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    // paths are in logical pixels, convert to physical/image pixels
    qreal dpr = viewport->devicePixelRatioF();
    painter.scale(dpr, dpr);

    drawPaths(painter, viewport, true);
    painter.end();

    return img;
}

void DkPaintViewPort::setMode(int mode)
{
    selectedMode = mode;
    setCursor(defaultCursor);
    emit editShowSignal(false);

    this->repaint();
}

void DkPaintViewPort::textChange(const QString &text)
{
    QFont font;
    font.setFamily(font.defaultFamily());
    font.setPixelSize(mPen.width() * TextEnlarge);
    if (textinputenable) {
        sbuffer = text;
        paths.last() = QPainterPath();
        paths.last().addText(begin, font, text);
        update();
    }
}

void DkPaintViewPort::textEditFinsh()
{
    if (sbuffer.isEmpty())
        undoLastPaint();
    textinputenable = false;
    emit editShowSignal(false);
}

void DkPaintViewPort::clear()
{
    paths.clear();
    pathsPen.clear();
    pathsMode.clear();
}

void DkPaintViewPort::setBrush(const QBrush &brush)
{
    this->mBrush = brush;
}

void DkPaintViewPort::setPen(const QPen &pen)
{
    this->mPen = pen;
}

void DkPaintViewPort::setPenWidth(int width)
{
    this->mPen.setWidth(width);
}

void DkPaintViewPort::setPenColor(QColor color)
{
    this->mPen.setColor(color);
}

void DkPaintViewPort::setPanning(bool checked)
{
    this->panning = checked;
    if (checked)
        defaultCursor = Qt::OpenHandCursor;
    else
        defaultCursor = Qt::CrossCursor;
    setCursor(defaultCursor);
}

void DkPaintViewPort::applyChangesAndClose()
{
    cancelTriggered = false;
    emit closePlugin();
}

void DkPaintViewPort::discardChangesAndClose()
{
    cancelTriggered = true;
    emit closePlugin();
}

QBrush DkPaintViewPort::getBrush() const
{
    return mBrush;
}

QPen DkPaintViewPort::getPen() const
{
    return mPen;
}

bool DkPaintViewPort::isCanceled()
{
    return cancelTriggered;
}

void DkPaintViewPort::setVisible(bool visible)
{
    if (paintToolbar)
        nmc::DkToolBarManager::inst().showToolBar(paintToolbar, visible);

    DkPluginViewPort::setVisible(visible);
}

/*-----------------------------------DkPaintToolBar ---------------------------------------------*/
DkPaintToolBar::DkPaintToolBar(const QString &title, QWidget *parent /* = 0 */)
    : QToolBar(title, parent)
{
    setObjectName("paintToolBar");
    createIcons();
    createLayout();
    QMetaObject::connectSlotsByName(this);

    qDebug() << "[PAINT TOOLBAR] created...";
}

DkPaintToolBar::~DkPaintToolBar()
{
    qDebug() << "[PAINT TOOLBAR] deleted...";
}

void DkPaintToolBar::createIcons()
{
    // create icons
    icons.resize(icons_end);

    icons[apply_icon] = nmc::DkImage::loadIcon(":/nomacs/img/save.svg");
    icons[cancel_icon] = nmc::DkImage::loadIcon(":/nomacs/img/close.svg");
    icons[pan_icon] = nmc::DkImage::loadIcon(":/nomacs/img/pan.svg");
    icons[pan_icon].addFile(":/nomacs/img/pan-checked.svg", QSize(), QIcon::Normal, QIcon::On);
    icons[undo_icon] = nmc::DkImage::loadIcon(":/nomacs/img/edit-undo.svg");

    icons[pencil_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/pencil.svg");
    icons[line_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/line.svg");
    icons[arrow_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/arrow.svg");
    icons[circle_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/circle-outline.svg");
    icons[square_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/square-outline.svg");
    icons[square_fill_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/square.svg");
    icons[blur_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/blur.svg");
    icons[text_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/text.svg");
}

void DkPaintToolBar::createLayout()
{
    QList<QKeySequence> enterSc;
    enterSc.append(QKeySequence(Qt::Key_Enter));
    enterSc.append(QKeySequence(Qt::Key_Return));

    auto *applyAction = new QAction(icons[apply_icon], tr("Apply (ENTER)"), this);
    applyAction->setShortcuts(enterSc);
    connect(applyAction, &QAction::triggered, this, &DkPaintToolBar::applySignal);

    auto *cancelAction = new QAction(icons[cancel_icon], tr("Cancel (ESC)"), this);
    cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(cancelAction, &QAction::triggered, this, &DkPaintToolBar::cancelSignal);

    panAction = new QAction(icons[pan_icon], tr("Pan"), this);
    panAction->setShortcut(QKeySequence(Qt::Key_P));
    panAction->setCheckable(true);
    panAction->setChecked(false);
    connect(panAction, &QAction::triggered, this, &DkPaintToolBar::panSignal);

    // mBrush modes
    pencilAction = new QAction(icons[pencil_icon], tr("Pencil"), this);
    pencilAction->setCheckable(true);
    pencilAction->setChecked(true);
    connect(pencilAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_pencil);
    });

    lineAction = new QAction(icons[line_icon], tr("Line"), this);
    lineAction->setCheckable(true);
    lineAction->setChecked(false);
    connect(lineAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_line);
    });

    arrowAction = new QAction(icons[arrow_icon], tr("Arrow"), this);
    arrowAction->setCheckable(true);
    arrowAction->setChecked(false);
    connect(arrowAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_arrow);
    });

    circleAction = new QAction(icons[circle_icon], tr("Circle"), this);
    circleAction->setCheckable(true);
    circleAction->setChecked(false);
    connect(circleAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_circle);
    });

    squareAction = new QAction(icons[square_icon], tr("Square"), this);
    squareAction->setCheckable(true);
    squareAction->setChecked(false);
    connect(squareAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_square);
    });

    squarefillAction = new QAction(icons[square_fill_icon], tr("Filled Square"), this);
    squarefillAction->setCheckable(true);
    squarefillAction->setChecked(false);
    connect(squarefillAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_square_fill);
    });

    blurAction = new QAction(icons[blur_icon], tr("Blur"), this);
    blurAction->setCheckable(true);
    blurAction->setChecked(false);
    connect(blurAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_blur);
    });

    textAction = new QAction(icons[text_icon], tr("Text"), this);
    textAction->setCheckable(true);
    textAction->setChecked(false);
    connect(textAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_text);
    });

    textInput = new QLineEdit(this);
    textInput->setFixedWidth(100);
    connect(textInput, &QLineEdit::textChanged, this, &DkPaintToolBar::textChangeSignal);
    connect(textInput, &QLineEdit::editingFinished, this, [this] {
        emit editFinishSignal();
        textInput->clear();
    });

    // mPen color
    penCol = QColor(0, 0, 0);
    penColButton = new QPushButton(this);
    penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol)
                                + "; border: 1px solid #888;}");
    penColButton->setToolTip(tr("Background Color"));
    penColButton->setStatusTip(penColButton->toolTip());
    connect(penColButton, &QPushButton::clicked, this, &DkPaintToolBar::choosePenColor);

    // undo Button
    undoAction = new QAction(icons[undo_icon], tr("Undo (CTRL+Z)"), this);
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &DkPaintToolBar::undoSignal);

    colorDialog = new QColorDialog(this);
    colorDialog->setObjectName("colorDialog");

    // mPen width
    widthBox = new QSpinBox(this);
    widthBox->setSuffix("px");
    widthBox->setMinimum(1);
    widthBox->setMaximum(500); // huge sizes since images might have high resolutions
    connect(widthBox, &QSpinBox::valueChanged, this, &DkPaintToolBar::widthSignal);

    // mPen alpha
    alphaBox = new QSpinBox(this);
    alphaBox->setSuffix("%");
    alphaBox->setMinimum(0);
    alphaBox->setMaximum(100);
    connect(alphaBox, &QSpinBox::valueChanged, this, [this](int val) {
        penAlpha = val;
        QColor penColWA = penCol;
        penColWA.setAlphaF(penAlpha / 100.0);
        emit colorSignal(penColWA);
    });

    auto *modesGroup = new QActionGroup(this);
    modesGroup->addAction(pencilAction);
    modesGroup->addAction(lineAction);
    modesGroup->addAction(arrowAction);
    modesGroup->addAction(circleAction);
    modesGroup->addAction(squareAction);
    modesGroup->addAction(squarefillAction);
    modesGroup->addAction(blurAction);
    modesGroup->addAction(textAction);

    toolbarWidgetList = QMap<QString, QAction *>();

    addAction(applyAction);
    addAction(cancelAction);
    addSeparator();
    addAction(panAction);
    addAction(undoAction);
    addSeparator();
    addAction(pencilAction);
    addAction(lineAction);
    addAction(arrowAction);
    addAction(circleAction);
    addAction(squareAction);
    addAction(squarefillAction);
    addAction(blurAction);
    addAction(textAction);
    addSeparator();
    addWidget(widthBox);
    addWidget(penColButton);
    addWidget(alphaBox);
    addSeparator();
    // addWidget(textInput);
    toolbarWidgetList.insert(textInput->objectName(), this->addWidget(textInput));

    showLineEdit(false);
}

void DkPaintToolBar::showLineEdit(bool show)
{
    if (show) {
        toolbarWidgetList.value(textInput->objectName())->setVisible(true);
        textInput->setFocus();
    } else
        toolbarWidgetList.value(textInput->objectName())->setVisible(false);
}

void DkPaintToolBar::setVisible(bool visible)
{
    // if (!visible)
    //	emit colorSignal(QColor(0,0,0));
    if (visible) {
        // emit colorSignal(penCol);
        // widthBox->setValue(10);
        // alphaBox->setValue(100);
        panAction->setChecked(false);
    }

    qDebug() << "[PAINT TOOLBAR] set visible: " << visible;

    QToolBar::setVisible(visible);
}

void DkPaintToolBar::setPenColor(const QColor &col)
{
    penCol = col;
    penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol)
                                + "; border: 1px solid #888;}");
    penAlpha = col.alpha();
    alphaBox->setValue(col.alphaF() * 100);
}

void DkPaintToolBar::setPenWidth(int width)
{
    widthBox->setValue(width);
}

void DkPaintToolBar::choosePenColor()
{
    QColor tmpCol = penCol;

    colorDialog->setCurrentColor(tmpCol);
    int ok = colorDialog->exec();

    if (ok == QDialog::Accepted) {
        penCol = colorDialog->currentColor();
        penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol)
                                    + "; border: 1px solid #888;}");

        QColor penColWA = penCol;
        penColWA.setAlphaF(penAlpha / 100.0);
        emit colorSignal(penColWA);
    }
}

};
