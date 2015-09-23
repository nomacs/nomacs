/*******************************************************************************************************
 DkFotojiffy.cpp
 Created on:	14.09.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include "DkFotojiffy.h"
#include "DkImage.h"
#include "DkWidgets.h"

namespace nmc {


// DkSocialConfirmDialog --------------------------------------------------------------------
DkSocialConfirmDialog::DkSocialConfirmDialog(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : DkWidget(parent, flags) {

	//setAttribute(Qt::WA_MouseTracking);
	setMouseTracking(true);
	qDebug() << "confirm attrs: " << this->testAttribute(Qt::WA_TransparentForMouseEvents);

	createLayout();
	QMetaObject::connectSlotsByName(this);
	confirmToggled(true);
}

void DkSocialConfirmDialog::createLayout() {

	QIcon icon = QIcon(":/nomacs/img/checkbox-on.png");
	icon.addPixmap(QPixmap(":/nomacs/img/checkbox-on.png"), QIcon::Normal, QIcon::On);
	icon.addPixmap(QPixmap(":/nomacs/img/checkbox-off.png"), QIcon::Normal, QIcon::Off);
	checkBox = new QPushButton(icon, tr(""), this);
	checkBox->setIcon(icon);
	checkBox->setStyleSheet("QPushButton{background-color: argb(0,0,0,0); margin: 10,0,0,0;}");
	checkBox->setIconSize(QSize(52,52));
	checkBox->setCheckable(true);
	checkBox->setChecked(true);
	checkBox->setFlat(true);
	connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(confirmToggled(bool)));

	okButton = new QPushButton(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_confirm_ok], this);
	//okButton->setFlat(true);
	okButton->setObjectName("okButton");
	cancelButton	= new QPushButton(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_confirm_cancel], this);
	cancelButton->setObjectName("cancelButton");
	okButton->setStyleSheet(QString("QPushButton{background-color: #71c9c2; color: white; font: bold 14px;") +
		QString("border: 0px; width: 130px; padding: 10px 0px 10px 0px; margin-bottom: 30px; margin-right: 30px}") +
		QString("QPushButton::disabled{background-color: #466160;}"));
	cancelButton->setStyleSheet(QString("QPushButton{background-color: #71c9c2; color: white; font: bold 14px;") +
		QString("border: 0px; width: 130px; padding: 10px 0px 10px 0px; margin-bottom: 30px; margin-left: 30px;}") +
		QString("QPushButton::disabled{background-color: #466160;}"));
	//cancelButton->setFlat(true);

	QWidget* buttonWidget = new QWidget(this);
	QHBoxLayout* hLayout = new QHBoxLayout(buttonWidget);
	hLayout->addWidget(cancelButton);
	hLayout->addWidget(okButton);
	buttonWidget->adjustSize();

	infoText = new QLabel(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_confirm_text], this);
	infoText->setWordWrap(true);
	infoText->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	infoText->setStyleSheet("QLabel{color: white; font: italic 12px; margin: 0 30 10 30;} QLabel::disabled{color: #d64949;}");

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	layout->addWidget(checkBox);
	layout->addWidget(infoText);
	layout->addWidget(buttonWidget);


	setObjectName("DkConfirmDialog");
	//setGeometry(100, 100, 400, 400);
	//move(100,100);
	setStyleSheet("DkConfirmDialog{padding: 30px;}");

	adjustSize();

}

void DkSocialConfirmDialog::paintEvent(QPaintEvent *event) {

	if (static_cast<QWidget* >(parent())) {

		QWidget* p = static_cast<QWidget* >(parent());
		QPoint topLeft(p->size().width()-30-width(), p->size().height()-120-height());

		if (geometry().topLeft() != topLeft)
			move(topLeft);		// this is hot shit (we are in paintEvent)
	}

	QRect bgRect = QRect(QPoint(), QSize(width(), height()-15));
	QPoint pc(bgRect.width()-32, bgRect.height()+15);

	QPolygon p;
	p.append(QPoint(pc.x()-15, bgRect.bottom()+1));
	p.append(QPoint(pc.x()+15, bgRect.bottom()+1));
	p.append(pc);
	p.append(QPoint(pc.x()-15, bgRect.bottom()+1));

	QPainter painter(this);

	// painting
	painter.setPen(Qt::NoPen);
	painter.setBrush(DkSettings::display.bgColorWidget);
	painter.drawRect(bgRect);
	painter.drawPolygon(p);
	// end painting

	painter.end();
	DkWidget::paintEvent(event);
}

void DkSocialConfirmDialog::show() {

	checkBox->setChecked(true);
	DkWidget::show();
}

void DkSocialConfirmDialog::confirmToggled(bool checked) {

	okButton->setEnabled(checked);
	infoText->setEnabled(checked);

	adjustSize();
}

void DkSocialConfirmDialog::on_okButton_clicked() {

	emit saveImageSignal(DkSettings::fotojiffy.facebookPath);
	hide();
}

void DkSocialConfirmDialog::on_cancelButton_clicked() {
	hide();
}

void DkSocialConfirmDialog::toggleShow() {

	if (isVisible())
		hide();
	else 
		show();
}



// DkQrCodeConfirmDialog --------------------------------------------------------------------
DkQrCodeConfirmDialog::DkQrCodeConfirmDialog(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : DkWidget(parent, flags) {

	//setAttribute(Qt::WA_MouseTracking);
	setMouseTracking(true);
	qDebug() << "confirm attrs: " << this->testAttribute(Qt::WA_TransparentForMouseEvents);

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkQrCodeConfirmDialog::createLayout() {

	//setStyleSheet("QWidget{background-color: QColor(0,0,0,20); border: 1px solid #000000;}");

	// context menu
	QAction* changeImage = new QAction(tr("Change Image"), this);
	connect(changeImage, SIGNAL(triggered()), this, SLOT(changeImage()));

	cm = new QMenu(tr(""), this);
	cm->addAction(changeImage);

	imgLabel = new QLabel(this);
	imgLabel->setPixmap(DkSettings::fotojiffy.qrCodeConfirmImageUrl);
	imgLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	imgLabel->setStyleSheet("QLabel{margin-top: 0px;}");

	infoText = new QLabel(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_qrconfirm_info], this);
	infoText->setWordWrap(true);
	infoText->setFixedWidth(180);
	infoText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	infoText->setStyleSheet("QLabel{color: #8a8a8a; font: 12px; margin-left: 10px; margin-top: 0px;} QLabel::disabled{color: #d64949;}");

	urlText = new QLabel(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_qrconfirm_url], this);
	urlText->setWordWrap(true);
	urlText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	urlText->setStyleSheet("QLabel{color: white; font: bold italic 12px; margin-left: 0px; margin-bottom: 10px; margin-top: 5px;} QLabel::disabled{color: #d64949;}");

	QWidget* hWidget = new QWidget(this);
	QHBoxLayout* hBox = new QHBoxLayout(hWidget);
	hBox->setMargin(0);
	hBox->setSpacing(0);
	hBox->setContentsMargins(0,0,0,0);
	hBox->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	hBox->addWidget(imgLabel);
	hBox->addWidget(infoText);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	layout->setMargin(20);
	layout->addWidget(hWidget);
	layout->addWidget(urlText);

	setObjectName("DkQrCodeConfirmDialog");
	//setGeometry(100, 100, 400, 400);
	//move(100,100);
	setStyleSheet("DkConfirmDialog{padding: 30px;}");

	adjustSize();

}

void DkQrCodeConfirmDialog::contextMenuEvent(QContextMenuEvent *event) {

	DkWidget::contextMenuEvent(event);

	if (!event->isAccepted())
		cm->exec(event->globalPos());

	event->accept();

}

void DkQrCodeConfirmDialog::changeImage() {

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
		"C:\\", 
		DkSettings::openFilters.join(";;"));

	// check if we can load it
	QPixmap testP(fileName);
	if (testP.isNull())
		return;

	DkSettings::fotojiffy.qrCodeConfirmImageUrl = fileName;
	imgLabel->setPixmap(fileName);

	DkSettings::save();
}

void DkQrCodeConfirmDialog::toggleShow() {

	if (isVisible())
		hide();
	else 
		show();
}

void DkQrCodeConfirmDialog::paintEvent(QPaintEvent *event) {

	if (static_cast<QWidget* >(parent())) {

		QWidget* p = static_cast<QWidget* >(parent());
		QPoint topRight(30, p->size().height()-120-height());

		if (geometry().topLeft() != topRight)
			move(topRight);		// this is hot shit (we are in paintEvent)
	}

	QRect bgRect = QRect(QPoint(), QSize(width(), height()-15));
	QPoint pc(32, bgRect.height()+15);

	QPolygon p;
	p.append(QPoint(pc.x()-15, bgRect.bottom()+1));
	p.append(QPoint(pc.x()+15, bgRect.bottom()+1));
	p.append(pc);
	p.append(QPoint(pc.x()-15, bgRect.bottom()+1));

	QPainter painter(this);

	// painting
	painter.setPen(Qt::NoPen);
	painter.setBrush(DkSettings::display.bgColorWidget);
	painter.drawRect(bgRect);
	painter.drawPolygon(p);
	// end painting

	painter.end();
	DkWidget::paintEvent(event);
}

// DkSocialButton --------------------------------------------------------------------
DkSocialButton::DkSocialButton(int mode, QWidget* parent, Qt::WFlags flags) : QLabel(parent, flags) {

	this->mode = mode;
	init();
}

void DkSocialButton::init() {

	setMouseTracking(true);

	showing = false;
	hiding = false;
	displaySettingsBits = 0;
	opacityEffect = 0;

	// painter problems if the widget is a child of another that has the same graphicseffect
	// widget starts on hide
	opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0);
	opacityEffect->setEnabled(false);
	setGraphicsEffect(opacityEffect);

	setVisible(false);

	// context menu
	QAction* changeImage = new QAction(tr("Change Image"), this);
	connect(changeImage, SIGNAL(triggered()), this, SLOT(changeImage()));

	cm = new QMenu(tr(""), this);
	cm->addAction(changeImage);

	if (mode == facebook) {
		setPixmap(DkSettings::fotojiffy.socialImageUrl);
		qDebug() << DkSettings::fotojiffy.socialImageUrl << " loaded...";
	}
	else if (mode == qrcode)
		setPixmap(DkSettings::fotojiffy.qrCodeImageUrl);
	else if (mode == reset_strip)
		setPixmap(DkSettings::fotojiffy.resetStripUrl);
	else
		qDebug() << "mode: " << mode << " is not a known social media mode!";
}

void DkSocialButton::contextMenuEvent(QContextMenuEvent *event) {

	QLabel::contextMenuEvent(event);

	if (!event->isAccepted())
		cm->exec(event->globalPos());

	event->accept();

}

void DkSocialButton::mousePressEvent(QMouseEvent *event) {

	QLabel::mousePressEvent(event);
}

void DkSocialButton::mouseReleaseEvent(QMouseEvent *event) {

	QRect pr = pixmap()->rect();

	// ok that's not beautiful but it does the trick
	if (mode == qrcode)
		pr.moveTopLeft(QPoint(contentsMargins().left(), contentsMargins().top()));
	else if (mode == facebook || mode == reset_strip)
		pr.moveTopRight(QPoint(width()-contentsMargins().right(), contentsMargins().top()));

	// do not track mouse clicks outside the pixmap area
	if ((mode == qrcode || mode == facebook || reset_strip) && pr.contains(event->pos()))
		emit showConfirmDialogSignal();

	QLabel::mouseReleaseEvent(event);
}

void DkSocialButton::changeImage() {

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
		"C:\\", 
		DkSettings::openFilters.join(";;"));

	// check if we can load it
	QPixmap testP(fileName);
	if (testP.isNull())
		return;

	if (mode == facebook)
		DkSettings::fotojiffy.socialImageUrl = fileName;
	else if (mode == qrcode)
		DkSettings::fotojiffy.qrCodeImageUrl = fileName;
	else if (mode == reset_strip)
		DkSettings::fotojiffy.resetStripUrl = fileName;

	setPixmap(fileName);

	DkSettings::save();

}

void DkSocialButton::show() {

	// here is a strange problem if you add a DkWidget to another DkWidget -> painters crash
	if (!showing) {
		hiding = false;
		showing = true;
		setVisible(true);
		animateOpacityUp();
	}
}

void DkSocialButton::hide() {

	if (!hiding) {
		hiding = true;
		showing = false;
		animateOpacityDown();

		// set display bit here too -> since the final call to setVisible takes a few seconds
		if (displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
			displaySettingsBits->setBit(DkSettings::app.currentAppMode, false);
		}
	}
}

void DkSocialButton::setVisible(bool visible) {

	if (visible && !isVisible() && !showing)
		opacityEffect->setOpacity(100);

	QLabel::setVisible(visible);
	emit visibleSignal(visible);	// if this gets slow -> put it into hide() or show()

	if (displaySettingsBits && displaySettingsBits->size() > DkSettings::app.currentAppMode) {
		displaySettingsBits->setBit(DkSettings::app.currentAppMode, visible);
	}
}

void DkSocialButton::animateOpacityUp() {

	if (!showing)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() >= 1.0f || !showing) {
		opacityEffect->setOpacity(1.0f);
		showing = false;
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
	opacityEffect->setOpacity(opacityEffect->opacity()+0.05);
}

void DkSocialButton::animateOpacityDown() {

	if (!hiding)
		return;

	opacityEffect->setEnabled(true);
	if (opacityEffect->opacity() <= 0.0f) {
		opacityEffect->setOpacity(0.0f);
		hiding = false;
		setVisible(false);	// finally hide the widget
		opacityEffect->setEnabled(false);
		return;
	}

	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
	opacityEffect->setOpacity(opacityEffect->opacity()-0.05);
}

// DkFotojiffyButton --------------------------------------------------------------------
//DkFotojiffyButton::DkFotojiffyButton(const QIcon& icon, const QString& objectName, const QWidget* parent) : QPushButton(icon, objectName, parent) {
//
//}

// DkStripDock --------------------------------------------------------------------
DkStripDock::DkStripDock(const QString& title, QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */ ) : QDockWidget(title, parent, flags) {

	createLayout();

	dirWatcher = new QFileSystemWatcher(this);
	connect(dirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));
	restartWatcher();
	updateLabels();
	setObjectName("stripModeDock");

	fadeOutTimer.setInterval(5);
	connect(&fadeOutTimer, SIGNAL(timeout()), this, SLOT(fadeOut()));

	fadeInTimer.setInterval(20);
	connect(&fadeInTimer, SIGNAL(timeout()), this, SLOT(fadeIn()));

	waitForScriptTimer.setInterval(2000);
	waitForScriptTimer.setSingleShot(true);
	connect(&waitForScriptTimer, SIGNAL(timeout()), this, SLOT(resetStrip()));

	reloadImgTimer.setInterval(500);
	reloadImgTimer.setSingleShot(true);
	connect(&reloadImgTimer, SIGNAL(timeout()), this, SLOT(updateLabels()));

	setFullScreen(true);
	finishStrip = false;
	//setStyleSheet("QWidget{background-color: QColor(0,0,0,20); border: 1px solid #000000;}");
}

void DkStripDock::createLayout() {

	// some defines
	int numImages = 4;
	int labelSpacing = 10;

	// reset strip
	DkSocialButton* resetStripButton = new DkSocialButton(DkSocialButton::reset_strip, this);
	resetStripButton->show();
	resetStripButton->setAlignment(Qt::AlignRight);
	connect(resetStripButton, SIGNAL(showConfirmDialogSignal()), this, SLOT(resetStrip()));

	QLabel* resetTextLabel = new QLabel(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_reset], this);
	resetTextLabel->setStyleSheet("QLabel{color: #FFFFFF;};");
	resetTextLabel->setAlignment(Qt::AlignRight);

	resetWidget = new DkWidget(this);
	resetWidget->setReallyHide(false);

	QVBoxLayout* resetLayout = new QVBoxLayout(resetWidget);
	resetLayout->setAlignment(Qt::AlignRight);
	resetLayout->addWidget(resetStripButton);
	resetLayout->addWidget(resetTextLabel);
	
	// reserve space
	resetWidget->setVisible(true);
	resetWidget->hide();

	titleLabel = new QLabel(this);
	titleLabel->setObjectName("titleLabel");
	titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	titleLabel->setStyleSheet("QLabel#titleLabel{color: #FFFFFF; font-size: 14px; font-weight: bold;}");

	// foto strip
	stripBackground = new QWidget(this);
	stripBackground->setObjectName("stripBackground");
	stripBackground->setStyleSheet("QWidget#stripBackground{background: #FFFFFF;}");
	stripBackground->show();

	opacityEffect = new QGraphicsOpacityEffect(stripBackground);
	opacityEffect->setEnabled(false);
	stripBackground->setGraphicsEffect(opacityEffect);

	QVBoxLayout* stripLayout = new QVBoxLayout(stripBackground);
	stripLayout->setAlignment(Qt::AlignHCenter);
	stripLayout->setSpacing(labelSpacing);

	labels.resize(numImages);

	for (int idx = 0; idx < labels.size(); idx++) {
		QLabel* cLabel = new QLabel(this);
		stripLayout->addWidget(cLabel);
		labels[idx] = cLabel;
	}

	resetLabels();
	if (!labels.empty()) {
		stripBackground->setMinimumWidth(labels.at(0)->minimumWidth()+20);
		stripBackground->setMaximumWidth(labels.at(0)->maximumWidth()+20);
		stripBackground->setMinimumHeight((labels.at(0)->minimumHeight()+labelSpacing)*labels.size()+40);
		stripBackground->setMaximumHeight((labels.at(0)->maximumHeight()+labelSpacing)*labels.size()+40);
	}

	stripLayout->addSpacing(50);

	QWidget* containerWidget = new QWidget(this);
	QGridLayout* containerLayout = new QGridLayout(containerWidget);
	//containerLayout->setAlignment(Qt::AlignHCenter);
	containerLayout->addWidget(titleLabel, 0, 0, 1, 3, Qt::AlignHCenter);
	containerLayout->addWidget(stripBackground, 1, 1, 1, 1, Qt::AlignHCenter);
	containerLayout->addWidget(resetWidget, 2, 0, 1, 3, Qt::AlignRight);
	//containerLayout->setRowStretch(3, 50);

	bgWidget = new QLabel(this);
	bgWidget->setObjectName("bgWidget");
	bgWidget->setStyleSheet("QLabel#bgWidget{background: " + DkUtils::colorToString(DkSettings::display.iconColor) + ";}");

	QVBoxLayout* layout = new QVBoxLayout(bgWidget);
	layout->addWidget(containerWidget);

	// waiting widgets
	waitContainer = new QWidget(containerWidget);
	waitContainer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	//waitContainer->setMaximumSize(1920, 300);
	//waitContainer->setStyleSheet("QWidget{margin: 0px;}");
	QGridLayout* waitLayout = new QGridLayout(waitContainer);
	//waitLayout->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

	QLabel* waitText = new QLabel(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_wait], waitContainer);
	waitText->setObjectName("waitText");
	waitText->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	waitText->setStyleSheet("QLabel#waitText{color: #FFFFFF; font-size: 14px; font-weight: bold;}");
	waitText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
	DkRotateImageLabel* waitAnimation = new DkRotateImageLabel(":/nomacs/img/load-strip.png", waitContainer);
	waitAnimation->setFixedSize(50,50);
	waitAnimation->setAlignment(Qt::AlignHCenter);
	//waitAnimation->show();

	waitLayout->setContentsMargins(0,0,0,0);
	waitLayout->addWidget(waitAnimation, 0, 1, 1, 1, Qt::AlignHCenter);
	waitLayout->addWidget(waitText, 1, 0, 1, 3, Qt::AlignHCenter);
	waitLayout->setRowStretch(2, 10);

	waitOpacityEffect = new QGraphicsOpacityEffect(waitContainer);
	waitOpacityEffect->setEnabled(false);
	waitContainer->setGraphicsEffect(waitOpacityEffect);
	waitContainer->hide();

	// strip finished
	//stripFinished = new DkStripFinished(this);
	//connect(stripFinished, SIGNAL(startNewStripSignal()), this, SLOT(startNewStrip()));
	//connect(stripFinished, SIGNAL(showStripSignal()), this, SIGNAL(loadLastImageSignal()));

	//QWidget* stackedWidget = new QWidget(this);
	//stackedLayout = new QStackedLayout(stackedWidget);
	//stackedLayout->addWidget(bgWidget);
	//stackedLayout->addWidget(stripFinished);
	//stackedLayout->setCurrentWidget(bgWidget);

	setWidget(bgWidget);

}

void DkStripDock::startNewStrip() {
	//stackedLayout->setCurrentWidget(bgWidget);
}

void DkStripDock::resetLabels() {

	for (int idx = 0; idx < labels.size(); idx++) {
		
		QLabel* cLabel = labels.at(idx);
		cLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		cLabel->setMinimumSize(200, 70);
		cLabel->setMaximumSize(200, 140);
		cLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		cLabel->setObjectName("stripLabel");
		cLabel->setStyleSheet("QLabel#stripLabel{background: #C4C4C4; color: #FFFFFF; font-size: 30px;}");
		cLabel->setText(QString::number(idx+1));
	}

	updateTitleMissing(labels.size()-stripImages.size());

	resetWidget->hide();
}

void DkStripDock::updateTitleMissing(int num) {

	QString titleText = DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_status];
	titleText += "\n" + DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_missing];
	titleText.replace("#Num", QString::number(num));
	titleLabel->setText(titleText);
}

void DkStripDock::setFullScreen(bool fullscreen) {
	
	if (fullscreen)
		setTitleBarWidget(new QWidget(this));
}

void DkStripDock::directoryChanged(QString dirPath) {

	updateLabels();
}

void DkStripDock::restartWatcher() {

	dirWatcher->addPath(DkSettings::fotojiffy.stripPath);
}

void DkStripDock::updateLabels() {

	// true file list
	QDir tmpDir(DkSettings::fotojiffy.stripPath);
	tmpDir.setSorting(QDir::LocaleAware);
	QStringList fileNameList = tmpDir.entryList(DkSettings::fileFilters);

	//showGenerating(false);

	for (int idx = 0; idx < stripImages.size() && idx < labels.size(); idx++)
		qDebug() << "image in list: " << stripImages.at(idx)->file().fileName();

	for (int idx = 0; idx < fileNameList.size(); idx++) {
		
		QFileInfo cFile(tmpDir, fileNameList.at(idx));

		bool imageLoaded = false;
		for (int fIdx = 0; fIdx < stripImages.size(); fIdx++) {
	
			if (cFile == stripImages.at(fIdx)->file()) {
				imageLoaded = true;
				break;
			}
		}

		qDebug() << "adding file: " << idx << " " <<  fileNameList.at(idx);
		
		if (!imageLoaded && stripImages.size() < labels.size()) {
			QSharedPointer<DkImageContainerT> cImg(new DkImageContainerT(cFile));
			QSharedPointer<DkThumbNailT> cThumb = cImg->getThumb();
			connect(cThumb.data(), SIGNAL(thumbLoadedSignal()), this, SLOT(updateImage()));
			cThumb->fetchThumb();

			stripImages.append(cImg);
		}
	}
}

void DkStripDock::updateImage() {

	DkThumbNailT* thumb = static_cast<DkThumbNailT*>(QObject::sender());

	if (!thumb)
		return;

	QPixmap pm = QPixmap::fromImage(thumb->getImage());

	if (pm.isNull()) {

		for (int idx = 0; idx < stripImages.size(); idx++) {

			if (stripImages.at(idx)->file() == thumb->getFile()) {
				stripImages.remove(idx);
				break;
			}
		}

		qDebug() << "trying to reload...";
		reloadImgTimer.start();

		return;
	}

	for (int idx = 0; idx < labels.size(); idx++) {

		QLabel* cLabel = labels.at(idx);

		if (!cLabel->pixmap() || cLabel->pixmap()->isNull()) {
			pm = pm.scaled(cLabel->width(), pm.height()*cLabel->width()/pm.width());
			cLabel->setPixmap(pm);
			cLabel->setStyleSheet("QLabel#stripLabel{background: rgba(0,0,0,0);}");
			updateTitleMissing(labels.size()-stripImages.size());

			if (idx == labels.size()-1) {
				//showGenerating(true);
				copyImagesToTemp();
			}
			else
				resetWidget->show();

			break;
		}
	}

}

void DkStripDock::fadeOut() {

	float multiplyFactor = (finishStrip) ? 1.0f : 2.0f;	// controls speed
	stripOpacity -= 0.0033f * multiplyFactor;

	opacityEffect->setOpacity(stripOpacity);
	waitOpacityEffect->setOpacity(1-stripOpacity);

	if (stripOpacity <= 0) {
		stripOpacity = 0;	// just to be save
		clearImages();
		fadeOutTimer.stop();
		waitContainer->hide();
		waitOpacityEffect->setEnabled(false);
	}

}

void DkStripDock::fadeIn() {

	float multiplyFactor = (finishStrip) ? 1.0f : 2.0f;	// controls speed
	stripOpacity += 0.04f * multiplyFactor;

	opacityEffect->setOpacity(stripOpacity);
	//waitOpacityEffect->setOpacity(1-stripOpacity);

	if (stripOpacity >= 1) {

		waitContainer->setVisible(false);
		fadeInTimer.stop();
		opacityEffect->setEnabled(false);
		waitOpacityEffect->setEnabled(false);
		finishStrip = false;
	}

}

void DkStripDock::clearImages() {


	for (int idx = 0; idx < stripImages.size(); idx++) {

		if (stripImages.at(idx)->exists()) {
			QFile cFile(stripImages.at(idx)->file().absoluteFilePath());
			qDebug() << "removing: " << stripImages.at(idx)->file().fileName();
			cFile.remove();
		}
	}

	stripImages.clear();
	resetLabels();

	fadeInTimer.start();
}

void DkStripDock::copyImagesToTemp() {

	//if (!tmpPath.exists())
	QDir tmpPath;
	tmpPath.mkpath(DkSettings::fotojiffy.stripTmpPath);
	tmpPath = DkSettings::fotojiffy.stripTmpPath;

	qDebug() << "copying to: " << tmpPath.absolutePath();

	for (int idx = 0; idx < stripImages.size(); idx++) {

		QFile cFile(stripImages.at(idx)->file().absoluteFilePath());
		cFile.copy(QFileInfo(tmpPath, stripImages.at(idx)->file().fileName()).absoluteFilePath());
		stripImages.at(idx)->clear();
	}

	finishStrip = true;
	resetWidget->hide();
	waitForScriptTimer.start();
	titleLabel->setText(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_status] + "\n" + DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_generating]);
}

void DkStripDock::resetStrip() {

	if (stripImages.isEmpty() || opacityEffect->isEnabled())
		return;

	stripOpacity = 1.0;
	opacityEffect->setEnabled(true);

	if (finishStrip) {
		waitOpacityEffect->setOpacity(0);
		waitOpacityEffect->setEnabled(true);
		waitContainer->setGeometry(0, 280, width()-10, 200);
		waitContainer->show();
	}
	fadeOutTimer.start();
	resetWidget->hide();
	//finishStrip = false;
}

//void DkStripDock::showGenerating(bool show) {
//
//	if (show) {
//		opacityEffect->setEnabled(true);
//		waitContainer->setGeometry(0, 300, width()-10, 200);
//		waitContainer->show();
//		titleLabel->setText(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_status] + "\n" + DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_generating]);
//	}
//	else {
//		waitContainer->hide();
//		opacityEffect->setEnabled(false);
//		updateTitleMissing(labels.size()-stripImages.size());
//	}
//
//}

// DkStripFinished --------------------------------------------------------------------
DkStripFinished::DkStripFinished(QWidget* parent /* = 0 */) : QLabel(parent) {

	createLayout();
}

void DkStripFinished::createLayout() {

	setObjectName("stripFinished");
	setStyleSheet("QLabel#stripFinished{background: " + DkUtils::colorToString(DkSettings::display.highlightColor) + ";}");

	QWidget* infoContainer = new QWidget(this);
	QGridLayout* infoLayout = new QGridLayout(infoContainer);

	QLabel* finishedLabel = new QLabel(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_finished], infoContainer);
	finishedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignHCenter);
	finishedLabel->setStyleSheet("QLabel{color: #FFFFFF; font-weight: bold; font-size: 14px;}");

	QLabel* okLabel = new QLabel(infoContainer);
	okLabel->setPixmap(QPixmap(":/nomacs/img/ok.png").scaled(60,60));
	//okLabel->setFixedSize(70, 70);

	QLabel* infoLabel = new QLabel(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_choose], infoContainer);
	infoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignHCenter);
	infoLabel->setStyleSheet("QLabel{color: #FFFFFF; font-size: 14px;}");

	//infoLayout->setContentsMargins(0, 50, 0, 0);
	infoLayout->setSpacing(30);
	infoLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	infoLayout->addWidget(finishedLabel, 0, 0, 1, 3);
	infoLayout->addWidget(okLabel, 1, 1);
	infoLayout->addWidget(infoLabel, 2, 0, 1, 3);

	QWidget* buttonContainer = new QWidget(this);
	buttonContainer->setStyleSheet("QPushButton{background-color: #FFFFFF; border: 0px; font-size: 14px;}");
	QVBoxLayout* buttonLayout = new QVBoxLayout(buttonContainer);

	QPushButton* showInViewerButton = new QPushButton(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_show_in_viewer], buttonContainer);
	showInViewerButton->setFixedSize(250, 40);
	connect(showInViewerButton, SIGNAL(clicked()), this, SIGNAL(showStripSignal()));

	QPushButton* newStripButton = new QPushButton(DkSettings::fotojiffy.fotoStrings[DkSettings::foto_strip_new], buttonContainer);
	newStripButton->setFixedSize(250, 40);
	connect(newStripButton, SIGNAL(clicked()), this, SIGNAL(startNewStripSignal()));

	buttonLayout->setAlignment(Qt::AlignHCenter);
	buttonLayout->addWidget(showInViewerButton);
	buttonLayout->addWidget(newStripButton);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addStretch();
	layout->addWidget(infoContainer);
	layout->addStretch();
	layout->addWidget(buttonContainer);
	layout->addStretch();
}

// DkRotationImageLabel --------------------------------------------------------------------
DkRotateImageLabel::DkRotateImageLabel(const QString& imgPath, QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QLabel(parent, f) {
	
	pm = QPixmap(imgPath);
	setMaximumSize(pm.size());
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	animationTimer.setInterval(20);
	connect(&animationTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
	animationAngle = 0;
}

void DkRotateImageLabel::setVisible(bool visible) {

	if (visible)
		animationTimer.start();
	else
		animationTimer.stop();

	QLabel::setVisible(visible);
}

void DkRotateImageLabel::updateAnimation() {

	animationAngle += 6;
	animationAngle %= 360;
	update();
}

void DkRotateImageLabel::paintEvent(QPaintEvent *event) {

	QTransform t;
	t.translate(width()/2, height()/2);
	t.rotate((double)animationAngle); 
	t.translate(-width()/2, -height()/2); 

	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setTransform(t);
	painter.drawPixmap(QRect(QPoint(), size()), pm, QRect(QPoint(), pm.size()));
	painter.end();

	QLabel::paintEvent(event);
}

}