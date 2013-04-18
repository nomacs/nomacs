/*******************************************************************************************************
 DkTransferToolBar.cpp
 Created on:	13.02.2012
 
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

#include "DkTransferToolBar.h"

#include "DkMath.h"

namespace nmc {

//
DkTransferToolBar::DkTransferToolBar(QWidget * parent) 
	: QToolBar(parent) {

	gradX = 100;
	gradY = 0;
	gradWidth = 1000;
	gradHeight = 20;

	this->setMinimumWidth(400);
	//this->setMinimumHeight(500);

	enableTFCheckBox = new QCheckBox(tr("Enable"));
	enableTFCheckBox->setStatusTip(tr("Disables the pseudocolor function"));
	
	this->addWidget(enableTFCheckBox);

	// >DIR: more compact gui [2.3.2012 markus]
	this->addSeparator();
	//this->addWidget(new QLabel(tr("Active channel:")));

	channelComboBox = new QComboBox(this);
	channelComboBox->setStatusTip(tr("Changes the displayed color channel"));
	this->addWidget(channelComboBox);

	//this->addSeparator();

	gradient = new DkGradient(this);
	gradient->setStatusTip(tr("Click into the field for a new slider"));
	addWidget(gradient);

	//this->addSeparator();

	effect = new QGraphicsOpacityEffect(gradient);
	effect->setOpacity(1);
	gradient->setGraphicsEffect(effect);
		
	createIcons();
		
	pickColorButton = new QPushButton(tr("Color picker"));

	// Disable the entire transfer toolbar:
	//enableTF(Qt::Unchecked);

	// Initialize the combo box for color images:
	imageMode = mode_uninitialized;
	applyImageMode(mode_rgb);
	
	enableToolBar(false);
	enableTFCheckBox->setEnabled(true);	

	connect(enableTFCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableTFCheckBoxClicked(int)));
	connect(gradient, SIGNAL(gradientChanged()), this, SLOT(applyTF()));
				
	// Actions called triggered by toolbar buttons:
	connect(toolBarActions[icon_toolbar_reset], SIGNAL(triggered()), this, SLOT(reset()));
	connect(toolBarActions[toolbar_pipette], SIGNAL(triggered()), this, SLOT(pickColor()));
	
};

#define ICON(theme, backup) QIcon::fromTheme((theme), QIcon((backup)))

void DkTransferToolBar::createIcons() {

	// user needs to decide...
	//this->setIconSize(QSize(16,16));
			
	toolBarIcons.resize(icon_toolbar_end);

	toolBarIcons[icon_toolbar_reset] = ICON("", ":/nomacs/img/gradient-reset.png");
	toolBarIcons[icon_toolbar_pipette] = ICON("", ":/nomacs/img/pipette.png");

	
	toolBarActions.resize(toolbar_end);
	toolBarActions[toolbar_reset] = new QAction(toolBarIcons[icon_toolbar_reset], tr("Reset"), this);
	toolBarActions[toolbar_reset]->setStatusTip(tr("Resets the pseudocolor function"));
	//toolBarActions[toolbar_reset]->setToolTip("was geht?");

	toolBarActions[toolbar_pipette] = new QAction(toolBarIcons[icon_toolbar_pipette], tr("Select Color"), this);
	toolBarActions[toolbar_pipette]->setStatusTip(tr("Adds a slider at the selected color value"));

	addActions(toolBarActions.toList());

}

DkTransferToolBar::~DkTransferToolBar() {


};

void DkTransferToolBar::insertSlider(qreal pos) {

	gradient->insertSlider(pos);

};

void DkTransferToolBar::setImageMode(int mode) {

	applyImageMode(mode);

};

void DkTransferToolBar::applyImageMode(int mode) {

	// At first check if the right mode is already set. If so, don't do nothing.

	if (mode == imageMode)
		return;

	if (imageMode == mode_invalid_format) {
		enableToolBar(true);
		emit channelChanged(0);
	}

	imageMode = mode;
	
	if (imageMode == mode_invalid_format) {
		enableToolBar(false);
		return;
	}
	
	enableTFCheckBox->setEnabled(true);	

	disconnect(channelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeChannel(int)));

	channelComboBox->clear();

	if (mode == mode_gray) {
		channelComboBox->addItem(tr("Gray"));
	}
	else if (mode == mode_rgb) {
		channelComboBox->addItem(tr("RGB"));
		channelComboBox->addItem(tr("Red"));
		channelComboBox->addItem(tr("Green"));
		channelComboBox->addItem(tr("Blue"));
	}

	channelComboBox->setCurrentIndex(0);

	connect(channelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeChannel(int)));

};

void DkTransferToolBar::pickColor() {

	emit pickColorRequest();
	
};

void DkTransferToolBar::enableTFCheckBoxClicked(int state) {

	bool enabled;
	if (state == Qt::Checked)
		enabled = true;
	else
		enabled = false;

	enableToolBar(enabled);

	// At this point the checkbox is disabled, hence enable it...
	enableTFCheckBox->setEnabled(true);

	if (enabled)
		enableTFCheckBox->setStatusTip(tr("Disables the pseudocolor function"));
	else
		enableTFCheckBox->setStatusTip(tr("Enables the pseudocolor function"));

	emit tFEnabled(enabled);

}

void DkTransferToolBar::enableToolBar(bool enable) {

	QObjectList list = this->children();

	for (int i = 0; i < list.count(); i++) {
		if (QWidget *action = qobject_cast<QWidget*>(list.at(i)))
			action->setEnabled(enable);
	}
	
	if (enable)
		effect->setOpacity(1);
	else
		effect->setOpacity(.5);

}


void DkTransferToolBar::applyTF() {

	QGradientStops stops = gradient->getGradientStops();

	emit colorTableChanged(stops);

};

void DkTransferToolBar::changeChannel(int index) {

	emit channelChanged(index);

}

void DkTransferToolBar::reset() {

	gradient->reset();

	QGradientStops stops = gradient->getGradientStops();

	emit colorTableChanged(stops);

}



void DkTransferToolBar::paintEvent(QPaintEvent* event) {

	QToolBar::paintEvent(event);

}

DkGradient::DkGradient(QWidget *parent) 
	: QWidget(parent){


	this->setMinimumWidth(parent->minimumWidth());
	this->setMinimumHeight(40);
	isSliderDragged = false;
	clickAreaHeight = 20;
	deleteSliderDist = 50;
	
	// Note that sliderWidth should be odd, in order to get a pretty rendered slider.
	sliderWidth = 10;
	halfSliderWidth = sliderWidth / 2;
	gradient = QLinearGradient(0, 0, width(), height() - clickAreaHeight);
	
	sliders = QVector<DkColorSlider*>();
	init();

};

DkGradient::~DkGradient() {


};

void DkGradient::init() {

	DkColorSlider *slider;

	isActiveSliderExisting = false;

	for (int i = 0; i < sliders.size(); i++) {
		slider = sliders.at(i);
		delete slider;
	}

	sliders.clear();

	addSlider(0, Qt::black);
	addSlider(1, Qt::white);
	
	updateGradient();


};

void DkGradient::reset() {

	init();
	update();

}


void DkGradient::addSlider(qreal pos, QColor color) {


	DkColorSlider *actSlider =  new DkColorSlider(this, pos, color, sliderWidth);
	sliders.append(actSlider);
	connect(actSlider, SIGNAL(sliderMoved(DkColorSlider*, int, int)), this, SLOT(moveSlider(DkColorSlider*, int, int)));
	connect(actSlider, SIGNAL(colorChanged(DkColorSlider*)), this, SLOT(changeColor(DkColorSlider*)));
	connect(actSlider, SIGNAL(sliderActivated(DkColorSlider*)), this, SLOT(activateSlider(DkColorSlider*)));

}

void DkGradient::insertSlider(qreal pos) {

	// Inserts a new slider at position pos and calculates the color, interpolated from the closest neighbors.

	// Find the neighbors of the new slider, since we need it for the color interpolation:
	QColor leftColor, rightColor, actColor;
	qreal dist;
	qreal initValue = DBL_MAX; //std::numeric_limits<qreal>::max();	// >DIR: fix for linux [9.2.2012 markus]
	qreal leftDist = initValue;
	qreal rightDist = initValue;

	int leftIdx, rightIdx;
	
	for (int i = 0; i < sliders.size(); i++) {
		dist = sliders.at(i)->getNormedPos() - pos;
		if (dist < 0) {
			if (abs(dist) < leftDist) {
				leftDist = (abs(dist));
				leftIdx = i;
			}
		}
		else if (dist > 0){
			if (abs(dist) < rightDist) {
				rightDist = (abs(dist));
				rightIdx = i;
			}
		}
		else {
			actColor = sliders.at(i)->getColor();
			break;
		}
	}

	

	if ((leftDist == initValue) && (rightDist == initValue))
		actColor = Qt::black;
	// The slider is most left:
	else if (leftDist == initValue)
		actColor = sliders.at(rightIdx)->getColor();
	// The slider is most right:
	else if (rightDist == initValue)
		actColor = sliders.at(leftIdx)->getColor();
	// The slider has a neighbor to the left and to the right:
	else {
		int rLeft, rRight, rNew, gLeft, gRight, gNew, bLeft, bRight, bNew;
		
		sliders.at(leftIdx)->getColor().getRgb(&rLeft, &gLeft, &bLeft);
		sliders.at(rightIdx)->getColor().getRgb(&rRight, &gRight, &bRight);
		
		qreal fac = leftDist / (leftDist + rightDist);
		rNew = rLeft * (1 - fac) + rRight * fac;
		gNew = gLeft * (1 - fac) + gRight * fac;
		bNew = bLeft * (1 - fac) + bRight * fac;

		actColor = QColor(rNew, gNew, bNew);

	}


	addSlider(pos, actColor);
	// The last slider in the list is the last one added, now make this one active:
	activateSlider(sliders.last());

	updateGradient();
	update();


}

void DkGradient::mousePressEvent(QMouseEvent *event) {

	QPointF enterPos = event->pos();
	qreal pos = (qreal)(enterPos.x() - halfSliderWidth) / (qreal)(width()-sliderWidth);

	insertSlider(pos);
	
}

void DkGradient::updateGradient() {

	gradient = QLinearGradient(0, 0, width(), height() - clickAreaHeight);

	for (int i = 0; i < sliders.size(); i++) 
		gradient.setColorAt(sliders.at(i)->getNormedPos(), sliders.at(i)->getColor());


}

QGradientStops DkGradient::getGradientStops() {

	return gradient.stops();

};


void DkGradient::moveSlider(DkColorSlider* sender, int dragDistX, int yPos) {


	// Delete the actual slider:
	if (yPos > deleteSliderDist) {
		int idx = sliders.lastIndexOf(sender);
		if (idx != -1) {
			sliders.remove(idx);
			delete sender;
			isActiveSliderExisting = false;
		}
	}

	// Insert a new slider:
	else {

		int newPos = sender->pos().x() + dragDistX;

		if (newPos < 0)
			newPos = 0;
		else if (newPos > width() - sliderWidth - 1)
			newPos = width() - sliderWidth - 1;

		qreal normedSliderPos = getNormedPos(newPos);

		if (normedSliderPos > 1)
			normedSliderPos = 1;
		if (normedSliderPos < 0)
			normedSliderPos = 0;

		DkColorSlider *slider;
		// Check if the position is already assigned to another slider:
		for (int i = 0; i < sliders.size(); i++) {
			slider = sliders.at(i);
			if (slider != sender) {
				if (slider->getNormedPos() == normedSliderPos)
					return;
			}
		}

		sender->setNormedPos(normedSliderPos);
		sender->move(newPos, sender->pos().y());

	}

	updateGradient();
	update();
	
	emit gradientChanged();

}

qreal DkGradient::getNormedPos(int pos) {

	return (qreal)(pos) / (qreal)(width() - sliderWidth);

}

int DkGradient::getAbsolutePos(qreal pos) {

	return (int) pos * width();

}

void DkGradient::paintEvent(QPaintEvent* event) {


	QPainter painter(this);
	painter.setPen(Qt::gray);
	
	painter.fillRect(halfSliderWidth, 2, width() - sliderWidth, height() - clickAreaHeight, gradient);
	painter.drawRect(halfSliderWidth, 2, width() - sliderWidth, height() - clickAreaHeight);
	

};



void DkGradient::mouseReleaseEvent(QMouseEvent *event) {
	
	
	QPointF enterPos = event->pos();

}

void DkGradient::changeColor(DkColorSlider *slider) {

	updateGradient();

	update();

	emit gradientChanged();
}

void DkGradient::activateSlider(DkColorSlider *sender) {

	
	if (isActiveSliderExisting) 
		activeSlider->setActive(false);
	else
		isActiveSliderExisting = true;

	activeSlider = sender;
	activeSlider->setActive(true);

	update();

};


// DkColorSlider:


DkColorSlider::DkColorSlider(QWidget *parent, qreal normedPos, QColor color, int sliderWidth) 
	: QWidget(parent) {

	this->setStatusTip(tr("Drag the slider downwards for elimination"));
	this->normedPos = normedPos;
	this->color = color;
	this->sliderWidth = sliderWidth;
	isActive = false;

	sliderHalfWidth = cvCeil((double)sliderWidth / 2);
	//return (qreal)(pos) / (qreal)(width() - sliderWidth);
	
	int pos =normedPos * (parent->width() - sliderWidth - 1);

	setGeometry(pos, 23, sliderWidth + 1, sliderWidth + sliderHalfWidth + 1);

	show();

};

void DkColorSlider::paintEvent(QPaintEvent* event) {

	QPainter painter(this);

	painter.setPen(Qt::black);

	// Draw the filled triangle at the top of the slider:
	if (isActive) {

		QPainterPath path;
		path.moveTo(0, sliderHalfWidth);
		path.lineTo(sliderHalfWidth, 0);
		path.lineTo(sliderHalfWidth, 0);
		path.lineTo(sliderWidth, sliderHalfWidth);
	
		painter.fillPath(path, Qt::black);
		painter.drawPath(path);

	} 
	// Draw the empty triangle at the top of the slider:
	else {
		painter.drawLine(0, sliderHalfWidth, sliderHalfWidth, 0);
		painter.drawLine(sliderHalfWidth, 0, sliderWidth, sliderHalfWidth);
	}
	
	painter.drawRect(0, sliderHalfWidth, sliderWidth, sliderWidth);
	painter.fillRect(2, sliderHalfWidth+2, sliderWidth - 3, sliderWidth - 3, color);
	
 
}

void DkColorSlider::setActive(bool isActive) {

	this->isActive = isActive;

}

DkColorSlider::~DkColorSlider() {


};

QColor DkColorSlider::getColor() {

	return color;

};

qreal DkColorSlider::getNormedPos() {

	return normedPos;

};

void DkColorSlider::setNormedPos(qreal pos) {

	normedPos = pos;

};


void DkColorSlider::mousePressEvent(QMouseEvent *event) {
	
	isActive = true;
	dragStartX = event->pos().x();
	emit sliderActivated(this);		
}

void DkColorSlider::mouseMoveEvent(QMouseEvent *event) {
	
	// Pass the actual position to the Gradient:
	emit sliderMoved(this, event->pos().x() - dragStartX, event->pos().y());
		
}

void DkColorSlider::mouseDoubleClickEvent(QMouseEvent *event) {

	QColor color = QColorDialog::getColor(this->color, this);
	if (color.isValid())
		this->color = color;

	emit colorChanged(this);

}

}