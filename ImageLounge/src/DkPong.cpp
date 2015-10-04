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
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkPlayer --------------------------------------------------------------------
DkPongPlayer::DkPongPlayer(int unit, const QRect& field) {

	mUnit = unit;
	mField = field;
	mSpeed = 0;
	mPos = INT_MAX;

	mRect = QRect(QPoint(), QSize(unit, 2*unit));
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
	else if (mRect.bottom() + mSpeed > mField.height())
		mRect.moveBottom(mField.height());
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

void DkPongPlayer::setField(const QRect& field) {
	mField = field;
	mRect.setHeight(qRound(field.height()*0.3));
}

void DkPongPlayer::increaseScore() {
	mScore++;
}

int DkPongPlayer::score() const {
	return mScore;
}

// DkPongPort --------------------------------------------------------------------
DkPongPort::DkPongPort(QWidget *parent, Qt::WindowFlags) : QGraphicsView(parent) {

	setAttribute(Qt::WA_TranslucentBackground, true);

	field = rect();
	fieldColor = QColor(0,0,0);
	playerColor = QColor(255, 255, 255);

	unit = 10;
	mPlayerSpeed = qRound(unit*0.5);

	mBall = DkBall(unit, field);
	mPlayer1 = DkPongPlayer(unit);
	mPlayer2 = DkPongPlayer(unit);

	initGame();
	 
	eventLoop = new QTimer(this);
	eventLoop->setInterval(10);
	//eventLoop->start();

	connect(eventLoop, SIGNAL(timeout()), this, SLOT(gameLoop()));
}

void DkPongPort::initGame() {
	
	mBall.reset();
	mPlayer1.reset(QPoint(unit, qRound(height()*0.5f)));
	mPlayer2.reset(QPoint(qRound(width()-unit*1.5f), qRound(height()*0.5f)));

	qDebug() << mPlayer1.score() << ":" << mPlayer2.score();

	update();
}

void DkPongPort::togglePause() {

	if (eventLoop->isActive())
		eventLoop->stop();
	else
		eventLoop->start();
}

void DkPongPort::pauseGame(bool pause) {

	if (pause)
		eventLoop->stop();
	else
		eventLoop->start();
}

void DkPongPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());
	painter.setBackgroundMode(Qt::TransparentMode);

	painter.fillRect(QRect(QPoint(), size()), fieldColor);

	painter.fillRect(mBall.rect(), playerColor);
	painter.fillRect(mPlayer1.rect(), playerColor);
	painter.fillRect(mPlayer2.rect(), playerColor);

	painter.end();

	// propagate
	QGraphicsView::paintEvent(event);
	
}

void DkPongPort::resizeEvent(QResizeEvent *event) {

	//resize(event->size());

	field = QRect(QPoint(), event->size());
	mPlayer1.setField(field);
	mPlayer2.setField(field);
	mBall.setField(field);

	initGame();

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
DkBall::DkBall(int unit, const QRect& field) {

	qsrand(1);
	mUnit = unit;
	mField = field;

	mMinSpeed = qRound(unit*0.5);
	mMaxSpeed = qRound(unit*0.8);

	mRect = QRect(QPoint(), QSize(unit, unit));

	reset();
}

void DkBall::reset() {
	
	mDirection = DkVector(3, 0);// DkVector(mUnit*0.15f, mUnit*0.15f);
	mRect.moveCenter(QPoint(qRound(mField.width()*0.5f), qRound(mField.height()*0.5f)));
}

QRect DkBall::rect() const {
	return mRect;
}

QPoint DkBall::direction() const {
	return mDirection.getQPointF().toPoint();
}

bool DkBall::move(DkPongPlayer& player1, DkPongPlayer& player2) {

	// collision detection top & bottom
	if (mRect.top() <= mField.top() && mDirection.y < 0 || mRect.bottom() >= mField.bottom() && mDirection.y > 0) {
		mDirection.rotate(mDirection.angle()*2);
		//qDebug() << "collision...";
	}

	double nAngle = mDirection.angle() + CV_PI*0.5;//DkMath::normAngleRad(mDirection.angle()+CV_PI*0.5, 0, CV_PI*0.5);
	double magic = (double)qrand() / RAND_MAX * 0.1 - 0.05;

	// player collision
	if (player1.rect().intersects(mRect) && mDirection.x < 0) {

		mDirection.rotate((nAngle*2)+magic);
		double mod = (player1.pos() != INT_MAX) ? (player1.rect().center().y() - player1.pos())/(float)mField.height() : 0;
		mDirection.y += (float)mod*mUnit;
	}
	else if (player2.rect().intersects(mRect) && mDirection.x > 0) {

		mDirection.rotate((nAngle*2)+magic);
		double mod = (player2.pos() != INT_MAX) ? (player2.rect().center().y() - player2.pos())/(float)mField.height() : 0;
		mDirection.y += (float)mod*mUnit;
	}
	// collision detection left & right
	else if (mRect.left() <= mField.left()) {
		//ballDir.rotate(-ballDir.angle()*2);
		player2.increaseScore();
		return false;
	}
	else if (mRect.right() >= mField.right()) {
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

void DkBall::setField(const QRect& field) {
	mField = field;
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
