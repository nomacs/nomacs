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

#include "DkToolbars.h"
#include "DkSettings.h"
#include "DkMath.h"
#include "DkUtils.h"
#include "DkImageStorage.h"

namespace nmc {

#define ICON(theme, backup) QIcon::fromTheme((theme), QIcon((backup)))

// DkColorSlider:


DkColorSlider::DkColorSlider(QWidget *parent, qreal normedPos, QColor color, int sliderWidth) 
	: QWidget(parent) {

	this->setStatusTip(tr("Drag the slider downwards for elimination"));
	this->normedPos = normedPos;
	this->color = color;
	this->sliderWidth = sliderWidth;
	isActive = false;

	sliderHalfWidth = cvCeil((float)sliderWidth / 2);
	//return (qreal)(pos) / (qreal)(width() - sliderWidth);
	
	int pos = qRound(normedPos * (parent->width() - sliderWidth - 1));

	setGeometry(pos, 23, sliderWidth + 1, sliderWidth + sliderHalfWidth + 1);

	show();

};

void DkColorSlider::paintEvent(QPaintEvent*) {

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

void DkColorSlider::updatePos(int parentWidth) {

	int pos = qRound(normedPos * (parentWidth - sliderWidth - 1));
	setGeometry(pos, 23, sliderWidth + 1, sliderWidth + sliderHalfWidth + 1);
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

void DkColorSlider::mouseDoubleClickEvent(QMouseEvent*) {

	QColor color = QColorDialog::getColor(this->color, this);
	if (color.isValid())
		this->color = color;

	emit colorChanged(this);

}

DkGradient::DkGradient(QWidget *parent) 
	: QWidget(parent){

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

	this->setMinimumWidth(100);
	this->setMaximumWidth(600);

	this->setFixedHeight(40);

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

	isActiveSliderExisting = false;

	clearAllSliders();

	addSlider(0, Qt::black);
	addSlider(1, Qt::white);
	
	updateGradient();


};

void DkGradient::clearAllSliders() {

	for (int i = 0; i < sliders.size(); i++) {
		DkColorSlider* slider = sliders.at(i);
		delete slider;
	}

	sliders.clear();

}

void DkGradient::setGradient(const QLinearGradient& gradient) {

	reset();
	clearAllSliders();	// reset adds a slider at the start and end

	this->gradient.setStops(gradient.stops());
	
	QVector<QGradientStop> stops = gradient.stops();

	for (int idx = 0; idx < stops.size(); idx++) {
		addSlider(stops.at(idx).first, stops.at(idx).second);
	}

	updateGradient();
	update();
	emit gradientChanged();

}

QLinearGradient DkGradient::getGradient() {

	return gradient;
}

void DkGradient::reset() {

	init();
	update();

}


void DkGradient::resizeEvent( QResizeEvent * event ) {

	if (event->size() == event->oldSize())
		return;

	DkColorSlider *slider;

	for (int i = 0; i < sliders.size(); i++) {
		slider = sliders.at(i);
		slider->updatePos(this->width());
	}

	//qDebug() << "resize gradient: " << event->size();

	updateGradient();

	QWidget::resizeEvent(event);
}


void DkGradient::addSlider(qreal pos, QColor color) {


	DkColorSlider *actSlider =  new DkColorSlider(this, pos, color, sliderWidth);
	sliders.append(actSlider);
	connect(actSlider, SIGNAL(sliderMoved(DkColorSlider*, int, int)), this, SLOT(moveSlider(DkColorSlider*, int, int)));
	connect(actSlider, SIGNAL(colorChanged(DkColorSlider*)), this, SLOT(changeColor(DkColorSlider*)));
	connect(actSlider, SIGNAL(sliderActivated(DkColorSlider*)), this, SLOT(activateSlider(DkColorSlider*)));

}

void DkGradient::insertSlider(qreal pos, QColor col) {

	// Inserts a new slider at position pos and calculates the color, interpolated from the closest neighbors.

	// Find the neighbors of the new slider, since we need it for the color interpolation:
	QColor leftColor, rightColor, actColor;
	qreal dist;
	qreal initValue = DBL_MAX; //std::numeric_limits<qreal>::max();	// >DIR: fix for linux [9.2.2012 markus]
	qreal leftDist = initValue;
	qreal rightDist = initValue;

	int leftIdx = 0, rightIdx = 0;
	
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
		rNew = qRound(rLeft * (1 - fac) + rRight * fac);
		gNew = qRound(gLeft * (1 - fac) + gRight * fac);
		bNew = qRound(bLeft * (1 - fac) + bRight * fac);

		actColor = QColor(rNew, gNew, bNew);

	}


	addSlider(pos, col.isValid() ? col : actColor);
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

void DkGradient::paintEvent(QPaintEvent*) {

	QPainter painter(this);
	painter.setPen(Qt::gray);
	
	painter.fillRect(halfSliderWidth, 2, width() - sliderWidth, height() - clickAreaHeight, gradient);
	painter.drawRect(halfSliderWidth, 2, width() - sliderWidth, height() - clickAreaHeight);
};



void DkGradient::mouseReleaseEvent(QMouseEvent *event) {
	
	
	QPointF enterPos = event->pos();

}

void DkGradient::changeColor(DkColorSlider*) {

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

//
DkTransferToolBar::DkTransferToolBar(QWidget * parent) 
	: QToolBar(tr("Pseudo Color Toolbar"), parent) {

	loadSettings();

	
	enableTFCheckBox = new QCheckBox(tr("Enable"));
	enableTFCheckBox->setStatusTip(tr("Enables the Pseudo Color function"));

	this->addWidget(enableTFCheckBox);

	// >DIR: more compact gui [2.3.2012 markus]
	this->addSeparator();
	//this->addWidget(new QLabel(tr("Active channel:")));

	channelComboBox = new QComboBox(this);
	channelComboBox->setStatusTip(tr("Changes the displayed color channel"));
	this->addWidget(channelComboBox);

	historyCombo = new QComboBox(this);

	QAction* delGradientAction = new QAction("Delete", historyCombo);
	connect(delGradientAction, SIGNAL(triggered()), this, SLOT(deleteGradient()));

	historyCombo->addAction(delGradientAction);
	historyCombo->setContextMenuPolicy(Qt::ActionsContextMenu);

	updateGradientHistory();
	connect(historyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(switchGradient(int)));
	connect(historyCombo, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(deleteGradientMenu(QPoint)));

	this->addWidget(historyCombo);

	createIcons();

	gradient = new DkGradient(this);
	gradient->setStatusTip(tr("Click into the field for a new slider"));
	this->addWidget(gradient);

	effect = new QGraphicsOpacityEffect(gradient);
	effect->setOpacity(1);
	gradient->setGraphicsEffect(effect);

	// Disable the entire transfer toolbar:
	//enableTF(Qt::Unchecked);

	// Initialize the combo box for color images:
	imageMode = mode_uninitialized;
	applyImageMode(mode_rgb);

	enableToolBar(false);
	enableTFCheckBox->setEnabled(true);	

	connect(enableTFCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableTFCheckBoxClicked(int)));
	connect(gradient, SIGNAL(gradientChanged()), this, SLOT(applyTF()));

	// needed for initialization
	connect(this, SIGNAL(gradientChanged()), gradient, SIGNAL(gradientChanged()));

	if (!oldGradients.empty())
		gradient->setGradient(oldGradients.first());

};

DkTransferToolBar::~DkTransferToolBar() {

	//saveSettings();
};


void DkTransferToolBar::createIcons() {

	// user needs to decide...
	//this->setIconSize(QSize(16,16));
			
	toolBarIcons.resize(icon_toolbar_end);

	toolBarIcons[icon_toolbar_reset] = ICON("", ":/nomacs/img/gradient-reset.png");
	toolBarIcons[icon_toolbar_pipette] = ICON("", ":/nomacs/img/pipette.png");
	toolBarIcons[icon_toolbar_save] = ICON("", ":/nomacs/img/save.png");

	if (!DkSettings::display.defaultIconColor || DkSettings::app.privateMode) {
		// now colorize the icons
		toolBarIcons[icon_toolbar_reset].addPixmap(DkImage::colorizePixmap(toolBarIcons[icon_toolbar_reset].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		toolBarIcons[icon_toolbar_pipette].addPixmap(DkImage::colorizePixmap(toolBarIcons[icon_toolbar_pipette].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		toolBarIcons[icon_toolbar_save].addPixmap(DkImage::colorizePixmap(toolBarIcons[icon_toolbar_save].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
	}
	
	toolBarActions.resize(toolbar_end);
	toolBarActions[toolbar_reset] = new QAction(toolBarIcons[icon_toolbar_reset], tr("Reset"), this);
	toolBarActions[toolbar_reset]->setStatusTip(tr("Resets the Pseudo Color function"));
	connect(toolBarActions[toolbar_reset], SIGNAL(triggered()), this, SLOT(resetGradient()));

	//toolBarActions[toolbar_reset]->setToolTip("was geht?");

	toolBarActions[toolbar_pipette] = new QAction(toolBarIcons[icon_toolbar_pipette], tr("Select Color"), this);
	toolBarActions[toolbar_pipette]->setStatusTip(tr("Adds a slider at the selected color value"));
	toolBarActions[toolbar_pipette]->setCheckable(true);
	toolBarActions[toolbar_pipette]->setChecked(false);
	connect(toolBarActions[toolbar_pipette], SIGNAL(triggered(bool)), this, SLOT(pickColor(bool)));

	toolBarActions[toolbar_save] = new QAction(toolBarIcons[icon_toolbar_save], tr("Save Gradient"), this);
	toolBarActions[toolbar_save]->setStatusTip(tr("Saves the current Gradient"));
	connect(toolBarActions[toolbar_save], SIGNAL(triggered()), this, SLOT(saveGradient()));

	addActions(toolBarActions.toList());

}

void DkTransferToolBar::saveSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("Pseudo Color");

	settings.beginWriteArray("oldGradients", oldGradients.size());

	for (int idx = 0; idx < oldGradients.size(); idx++) {
		settings.setArrayIndex(idx);

		QVector<QGradientStop> stops = oldGradients.at(idx).stops();
		settings.beginWriteArray("gradient", stops.size());

		for (int sIdx = 0; sIdx < stops.size(); sIdx++) {
			settings.setArrayIndex(sIdx);
			settings.setValue("posRGBA", (float)stops.at(sIdx).first);
			settings.setValue("colorRGBA", stops.at(sIdx).second.rgba());
		}
		settings.endArray();
	}

	settings.endArray();
	settings.endGroup();
}

void DkTransferToolBar::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("Pseudo Color");

	int gSize = settings.beginReadArray("oldGradients");

	for (int idx = 0; idx < gSize; idx++) {
		settings.setArrayIndex(idx);

		QVector<QGradientStop> stops;
		int sSize = settings.beginReadArray("gradient");

		for (int sIdx = 0; sIdx < sSize; sIdx++) {
			settings.setArrayIndex(sIdx);
			
			QGradientStop s;
			s.first = settings.value("posRGBA", 0).toFloat();
			s.second = QColor::fromRgba(settings.value("colorRGBA", QColor().rgba()).toInt());
			qDebug() << "pos: " << s.first << " col: " << s.second;
			stops.append(s);
		}
		settings.endArray();

		QLinearGradient g;
		g.setStops(stops);
		oldGradients.append(g);
	}

	settings.endArray();
	settings.endGroup();
}

void DkTransferToolBar::deleteGradientMenu(QPoint pos) {

	QMenu* cm = new QMenu(this);
	QAction* delAction = new QAction("Delete", this);
	connect(delAction, SIGNAL(triggered()), this, SLOT(deleteGradient()));
	cm->popup(historyCombo->mapToGlobal(pos));
	cm->exec();
}

void DkTransferToolBar::deleteGradient() {

	int idx = historyCombo->currentIndex();

	if (idx >= 0 && idx < oldGradients.size()) {
		oldGradients.remove(idx);
		historyCombo->removeItem(idx);
	}

}

void DkTransferToolBar::resizeEvent( QResizeEvent * event ) {

	gradient->resize(event->size().width() - gradient->x(), 40);

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

void DkTransferToolBar::pickColor(bool enabled) {

	emit pickColorRequest(enabled);
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
		enableTFCheckBox->setStatusTip(tr("Disables the Pseudo Color function"));
	else
		enableTFCheckBox->setStatusTip(tr("Enables the Pseudo Color function"));

	emit tFEnabled(enabled);
	emit gradientChanged();
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

void DkTransferToolBar::resetGradient() {

	gradient->reset();

	QGradientStops stops = gradient->getGradientStops();

	emit colorTableChanged(stops);
}

void DkTransferToolBar::paintEvent(QPaintEvent* event) {

	QToolBar::paintEvent(event);

}

void DkTransferToolBar::updateGradientHistory() {

	historyCombo->clear();
	historyCombo->setIconSize(QSize(50,10));

	for (int idx = 0; idx < oldGradients.size(); idx++) {

		QPixmap cg(50, 10);
		QLinearGradient g(QPoint(0,0), QPoint(50, 0));
		g.setStops(oldGradients[idx].stops());
		QPainter p(&cg);
		p.fillRect(cg.rect(), g);
		historyCombo->addItem(cg, tr(""));
	}
}

void DkTransferToolBar::switchGradient(int idx) {

	if (idx >= 0 && idx < oldGradients.size()) {
		gradient->setGradient(oldGradients[idx]);
	}

}

void DkTransferToolBar::saveGradient() {
	
	oldGradients.prepend(gradient->getGradient());
	updateGradientHistory();
	saveSettings();
}

// DkCropToolbar --------------------------------------------------------------------
DkCropToolBar::DkCropToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

	createIcons();
	createLayout();
	QMetaObject::connectSlotsByName(this);

	if (DkSettings::display.smallIcons)
		setIconSize(QSize(16, 16));
	else
		setIconSize(QSize(32, 32));

	if (DkSettings::display.toolbarGradient) 
		setObjectName("toolBarWithGradient");
	else
		setStyleSheet("QToolBar{spacing: 3px; padding: 3px;}");
}

DkCropToolBar::~DkCropToolBar() {
	saveSettings();
}

void DkCropToolBar::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("Crop");

	horValBox->setValue(settings.value("AspectRatioHorizontal", 0).toInt());
	verValBox->setValue(settings.value("AspectRatioVertical", 0).toInt());
	guideBox->setCurrentIndex(settings.value("guides", 0).toInt());
	invertAction->setChecked(settings.value("inverted", false).toBool());
	infoAction->setChecked(settings.value("info", true).toBool());
	settings.endGroup();
}

void DkCropToolBar::saveSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("Crop");

	settings.setValue("AspectRatioHorizontal", horValBox->value());
	settings.setValue("AspectRatioVertical", verValBox->value());
	settings.setValue("guides", guideBox->currentIndex());
	settings.setValue("inverted", invertAction->isChecked());
	settings.setValue("info", infoAction->isChecked());
	settings.endGroup();
}

void DkCropToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[crop_icon] = QIcon(":/nomacs/img/crop.png");
	icons[cancel_icon] = QIcon(":/nomacs/img/cancel.png");
	icons[pan_icon] = 	QIcon(":/nomacs/img/pan.png");
	icons[pan_icon].addPixmap(QPixmap(":/nomacs/img/pan_checked.png"), QIcon::Normal, QIcon::On);
	icons[invert_icon] = QIcon(":/nomacs/img/crop-invert.png");
	icons[invert_icon].addPixmap(QPixmap(":/nomacs/img/crop-invert-checked.png"), QIcon::Normal, QIcon::On);
	icons[info_icon] = QIcon(":/nomacs/img/info.png");

	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::display.iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		}
	}
}

void DkCropToolBar::createLayout() {

	QList<QKeySequence> enterSc;
	enterSc.append(QKeySequence(Qt::Key_Enter));
	enterSc.append(QKeySequence(Qt::Key_Return));

	QAction* cropAction = new QAction(icons[crop_icon], tr("Crop (ENTER)"), this);
	cropAction->setShortcuts(enterSc);
	cropAction->setObjectName("cropAction");

	QAction* cancelAction = new QAction(icons[cancel_icon], tr("Cancel (ESC)"), this);
	cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
	cancelAction->setObjectName("cancelAction");

	panAction = new QAction(icons[pan_icon], tr("Pan"), this);
	panAction->setShortcut(QKeySequence(Qt::Key_P));
	panAction->setObjectName("panAction");
	panAction->setCheckable(true);
	panAction->setChecked(false);

	QStringList ratios;
	ratios << "1:1" << "4:3" << "5:4" << "14:10" << "14:11" << "16:9" << "16:10";
	ratios.prepend(tr("User Defined"));
	ratios.prepend(tr("No Aspect Ratio"));
	ratioBox = new QComboBox(this);
	ratioBox->addItems(ratios);
	ratioBox->setObjectName("ratioBox");

	horValBox = new QDoubleSpinBox(this);
	horValBox->setObjectName("horValBox");
	horValBox->setSpecialValueText("  ");
	horValBox->setToolTip(tr("Horizontal Constraint"));
	horValBox->setStatusTip(horValBox->toolTip());

	QAction* swapAction = new QAction(QIcon(":/nomacs/img/swap.png"), tr("Swap"), this);
	swapAction->setObjectName("swapAction");
	swapAction->setToolTip(tr("Swap Dimensions"));
	swapAction->setStatusTip(swapAction->toolTip());

	verValBox = new QDoubleSpinBox(this);
	verValBox->setObjectName("verValBox");
	verValBox->setSpecialValueText("  ");
	horValBox->setToolTip(tr("Vertical Constraint"));
	horValBox->setStatusTip(horValBox->toolTip());

	angleBox = new QDoubleSpinBox(this);
	angleBox->setObjectName("angleBox");
	angleBox->setSuffix("°");
	angleBox->setMinimum(-180);
	angleBox->setMaximum(180);

	// background color
	bgCol = QColor(0,0,0,0);
	bgColButton = new QPushButton(this);
	bgColButton->setObjectName("bgColButton");
	bgColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(bgCol) + "; border: 1px solid #888;}");
	bgColButton->setToolTip(tr("Background Color"));
	bgColButton->setStatusTip(bgColButton->toolTip());

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");
	colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);

	// crop customization
	QStringList guides;
	guides << tr("Guides") << tr("Rule of Thirds") << tr("Grid");
	guideBox = new QComboBox(this);
	guideBox->addItems(guides);
	guideBox->setObjectName("guideBox");
	guideBox->setToolTip(tr("Show Guides in the Preview"));
	guideBox->setStatusTip(guideBox->toolTip());

	invertAction = new QAction(icons[invert_icon], tr("Invert Crop Tool Color"), this);
	invertAction->setObjectName("invertAction");
	invertAction->setCheckable(true);
	invertAction->setChecked(false);

	infoAction = new QAction(icons[info_icon], tr("Show Info"), this);
	infoAction->setObjectName("infoAction");
	infoAction->setCheckable(true);
	infoAction->setChecked(false);

	addAction(cropAction);
	addAction(panAction);
	addAction(cancelAction);
	addSeparator();
	addWidget(ratioBox);
	addWidget(horValBox);
	addAction(swapAction);
	addWidget(verValBox);
	addWidget(angleBox);
	addSeparator();
	addWidget(bgColButton);
	addSeparator();
	addWidget(guideBox);
	addAction(invertAction);
	addAction(infoAction);
}

void DkCropToolBar::setVisible(bool visible) {

	if (!visible)
		emit colorSignal(Qt::NoBrush);
	else
		emit colorSignal(bgCol);

	if (visible) {
		panAction->setChecked(false);
		angleBox->setValue(0);
	}

	QToolBar::setVisible(visible);
}

void DkCropToolBar::setAspectRatio(const QPointF& aRatio) {

	horValBox->setValue(aRatio.x());
	verValBox->setValue(aRatio.y());
}

void DkCropToolBar::on_cropAction_triggered() {
	emit cropSignal();
}

void DkCropToolBar::on_cancelAction_triggered() {
	emit cancelSignal();
}

void DkCropToolBar::on_invertAction_toggled(bool checked) {
	emit shadingHint(checked);
}

void DkCropToolBar::on_infoAction_toggled(bool checked) {
	emit showInfo(checked);
}

void DkCropToolBar::on_swapAction_triggered() {

	int tmpV = qRound(horValBox->value());
	horValBox->setValue(verValBox->value());
	verValBox->setValue(tmpV);

}

void DkCropToolBar::on_angleBox_valueChanged(double val) {

	emit angleSignal(DK_DEG2RAD*val);
}

void DkCropToolBar::angleChanged(double val) {
	
	qDebug() << val*DK_RAD2DEG;

	double angle = val*DK_RAD2DEG;
	while (angle > 90)
		angle -= 180;
	while (angle <= -90)
		angle += 180;

	angleBox->blockSignals(true);
	angleBox->setValue(angle);
	angleBox->blockSignals(false);
}

void DkCropToolBar::on_bgColButton_clicked() {

	QColor tmpCol = bgCol;
	if (!tmpCol.alpha()) tmpCol.setAlpha(255);	// avoid frustrated users
	
	colorDialog->setCurrentColor(tmpCol);
	int ok = colorDialog->exec();

	if (ok == QDialog::Accepted) {
		bgCol = colorDialog->currentColor();
		bgColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(bgCol) + "; border: 1px solid #888;}");
		emit colorSignal(bgCol);
	}

}


void DkCropToolBar::on_ratioBox_currentIndexChanged(const QString& text) {

	// user defined -> do nothing
	if (ratioBox->currentIndex() == 1)
		return;	

	// no aspect ratio -> clear boxes
	if (ratioBox->currentIndex() == 0) {
		horValBox->setValue(0);
		verValBox->setValue(0);
		return;
	}

	QStringList vals = text.split(":");

	qDebug() << vals;

	if (vals.size() == 2) {
		horValBox->setValue(vals[0].toDouble());
		verValBox->setValue(vals[1].toDouble());
	}
}

void DkCropToolBar::on_guideBox_currentIndexChanged(int idx) {

	emit paintHint(idx);
}

void DkCropToolBar::on_verValBox_valueChanged(double val) {

	// just pass it on
	on_horValBox_valueChanged(val);
}

void DkCropToolBar::on_horValBox_valueChanged(double) {

	DkVector diag = DkVector((float)horValBox->value(), (float)verValBox->value());
	emit aspectRatio(diag);

	QString rs = QString::number(horValBox->value()) + ":" + QString::number(verValBox->value());

	int idx = ratioBox->findText(rs);

	if (idx != -1)
		ratioBox->setCurrentIndex(idx);
	else if (horValBox->value() == 0 && verValBox->value() == 0)
		ratioBox->setCurrentIndex(0);
	else
		ratioBox->setCurrentIndex(1);	

}

void DkCropToolBar::on_panAction_toggled(bool checked) {

	emit panSignal(checked);
}

}
