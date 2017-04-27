/*******************************************************************************************************
RulerSettingsDialog.cpp

nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

Copyright (C) 2017 Pascal Plank

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

#include "RulerSettingsDialog.h"

#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>



namespace nmc {

	RulerSettingsDialog::RulerSettingsDialog(QString path, QString tickdist, int refpoints, int threshold, QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
		isOkPressed = false;

		init();
		tmp_templatePath = path;
		tmp_tickDistance = tickdist;
		tmp_referencePoints = refpoints;
		tmp_hessianThreshold = threshold;

		//qDebug() << "### Das kommt an: #" << tmp_templatePath << "#, #" << tmp_tickDistance << "#, #" << QString::number(tmp_referencePoints) << "#, #" << tmp_hessianThreshold << "#.";

		//qDebug() << "Where the tickentry was found: " << comboBox_tickDistance->findText(tmp_tickDistance);
		tickDist_cb->setCurrentIndex(tickDist_cb->findText(tmp_tickDistance));
		//qDebug() << "Where the refpoints were found: " << comboBox_referencePoints->findText(QString::number(tmp_referencePoints));
		refPoints_cb->setCurrentIndex(refPoints_cb->findText(QString::number(tmp_referencePoints)));
		hessianThres_sb->setValue(tmp_hessianThreshold);
	}

	RulerSettingsDialog::~RulerSettingsDialog() {

	}

	/**
	* initializes variables and creates layout
	**/
	void RulerSettingsDialog::init() {

		// ### General window stuff
		this->resize(317, 182);
		verticalLayout = new QVBoxLayout(this);
		verticalLayout->setSpacing(6);
		verticalLayout->setContentsMargins(11, 11, 11, 11);
		verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

		// ### Choosing template
		horizontalLayout = new QHBoxLayout();
		horizontalLayout->setSpacing(6);
		horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
		horizontalLayout->setContentsMargins(-1, 0, -1, -1);
		chooseImg_label = new QLabel(this);
		chooseImg_label->setObjectName(QStringLiteral("chooseImg_label"));
		chooseImg_label->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		horizontalLayout->addWidget(chooseImg_label);

		chooseImg_btn = new QPushButton(this);
		chooseImg_btn->setObjectName(QStringLiteral("chooseImg_btn"));
		connect(chooseImg_btn, SIGNAL(clicked()), this, SLOT(chooseTemplatePressed()));

		horizontalLayout->addWidget(chooseImg_btn);
		verticalLayout->addLayout(horizontalLayout);

		// ### Set tick distance
		horizontalLayout_2 = new QHBoxLayout();
		horizontalLayout_2->setSpacing(6);
		horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
		tickDist_label = new QLabel(this);
		tickDist_label->setObjectName(QStringLiteral("tickDist_label"));
		tickDist_label->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		horizontalLayout_2->addWidget(tickDist_label);

		tickDist_cb = new QComboBox(this);
		tickDist_cb->setObjectName(QStringLiteral("tickDist_cb"));

		horizontalLayout_2->addWidget(tickDist_cb);
		verticalLayout->addLayout(horizontalLayout_2);

		// ### Set reference points
		horizontalLayout_3 = new QHBoxLayout();
		horizontalLayout_3->setSpacing(6);
		horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
		refPoints_label = new QLabel(this);
		refPoints_label->setObjectName(QStringLiteral("refPoints_label"));
		refPoints_label->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		horizontalLayout_3->addWidget(refPoints_label);

		refPoints_cb = new QComboBox(this);
		refPoints_cb->setObjectName(QStringLiteral("refPoints_cb"));

		horizontalLayout_3->addWidget(refPoints_cb);
		verticalLayout->addLayout(horizontalLayout_3);

		// Set hessian threshold
		horizontalLayout_4 = new QHBoxLayout();
		horizontalLayout_4->setSpacing(6);
		horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
		hessianThres_label = new QLabel(this);
		hessianThres_label->setObjectName(QStringLiteral("hessianThres_label"));
		hessianThres_label->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		horizontalLayout_4->addWidget(hessianThres_label);

		hessianThres_sb = new QSpinBox(this);
		hessianThres_sb->setObjectName(QStringLiteral("hessianThres_sb"));
		hessianThres_sb->setMinimum(10);
		hessianThres_sb->setMaximum(1500);
		hessianThres_sb->setSingleStep(10);
		hessianThres_sb->setValue(200);

		horizontalLayout_4->addWidget(hessianThres_sb);
		verticalLayout->addLayout(horizontalLayout_4);
		//verticalLayout

		// ### Add buttons and connect SIGNALs to SLOTS
		horizontalLayout_5 = new QHBoxLayout();
		horizontalLayout_5->setSpacing(6);
		horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
		help_btn = new QPushButton(this);
		help_btn->setObjectName(QStringLiteral("help_btn"));
		connect(help_btn, SIGNAL(clicked()), this, SLOT(helpPressed()));

		horizontalLayout_5->addWidget(help_btn);

		OK_btn = new QPushButton(this);
		OK_btn->setObjectName(QStringLiteral("OK_btn"));
		connect(OK_btn, SIGNAL(clicked()), this, SLOT(okPressed()));
		horizontalLayout_5->addWidget(OK_btn);

		cancel_btn = new QPushButton(this);
		cancel_btn->setObjectName(QStringLiteral("cancel_btn"));
		connect(cancel_btn, SIGNAL(clicked()), this, SLOT(cancelPressed()));
		horizontalLayout_5->addWidget(cancel_btn);

		verticalLayout->addLayout(horizontalLayout_5);

		// ### Setting Buddies
		chooseImg_label->setBuddy(chooseImg_btn);
		tickDist_label->setBuddy(tickDist_cb);
		refPoints_label->setBuddy(refPoints_cb);
		hessianThres_label->setBuddy(hessianThres_sb);

		// ### Add text to all elements and insert values for combo/spinboxes
		this->setWindowTitle(QApplication::translate("Dialog", "Settings", Q_NULLPTR));
		chooseImg_label->setText(QApplication::translate("Dialog", "Template Path", Q_NULLPTR));
		chooseImg_btn->setText(QApplication::translate("Dialog", "Choose image", Q_NULLPTR));
		tickDist_label->setText(QApplication::translate("Dialog", "Tick Distance", Q_NULLPTR));
		tickDist_cb->clear();
		tickDist_cb->insertItems(0, QStringList()
			<< QApplication::translate("Dialog", "mm", Q_NULLPTR)
			<< QApplication::translate("Dialog", "cm", Q_NULLPTR)
			<< QApplication::translate("Dialog", "inch", Q_NULLPTR)
		);
		refPoints_label->setText(QApplication::translate("Dialog", "Reference Points", Q_NULLPTR));
		refPoints_cb->clear();
		refPoints_cb->insertItems(0, QStringList()
			<< QApplication::translate("Dialog", "3", Q_NULLPTR)
			<< QApplication::translate("Dialog", "5", Q_NULLPTR)
			<< QApplication::translate("Dialog", "7", Q_NULLPTR)
		);
		hessianThres_label->setText(QApplication::translate("Dialog", "Hessian Threshold", Q_NULLPTR));
		help_btn->setText(QApplication::translate("Dialog", "Help", Q_NULLPTR));
		OK_btn->setText(QApplication::translate("Dialog", "OK", Q_NULLPTR));
		cancel_btn->setText(QApplication::translate("Dialog", "Cancel", Q_NULLPTR));
	}




	//##############################################################################
	//##############################################################################
	//############################ GETTER METHODS ##################################
	//##############################################################################
	//##############################################################################
	QString RulerSettingsDialog::getTemplatePath() {
		return tmp_templatePath;
	}

	QString RulerSettingsDialog::getTickDistance() {
		return tmp_tickDistance;
	}

	int RulerSettingsDialog::getReferencePoints() {
		return tmp_referencePoints;
	}

	int RulerSettingsDialog::getHessianThreshold() {
		return tmp_hessianThreshold;
	}

	bool RulerSettingsDialog::isOkClicked() {
		return isOkPressed;
	}




	//##############################################################################
	//##############################################################################
	//############################# PRESS EVENTS ###################################
	//##############################################################################
	//##############################################################################
	/**
	* on button ok pressed event (save infos and close dialog)
	**/
	void RulerSettingsDialog::okPressed() {
		qInfo() << "[RULER DETECTION] Settings: ok was pressed";

		// Save inputs in tmp_vars, so the parent window can access them via getters
		tmp_tickDistance = tickDist_cb->currentText();
		tmp_referencePoints = refPoints_cb->currentText().toInt();
		tmp_hessianThreshold = hessianThres_sb->value();

		isOkPressed = true;

		this->close();
	}

	/**
	* on button cancel pressed event (just close the settings dialog)
	**/
	void RulerSettingsDialog::cancelPressed() {
		qInfo() << "[RULER DETECTION] Settings: cancel was pressed";
		isOkPressed = false;

		this->close();
	}

	/**
	* help button pressed event (display helptext in QMessageBox)
	**/
	void RulerSettingsDialog::helpPressed() {
		qInfo() << "[RULER DETECTION] Settings: help was pressed";
		QMessageBox::information(this, "Settings Help", "-) Ruler template image: the image must be a vertical ruler (lying sideways) and have a high enough resolution to distinguish the ticks. \n\n-) Tick Distance: the distance between 2 ticks of your ruler. \n\n-) Reference Points: a lower value is more error tolerant and a higher value gives more accurate measurements. \n\n-) Hessian Threshold: change only if the ruler cannot be found. A higher value means less SURF-features are calculated (faster), a lower value means more calculation (better chance to find good matches)");
	}

	/**
	* choose template pressed event
	**/
	void RulerSettingsDialog::chooseTemplatePressed() {
		qInfo() << "[RULER DETECTION] Settings: choose template was pressed";

		QString filename;
		if (tmp_templatePath == "") {
			// Default homepath
			filename = QFileDialog::getOpenFileName(this, "Choose a ruler template image", QDir::homePath());
		} else {
			// use previously specified path if it exists
			filename = QFileDialog::getOpenFileName(this, "Choose a ruler template image", tmp_templatePath);
		}

		// Check filename
		if (filename != "") {
			// a file was chosen
			tmp_templatePath = filename;
			//QMessageBox::information(this, "Settings Help", tmp_templatePath);
		} else {
			// no file was chosen --> do nothing
		}
	}
};


