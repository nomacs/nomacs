/*******************************************************************************************************
DkViewPort.cpp
Created on:	05.05.2011

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

#include "DkPong.h"

#include "DkSettings.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QScreen>
#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QVector2D>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

// DkPongSettings --------------------------------------------------------------------
DkPongSettings::DkPongSettings()
{
    loadSettings();
}

void DkPongSettings::setField(const QRect &field)
{
    mField = field;
}

QRect DkPongSettings::field() const
{
    return mField;
}

void DkPongSettings::setUnit(int unit)
{
    mUnit = unit;
}

int DkPongSettings::unit() const
{
    return mUnit;
}

void DkPongSettings::setBackgroundColor(const QColor &col)
{
    mBgCol = col;
}

QColor DkPongSettings::backgroundColor() const
{
    return mBgCol;
}

void DkPongSettings::setForegroundColor(const QColor &col)
{
    mFgCol = col;
}

QColor DkPongSettings::foregroundColor() const
{
    return mFgCol;
}

void DkPongSettings::setTotalScore(int maxScore)
{
    mTotalScore = maxScore;
}

int DkPongSettings::totalScore() const
{
    return mTotalScore;
}

void DkPongSettings::writeSettings()
{
    DefaultSettings settings;
    settings.beginGroup("DkPong");

    settings.setValue("field", mField);
    settings.setValue("unit", mUnit);
    settings.setValue("totalScore", mTotalScore);

    settings.setValue("backgroundColor", mBgCol.name());
    settings.setValue("foregroundColor", mFgCol.name());

    settings.setValue("backgroundAlpha", mBgCol.alpha());
    settings.setValue("foregroundAlpha", mFgCol.alpha());

    settings.setValue("player1Name", mPlayer1Name);
    settings.setValue("player2Name", mPlayer2Name);

    settings.setValue("playerRatio", qRound(mPlayerRatio * 100.0f));

    settings.endGroup();

    qDebug() << "settings written...";
}

QString DkPongSettings::player1Name() const
{
    return mPlayer1Name;
}

QString DkPongSettings::player2Name() const
{
    return mPlayer2Name;
}

float DkPongSettings::playerRatio() const
{
    return mPlayerRatio;
}

void DkPongSettings::loadSettings()
{
    DefaultSettings settings;
    settings.beginGroup("DkPong");

    mField = settings.value("field", mField).toRect();
    mUnit = settings.value("unit", mUnit).toInt();
    mTotalScore = settings.value("totalScore", mTotalScore).toInt();

    mPlayer1Name = settings.value("player1Name", mPlayer1Name).toString();
    mPlayer2Name = settings.value("player2Name", mPlayer2Name).toString();

    mPlayerRatio = settings.value("playerRatio", qRound(mPlayerRatio * 100)).toInt() / 100.0f;

    int bgAlpha = settings.value("backgroundAlpha", mBgCol.alpha()).toInt();
    int fgAlpha = settings.value("foregroundAlpha", mFgCol.alpha()).toInt();

    mBgCol.setNamedColor(settings.value("backgroundColor", mBgCol.name()).toString());
    mFgCol.setNamedColor(settings.value("foregroundColor", mFgCol.name()).toString());

    mBgCol.setAlpha(bgAlpha);
    mFgCol.setAlpha(fgAlpha);

    settings.endGroup();
}

// DkPlayer --------------------------------------------------------------------
DkPongPlayer::DkPongPlayer(const QString &playerName, QSharedPointer<DkPongSettings> settings)
{
    mPlayerName = playerName;
    mS = settings;
    mSpeed = 0;
    mPos = INT_MAX;

    mRect = QRect(QPoint(), QSize(settings->unit(), 2 * settings->unit()));
}

void DkPongPlayer::reset(const QPoint &pos)
{
    mRect.moveCenter(pos);
}

int DkPongPlayer::pos() const
{
    return mPos;
}

QRect DkPongPlayer::rect() const
{
    return mRect;
}

void DkPongPlayer::setHeight(int newHeight)
{
    mRect.setHeight(newHeight);
}

void DkPongPlayer::move()
{
    if (mRect.top() + mSpeed < 0)
        mRect.moveTop(0);
    else if (mRect.bottom() + mSpeed > mS->field().height())
        mRect.moveBottom(mS->field().height());
    else
        mRect.moveTop(mRect.top() + mSpeed);
}

void DkPongPlayer::setSpeed(int speed)
{
    mSpeed = speed;

    if (speed != 0)
        mPos = mRect.center().y();
    else
        mPos = INT_MAX;
}

void DkPongPlayer::updateSize()
{
    mRect.setHeight(qRound(mS->field().height() * mS->playerRatio()));
}

void DkPongPlayer::increaseScore()
{
    mScore++;
}

void DkPongPlayer::resetScore()
{
    mScore = 0;
}

int DkPongPlayer::score() const
{
    return mScore;
}

QString DkPongPlayer::name() const
{
    return mPlayerName;
}

// DkScoreLabel --------------------------------------------------------------------
DkScoreLabel::DkScoreLabel(Qt::Alignment align, QWidget *parent, QSharedPointer<DkPongSettings> settings)
    : QLabel(parent)
{
    mS = settings;
    mAlign = align;
    setStyleSheet("QLabel{ color: #fff;}");
    setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    mFont = QFont("terminal", 6);
    setFont(mFont);
    qDebug() << "using:" << mFont.family();
}

void DkScoreLabel::paintEvent(QPaintEvent * /*ev*/)
{
    QFontMetrics m(mFont);

    QPixmap buffer(m.horizontalAdvance(text()) - 1, m.height());
    buffer.fill(Qt::transparent);
    // buffer.fill(Qt::red);

    // draw font
    QPen fontPen(mS->foregroundColor());

    QPainter bp(&buffer);
    bp.setPen(fontPen);
    bp.setFont(mFont);
    bp.drawText(buffer.rect(), Qt::AlignHCenter | Qt::AlignVCenter, text());
    bp.end();

    QSize bSize(size());
    bSize.setHeight(qRound(bSize.height() - mS->unit() * 0.5));
    buffer = buffer.scaled(bSize, Qt::KeepAspectRatio, Qt::FastTransformation);

    QRect r(buffer.rect());

    if (mAlign & Qt::AlignRight)
        r.moveLeft(width() - (mS->unit() * 3 + buffer.width()));
    else if (mAlign & Qt::AlignHCenter)
        r.moveLeft(qRound((width() - buffer.width()) * 0.5f));
    else
        r.moveLeft(mS->unit() * 3);

    if (mAlign & Qt::AlignBottom)
        r.moveBottom(height());
    else
        r.moveTop(qRound((height() - buffer.height()) / 2.0f)); // default: center

    QPainter p(this);
    p.drawPixmap(r, buffer);

    // QLabel::paintEvent(ev);
}

// DkPongPort --------------------------------------------------------------------
DkPongPort::DkPongPort(QWidget *parent, Qt::WindowFlags)
    : QGraphicsView(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);

    mS = QSharedPointer<DkPongSettings>(new DkPongSettings());
    mPlayerSpeed = qRound(mS->field().width() * 0.007);

    mBall = DkBall(mS);
    mPlayer1 = DkPongPlayer(mS->player1Name(), mS);
    mPlayer2 = DkPongPlayer(mS->player2Name(), mS);

    mP1Score = new DkScoreLabel(Qt::AlignRight, this, mS);
    mP2Score = new DkScoreLabel(Qt::AlignLeft, this, mS);
    mLargeInfo = new DkScoreLabel(Qt::AlignHCenter | Qt::AlignBottom, this, mS);
    mSmallInfo = new DkScoreLabel(Qt::AlignHCenter, this, mS);

    mEventLoop = new QTimer(this);
    mEventLoop->setInterval(10);
    // eventLoop->start();

    mCountDownTimer = new QTimer(this);
    mCountDownTimer->setInterval(500);

    connect(mEventLoop, SIGNAL(timeout()), this, SLOT(gameLoop()));
    connect(mCountDownTimer, SIGNAL(timeout()), this, SLOT(countDown()));

    initGame();
    pauseGame();
}

void DkPongPort::initGame()
{
    mBall.reset();
    mPlayer1.reset(QPoint(mS->unit(), qRound(height() * 0.5f)));
    mPlayer2.reset(QPoint(qRound(width() - mS->unit() * 1.5f), qRound(height() * 0.5f)));

    if (mPlayer1.score() == 0 && mPlayer2.score() == 0) {
        mP1Score->setText(mPlayer1.name());
        mP2Score->setText(mPlayer2.name());
    } else {
        mP1Score->setText(QString::number(mPlayer1.score()));
        mP2Score->setText(QString::number(mPlayer2.score()));
    }

    qDebug() << mPlayer1.score() << ":" << mPlayer2.score();

    update();
}

void DkPongPort::togglePause()
{
    pauseGame(mEventLoop->isActive());
}

void DkPongPort::pauseGame(bool pause)
{
    if (pause) {
        mCountDownTimer->stop();
        mEventLoop->stop();
        mLargeInfo->setText(tr("PAUSED"));
        mSmallInfo->setText(tr("Press <SPACE> to start."));
    } else {
        mP1Score->setText(QString::number(mPlayer1.score()));
        mP2Score->setText(QString::number(mPlayer2.score()));

        if (mPlayer1.score() >= mS->totalScore() || mPlayer2.score() >= mS->totalScore()) {
            mPlayer1.resetScore();
            mPlayer2.resetScore();
            initGame();
        }

        mEventLoop->start();
    }

    mLargeInfo->setVisible(pause);
    mSmallInfo->setVisible(pause);
}

DkPongPort::~DkPongPort()
{
}

QSharedPointer<DkPongSettings> DkPongPort::settings() const
{
    return mS;
}

void DkPongPort::countDown()
{
    mCountDownSecs--;

    if (mCountDownSecs == 0) {
        mCountDownTimer->stop();
        pauseGame(false); // start
    } else
        mLargeInfo->setText(QString::number(mCountDownSecs));
}

void DkPongPort::paintEvent(QPaintEvent *event)
{
    // propagate
    QGraphicsView::paintEvent(event);

    QPainter p(viewport());
    p.setBackgroundMode(Qt::TransparentMode);

    p.fillRect(QRect(QPoint(), size()), mS->backgroundColor());
    drawField(p);

    p.fillRect(mBall.rect(), mS->foregroundColor());
    p.fillRect(mPlayer1.rect(), mS->foregroundColor());
    p.fillRect(mPlayer2.rect(), mS->foregroundColor());

    // clear area under text
    if (mLargeInfo->isVisible()) {
        p.fillRect(mLargeInfo->geometry(), mS->foregroundColor());
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(mLargeInfo->geometry(), mS->backgroundColor());
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    // clear area under small text
    if (mSmallInfo->isVisible()) {
        p.fillRect(mSmallInfo->geometry(), mS->foregroundColor());
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(mSmallInfo->geometry(), mS->backgroundColor());
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    p.end();
}

void DkPongPort::drawField(QPainter &p)
{
    QPen cPen = p.pen();

    // set dash pattern
    QVector<qreal> dashes;
    dashes << 0.1 << 3;

    // create style
    QPen linePen;
    linePen.setColor(mS->foregroundColor());
    linePen.setWidth(qRound(mS->unit() * 0.5));
    linePen.setDashPattern(dashes);
    p.setPen(linePen);

    // set line
    QLine line(QPoint(qRound(width() * 0.5f), 0), QPoint(qRound(width() * 0.5f), height()));
    p.drawLine(line);

    p.setPen(cPen);
}

void DkPongPort::startCountDown(int sec)
{
    mCountDownSecs = sec;
    pauseGame();
    mCountDownTimer->start();
    mLargeInfo->setText(QString::number(mCountDownSecs));
    mLargeInfo->show();
    mSmallInfo->hide();
}

void DkPongPort::resizeEvent(QResizeEvent *event)
{
    // resize(event->size());

    mS->setField(QRect(QPoint(), event->size()));
    mPlayerSpeed = qRound(mS->field().width() * 0.007);
    mPlayer1.updateSize();
    mPlayer2.updateSize();
    mBall.updateSize();

    initGame();

    // resize player scores
    QRect sR(QPoint(0, mS->unit() * 3), QSize(qRound(width() * 0.5), qRound(height() * 0.15)));
    QRect sR1 = sR;
    QRect sR2 = sR;
    sR2.moveLeft(qRound(width() * 0.5));
    mP1Score->setGeometry(sR1);
    mP2Score->setGeometry(sR2);

    // resize info labels
    QRect lIR(QPoint(qRound(width() * 0.15), 0), QSize(qRound(width() * 0.7), qRound(height() * 0.15)));
    lIR.moveBottom(qRound(height() * 0.5 + mS->unit()));
    mLargeInfo->setGeometry(lIR);

    QRect sIR(QPoint(qRound(width() * 0.15), 0), QSize(qRound(width() * 0.7), qRound(height() * 0.08)));
    sIR.moveTop(qRound(height() * 0.5 + mS->unit() * 2));
    mSmallInfo->setGeometry(sIR);

    QWidget::resizeEvent(event);
}

void DkPongPort::gameLoop()
{
    // logic first
    if (!mBall.move(mPlayer1, mPlayer2)) {
        initGame();

        // check if somebody won
        if (mPlayer1.score() >= mS->totalScore() || mPlayer2.score() >= mS->totalScore()) {
            pauseGame();
            mLargeInfo->setText(tr("%1 won!").arg(mPlayer1.score() > mPlayer2.score() ? mPlayer1.name() : mPlayer2.name()));
            mSmallInfo->setText(tr("Hit <SPACE> to start a new Game"));
        } else
            startCountDown();

        return;
    }

    mPlayer1.move();
    mPlayer2.move();

    // repaint();
    viewport()->update();

    // QGraphicsView::update();
}

void DkPongPort::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up && !event->isAutoRepeat()) {
        mPlayer2.setSpeed(-mPlayerSpeed);
    }
    if (event->key() == Qt::Key_Down && !event->isAutoRepeat()) {
        mPlayer2.setSpeed(mPlayerSpeed);
    }
    if (event->key() == Qt::Key_W && !event->isAutoRepeat()) {
        mPlayer1.setSpeed(-mPlayerSpeed);
    }
    if (event->key() == Qt::Key_S && !event->isAutoRepeat()) {
        mPlayer1.setSpeed(mPlayerSpeed);
    }
    if (event->key() == Qt::Key_Space) {
        togglePause();
    }

    QWidget::keyPressEvent(event);
}

void DkPongPort::keyReleaseEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Up && !event->isAutoRepeat()) || (event->key() == Qt::Key_Down && !event->isAutoRepeat())) {
        mPlayer2.setSpeed(0);
    }
    if ((event->key() == Qt::Key_W && !event->isAutoRepeat()) || (event->key() == Qt::Key_S && !event->isAutoRepeat())) {
        mPlayer1.setSpeed(0);
    }

    QWidget::keyReleaseEvent(event);
}

// DkBall --------------------------------------------------------------------
DkBall::DkBall(QSharedPointer<DkPongSettings> settings)
{
    mS = settings;

    mMinSpeed = qRound(mS->field().width() * 0.005);
    mMaxSpeed = qRound(mS->field().width() * 0.01);
    qDebug() << "maxSpeed: " << mMaxSpeed;

    mRect = QRect(QPoint(), QSize(mS->unit(), mS->unit()));

    // setDirection(DkVector(10, 10));

    reset();
}

void DkBall::reset()
{
    // mDirection = DkVector(3, 0);// DkVector(mUnit*0.15f, mUnit*0.15f);
    mRect.moveCenter(QPoint(qRound(mS->field().width() * 0.5f), qRound(mS->field().height() * 0.5f)));
}

void DkBall::updateSize()
{
    mMinSpeed = qRound(mS->field().width() * 0.005);
    mMaxSpeed = qRound(mS->field().width() * 0.01);
    setDirection(DkVector(QRandomGenerator::global()->generateDouble() * 10.0f - 5.0f, QRandomGenerator::global()->generateDouble() * 5.0f - 2.5f));
    // setDirection(DkVector(10,10));
}

QRect DkBall::rect() const
{
    return mRect;
}

QPoint DkBall::direction() const
{
    return mDirection.toQPointF().toPoint();
}

bool DkBall::move(DkPongPlayer &player1, DkPongPlayer &player2)
{
    DkVector dir = mDirection;

    // collision detection top & bottom
    if ((mRect.top() <= mS->field().top() && dir.y < 0) || (mRect.bottom() >= mS->field().bottom() && dir.y > 0)) {
        dir.rotate(dir.angle() * 2);
        // qDebug() << "collision...";
    }

    double nAngle = dir.angle() + CV_PI * 0.5; // DkMath::normAngleRad(dir.angle()+CV_PI*0.5, 0, CV_PI*0.5);
    double magic = QRandomGenerator::global()->generateDouble() * 0.5 - 0.25;

    // player collision
    if (player1.rect().intersects(mRect) && dir.x < 0) {
        dir.rotate((nAngle * 2) + magic);

        // change the angle if the ball becomes horizontal
        if (DkMath::distAngle(dir.angle(), 0.0) > 0.01)
            dir.rotate(0.7);

        double mod = (player1.pos() != INT_MAX) ? (player1.rect().center().y() - player1.pos()) / (float)mS->field().height() : 0;
        dir.y += (float)mod * mS->unit();
    } else if (player2.rect().intersects(mRect) && dir.x > 0) {
        dir.rotate((nAngle * 2) + magic);

        // change the angle if the ball becomes horizontal
        if (DkMath::distAngle(dir.angle(), 0.0) > 0.01)
            dir.rotate(0.7);

        double mod = (player2.pos() != INT_MAX) ? (player2.rect().center().y() - player2.pos()) / (float)mS->field().height() : 0;
        dir.y += (float)mod * mS->unit();
    }
    // collision detection left & right
    else if (mRect.left() <= mS->field().left()) {
        dir = QPointF(player2.rect().center()) - mS->field().center();
        dir.normalize();
        dir *= (float)mMinSpeed;
        setDirection(dir);
        player2.increaseScore();
        return false;
    } else if (mRect.right() >= mS->field().right()) {
        dir = QPointF(player1.rect().center()) - mS->field().center();
        dir.normalize();
        dir *= (float)mMinSpeed;
        setDirection(dir);
        player1.increaseScore();
        return false;
    }

    // qDebug() << ballDir.angle();

    setDirection(dir);
    mRect.moveCenter(mRect.center() + mDirection.toQPointF().toPoint());

    return true;
}

void DkBall::setDirection(const DkVector &dir)
{
    mDirection = dir;

    // check angle
    fixAngle();

    if (mDirection.norm() > mMaxSpeed) {
        mDirection.normalize();
        mDirection *= (float)mMaxSpeed;
    } else if (mDirection.norm() < mMinSpeed) {
        mDirection.normalize();
        mDirection *= (float)mMinSpeed;
    }
}

void DkBall::fixAngle()
{
    double angle = mDirection.angle();
    double range = CV_PI / 5.0;
    double sign = angle > 0 ? 1.0 : -1.0;
    angle = fabs(angle);
    double newAngle = 0.0;

    if (angle < CV_PI * 0.5 && angle > CV_PI * 0.5 - range) {
        newAngle = CV_PI * 0.5 - range;
    } else if (angle > CV_PI * 0.5 && angle < CV_PI * 0.5 + range) {
        newAngle = CV_PI * 0.5 + range;
    }

    if (newAngle != 0.0) {
        mDirection.rotate(mDirection.angle() - (newAngle * sign));
        // qDebug() << "angle: " << angle << " new angle: " << newAngle;
    }
}

// DkBall --------------------------------------------------------------------
DkPong::DkPong(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setStyleSheet("QWidget{background-color: rgba(0,0,0,0); border: none;}");
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    mViewport = new DkPongPort(this);

    QRect screenRect = QApplication::primaryScreen()->availableGeometry();
    QRect winRect = screenRect;

    if (mViewport->settings()->field() == QRect())
        winRect.setSize(screenRect.size() * 0.5);
    else
        winRect = mViewport->settings()->field();

    winRect.moveCenter(screenRect.center());
    setGeometry(winRect);

    setCentralWidget(mViewport);
    show();
}

void DkPong::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

void DkPong::closeEvent(QCloseEvent *event)
{
    mViewport->settings()->writeSettings();

    QMainWindow::closeEvent(event);
}

}
