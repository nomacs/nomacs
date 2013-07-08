/****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #include <QtGui>

 #include "BorderLayout.h"

 BorderLayout::BorderLayout(QWidget *parent, int margin, int spacing)
     : QLayout(parent)
 {
     setMargin(margin);
     setSpacing(spacing);
 }

 BorderLayout::BorderLayout(int spacing)
 {
     setSpacing(spacing);
 }

 BorderLayout::~BorderLayout()
 {
	 foreach (ItemWrapper* wrapper, list) {
		 if (wrapper->item)
			 delete wrapper->item;
		 wrapper->item = 0;
		 if (wrapper)
			 delete wrapper;
		 wrapper = 0;
	 }
 }

 void BorderLayout::addItem(QLayoutItem *item)
 {
     add(item, West);
 }

 void BorderLayout::addWidget(QWidget *widget, Position position)
 {
     add(new QWidgetItem(widget), position);
 }

 Qt::Orientations BorderLayout::expandingDirections() const
 {
     return Qt::Horizontal | Qt::Vertical;
 }

 bool BorderLayout::hasHeightForWidth() const
 {
     return false;
 }

 int BorderLayout::count() const
 {
     return list.size();
 }

 QLayoutItem *BorderLayout::itemAt(int index) const
 {
     ItemWrapper *wrapper = list.value(index);
     if (wrapper)
         return wrapper->item;
     else
         return 0;
 }

 QSize BorderLayout::minimumSize() const
 {
     return calculateSize(MinimumSize);
 }

 void BorderLayout::setGeometry(const QRect &rect)
 {
     ItemWrapper *center = 0;
     int eastWidth = 0;
     int westWidth = 0;
     int northHeight = 0;
     int southHeight = 0;
     int centerHeight = 0;
     int i;

     QLayout::setGeometry(rect);

     for (i = 0; i < list.size(); ++i) {
         ItemWrapper *wrapper = list.at(i);
         QLayoutItem *item = wrapper->item;
         Position position = wrapper->position;

         if (position == North) {
             item->setGeometry(QRect(rect.x(), northHeight, rect.width(),
                                     item->sizeHint().height()));

             northHeight += item->geometry().height() + spacing();
         } else if (position == South) {
             item->setGeometry(QRect(item->geometry().x(),
                                     item->geometry().y(), rect.width(),
                                     item->sizeHint().height()));

             southHeight += item->geometry().height() + spacing();

             item->setGeometry(QRect(rect.x(),
                               rect.y() + rect.height() - southHeight + spacing(),
                               item->geometry().width(),
                               item->geometry().height()));
         } else if (position == Center) {
             center = wrapper;
         }
     }

     centerHeight = rect.height() - northHeight - southHeight;

     for (i = 0; i < list.size(); ++i) {
         ItemWrapper *wrapper = list.at(i);
         QLayoutItem *item = wrapper->item;
         Position position = wrapper->position;

         if (position == West) {
             item->setGeometry(QRect(rect.x() + westWidth, northHeight,
                                     item->sizeHint().width(), centerHeight));

             westWidth += item->geometry().width() + spacing();
         } else if (position == East) {
             item->setGeometry(QRect(item->geometry().x(), item->geometry().y(),
                                     item->sizeHint().width(), centerHeight));

             eastWidth += item->geometry().width() + spacing();

             item->setGeometry(QRect(
                               rect.x() + rect.width() - eastWidth + spacing(),
                               northHeight, item->geometry().width(),
                               item->geometry().height()));
         }
     }

     if (center)
         center->item->setGeometry(QRect(westWidth, northHeight,
                                         rect.width() - eastWidth - westWidth,
                                         centerHeight));
 }

 QSize BorderLayout::sizeHint() const
 {
     return calculateSize(SizeHint);
 }

 QLayoutItem *BorderLayout::takeAt(int index)
 {
     if (index >= 0 && index < list.size()) {
         ItemWrapper *layoutStruct = list.takeAt(index);
         return layoutStruct->item;
     }
     return 0;
 }

 void BorderLayout::add(QLayoutItem *item, Position position)
 {
     list.append(new ItemWrapper(item, position));
 }

 QSize BorderLayout::calculateSize(SizeType sizeType) const
 {
     QSize totalSize;

     for (int i = 0; i < list.size(); ++i) {
         ItemWrapper *wrapper = list.at(i);
         Position position = wrapper->position;
         QSize itemSize;

         if (sizeType == MinimumSize)
             itemSize = wrapper->item->minimumSize();
         else // (sizeType == SizeHint)
             itemSize = wrapper->item->sizeHint();

         if (position == North || position == South || position == Center)
             totalSize.rheight() += itemSize.height();

         if (position == West || position == East || position == Center)
             totalSize.rwidth() += itemSize.width();
     }
     return totalSize;
 }
