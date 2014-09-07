/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HPRODUCT_TOKENS_H
#define HPRODUCT_TOKENS_H

#include <HUpnpCore/HUpnp>

#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

template<typename T>
class QVector;

namespace Herqq
{

namespace Upnp
{

/*!
 * \brief This class represents a <em>product token</em> as defined in the RFC 2616,
 * section 3.8.
 *
 * \headerfile hproduct_tokens.h HProductToken
 *
 * \remarks This class is not thread-safe.
 *
 * \ingroup hupnp_common
 */
class H_UPNP_CORE_EXPORT HProductToken
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HProductToken& obj1, const HProductToken& obj2);

private:

    QString m_token;
    QString m_productVersion;

public:

    /*!
     * \brief Creates a new, empty instance. An object created with the default
     * constructor is invalid.
     *
     * \sa isValid()
     */
    HProductToken();

    /*!
     * Creates a new object based on the provided token data. If the token data
     * is invalid, the object will be invalid as well.
     *
     * \param token specifies the token part, which is supposed to identify
     * a product. If this is empty, the created object will be invalid.
     *
     * \param productVersion specifies the version part. If this is empty,
     * the created object will be invalid.
     *
     * \sa isValid()
     */
    HProductToken(const QString& token, const QString& productVersion);

    /*!
     * \brief Destroys the instance.
     */
    ~HProductToken();

    /*!
     * \brief Indicates if the object is valid, i.e both the token and the product
     * version are defined.
     *
     * \param checkLevel specifies whether the contents of the object are checked
     * for strict validity. Only an object that is strictly valid
     * contains information as defined in the UDA. In other words,
     * a strictly valid product token takes the form
     * <c>token/majorVersion.minorVersion</c>.
     *
     * \return true in case both the \e token and <em>product version</em>
     * are appropriately specified.
     *
     * \sa token(), productVersion()
     */
    bool isValid(HValidityCheckLevel checkLevel) const;

    /*!
     * \brief Indicates if the object is a valid UPnP token, i.e both the token
     * and the product version are defined.
     *
     * \return true in case both the \e token and <em>product version</em>
     * are defined according to the UDA v1.1.
     *
     * \sa token(), productVersion()
     */
    bool isValidUpnpToken() const;

    /*!
     * \brief Indicates if the object is a valid DLNADOC token, i.e both the token
     * and the product version are defined.
     *
     * \return true in case both the \e token and <em>product version</em>
     * are defined according to the DLNA Guideline.
     *
     * \sa token(), productVersion()
     */
    bool isValidDlnaDocToken() const;

    /*!
     * \brief Returns the \e token part.
     *
     * \return The \e token part in case the object is valid.
     * The token part is used to identify the product and an example
     * of a token is for instance \c "Apache". An empty string is returned in case
     * the object is invalid.
     *
     * \sa isValid()
     */
    inline QString token() const { return m_token; }

    /*!
     * \brief Returns the \e version part.
     *
     * \return The \e version part in case the object is valid. An example of a
     * version part is \c "1.0". An empty string is returned in case
     * the object is invalid.
     *
     * \sa isValid()
     */
    inline QString version() const { return m_productVersion; }

    /*!
     * \brief Returns a string representation of the object.
     *
     * The format of the returned string is \c "token"/"version".
     *
     * \return a string representation of the object.
     */
    QString toString() const;

    /*!
     * Attempts to parse the \e version part of a product token to a major and
     * minor component and returns the minor component if the function succeeded.
     *
     * For the function to succeed the specified product token has to contain
     * a version string that follows the format "major.minor". The function ignores
     * any further "version information" after the "minor" component separated
     * by a dot. The "minor" component has to be convertible to an integer.
     *
     * \return The minor version component of the specified product token or -1
     * if the specified token does not contain a minor version component that
     * can be represented as an integer.
     */
    qint32 minorVersion();
    // TODO for v2, should be const

    /*!
     * Attempts to parse the \e version part of a product token to a major and
     * minor component and returns the major component if the function succeeded.
     *
     * For the function to succeed the specified product token has to contain
     * a version string that either
     * - can be directly converted to an integer or
     * - contains information following the format "major.minor", where
     * the major component can be converted to an integer. Any data following
     * the "minor" component and separated from it by a dot is ignored.
     *
     * \return The major version component of the specified product token or -1
     * if the specified token does not contain a major version component that
     * can be represented as an integer.
     */
    qint32 majorVersion();
    // TODO for v2, should be const
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HProductToken
 */
H_UPNP_CORE_EXPORT bool operator==(const HProductToken&, const HProductToken&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HProductToken
 */
H_UPNP_CORE_EXPORT bool operator!=(const HProductToken&, const HProductToken&);

class HProductTokensPrivate;

/*!
 * \brief This class is used to parse the <em>product tokens</em> defined by HTTP/1.1.
 *
 * According to the HTTP/1.1, <em>Product tokens are used to allow communicating applications
 * to identify themselves by software name and version</em>. In UDA,
 * the <em>product tokens</em> consist of three tokens,
 * in which <em>The first product token identifes the operating system in
 * the form OS name/OS version, the second token represents the UPnP version and
 * MUST be UPnP/1.1, and the third token identifes the product using the form
 * product name/product version</em>. For example, "SERVER: unix/5.1 UPnP/1.1 MyProduct/1.0".
 *
 * Unfortunately, product tokens found in UPnP products are rarely conforming to
 * the HTTP/1.1 and UDA specifications. Many products handle "product tokens" as a string that
 * contains "key/value" pairs laid out and delimited arbitrarily. Because of this,
 * \c %HProductTokens has to accept input that is not strictly standard-conformant.
 * However, at absolute minimum UPnP devices have to provide the UPnP version token.
 * Because of that, \c %HProductTokens instance is considered valid if the
 * instance contains a valid UPnP version token. All other tokens are considered
 * optional and they may not be present in a valid instance. In practice this means that
 * if isValid() returns true the instance contains a valid UPnP version
 * token, which can be retrieved using upnpToken(). In that case the tokens()
 * returns a list at least of size one. In addition, the instance may contain other
 * data that could not be parsed following the HTTP/1.1 and UDA specifications. This
 * data cannot be retrieved using any of the functions that return HProductToken
 * instances, but you can retrieve the full unparsed product tokens string using
 * toString().
 *
 * \headerfile hproduct_tokens.h HProductTokens
 *
 * \remarks This class is not thread-safe.
 *
 * \ingroup hupnp_common
 */
class H_UPNP_CORE_EXPORT HProductTokens
{
private:

    QSharedDataPointer<HProductTokensPrivate> h_ptr;

public:

    /*!
     * Constructs a new invalid and empty instance.
     *
     * \sa isValid(), isEmpty()
     */
    HProductTokens();

    /*!
     * Creates a new instance based on the provided argument.
     *
     * \param arg specifies the product tokens. In case the specified argument
     * does not contain a valid UPnP version token the created
     * object will be invalid. However, the object will not be empty and the
     * provided string is returned when toString() is called.
     *
     * \sa isValid(), isEmpty(), toString(), upnpToken()
     */
    explicit HProductTokens(const QString& arg);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of the other object.
     */
    HProductTokens(const HProductTokens&);

    /*!
     * \brief Destroys the instance.
     */
    ~HProductTokens();

    /*!
     * Assigns the contents of the other to this.
     *
     * \return a reference to this object.
     */
    HProductTokens& operator=(const HProductTokens&);

    /*!
     * \brief Indicates whether the object contains at least the UPnP
     * version token defined in the UDA.
     *
     * \return \e true in case the object contains at least the UPnP version
     * token defined in the UDA.
     *
     * \remarks A strictly valid instance should always have a UPnP token. However,
     * if an instance is missing the UPnP token, it is not necessarily empty. Furthermore,
     * an object may contain data that could not be parsed into ProductToken objects. In this case
     * you can call toString() to retrieve the full product tokens string.
     *
     * \sa isEmpty()
     */
    bool hasUpnpToken() const;

    /*!
     * \brief Indicates whether the object contains the DLNADOC version token
     * defined in the DLNA Guideline.
     *
     * \return \e true in case the object contains the DLNADOC version
     * token defined in the DLNA Guideline.
     *
     * \sa isEmpty()
     */
    bool hasDlnaDocToken() const;

    /*!
     * \brief Indicates whether the object contains any information at all.
     *
     * \return true in case the object does not contain any information.
     *
     * \remarks an empty object is also invalid.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the product token that defines UPnP version.
     *
     * \return The product token that defines UPnP version. This token always
     * follows the format "UPnP"/majorVersion.minorVersion, where \e majorVersion
     * and \e minorVersion are positive integers. Furthermore, currently
     * the \e majorVersion is \b always 1 and the \e minorVersion is either 0
     * or 1.
     *
     * \remarks This is always defined in a valid object.
     *
     * \sa hasUpnpToken()
     */
    HProductToken upnpToken() const;

    /*!
     * \brief Returns the product token that defines DLNADOC version.
     *
     * \return The product token that defines DLNADOC version. This token always
     * follows the format "DLNADOC"/majorVersion.minorVersion, where \e majorVersion
     * and \e minorVersion are positive integers.
     *
     * \sa hasUpnpToken()
     */
    HProductToken dlnaDocToken() const;

    /*!
     * \brief Returns all product tokens the instance contains.
     *
     * A valid \c %HProductTokens object will return a vector that contains
     * at least one entry, the upnpToken(). A strictly valid \c %HProductTokens
     * object will return a vector that contains at least three entries,
     * the osToken(), upnpToken() and producToken(). If the object contains
     * extra tokens the extra tokens are appended to the returned vector.
     *
     * \return all product tokens in a vector. An invalid object returns a
     * vector with no elements. However, even in this case the object may not be empty.
     *
     * \sa isValid(), isEmpty(), extraTokens(), toString()
     */
    QVector<HProductToken> tokens() const;

    /*!
     * \brief Returns a string representation of the object.
     *
     * \return a string representation of the object.
     *
     * \remarks This method may return a non-empty string even in case
     * isValid() returns false. In this case the instance was created with
     * a string that could not be tokenized according to the UDA and HTTP 1.1
     * specifications.
     *
     * \sa isEmpty(), isValid()
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HProductTokens
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HProductTokens&, const HProductTokens&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HProductTokens
 */
inline bool operator!=(const HProductTokens& obj1, const HProductTokens& obj2)
{
    return !(obj1 == obj2);
}

}
}

#endif /* HPRODUCT_TOKENS_H */
