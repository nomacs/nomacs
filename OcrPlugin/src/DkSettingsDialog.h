#ifndef DK_SETTINGS_DIALOG_H
#define DK_SETTINGS_DIALOG_H

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class FindDialog : public QDialog
{
	Q_OBJECT

public:
	FindDialog(QWidget *parent = 0);

private:
	QLabel *label;
	QLineEdit *lineEdit;
	QCheckBox *caseCheckBox;
	QCheckBox *fromStartCheckBox;
	QCheckBox *wholeWordsCheckBox;
	QCheckBox *searchSelectionCheckBox;
	QCheckBox *backwardCheckBox;
	QDialogButtonBox *buttonBox;
	QPushButton *findButton;
	QPushButton *moreButton;
	QWidget *extension;
};

#endif
