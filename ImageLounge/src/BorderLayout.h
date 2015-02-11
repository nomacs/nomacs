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

 #ifndef BORDERLAYOUT_H
 #define BORDERLAYOUT_H

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QLayout>
#pragma warning(pop)		// no warnings from includes - end

#pragma warning(disable: 4251)	// TODO: remove

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

class DllExport BorderLayout : public QLayout {

public:
     enum Position { West, North, South, East, Center };

     BorderLayout(QWidget *parent, int margin = 0, int spacing = -1);
     BorderLayout(int spacing = -1);
     ~BorderLayout();

     void addItem(QLayoutItem *item);
     void addWidget(QWidget *widget, Position position);
     Qt::Orientations expandingDirections() const;
     bool hasHeightForWidth() const;
     int count() const;
     QLayoutItem *itemAt(int index) const;
     QSize minimumSize() const;
     void setGeometry(const QRect &rect);
     QSize sizeHint() const;
     QLayoutItem *takeAt(int index);

     void add(QLayoutItem *item, Position position);

 private:
     struct ItemWrapper
     {
         ItemWrapper(QLayoutItem *i, Position p) {
             item = i;
             position = p;
         }

         QLayoutItem *item;
         Position position;
     };

     enum SizeType { MinimumSize, SizeHint };
     QSize calculateSize(SizeType sizeType) const;

     QList<ItemWrapper *> list;
 };

 #endif
