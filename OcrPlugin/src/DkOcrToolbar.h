#ifndef DK_ORC_TOOLBAR_H
#define DK_ORC_TOOLBAR_H

#include <QObject>
#include <QImage>
#include <QMessageBox>
#include <QAction>
#include <QToolBar>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QToolbar>
#include <QPushButton>

class DkOcrToolbar : public QToolBar {
	Q_OBJECT


public:

	DkOcrToolbar(QWidget* parent);
	virtual ~DkOcrToolbar();

public slots:
public slots :
	void on_test();

protected:
	QPushButton* test;
	void createLayout();
};

#endif
