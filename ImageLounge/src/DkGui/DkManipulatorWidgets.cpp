/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkManipulatorWidgets.h"
#include "DkActionManager.h"
#include "DkWidgets.h"
#include "DkTimer.h"
#include "DkUtils.h"
#include "DkSettings.h"
#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#pragma warning(pop)

namespace nmc {

// DkManipulatorWidget --------------------------------------------------------------------
DkManipulatorWidget::DkManipulatorWidget(QWidget* parent) : DkWidget(parent) {
	
	setObjectName("DkPreferenceTabs");
	createLayout();

	DkActionManager& am = DkActionManager::instance();
	for (QAction* a : am.manipulatorActions())
		connect(a, SIGNAL(triggered()), this, SLOT(selectManipulator()), Qt::UniqueConnection);

}

void DkManipulatorWidget::createLayout() {

	// actions
	QWidget* actionWidget = new QWidget(this);
	QVBoxLayout* aLayout = new QVBoxLayout(actionWidget);
	aLayout->setAlignment(Qt::AlignTop);
	aLayout->setContentsMargins(0, 0, 0, 0);
	aLayout->setSpacing(0);

	QButtonGroup* group = new QButtonGroup(this);

	DkActionManager& am = DkActionManager::instance();
	for (QAction* a : am.manipulatorActions()) {

		DkTabEntryWidget* mpl = new DkTabEntryWidget(a->icon(), a->text(), this);
		connect(mpl, SIGNAL(clicked()), a, SIGNAL(triggered()), Qt::UniqueConnection);	// TODO: different connection if ManipulatorExt?
		
		aLayout->addWidget(mpl);
		group->addButton(mpl);
	}

	QString scrollbarStyle = 
		QString("QScrollBar:vertical {border: 1px solid " + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
		+ QString("QScrollBar::handle:vertical {background: " + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; min-height: 0px;}")
		+ QString("QScrollBar::add-line:vertical {height: 0px;}")
		+ QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: 1px;}")
		+ QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}");

	QScrollArea* actionScroller = new QScrollArea(this);
	actionScroller->setStyleSheet(scrollbarStyle + actionScroller->styleSheet());
	actionScroller->setWidgetResizable(true);
	actionScroller->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	actionScroller->setWidget(actionWidget);
	actionScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// settings
	mSettingsWidget = new QWidget(this);

	// preview
	mPreview = new QLabel(this);

	// undo
	QPixmap pm = DkImage::colorizePixmap(QIcon(":/nomacs/img/rotate-cc.svg").pixmap(QSize(32, 32)), QColor(255, 255, 255));
	QPushButton* undoButton = new QPushButton(pm, "", this);
	undoButton->setFlat(true);
	undoButton->setIconSize(QSize(32, 32));
	undoButton->setObjectName("DkRestartButton");
	undoButton->setStatusTip(tr("Undo"));
	connect(undoButton, SIGNAL(clicked()), am.action(DkActionManager::menu_edit_undo), SIGNAL(triggered()));

	QWidget* mplWidget = new QWidget(this);
	QVBoxLayout* mplLayout = new QVBoxLayout(mplWidget);
	mplLayout->setAlignment(Qt::AlignBottom);
	mplLayout->addWidget(undoButton);
	mplLayout->addWidget(mSettingsWidget);
	mplLayout->addWidget(mPreview);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(actionScroller);
	layout->addWidget(mplWidget);
}

void DkManipulatorWidget::setImage(QSharedPointer<DkImageContainerT> imgC) {
	mImgC = imgC;

	if (mImgC) {
		QImage img = mImgC->imageScaledToWidth(qMin(mPreview->width(), 300));
		mPreview->setPixmap(QPixmap::fromImage(img));
		mPreview->show();
	}
	else
		mPreview->hide();
}

void DkManipulatorWidget::selectManipulator() {

	QAction* action = dynamic_cast<QAction*>(QObject::sender());

	DkActionManager& am = DkActionManager::instance();
	QSharedPointer<DkBaseManipulator> mpl = am.manipulatorManager().manipulator(action);
	QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mpl);

	// compute preview
	const QPixmap* pm = mPreview->pixmap();

	if (mpl && pm) {
		DkTimer dt;
		QImage img = mpl->apply(pm->toImage());
		
		if (!img.isNull())
			mPreview->setPixmap(QPixmap::fromImage(img));
		qDebug() << "preview computed in " << dt;
	}

	if (!mplExt) {
		if (mSettingsWidget)
			mSettingsWidget->hide();
		return;
	}

	mSettingsWidget = mplExt->widget();
}

// DkMainpulatorDoc --------------------------------------------------------------------
DkEditDock::DkEditDock(const QString& title, QWidget* parent, Qt::WindowFlags flags) : DkDockWidget(title, parent, flags) {

	createLayout();
}

void DkEditDock::createLayout() {

	mMplWidget = new DkManipulatorWidget(this);
	setWidget(mMplWidget);
}

void DkEditDock::setImage(QSharedPointer<DkImageContainerT> imgC) {
	mMplWidget->setImage(imgC);
}

}