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

#include <QtGui/QMainWindow>
#include <QtGui/QGraphicsView>
#include <QTimer>
#include <QDebug>
#include <QVector2D>
#include <QKeyEvent>

#include "DkMath.h"

namespace nmc {

class DkPongPort : public QGraphicsView {
	Q_OBJECT

public:
	DkPongPort(QWidget *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkPongPort() {};

public slots:
	void gameLoop();
	
protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);

	void moveBall();
	void initGame();
	void movePlayer(QRect& player, int velocity);

private:
	QTimer *eventLoop;
	int unit;
	int playerSpeed;
	int minBallSpeed;
	int maxBallSpeed;
	
	DkVector ballDir;
	int player1Speed;
	int player2Speed;

	int player1Pos;
	int player2Pos;

	QRect ball;
	QRect player1;
	QRect player2;

	QRect field;

	QColor fieldColor;
	QColor playerColor;
};


class DkPong : public QMainWindow {
	Q_OBJECT

public:
	DkPong(QWidget *parent = 0, Qt::WFlags flags = 0);
	virtual ~DkPong() {};

protected slots:
	void keyPressEvent(QKeyEvent *event);
	
};


};
