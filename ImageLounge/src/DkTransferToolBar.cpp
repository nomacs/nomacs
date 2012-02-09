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

	this->setMinimumWidth(500);
	//this->setMinimumHeight(500);

	enableTFCheckBox = new QCheckBox("Enable");
	//enableTFCheckBox->setChecked(true);
	this->addWidget(enableTFCheckBox);

	this->addSeparator();

	this->addWidget(new QLabel("Active channel:"));

	channelComboBox = new QComboBox(this);
	// Initialize the combo box for color images:
	imageMode = mode_uninitialized;
	applyImageMode(mode_rgb);
	channelComboBox->setStatusTip(tr("Changes the displayed color channel"));
	this->addWidget(channelComboBox);

	this->addSeparator();

	gradient = new DkGradient(this);
	gradient->setStatusTip(tr("Click into the field for a new slider"));
	addWidget(gradient);

	this->addSeparator();

	effect = new QGraphicsOpacityEffect(gradient);
	effect->setOpacity(1);
	gradient->setGraphicsEffect(effect);
		
	createIcons();
		
	pickColorButton = new QPushButton("Color picker");

	connect(enableTFCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableTF(int)));
	enableTFCheckBox->setStatusTip(tr("Disables the transfer function"));
	connect(gradient, SIGNAL(gradientChanged()), this, SLOT(applyTF()));
				
	// Actions called triggered by toolbar buttons:
	connect(toolBarActions[icon_toolbar_reset], SIGNAL(triggered()), this, SLOT(reset()));
	connect(toolBarActions[toolbar_pipette], SIGNAL(triggered()), this, SLOT(pickColor()));


	
};

#define ICON(theme, backup) QIcon::fromTheme((theme), QIcon((backup)))

void DkTransferToolBar::createIcons() {

	this->setIconSize(QSize(16,16));
			
	toolBarIcons.resize(icon_toolbar_end);

	toolBarIcons[icon_toolbar_reset] = ICON("document-open-folder", ":/nomacs/img/gradient-reset.png");
	toolBarIcons[icon_toolbar_pipette] = ICON("document-open-folder", ":/nomacs/img/pipette.png");

	
	toolBarActions.resize(toolbar_end);
	toolBarActions[toolbar_reset] = new QAction(toolBarIcons[icon_toolbar_reset], tr("Reset"), this);
	toolBarActions[toolbar_reset]->setStatusTip(tr("Resets the transfer function"));

	toolBarActions[toolbar_pipette] = new QAction(toolBarIcons[icon_toolbar_pipette], tr("Select Color"), this);
	toolBarActions[toolbar_pipette]->setStatusTip(tr("Adds a slider at the selected color value"));

	addActions(toolBarActions.toList());

}

DkTransferToolBar::~DkTransferToolBar() {


};

void DkTransferToolBar::insertSlider(qreal pos) {

	gradient->insertSlider(pos);

};

void DkTransferToolBar::setImageMode(bool isGrayScale) {

	// Note: We convert here from boolean to integer, since there is also a third state (mode_uninitialized), which is used at the program start.
	if (isGrayScale)
		applyImageMode(mode_gray);
	else
		applyImageMode(mode_rgb);

};

void DkTransferToolBar::applyImageMode(int mode) {

	// At first check if the right mode is already set. If so, don't do nothing.

	if (mode == imageMode)
		return;

	imageMode = mode;

	disconnect(channelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeChannel(int)));

	channelComboBox->clear();

	if (mode == mode_gray) {
		channelComboBox->addItem("Gray");
	}
	else if (mode == mode_rgb) {
		channelComboBox->addItem("Red");
		channelComboBox->addItem("Green");
		channelComboBox->addItem("Blue");
		channelComboBox->addItem("RGB");
	}

	channelComboBox->setCurrentIndex(0);

	connect(channelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeChannel(int)));

};

void DkTransferToolBar::pickColor() {

	emit pickColorRequest();
	
};

void DkTransferToolBar::enableTF(int state) {

	bool enabled;
	if (state == Qt::Checked)
		enabled = true;
	else
		enabled = false;


	QObjectList list = this->children();

	for (int i = 0; i < list.count(); i++) {
		if (QWidget *action = qobject_cast<QWidget*>(list.at(i)))
			action->setEnabled(enabled);
	}
	
	if (enabled)
		effect->setOpacity(1);
	else
		effect->setOpacity(.5);


	
	enableTFCheckBox->setEnabled(true);
	if (enabled)
		enableTFCheckBox->setStatusTip(tr("Disables the transfer function"));
	else
		enableTFCheckBox->setStatusTip(tr("Enables the transfer function"));

	emit tFEnabled(enabled);

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