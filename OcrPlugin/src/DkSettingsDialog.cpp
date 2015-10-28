#include <QtWidgets>

#include "DkSettingsDialog.h"

FindDialog::FindDialog(QWidget *parent)
	: QDialog(parent)
{
	label = new QLabel(tr("Find &what:"));
	lineEdit = new QLineEdit;
	label->setBuddy(lineEdit);

	caseCheckBox = new QCheckBox(tr("Match &case"));
	fromStartCheckBox = new QCheckBox(tr("Search from &start"));
	fromStartCheckBox->setChecked(true);

	findButton = new QPushButton(tr("&Find"));
	findButton->setDefault(true);

	moreButton = new QPushButton(tr("&More"));
	moreButton->setCheckable(true);
	moreButton->setAutoDefault(false);


	extension = new QWidget;

	wholeWordsCheckBox = new QCheckBox(tr("&Whole words"));
	backwardCheckBox = new QCheckBox(tr("Search &backward"));
	searchSelectionCheckBox = new QCheckBox(tr("Search se&lection"));

	buttonBox = new QDialogButtonBox(Qt::Vertical);
	buttonBox->addButton(findButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(moreButton, QDialogButtonBox::ActionRole);

	connect(moreButton, SIGNAL(toggled(bool)), extension, SLOT(setVisible(bool)));

	QVBoxLayout *extensionLayout = new QVBoxLayout;
	extensionLayout->setMargin(0);
	extensionLayout->addWidget(wholeWordsCheckBox);
	extensionLayout->addWidget(backwardCheckBox);
	extensionLayout->addWidget(searchSelectionCheckBox);
	extension->setLayout(extensionLayout);

	QHBoxLayout *topLeftLayout = new QHBoxLayout;
	topLeftLayout->addWidget(label);
	topLeftLayout->addWidget(lineEdit);

	QVBoxLayout *leftLayout = new QVBoxLayout;
	leftLayout->addLayout(topLeftLayout);
	leftLayout->addWidget(caseCheckBox);
	leftLayout->addWidget(fromStartCheckBox);

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addLayout(leftLayout, 0, 0);
	mainLayout->addWidget(buttonBox, 0, 1);
	mainLayout->addWidget(extension, 1, 0, 1, 2);
	mainLayout->setRowStretch(2, 1);

	setLayout(mainLayout);

	setWindowTitle(tr("Extension"));
	extension->hide();
}