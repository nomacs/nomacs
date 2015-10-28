

#include "DkOcrToolbar.h"
#include <iostream> // removeme


DkOcrToolbar::DkOcrToolbar(QWidget* parent) : QToolBar("test title", parent)
{
	createLayout();
	QMetaObject::connectSlotsByName(this);
}

DkOcrToolbar::~DkOcrToolbar()
{
}

void DkOcrToolbar::on_test()
{
	std::cout << "test" << std::endl;
}

void DkOcrToolbar::createLayout()
{
	test = new QPushButton(tr("Auto &Rotate"), this);
	test->setObjectName("autoRotateButton");
	test->setToolTip(tr("Automatically rotate imgC for small skewness"));
	addWidget(test);
	//test->setStatusTip(autoRotateButton->toolTip()); // Tooltips
}