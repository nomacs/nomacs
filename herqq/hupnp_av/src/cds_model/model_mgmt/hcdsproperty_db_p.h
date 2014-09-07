/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HCDSPROPERTY_DB_P_H_
#define HCDSPROPERTY_DB_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hcdsproperty_db.h"
#include "../cds_objects/hobject.h"
#include "../../common/hresource.h"
#include "../../common/hradioband.h"

#include "../../hav_global.h"

#include <HUpnpCore/private/hmisc_utils_p.h>

#include <QtCore/QUrl>
#include <QtCore/QSet>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QReadWriteLock>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

//
//
//
class HCdsPropertyDbPrivate
{
H_DISABLE_COPY(HCdsPropertyDbPrivate)

public:

    QHash<QString, HCdsProperty> m_properties;
    QSet<QString> m_didlLiteDependentProperties;
    QReadWriteLock m_propertiesLock;

    void insert(const HCdsProperty& prop);
    void remove(const QString& propName);
    QString variantAsString(const QVariant& var) const;

    bool serializeHResourceOut      (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeHResourceIn       (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeClassInfoOut      (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeClassInfoIn       (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeRoledPersonOut    (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeRoledPersonIn     (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeContentDurationOut(const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeContentDurationIn (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeWeekDayOut        (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeWeekDayIn         (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeMatchedIdOut      (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeMatchedIdIn       (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeProgramCodeOut    (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeProgramCodeIn     (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeRatingOut         (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeRatingIn          (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeEpTypeOut         (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeEpTypeIn          (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeChGroupNameOut    (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeChGroupNameIn     (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializePriceOut          (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializePriceIn           (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeDtRangeOut        (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeDtRangeIn         (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeScheduledTimeOut  (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeScheduledTimeIn   (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeStateInfoOut      (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeStateInfoIn       (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeSvCollectionOut   (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeSvCollectionIn    (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeDescOut           (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeDescIn            (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeFmDataOut         (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeFmDataIn          (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeGenreElementOut   (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeGenreElementIn    (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeStorageMediumOut  (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeStorageMediumIn   (const QString&, QVariant*, QXmlStreamReader* = 0) const;

    bool serializeWriteStatusOut    (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeWriteStatusIn     (const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeElementOut        (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeMultiValuedElementOut(const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeCDSListElementIn  (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeElementIn         (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeBoolElementIn     (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeUIntElementIn     (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeULongElementIn    (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeIntElementIn      (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeUriElementIn      (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeDateElementIn     (const QString&, QVariant*, QXmlStreamReader*) const;
    bool serializeRadiobandElementOut(const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeRadiobandElementIn(const QString&, QVariant*, QXmlStreamReader* = 0) const;
    bool serializeAttributeOut      (const QString&, const QVariant&, QXmlStreamWriter&) const;
    bool serializeAttributeIn       (const QString&, QVariant*, QXmlStreamReader* = 0) const;

    bool compareResources           (const QVariant& var1, const QVariant& var2, qint32* retVal) const;
    bool compareCaseSensitiveStrings(const QVariant& var1, const QVariant& var2, qint32* retVal) const;
    bool compareUnsignedLongs(const QVariant& var1, const QVariant& var2, qint32* retVal) const;
    bool compareSignedLongs(const QVariant& var1, const QVariant& var2, qint32* retVal) const;
    bool compareBools(const QVariant& var1, const QVariant& var2, qint32* retVal) const;
    bool compareDates(const QVariant& var1, const QVariant& var2, qint32* retVal) const;
    bool notEmpty(const QVariant& var) const;

public:

    HCdsPropertyDbPrivate();
    ~HCdsPropertyDbPrivate();

    void init();
};

}
}
}

#endif /* HCDSPROPERTY_DB_P_H_ */
