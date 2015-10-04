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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTimer>
#include <QDebug>
#include <QVector2D>
#include <QKeyEvent>
#include <QStyleOption>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {


// DkPongSettings --------------------------------------------------------------------
DkPongSettings::DkPongSettings() {

}

void DkPongSettings::setField(const QRect & field) {
	mField = field;
}

QRect DkPongSettings::field() const {
	return mField;
}

void DkPongSettings::setUnit(int unit) {
	mUnit = unit;
}

int DkPongSettings::unit() const {
	return mUnit;
}

void DkPongSettings::setBackgroundColor(const QColor & col) {
	mBgCol = col;
}

QColor DkPongSettings::backgroundColor() const {
	return mBgCol;
}

void DkPongSettings::setForegroundColor(const QColor & col) {
	mFgCol = col;
}

QColor DkPongSettings::foregroundColor() const {
	return mFgCol;
}

// DkPlayer --------------------------------------------------------------------
DkPongPlayer::DkPongPlayer(QSharedPointer<DkPongSettings> settings) {

	mS = settings;
	mSpeed = 0;
	mPos = INT_MAX;

	mRect = QRect(QPoint(), QSize(settings->unit(), 2*settings->unit()));
}

void DkPongPlayer::reset(const QPoint& pos) {
	mRect.moveCenter(pos);
}

int DkPongPlayer::pos() const {
	return mPos;
}

QRect DkPongPlayer::rect() const {
	return mRect;
}

void DkPongPlayer::setHeight(int newHeight) {
	mRect.setHeight(newHeight);
}

void DkPongPlayer::move() {

	if (mRect.top() + mSpeed < 0)
		mRect.moveTop(0);
	else if (mRect.bottom() + mSpeed > mS->field().height())
		mRect.moveBottom(mS->field().height());
	else
		mRect.moveTop(mRect.top() + mSpeed);
}

void DkPongPlayer::setSpeed(int speed) {
	
	mSpeed = speed;

	if (speed != 0)
		mPos = mRect.center().y();
	else
		mPos = INT_MAX;
}

void DkPongPlayer::updateSize() {
	mRect.setHeight(qRound(mS->field().height()*mPlayerRatio));
}

void DkPongPlayer::increaseScore() {
	mScore++;
}

int DkPongPlayer::score() const {
	return mScore;
}

// DkScoreLabel --------------------------------------------------------------------
DkScoreLabel::DkScoreLabel(Qt::Alignment align, QWidget* parent, QSharedPointer<DkPongSettings> settings) : QLabel(parent) {
	
	mS = settings;
	mAlign = align;
	setStyleSheet("QLabel{ color: #fff;}");
	setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	
	mFont = QFont("terminal", 7);
	setFont(mFont);
	qDebug() << "using:" << mFont.family();
}

void DkScoreLabel::paintEvent(QPaintEvent* /*ev*/) {

	QFontMetrics m(mFont);
	
	QPixmap buffer(m.width(text())-1, m.height());
	buffer.fill(Qt::transparent);

	// draw font
	QPen fontPen(mS->foregroundColor());

	QPainter bp(&buffer);
	bp.setPen(fontPen);
	bp.setFont(mFont);
	bp.drawText(buffer.rect(), Qt::AlignHCenter, text());
	bp.end();

	buffer = buffer.scaled(size(), Qt::KeepAspectRatio);

	qDebug() << "buffer size: " << buffer.size() << "my size:" << size();

	QRect r(buffer.rect());
	if (mAlign & Qt::AlignRight)
		r.moveLeft(width() - (mS->unit() * 3 + buffer.width()));
	else if (mAlign & Qt::AlignHCenter)
		r.moveLeft(qRound((width() - buffer.width())*0.5f));
	else
		r.moveLeft(mS->unit() * 3);

	if (mAlign & Qt::AlignBottom)
		r.moveBottom(height());

	QPainter p(this);
	p.drawPixmap(r, buffer);

	//QLabel::paintEvent(ev);
}

// DkPongPort --------------------------------------------------------------------
DkPongPort::DkPongPort(QWidget *parent, Qt::WindowFlags) : QGraphicsView(parent) {

	setAttribute(Qt::WA_TranslucentBackground, true);

	mS = QSharedPointer<DkPongSettings>(new DkPongSettings());
	mPlayerSpeed = qRound(mS->unit()*0.5);

	mBall = DkBall(mS);
	mPlayer1 = DkPongPlayer(mS);
	mPlayer2 = DkPongPlayer(mS);

	mP1Score = new DkScoreLabel(Qt::AlignRight, this, mS);
	mP2Score = new DkScoreLabel(Qt::AlignLeft, this, mS);
	mLargeInfo = new DkScoreLabel(Qt::AlignHCenter | Qt::AlignBottom, this, mS);
	mSmallInfo = new DkScoreLabel(Qt::AlignHCenter, this, mS);
	 
	eventLoop = new QTimer(this);
	eventLoop->setInterval(10);
	//eventLoop->start();

	connect(eventLoop, SIGNAL(timeout()), this, SLOT(gameLoop()));

	initGame();
	pauseGame();
}

void DkPongPort::initGame() {
	
	mBall.reset();
	mPlayer1.reset(QPoint(mS->unit(), qRound(height()*0.5f)));
	mPlayer2.reset(QPoint(qRound(width()-mS->unit()*1.5f), qRound(height()*0.5f)));

	mP1Score->setText(QString::number(mPlayer1.score()));
	mP2Score->setText(QString::number(mPlayer2.score()));

	qDebug() << mPlayer1.score() << ":" << mPlayer2.score();

	update();
}

void DkPongPort::togglePause() {

	pauseGame(eventLoop->isActive());
}

void DkPongPort::pauseGame(bool pause) {

	if (pause) {
		eventLoop->stop();
		mLargeInfo->setText(tr("PAUSED"));
		mSmallInfo->setText(tr("Press <SPACE> to start."));
	}
	else {
		eventLoop->start();
	}

	mLargeInfo->setVisible(pause);
	mSmallInfo->setVisible(pause);
}

void DkPongPort::paintEvent(QPaintEvent* event) {

	// propagate
	QGraphicsView::paintEvent(event);

	QPainter p(viewport());
	p.setBackgroundMode(Qt::TransparentMode);

	p.fillRect(QRect(QPoint(), size()), mS->backgroundColor());
	drawField(p);

	if (mLargeInfo->isVisible())
		p.fillRect(mLargeInfo->geometry(), mS->backgroundColor());

	if (mSmallInfo->isVisible())
		p.fillRect(mSmallInfo->geometry(), mS->backgroundColor());

	p.fillRect(mBall.rect(), mS->foregroundColor());
	p.fillRect(mPlayer1.rect(), mS->foregroundColor());
	p.fillRect(mPlayer2.rect(), mS->foregroundColor());

	p.end();

}

void DkPongPort::drawField(QPainter& p) {

	QPen cPen = p.pen();
	
	// set dash pattern
	QVector<qreal> dashes;
	dashes << 0.1 << 3;

	// create style
	QPen linePen;
	linePen.setColor(mS->foregroundColor());
	linePen.setWidth(qRound(mS->unit()*0.5));
	linePen.setDashPattern(dashes);
	p.setPen(linePen);

	// set line
	QLine line(QPoint(qRound(width()*0.5f), 0), QPoint(qRound(width()*0.5f),height()));
	p.drawLine(line);

	p.setPen(cPen);
}

void DkPongPort::resizeEvent(QResizeEvent *event) {

	//resize(event->size());

	mS->setField(QRect(QPoint(), event->size()));
	mPlayer1.updateSize();
	mPlayer2.updateSize();

	initGame();

	// resize player scores
	QRect sR(QPoint(0, mS->unit()*3), QSize(width()*0.5, width()*0.1));
	QRect sR1 = sR;
	QRect sR2 = sR;
	sR2.moveLeft(qRound(width()*0.5));
	mP1Score->setGeometry(sR1);
	mP2Score->setGeometry(sR2);
	
	// resize info labels
	QRect lIR(QPoint(width()*0.15,0), QSize(width()*0.7, width()*0.1));
	lIR.moveBottom(qRound(height()*0.5 - mS->unit()));
	mLargeInfo->setGeometry(lIR);
	
	QRect sIR(QPoint(width()*0.15,0), QSize(width()*0.7, width()*0.05));
	sIR.moveTop(qRound(height()*0.5 + mS->unit()));
	mSmallInfo->setGeometry(sIR);

	QWidget::resizeEvent(event);
	
}

void DkPongPort::gameLoop() {

	// logic first
	if (!mBall.move(mPlayer1, mPlayer2)) {
		initGame();
		pauseGame();
		return;
	}

	mPlayer1.move();
	mPlayer2.move();

	//repaint();
	viewport()->update();
	
	//QGraphicsView::update();
}

void DkPongPort::keyPressEvent(QKeyEvent *event) {

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

void DkPongPort::keyReleaseEvent(QKeyEvent* event) {

	if (event->key() == Qt::Key_Up && !event->isAutoRepeat() || event->key() == Qt::Key_Down && !event->isAutoRepeat()) {
		mPlayer2.setSpeed(0);
	}
	if (event->key() == Qt::Key_W && !event->isAutoRepeat() || event->key() == Qt::Key_S && !event->isAutoRepeat()) {
		mPlayer1.setSpeed(0);
	}

	QWidget::keyReleaseEvent(event);
}

// DkBall --------------------------------------------------------------------
DkBall::DkBall(QSharedPointer<DkPongSettings> settings) {

	qsrand(1);
	mS = settings;

	mMinSpeed = qRound(mS->unit()*0.5);
	mMaxSpeed = qRound(mS->unit()*0.8);

	mRect = QRect(QPoint(), QSize(mS->unit(), mS->unit()));

	reset();
}

void DkBall::reset() {
	
	mDirection = DkVector(3, 0);// DkVector(mUnit*0.15f, mUnit*0.15f);
	mRect.moveCenter(QPoint(qRound(mS->field().width()*0.5f), qRound(mS->field().height()*0.5f)));
}

QRect DkBall::rect() const {
	return mRect;
}

QPoint DkBall::direction() const {
	return mDirection.getQPointF().toPoint();
}

bool DkBall::move(DkPongPlayer& player1, DkPongPlayer& player2) {

	// collision detection top & bottom
	if (mRect.top() <= mS->field().top() && mDirection.y < 0 || mRect.bottom() >= mS->field().bottom() && mDirection.y > 0) {
		mDirection.rotate(mDirection.angle()*2);
		//qDebug() << "collision...";
	}

	double nAngle = mDirection.angle() + CV_PI*0.5;//DkMath::normAngleRad(mDirection.angle()+CV_PI*0.5, 0, CV_PI*0.5);
	double magic = (double)qrand() / RAND_MAX * 0.1 - 0.05;

	// player collision
	if (player1.rect().intersects(mRect) && mDirection.x < 0) {

		mDirection.rotate((nAngle*2)+magic);
		double mod = (player1.pos() != INT_MAX) ? (player1.rect().center().y() - player1.pos())/(float)mS->field().height() : 0;
		mDirection.y += (float)mod*mS->unit();
	}
	else if (player2.rect().intersects(mRect) && mDirection.x > 0) {

		mDirection.rotate((nAngle*2)+magic);
		double mod = (player2.pos() != INT_MAX) ? (player2.rect().center().y() - player2.pos())/(float)mS->field().height() : 0;
		mDirection.y += (float)mod*mS->unit();
	}
	// collision detection left & right
	else if (mRect.left() <= mS->field().left()) {
		//ballDir.rotate(-ballDir.angle()*2);
		player2.increaseScore();
		return false;
	}
	else if (mRect.right() >= mS->field().right()) {
		player1.increaseScore();
		return false;
	}

	// check angle
	fixAngle();

	if (mDirection.norm() > mMaxSpeed) {
		mDirection.normalize();
		mDirection *= (float)mMaxSpeed;
	}
	else if (mDirection.norm() < mMinSpeed) {
		mDirection.normalize();
		mDirection *= (float)mMinSpeed;
	}

	//qDebug() << ballDir.angle();

	mRect.moveCenter(mRect.center() + mDirection.getQPointF().toPoint());
	
	return true;
}

void DkBall::fixAngle() {

	double angle = mDirection.angle();
	double range = CV_PI / 5.0;
	double sign = angle > 0 ? 1.0 : -1.0;
	angle = abs(angle);
	double newAngle = 0.0;

	if (angle < CV_PI*0.5 && angle > CV_PI*0.5 - range) {
		newAngle = CV_PI*0.5 - range;
	}
	else if (angle > CV_PI*0.5 && angle < CV_PI*0.5 + range) {
		newAngle = CV_PI*0.5 + range;
	}

	if (newAngle != 0.0) {
		mDirection.rotate(mDirection.angle() - (newAngle*sign));
		qDebug() << "angle: " << angle << " new angle: " << newAngle;
	}
}

// DkBall --------------------------------------------------------------------
DkPong::DkPong(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {

	resize(800, 600);

	//showFullScreen();
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	
	DkPongPort* viewport = new DkPongPort(this);
	setCentralWidget(viewport);
	//setWindowOpacity(20);

	show();
}

void DkPong::keyPressEvent(QKeyEvent *event) {

	qDebug() << "escape pressed";

	if (event->key() == Qt::Key_Escape)
		close();
}

}
