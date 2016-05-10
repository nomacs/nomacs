#pragma once
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

//source:
//http://stackoverflow.com/questions/8211982/qt-resizing-a-qlabel-containing-a-qpixmap-while-keeping-its-aspect-ratio

class AspectRatioPixmapLabel : public QLabel
{
	Q_OBJECT
public:
	explicit AspectRatioPixmapLabel(QWidget *parent = 0);
	virtual int heightForWidth(int width) const;
	virtual QSize sizeHint() const;
signals:

	public slots :
		void setPixmap(const QPixmap &);
	void resizeEvent(QResizeEvent *);
private:
	QPixmap pix;
};
