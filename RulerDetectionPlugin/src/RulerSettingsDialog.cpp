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

#include <qmessagebox.h>
#include <qdebug.h>
#include <qfiledialog.h>
#include <qsizepolicy.h>
#include <qlayout.h>

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
		comboBox_tickDistance->setCurrentIndex(comboBox_tickDistance->findText(tmp_tickDistance));
		//qDebug() << "Where the refpoints were found: " << comboBox_referencePoints->findText(QString::number(tmp_referencePoints));
		comboBox_referencePoints->setCurrentIndex(comboBox_referencePoints->findText(QString::number(tmp_referencePoints)));
		spinBox_HessianThreshold->setValue(tmp_hessianThreshold);
	}

	RulerSettingsDialog::~RulerSettingsDialog() {

	}

	/**
	* initializes variables and creates layout
	**/
	void RulerSettingsDialog::init() {
		this->setWindowModality(Qt::NonModal);
		this->setEnabled(true);
		//this->setFixedSize(260, 150);
		this->setMinimumSize(310, 166);
		this->setMaximumSize(500, 400);
		//this->layout()->setSizeConstraint(QLayout::SetNoConstraint);
		this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

		centralWidget = new QWidget(this);
		centralWidget->setObjectName(QStringLiteral("centralWidget"));
		verticalLayout = new QVBoxLayout(centralWidget);
		verticalLayout->setSpacing(6);
		verticalLayout->setContentsMargins(11, 11, 11, 11);
		verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
		horizontalLayout_2 = new QHBoxLayout();
		horizontalLayout_2->setSpacing(6);
		horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
		label = new QLabel(centralWidget);
		label->setObjectName(QStringLiteral("label"));

		horizontalLayout_2->addWidget(label);

		pushButton_chooseTemplate = new QPushButton(centralWidget);
		connect(pushButton_chooseTemplate, SIGNAL(clicked()), this, SLOT(chooseTemplatePressed()));
		pushButton_chooseTemplate->setObjectName(QStringLiteral("pushButton_chooseTemplate"));

		horizontalLayout_2->addWidget(pushButton_chooseTemplate);

		verticalLayout->addLayout(horizontalLayout_2);

		horizontalLayout_3 = new QHBoxLayout();
		horizontalLayout_3->setSpacing(6);
		horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
		label_2 = new QLabel(centralWidget);
		label_2->setObjectName(QStringLiteral("label_2"));

		horizontalLayout_3->addWidget(label_2);

		comboBox_tickDistance = new QComboBox(centralWidget);
		comboBox_tickDistance->setObjectName(QStringLiteral("comboBox_tickDistance"));

		horizontalLayout_3->addWidget(comboBox_tickDistance);

		verticalLayout->addLayout(horizontalLayout_3);

		horizontalLayout_4 = new QHBoxLayout();
		horizontalLayout_4->setSpacing(6);
		horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
		label_3 = new QLabel(centralWidget);
		label_3->setObjectName(QStringLiteral("label_3"));

		horizontalLayout_4->addWidget(label_3);

		comboBox_referencePoints = new QComboBox(centralWidget);
		comboBox_referencePoints->setObjectName(QStringLiteral("comboBox_referencePoints"));

		horizontalLayout_4->addWidget(comboBox_referencePoints);

		verticalLayout->addLayout(horizontalLayout_4);

		horizontalLayout_5 = new QHBoxLayout();
		horizontalLayout_5->setSpacing(6);
		horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
		label_4 = new QLabel(centralWidget);
		label_4->setObjectName(QStringLiteral("label_4"));

		horizontalLayout_5->addWidget(label_4);

		spinBox_HessianThreshold = new QSpinBox(centralWidget);
		spinBox_HessianThreshold->setObjectName(QStringLiteral("spinBox_HessianThreshold"));
		spinBox_HessianThreshold->setMinimum(10);
		spinBox_HessianThreshold->setMaximum(1000);
		spinBox_HessianThreshold->setSingleStep(10);
		spinBox_HessianThreshold->setValue(150);

		horizontalLayout_5->addWidget(spinBox_HessianThreshold);

		verticalLayout->addLayout(horizontalLayout_5);

		horizontalLayout_6 = new QHBoxLayout();
		horizontalLayout_6->setSpacing(6);
		horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
		pushButton_Help = new QPushButton(centralWidget);
		connect(pushButton_Help, SIGNAL(clicked()), this, SLOT(helpPressed()));
		pushButton_Help->setObjectName(QStringLiteral("pushButton_Help"));

		horizontalLayout_6->addWidget(pushButton_Help);

		pushButton_OK = new QPushButton(centralWidget);
		connect(pushButton_OK, SIGNAL(clicked()), this, SLOT(okPressed()));
		pushButton_OK->setObjectName(QStringLiteral("pushButton_OK"));

		horizontalLayout_6->addWidget(pushButton_OK);

		pushButton_Cancel = new QPushButton(centralWidget);
		connect(pushButton_Cancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));
		pushButton_Cancel->setObjectName(QStringLiteral("pushButton_Cancel"));

		horizontalLayout_6->addWidget(pushButton_Cancel);

		verticalLayout->addLayout(horizontalLayout_6);

		//this->setCentralWidget(centralWidget);
#ifndef QT_NO_SHORTCUT
		label->setBuddy(pushButton_chooseTemplate);
		label_2->setBuddy(comboBox_tickDistance);
		label_3->setBuddy(comboBox_referencePoints);
		label_4->setBuddy(spinBox_HessianThreshold);
#endif // QT_NO_SHORTCUT
		QWidget::setTabOrder(pushButton_chooseTemplate, comboBox_tickDistance);
		QWidget::setTabOrder(comboBox_tickDistance, comboBox_referencePoints);
		QWidget::setTabOrder(comboBox_referencePoints, spinBox_HessianThreshold);
		QWidget::setTabOrder(spinBox_HessianThreshold, pushButton_Help);
		QWidget::setTabOrder(pushButton_Help, pushButton_OK);
		QWidget::setTabOrder(pushButton_OK, pushButton_Cancel);

		this->setWindowTitle(tr("Ruler Detection Settings"));
		label->setText(tr("Template Path"));
		pushButton_chooseTemplate->setText(tr("Choose image"));
		label_2->setText(tr("Tick Distance"));
		comboBox_tickDistance->clear();
		comboBox_tickDistance->insertItems(0, QStringList()
			<< tr("mm")
			<< tr("cm")
			<< tr("inch")
		);

		label_3->setText(tr("Reference points"));
		comboBox_referencePoints->clear();
		comboBox_referencePoints->insertItems(0, QStringList()
			<< tr("3")
			<< tr("5")
			<< tr("7")
		);
		label_4->setText(tr("Hessian Threshold"));
		pushButton_Help->setText(tr("Help"));
		pushButton_OK->setText(tr("OK"));
		pushButton_Cancel->setText(tr("Cancel"));

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
		tmp_tickDistance = comboBox_tickDistance->currentText();
		tmp_referencePoints = comboBox_referencePoints->currentText().toInt();
		tmp_hessianThreshold = spinBox_HessianThreshold->value();

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


