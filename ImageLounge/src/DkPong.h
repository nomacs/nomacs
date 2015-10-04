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

#pragma once

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QMainWindow>
#include <QGraphicsView>
#include <QRect>
#pragma warning(pop)		// no warnings from includes - end

#include "DkMath.h"

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

namespace nmc {

class DllExport DkPongPlayer {
	
public:
	DkPongPlayer(int unit = 1, const QRect& field = QRect());

	void reset(const QPoint& pos);
	QRect rect() const;
	int pos() const;
	void setHeight(int newHeight);

	void move();
	void setSpeed(int speed);

	void setField(const QRect& field);
	void increaseScore();
	int score() const;

protected:
	int mUnit;
	int mSpeed;
	int mVelocity;

	int mScore = 0;
	int mPos = INT_MAX;

	QRect mRect;
	QRect mField;
};

class DllExport DkBall {

public:
	DkBall(int unit = 1, const QRect& field = QRect());

	void reset();

	QRect rect() const;
	QPoint direction() const;
	void setField(const QRect& field);

	bool move(DkPongPlayer& player1, DkPongPlayer& player2);

protected:
	int mMinSpeed;
	int mMaxSpeed;

	int mUnit;

	DkVector mDirection;
	QRect mRect;

	QRect mField;

	void fixAngle();
};

class DllExport DkPongPort : public QGraphicsView {
	Q_OBJECT

public:
	DkPongPort(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkPongPort() {};

public slots:
	void gameLoop();
	
protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);

	void initGame();
	void togglePause();
	void pauseGame(bool pause = true);

private:
	QTimer *eventLoop;
	int unit;

	QRect field;

	QColor fieldColor;
	QColor playerColor;

	int mPlayerSpeed;

	DkBall mBall;
	DkPongPlayer mPlayer1;
	DkPongPlayer mPlayer2;
};

class DllExport DkPong : public QMainWindow {
	Q_OBJECT

public:
	DkPong(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkPong() {};

protected slots:
	void keyPressEvent(QKeyEvent *event);
	
};


};
