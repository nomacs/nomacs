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
    mViewPort = nullptr;
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

    // snap to pixels without jumping around
    QPoint topLeft(qFloor(selection.left()), qFloor(selection.top()));
    QPoint botRight(qFloor(selection.right()), qFloor(selection.bottom()));
    QRect selectionScaled{topLeft, botRight};

    // much faster to render into a subimage
    // also gives consistent result between preview and final image
    QImage blurImg = img.copy(selectionScaled);
    QPainter blurPainter(&blurImg);

    auto *item = new QGraphicsPixmapItem(QPixmap::fromImage(blurImg));
    auto *blur = new QGraphicsBlurEffect;
    item->setGraphicsEffect(blur);

    QGraphicsScene scene;
    scene.addItem(item);

    blur->setBlurRadius(radius);
    scene.render(&blurPainter);
    blur->setBlurRadius(radius + 2);
    scene.render(&blurPainter);
    scene.render(&blurPainter);

    blurPainter.end();
    painter->drawImage(selectionScaled, blurImg);
}

QSharedPointer<nmc::DkImageContainer> DkPaintPlugin::runPlugin(const QString &runID,
                                                               QSharedPointer<nmc::DkImageContainer> image) const
{
    Q_UNUSED(runID);

    if (!image)
        return image;

    // for a viewport plugin runID and image are null
    if (mViewPort) {
        auto *paintViewport = dynamic_cast<DkPaintViewPort *>(mViewPort);

        if (!paintViewport->isCanceled())
            image->setImage(paintViewport->getPaintedImage(), tr("Drawings Added"));
    }

    return image;
};

nmc::DkPluginViewPort *DkPaintPlugin::getViewPort()
{
    return mViewPort;
}

bool DkPaintPlugin::createViewPort(QWidget *parent)
{
    mViewPort = new DkPaintViewPort(parent);
    mToolBar = new DkPaintToolBar(tr("Paint Toolbar"), parent);

    QPen pen = mViewPort->getPen();
    mToolBar->setPenColor(pen.color());
    mToolBar->setPenWidth(pen.width());
    // mToolBar->setAlpha();

    connect(mToolBar, &DkPaintToolBar::colorSignal, mViewPort, &DkPaintViewPort::setPenColor);
    connect(mToolBar, &DkPaintToolBar::widthSignal, mViewPort, &DkPaintViewPort::setPenWidth);
    connect(mToolBar, &DkPaintToolBar::panSignal, mViewPort, &DkPaintViewPort::setPanning);
    connect(mToolBar, &DkPaintToolBar::cancelSignal, mViewPort, &DkPaintViewPort::discardChangesAndClose);
    connect(mToolBar, &DkPaintToolBar::undoSignal, mViewPort, &DkPaintViewPort::undoLastPaint);
    connect(mToolBar, &DkPaintToolBar::modeChangeSignal, mViewPort, &DkPaintViewPort::setMode);
    connect(mToolBar, &DkPaintToolBar::applySignal, mViewPort, &DkPaintViewPort::applyChangesAndClose);
    connect(mToolBar, &DkPaintToolBar::textChangeSignal, mViewPort, &DkPaintViewPort::textChange);
    connect(mToolBar, &DkPaintToolBar::editFinishSignal, mViewPort, &DkPaintViewPort::textEditFinsh);

    connect(mViewPort, &DkPaintViewPort::editShowSignal, mToolBar, &DkPaintToolBar::showLineEdit);

    return true;
}

void DkPaintPlugin::setVisible(bool visible)
{
    if (!mViewPort)
        return;

    nmc::DkToolBarManager::inst().showToolBar(mToolBar, visible);

    mViewPort->setPanning(false); // always reset panning and make painting tool active

    if (!visible)
        mViewPort->clear();
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
    mPanningToolActive = false;
    mCanceledEditing = false;
    mCurrentCursor = Qt::CrossCursor;
    setCursor(mCurrentCursor);
    mPen = QColor(0, 0, 0);
    mPen.setCapStyle(Qt::RoundCap);
    mPen.setJoinStyle(Qt::RoundJoin);
    mPen.setWidth(1);
    mMouseDown = false;

    loadSettings();
    mTextInputActive = false;
}

void DkPaintViewPort::undoLastPaint()
{
    if (mPaths.empty())
        return; // nothing to undo

    mPaths.pop_back();
    mPathsPen.pop_back();
    mPathsMode.pop_back();
    update();
}

void DkPaintViewPort::mousePressEvent(QMouseEvent *event)
{
    auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
    if (!viewport)
        return;
    if (event->buttons() != Qt::LeftButton)
        return;

    event->accept();
    mMouseDown = true;

    const QPointF pos = event->position();
    mLastMousePos = pos;

    if (mPanningToolActive || event->modifiers() == nmc::DkSettingsManager::param().global().altMod) {
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    const QPointF currPos = mapViewPortToImage(pos);

    bool isMouseOutside = !QRectF(QPointF(), viewport->getImage().size()).contains(currPos);
    if (isMouseOutside)
        return;

    // rollback empty painterpath from click but no dragging
    if (!mPaths.empty() && mPaths.last().isEmpty())
        undoLastPaint();

    // create a new painterpath
    mBeginPos = currPos;
    mPaths.append(QPainterPath{mBeginPos});
    mPathsPen.append(mPen);
    mPathsMode.append(mCurrentMode);

    // lineedit shows only when in text mode
    if (mCurrentMode == mode_text) {
        mTextInputActive = true;
        emit editShowSignal(true);
    }

    update();
}

void DkPaintViewPort::mouseMoveEvent(QMouseEvent *event)
{
    auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
    if (!viewport)
        return;
    if (event->buttons() != Qt::LeftButton)
        return;

    event->accept();
    if (!mMouseDown) // there was no mouse press event, we are not dragging
        return;

    const QPointF pos = event->position();

    if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || mPanningToolActive) {
        QPointF w1 = mapToViewport(pos); // convert to world coordinates
        QPointF w0 = mapToViewport(mLastMousePos);
        mLastMousePos = pos;
        viewport->moveView(w1 - w0);
        return;
    }

    viewport->unsetCursor();
    const QPointF currPos = mapViewPortToImage(pos);
    bool isMouseOutside = !QRectF(QPointF(), viewport->getImage().size()).contains(currPos);

    if (isMouseOutside) {
        // FIXME: creates empty paths outside image
        mPaths.append(QPainterPath{currPos});
        mPathsPen.append(mPen);
        mPathsMode.append(mCurrentMode);
    } else {
        switch (mCurrentMode) {
        case mode_pencil:
            mPaths.last().lineTo(currPos);
            break;
        case mode_line:
        case mode_arrow:
            mPaths.last() = QPainterPath{mBeginPos};
            mPaths.last().lineTo(currPos);
            break;
        case mode_circle:
            mPaths.last() = QPainterPath();
            mPaths.last().addEllipse(QRectF(mBeginPos, currPos));
            break;
        case mode_square:
        case mode_square_fill:
        case mode_blur:
            mPaths.last() = QPainterPath();
            mPaths.last().addRect(QRectF(mBeginPos, currPos));
            break;
        case mode_text:
            break;
        default:
            mPaths.last().lineTo(currPos);
            break;
        }
    }

    update();
}

void DkPaintViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    if (!mMouseDown)
        return;

    event->accept();
    mMouseDown = false;
    setCursor(mCurrentCursor);
}

void DkPaintViewPort::drawPaths(QPainter &painter, nmc::DkBaseViewPort *viewport, bool toImage) const
{
    for (int idx = 0; idx < mPaths.size(); idx++) {
        painter.setPen(mPathsPen.at(idx));
        if (mPathsMode.at(idx) == mode_arrow) {
            painter.fillPath(getArrowHead(mPaths.at(idx), mPathsPen.at(idx).width()),
                             QBrush(mPathsPen.at(idx).color()));
            painter.drawLine(getShorterLine(mPaths.at(idx), mPathsPen.at(idx).width()));
        } else if (mPathsMode.at(idx) == mode_square_fill)
            painter.fillPath(mPaths.at(idx), QBrush(mPathsPen.at(idx).color()));
        else if (mPathsMode.at(idx) == mode_text) {
            painter.fillPath(mPaths.at(idx), QBrush(mPathsPen.at(idx).color()));
            if (!toImage && (idx == mPaths.size() - 1) && (mTextInputActive)) {
                QPointF p = mPaths.at(idx).boundingRect().bottomRight();
                painter.setPen(QPen(QBrush(QColor(0, 0, 0, 180)), mPathsPen.at(idx).width(), Qt::DotLine));
                if (!mHasTextInput)
                    painter.drawLine(QLineF(mBeginPos, mBeginPos - QPoint(0, mPathsPen.at(idx).width() * 10)));
                else
                    painter.drawLine(QLineF(p, p - QPoint(0, mPathsPen.at(idx).width() * 10)));
            }
        } else if (mPathsMode.at(idx) == mode_blur) {
            QImage img = viewport->getImage();
            QRectF rect = mPaths.at(idx).boundingRect();
            getBlur(rect, &painter, img, mPathsPen.at(idx).width());
        } else {
            painter.drawPath(mPaths.at(idx));
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

    // paths are in image coordinates, setup transform like DkViewPort::drawImage()
    painter.setWorldTransform(viewport->getImageMatrix() * viewport->getWorldMatrix());

    // this part gives us correct pixel sizes for lines, fonts, images etc
    painter.scale(1.0 / devicePixelRatioF(), 1.0 / devicePixelRatioF());

    drawPaths(painter, viewport, false);
}

QImage DkPaintViewPort::getPaintedImage()
{
    auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
    if (!viewport)
        return {};
    if (mPaths.empty())
        return {};

    QImage img = viewport->getImage();

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    drawPaths(painter, viewport, true);
    painter.end();

    return img;
}

void DkPaintViewPort::setMode(int mode)
{
    mCurrentMode = mode;
    setCursor(mCurrentCursor);
    emit editShowSignal(false);

    this->repaint();
}

void DkPaintViewPort::textChange(const QString &text)
{
    QFont font;
    font.setFamily(font.defaultFamily());
    font.setPixelSize(mPen.width() * TextEnlarge);
    if (mTextInputActive) {
        mHasTextInput = !text.isEmpty();
        mPaths.last() = QPainterPath();
        mPaths.last().addText(mBeginPos, font, text);
        update();
    }
}

void DkPaintViewPort::textEditFinsh()
{
    if (!mHasTextInput)
        undoLastPaint();
    mTextInputActive = false;
    emit editShowSignal(false);
}

QPointF DkPaintViewPort::mapViewPortToImage(const QPointF &pos) const
{
    auto *viewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
    if (!viewport)
        return {};
    QTransform tx = viewport->getWorldMatrix().inverted() * viewport->getImageMatrix().inverted()
        * viewport->devicePixelRatioF();
    return tx.map(pos);
}

void DkPaintViewPort::clear()
{
    mPaths.clear();
    mPathsPen.clear();
    mPathsMode.clear();
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
    this->mPanningToolActive = checked;
    if (checked)
        mCurrentCursor = Qt::OpenHandCursor;
    else
        mCurrentCursor = Qt::CrossCursor;
    setCursor(mCurrentCursor);
}

void DkPaintViewPort::applyChangesAndClose()
{
    mCanceledEditing = false;
    emit closePlugin();
}

void DkPaintViewPort::discardChangesAndClose()
{
    mCanceledEditing = true;
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
    return mCanceledEditing;
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
    mIcons.resize(icons_end);

    mIcons[apply_icon] = nmc::DkImage::loadIcon(":/nomacs/img/save.svg");
    mIcons[cancel_icon] = nmc::DkImage::loadIcon(":/nomacs/img/close.svg");
    mIcons[pan_icon] = nmc::DkImage::loadIcon(":/nomacs/img/pan.svg");
    mIcons[pan_icon].addFile(":/nomacs/img/pan-checked.svg", QSize(), QIcon::Normal, QIcon::On);
    mIcons[undo_icon] = nmc::DkImage::loadIcon(":/nomacs/img/edit-undo.svg");

    mIcons[pencil_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/pencil.svg");
    mIcons[line_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/line.svg");
    mIcons[arrow_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/arrow.svg");
    mIcons[circle_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/circle-outline.svg");
    mIcons[square_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/square-outline.svg");
    mIcons[square_fill_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/square.svg");
    mIcons[blur_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/blur.svg");
    mIcons[text_icon] = nmc::DkImage::loadIcon(":/nomacsPluginPaint/img/text.svg");
}

void DkPaintToolBar::createLayout()
{
    QList<QKeySequence> enterSc;
    enterSc.append(QKeySequence(Qt::Key_Enter));
    enterSc.append(QKeySequence(Qt::Key_Return));

    auto *applyAction = new QAction(mIcons[apply_icon], tr("Apply (ENTER)"), this);
    applyAction->setShortcuts(enterSc);
    connect(applyAction, &QAction::triggered, this, &DkPaintToolBar::applySignal);

    auto *cancelAction = new QAction(mIcons[cancel_icon], tr("Cancel (ESC)"), this);
    cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(cancelAction, &QAction::triggered, this, &DkPaintToolBar::cancelSignal);

    mPanAction = new QAction(mIcons[pan_icon], tr("Pan"), this);
    mPanAction->setShortcut(QKeySequence(Qt::Key_P));
    mPanAction->setCheckable(true);
    mPanAction->setChecked(false);
    connect(mPanAction, &QAction::triggered, this, &DkPaintToolBar::panSignal);

    // mBrush modes
    auto *pencilAction = new QAction(mIcons[pencil_icon], tr("Pencil"), this);
    pencilAction->setCheckable(true);
    pencilAction->setChecked(true);
    connect(pencilAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_pencil);
    });

    auto *lineAction = new QAction(mIcons[line_icon], tr("Line"), this);
    lineAction->setCheckable(true);
    lineAction->setChecked(false);
    connect(lineAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_line);
    });

    auto *arrowAction = new QAction(mIcons[arrow_icon], tr("Arrow"), this);
    arrowAction->setCheckable(true);
    arrowAction->setChecked(false);
    connect(arrowAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_arrow);
    });

    auto *circleAction = new QAction(mIcons[circle_icon], tr("Circle"), this);
    circleAction->setCheckable(true);
    circleAction->setChecked(false);
    connect(circleAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_circle);
    });

    auto *squareAction = new QAction(mIcons[square_icon], tr("Square"), this);
    squareAction->setCheckable(true);
    squareAction->setChecked(false);
    connect(squareAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_square);
    });

    auto *squarefillAction = new QAction(mIcons[square_fill_icon], tr("Filled Square"), this);
    squarefillAction->setCheckable(true);
    squarefillAction->setChecked(false);
    connect(squarefillAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_square_fill);
    });

    auto *blurAction = new QAction(mIcons[blur_icon], tr("Blur"), this);
    blurAction->setCheckable(true);
    blurAction->setChecked(false);
    connect(blurAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_blur);
    });

    auto *textAction = new QAction(mIcons[text_icon], tr("Text"), this);
    textAction->setCheckable(true);
    textAction->setChecked(false);
    connect(textAction, &QAction::triggered, this, [this] {
        emit modeChangeSignal(mode_text);
    });

    mTextInput = new QLineEdit(this);
    mTextInput->setFixedWidth(100);
    connect(mTextInput, &QLineEdit::textChanged, this, &DkPaintToolBar::textChangeSignal);
    connect(mTextInput, &QLineEdit::editingFinished, this, [this] {
        emit editFinishSignal();
        mTextInput->clear();
    });

    // mPen color
    mPenColor = QColor(0, 0, 0);
    mPenColorButton = new QPushButton(this);
    mPenColorButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(mPenColor)
                                   + "; border: 1px solid #888;}");
    mPenColorButton->setToolTip(tr("Background Color"));
    mPenColorButton->setStatusTip(mPenColorButton->toolTip());
    connect(mPenColorButton, &QPushButton::clicked, this, &DkPaintToolBar::choosePenColor);

    // undo Button
    auto *undoAction = new QAction(mIcons[undo_icon], tr("Undo (CTRL+Z)"), this);
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &DkPaintToolBar::undoSignal);

    mColorDialog = new QColorDialog(this);
    mColorDialog->setObjectName("colorDialog");

    // mPen width
    mWidthBox = new QSpinBox(this);
    mWidthBox->setSuffix("px");
    mWidthBox->setMinimum(1);
    mWidthBox->setMaximum(500); // huge sizes since images might have high resolutions
    connect(mWidthBox, &QSpinBox::valueChanged, this, &DkPaintToolBar::widthSignal);

    // mPen alpha
    mAlphaBox = new QSpinBox(this);
    mAlphaBox->setSuffix("%");
    mAlphaBox->setMinimum(0);
    mAlphaBox->setMaximum(100);
    connect(mAlphaBox, &QSpinBox::valueChanged, this, [this](int val) {
        mPenAlpha = val;
        QColor penColWA = mPenColor;
        penColWA.setAlphaF(mPenAlpha / 100.0);
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

    mToolbarWidgetList = QMap<QString, QAction *>();

    addAction(applyAction);
    addAction(cancelAction);
    addSeparator();
    addAction(mPanAction);
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
    addWidget(mWidthBox);
    addWidget(mPenColorButton);
    addWidget(mAlphaBox);
    addSeparator();
    // addWidget(textInput);
    mToolbarWidgetList.insert(mTextInput->objectName(), this->addWidget(mTextInput));

    showLineEdit(false);
}

void DkPaintToolBar::showLineEdit(bool show)
{
    if (show) {
        mToolbarWidgetList.value(mTextInput->objectName())->setVisible(true);
        mTextInput->setFocus();
    } else
        mToolbarWidgetList.value(mTextInput->objectName())->setVisible(false);
}

void DkPaintToolBar::setPenColor(const QColor &col)
{
    mPenColor = col;
    mPenColorButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(mPenColor)
                                   + "; border: 1px solid #888;}");
    mPenAlpha = col.alpha();
    mAlphaBox->setValue(col.alphaF() * 100);
}

void DkPaintToolBar::setPenWidth(int width)
{
    mWidthBox->setValue(width);
}

void DkPaintToolBar::choosePenColor()
{
    QColor tmpCol = mPenColor;

    mColorDialog->setCurrentColor(tmpCol);
    int ok = mColorDialog->exec();

    if (ok == QDialog::Accepted) {
        mPenColor = mColorDialog->currentColor();
        mPenColorButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(mPenColor)
                                       + "; border: 1px solid #888;}");

        QColor penColWA = mPenColor;
        penColWA.setAlphaF(mPenAlpha / 100.0);
        emit colorSignal(penColWA);
    }
}
};
