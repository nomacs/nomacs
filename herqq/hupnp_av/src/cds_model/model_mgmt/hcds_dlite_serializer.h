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

#ifndef HCDS_DLITE_SERIALIZER_H_
#define HCDS_DLITE_SERIALIZER_H_

#include <HUpnpAv/HUpnpAv>

class QStringList;

template <typename T>
class QSet;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HCdsDidlLiteSerializerPrivate;

/*!
 * \brief This class is used to serialize the HUPnPAv CDS model from or to a
 * DIDL-Lite document.
 *
 * \headerfile hcds_dlite_serializer.h HCdsDidlLiteSerializer
 *
 * \ingroup hupnp_av_cds_om_mgmt
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HCdsDidlLiteSerializer
{
H_DISABLE_COPY(HCdsDidlLiteSerializer)

private:

    HCdsDidlLiteSerializerPrivate* h_ptr;

public:

    /*!
     * \brief This enumeration specifies the different types of XML data a character
     * string can contain.
     */
    enum XmlType
    {
        /*!
         * The string contains a sequence of XML elements that are valid
         * child elements of the XML root element according to a specific
         * schema.
         */
        XmlExcerpt,

        /*!
         * The string contains a valid XML 1.0 document according to a specific
         * schema.
         */
        Document
    };

    /*!
     * \brief Constructs a new instance.
     */
    HCdsDidlLiteSerializer();

    /*!
     * \brief Destroys the instance.
     */
    ~HCdsDidlLiteSerializer();

    /*!
     * \brief Returns a description of the last error that occurred.
     *
     * \return a description of the last error that occurred.
     */
    QString lastErrorDescription() const;

    /*!
     * \brief Serializes HUPnPAv CDS objects from a DIDL-Lite document.
     *
     * \param didlLiteDoc specifies the DIDL-Lite document.
     *
     * \param retVal specifies a pointer to a list of pointers to
     * HObject instances. If the serialization succeeds, this list will contain
     * the serialized HObject pointers. The HObjects are heap-allocated and the
     * ownership of the objects is passed to the caller.
     *
     * \param inputType specifies the XML type of the string passed to the method.
     *
     * \param errDescr specifies a pointer to a \c QString, which will contain
     * a human readable error description in case the serialization failed.
     * This parameter is optional.
     *
     * \return \e true when the serialization succeeds.
     */
    bool serializeFromXml(
        const QString& didlLiteDoc, HObjects* retVal,
        XmlType inputType = Document);

    /*!
     * \brief Serializes the specified HObject into a DIDL-Lite document.
     *
     * \param object specifies the object to be serialized.
     *
     * \param outputType specifies whether the return value will be a valid
     * XML document or an excerpt containing only the contents of the
     * \a object.
     *
     * \return The object serialized to DIDL-Lite.
     */
    QString serializeToXml(
        const HObject& object, XmlType outputType = Document);

    /*!
     * \brief Serializes the specified HObject into a DIDL-Lite document.
     *
     * \param object specifies the object to be serialized.
     *
     * \param filter specifies the properties to be serialized. If this is not
     * empty or it doesn't contain an asterisk, only the properties specified
     * in the set will be serialized.
     *
     * \param outputType specifies whether the return value will be a valid
     * XML document or an excerpt containing only the contents of the
     * \a object.
     *
     * \return The object serialized to DIDL-Lite.
     */
    QString serializeToXml(
        const HObject& object, const QSet<QString>& filter,
        XmlType outputType = Document);

    /*!
     * \brief Serializes the specified HObjects into a DIDL-Lite document.
     *
     * \param objects specifies the objects to be serialized.
     *
     * \return The objects serialized to DIDL-Lite.
     */
    QString serializeToXml(const HObjects& objects);

    /*!
     * \brief Serializes the specified HObjects into a DIDL-Lite document.
     *
     * \param objects specifies the objects to be serialized.
     *
     * \param filter specifies the properties to be serialized. If this is not
     * empty or it doesn't contain an asterisk, only the properties specified
     * in the set will be serialized.
     *
     * \return The objects serialized to DIDL-Lite.
     */
    QString serializeToXml(const HObjects& objects, const QSet<QString>& filter);
};

}
}
}

#endif /* HCDS_DLITE_SERIALIZER_H_ */
