#include "aspectratiopixmaplabel.h"
//#include <QDebug>

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) :
	QLabel(parent)
{
	this->setMinimumSize(1, 1);
}

void AspectRatioPixmapLabel::setPixmap(const QPixmap & p)
{
	pix = p;
	QLabel::setPixmap(p);
	this->setMinimumSize(p.size());
}

int AspectRatioPixmapLabel::heightForWidth(int width) const
{
	return ((qreal)pix.height()*width) / pix.width();
}

QSize AspectRatioPixmapLabel::sizeHint() const
{
	int w = this->width();
	return QSize(w, heightForWidth(w));
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent * e)
{
	if (!pix.isNull())
		QLabel::setPixmap(pix.scaled(this->size(),
			Qt::KeepAspectRatio, Qt::SmoothTransformation));
}