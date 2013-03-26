 /*******************************************************************************************************
 DkViewPort.cpp
 Created on:	05.05.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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


namespace nmc {

// DkPongPort --------------------------------------------------------------------
DkPongPort::DkPongPort(QWidget *parent, Qt::WindowFlags flags) : QGraphicsView(parent) {

	//setAttribute(Qt::WA_TranslucentBackground, true);

	fieldColor = QColor(0,0,0);
	playerColor = QColor(255, 255, 255);

	unit = 10;
	playerSpeed = unit;
	minBallSpeed = 0.5*unit;
	maxBallSpeed = 4*unit;
	ballDir = DkVector(unit*0.15, unit*0.15);
	player1Speed = 0;
	player2Speed = 0;
	player1Pos = INT_MAX;
	player2Pos = INT_MAX;

	ball = QRect(QPoint(), QSize(unit, unit));
	player1 = QRect(QPoint(), QSize(unit, 2*unit));
	player2 = QRect(QPoint(), QSize(unit, 5*unit));

	initGame();
	 
	eventLoop = new QTimer(this);
	eventLoop->setInterval(10);
	eventLoop->start();

	connect(eventLoop, SIGNAL(timeout()), this, SLOT(gameLoop()));

	//show();
}

void DkPongPort::initGame() {
	
	ballDir = DkVector(unit*0.5, unit*0.5);
	ball.moveCenter(QPoint(width()*0.5, height()*0.5));
	player1.moveCenter(QPoint(unit, height()*0.5));
	player2.moveCenter(QPoint(width()-unit*1.5, height()*0.5));

}

void DkPongPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());
	painter.setBackgroundMode(Qt::TransparentMode);

	painter.fillRect(QRect(QPoint(), size()), fieldColor);

	painter.fillRect(ball, playerColor);
	painter.fillRect(player1, playerColor);
	painter.fillRect(player2, playerColor);

	painter.end();

	// propagate
	QGraphicsView::paintEvent(event);
	
}

void DkPongPort::resizeEvent(QResizeEvent *event) {

	//resize(event->size());
	initGame();

	field = QRect(QPoint(), event->size());
	player1.setHeight(field.height()*0.3);
	player2.setHeight(field.height()*0.3);

	QWidget::resizeEvent(event);
	
}

void DkPongPort::gameLoop() {

	// logic first
	moveBall();

	movePlayer(player1, player1Speed);
	movePlayer(player2, player2Speed);

	//repaint();
	viewport()->update();
	
	//QGraphicsView::update();
}


void DkPongPort::moveBall() {


	QPoint newCenter = ball.center() + ballDir.getQPointF().toPoint();
	
	// collision detection
	if (ball.top() <= field.top() && ballDir.y < 0 || ball.bottom() >= field.bottom() && ballDir.y > 0) {
		ballDir.rotate(ballDir.angle()*2);
		qDebug() << "collision...";
	}
	// collision detection
	if (ball.left() <= field.left() || ball.right() >= field.right()) {
		//ballDir.rotate(-ballDir.angle()*2);
		initGame();
		qDebug() << "collision...";
	}

	double maxAngle = CV_PI*0.45;

	// player collition
	if ((player1.contains(ball.topLeft()) || player1.contains(ball.bottomLeft())) && ballDir.x < 0) {
		
		ballDir.rotate(-ballDir.angle()*2);
		double mod = (player1Pos != INT_MAX) ? (player1.center().y() - player1Pos)/(float)field.height() : 0;
		ballDir.y += mod*unit;

		//qDebug() << "ballDir: " << ballDir.angle();

		//if (ballDir.angle() < maxAngle) {

		//	qDebug() << "correcting angle: " << ballDir.angle();
		//	ballDir.rotate(-DkMath::normAngleRad(ballDir.angle())+maxAngle);
		//	qDebug() << "angle corrected: " << ballDir.angle();


		//}
		//else if (ballDir.angle() > CV_PI-maxAngle) {
		//	qDebug() << "correcting angle: " << ballDir.angle();
		//	ballDir.rotate(DkMath::normAngleRad(ballDir.angle())-(CV_PI-maxAngle));
		//	qDebug() << "angle corrected: " << ballDir.angle();
		//}
		
	}
	if ((player2.contains(ball.topRight()) || player2.contains(ball.bottomRight())) && ballDir.x > 0) {
		
		ballDir.rotate(-ballDir.angle()*2);
		
		double mod = (player2Pos != INT_MAX) ? (player2.center().y() - player2Pos)/(float)field.height() : 0;
		ballDir.y += mod*unit;
	}


	//if (abs(ballDir.x) < 1)
	//	ballDir.x = (ballDir.x < 0) ? -unit*0.25 : unit*0.25;

	double normAngle = DkMath::normAngleRad(ballDir.angle(), 0, CV_PI*0.5);
	
	//if (normAngle > maxAngle) {


	//	ballDir.rotate(ballDir.angle() - (normAngle-maxAngle));
	//	qDebug() << "I did correct the angle...";
	//}
		

	if (ballDir.norm() > maxBallSpeed) {
		ballDir.normalize();
		ballDir *= maxBallSpeed;
	}
	else if (ballDir.norm() < minBallSpeed) {
		ballDir.normalize();
		ballDir *= minBallSpeed;
	}

	//qDebug() << ballDir.angle();

	ball.moveCenter(ball.center() + ballDir.getQPointF().toPoint());

}

void DkPongPort::movePlayer(QRect& player, int velocity) {

	if (player.top() + velocity < 0)
		player.moveTop(0);
	else if (player.bottom() + velocity > height())
		player.moveBottom(height());
	else
		player.moveTop(player.top() + velocity);

}

void DkPongPort::keyPressEvent(QKeyEvent *event) {

	if (event->key() == Qt::Key_Up && !event->isAutoRepeat()) {
		player2Speed = -playerSpeed;
		player2Pos = player2.center().y();
	}
	if (event->key() == Qt::Key_Down && !event->isAutoRepeat()) {
		player2Speed = playerSpeed;
		player2Pos = player2.center().y();
	}
	if (event->key() == Qt::Key_W && !event->isAutoRepeat()) {
		player1Speed = -playerSpeed;
		player1Pos = player1.center().y();
	}
	if (event->key() == Qt::Key_S && !event->isAutoRepeat()) {
		player1Speed = playerSpeed;
		player1Pos = player1.center().y();
	}

	QWidget::keyPressEvent(event);
}

void DkPongPort::keyReleaseEvent(QKeyEvent* event) {

	if (event->key() == Qt::Key_Up && !event->isAutoRepeat() || event->key() == Qt::Key_Down && !event->isAutoRepeat()) {
		player2Speed = 0;
		player2Pos = INT_MAX;
	}
	if (event->key() == Qt::Key_W && !event->isAutoRepeat() || event->key() == Qt::Key_S && !event->isAutoRepeat()) {
		player1Speed = 0;
		player1Pos = INT_MAX;
	}

	QWidget::keyReleaseEvent(event);
}



DkPong::DkPong(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {

	resize(800, 600);

	//showFullScreen();
	setWindowFlags(Qt::FramelessWindowHint);
	//setAttribute(Qt::WA_TranslucentBackground, true);
	
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