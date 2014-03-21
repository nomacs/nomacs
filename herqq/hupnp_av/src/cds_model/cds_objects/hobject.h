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

#ifndef HOBJECT_H_
#define HOBJECT_H_

#include <HUpnpAv/HUpnpAv>
#include <HUpnpAv/HCdsProperties>

#include <HUpnpCore/HClonable>

#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace Herqq
{

namespace Upnp
{

namespace Av
{

class HObjectEventInfoPrivate;

/*!
 * \brief This class is used to contain information of an \e Object \e Modification
 * event.
 *
 * \headerfile hobject.h HObjectEventInfo
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_AV_EXPORT HObjectEventInfo
{
private:

    QSharedDataPointer<HObjectEventInfoPrivate> h_ptr;

public:

    /*!
     * \brief Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HObjectEventInfo();

    /*!
     * \brief Creates a new instance.
     *
     * \param varName specifies the name of the property.
     *
     * \param oldValue specifies the value of the property before the change.
     *
     * \param newValue specifies the current value.
     */
    HObjectEventInfo(
        const QString& varName,
        const QVariant& oldValue,
        const QVariant& newValue,
        quint32 updateId = 0);

    /*!
     * \brief Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HObjectEventInfo(const HObjectEventInfo&);

    /*!
     * \brief Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HObjectEventInfo& operator=(const HObjectEventInfo&);

    /*!
     * \brief Destroys the instance.
     */
    ~HObjectEventInfo();

    /*!
     * \brief Indicates whether the object is valid.
     *
     * \return \e true in case the object is valid, i.e. at least the
     * variableName() is defined.
     */
    bool isValid() const;

    /*!
     * \brief Returns the name of variable (property) that changed.
     *
     * \return The name of variable (property) that changed.
     */
    QString variableName() const;

    /*!
     * \brief Returns the last value of the variable before the change.
     *
     * \return The last value of the variable before the change.
     */
    QVariant oldValue() const;

    /*!
     * \brief Returns the new (current) value of the variable.
     *
     * \return The new (current) value of the variable.
     */
    QVariant newValue() const;

    /*!
     * \brief Returns the value of \c SystemUpdateID state variable after the change.
     *
     * \return The value of \c SystemUpdateID state variable after the change.
     */
    quint32 updateId() const;

    /*!
     * \brief Specifies the name of variable (property) that changed.
     *
     * \param arg specifies the name of variable (property) that changed.
     */
    void setVariableName(const QString& arg);

    /*!
     * \brief Specifies the last value of the variable before the change.
     *
     * \param arg specifies the last value of the variable before the change.
     */
    void setOldValue(const QVariant& arg);

    /*!
     * \brief Specifies the new (current) value of the variable.
     *
     * \param arg specifies the new (current) value of the variable.
     */
    void setNewValue(const QVariant& arg);

    /*!
     * \brief Specifies the value of \c SystemUpdateID state variable after the change.
     *
     * \param arg specifies the value of \c SystemUpdateID state variable after the change.
     */
    void setUpdateId(quint32 arg);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HObjectEventInfo
 */
H_UPNP_AV_EXPORT bool operator==(const HObjectEventInfo& obj1, const HObjectEventInfo& obj2);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HObjectEventInfo
 */
inline bool operator!=(const HObjectEventInfo& obj1, const HObjectEventInfo& obj2)
{
    return !(obj1 == obj2);
}

class HObjectPrivate;

/*!
 * This is the root class of the ContentDirectoryService (CDS) class hierarchy.
 *
 * This is an abstract class that contains the properties common to all CDS
 * objects. The class identifier specified by the AV Working Committee is
 * \c object.
 *
 * \headerfile hobject.h HObject
 *
 * \ingroup hupnp_av_cds_objects
 *
 * \remarks This class is not thread-safe.
 *
 * \sa HItem, HContainer
 */
class H_UPNP_AV_EXPORT HObject :
    public QObject,
    public HClonable
{
Q_OBJECT
H_DISABLE_COPY(HObject)
H_DECLARE_PRIVATE(HObject)

friend class HCdsDidlLiteSerializerPrivate;

public:

    /*!
     * \brief This enumeration defines all the classes specified by the AV Working
     * Committee that can be instantiated.
     */
    enum CdsType
    {
        /*!
         * Undefined type.
         *
         * This is used in error situations.
         */
        UndefinedCdsType = 0,

        /*!
         * Individual content item.
         */
        Item = 0x00000001,

        /*!
         * Individual image content item.
         */
        ImageItem = 0x00000002,

        /*!
         * A still image.
         */
        Photo = 0x00000003,

        /*!
         * Individual audio content item.
         */
        AudioItem = 0x00000004,

        /*!
         * A song, or a track in an audio media.
         */
        MusicTrack = 0x00000005,

        /*!
         * A continuous stream from an audio broadcast.
         */
        AudioBroadcast = 0x00000006,

        /*!
         * Audio content that is the narration of a book.
         */
        AudioBook = 0x00000007,

        /*!
         * Content intended for viewing, as a combination of video and audio.
         */
        VideoItem = 0x00000008,

        /*!
         * Movie.
         */
        Movie = 0x00000009,

        /*!
         * Continuous stream from a video broadcast.
         */
        VideoBroadcast = 0x0000000a,

        /*!
         * Video content that is a clip supporting a song.
         */
        MusicVideoClip = 0x0000000b,

        /*!
         * A playable sequence of resources.
         */
        PlaylistItem = 0x0000000c,

        /*!
         * Content intended for reading.
         */
        TextItem = 0x0000000d,

        /*!
         * Data that can be used to recover previous state information of a
         * AVTransport and a RenderingControl service instances.
         */
        BookmarkItem = 0x0000000e,

        /*!
         * A program, such as a single radio show, a single TV show or a
         * series of programs.
         */
        EpgItem = 0x0000000f,

        /*!
         * Broadcast audio program, such as a radio show or a series of programs.
         */
        AudioProgram = 0x00000010,

        /*!
         * Video program, such as a single TV show or a series of programs.
         */
        VideoProgram = 0x00000011,

        /*!
         * Individual vendor-defined item.
         */
        VendorDefinedItem = 0x0000ffff,

        /*!
         * A container for collection of individual items.
         */
        Container = 0x00010000,

        /*!
         * An unordered collection of objects associated with a person.
         */
        Person = 0x00020000,

        /*!
         * An unordered collection of objects associated with a music artist.
         */
        MusicArtist = 0x00030000,

        /*!
         * A collection of objects, which may be a mixture of video, audio and
         * images and is typically created by a user.
         */
        PlaylistContainer = 0x00040000,

        /*!
         * An ordered collection of objects.
         */
        Album = 0x00050000,

        /*!
         * An ordered collection of music tracks and albums.
         */
        MusicAlbum = 0x00060000,

        /*!
         * An ordered collection of photo objects.
         */
        PhotoAlbum = 0x00070000,

        /*!
         * An unordered collection of objects that all belong to the same genre.
         */
        Genre = 0x00080000,

        /*!
         * An unordered collection of objects relating to a <em>style of music</em>.
         */
        MusicGenre = 0x00090000,

        /*!
         * An unordered collection of objects relating to a <em>movie style</em>.
         */
        MovieGenre = 0x000a0000,

        /*!
         * Groups together individual, but related broadcast channels.
         */
        ChannelGroup = 0x000b0000,

        /*!
         * Groups together individual, but related audio broadcast channels.
         */
        AudioChannelGroup = 0x000c0000,

        /*!
         * Groups together individual, but related video broadcast channels.
         */
        VideoChannelGroup = 0x000d0000,

        /*!
         * A program guide container, which may contain any any kind of objects
         * for EPG information, such as audio and video program items or other
         * EPG containers to organize these items.
         */
        EpgContainer = 0x000e0000,

        /*!
         * A potentially heterogeneous collection of storage media.
         */
        StorageSystem = 0x000f0000,

        /*!
         * Either physical storage unit or a partition of it.
         */
        StorageVolume = 0x00100000,

        /*!
         * A collection of objects stored on a storage medium.
         */
        StorageFolder = 0x00110000,

        /*!
         * An unordered collection of either book mark items or other book mark folders.
         */
        BookmarkFolder = 0x00120000,

        /*!
         * A vendor-defined container for collection of individual items.
         */
        VendorDefinedContainer = 0xffffffff
    };

    /*!
     * \brief This enumeration defines the values that can be used to control the
     * modifiability of the resources of a given object.
     */
    enum WriteStatus
    {
        /*!
         * The object's resource(s) write status is unknown.
         */
        UnknownWriteStatus,

        /*!
         * The object's resource(s) MAY be deleted and/or modified.
         */
        WritableWriteStatus,

        /*!
         * The object's resource(s) MAY NOT be deleted and/or modified
         */
        ProtectedWriteStatus,

        /*!
         * The object's resource(s) MAY NOT be modified.
         */
        NotWritableWriteStatus,

        /*!
         * Some of the object's resource(s) have a different write status.
         */
        MixedWriteStatus
    };

    /*!
     * \brief Returns a string representation of the specified value.
     *
     * \return a string representation of the specified value.
     */
    static QString writeStatusToString(WriteStatus status);

    /*!
     * \brief Returns a WriteStatus value corresponding to the specified string.
     *
     * \return a WriteStatus value corresponding to the specified string.
     */
    static WriteStatus writeStatusFromString(const QString& status);

protected: // attributes

    HObjectPrivate* h_ptr;

    /*!
     * Constructs a new instance.
     *
     * \param clazz specifies the UPnP class of the object. This cannot be empty.
     *
     * \param cdsType specifies the CDS type of the object. This cannot be
     * HObject::UndefinedCdsType.
     *
     * \sa isInitialized()
     */
    HObject(const QString& clazz, CdsType cdsType);

    //
    // \internal
    //
    HObject(HObjectPrivate&);

    /*!
     * Initializes the object.
     *
     * \param title specifies the title of the object.
     *
     * \param parentId specifies the ID of the object that contains this
     * object. If the object has no parent, this has to be left empty.
     *
     * \param id specifies the ID of this object. If this is not specified,
     * a unique identifier within the running process is created for the object.
     *
     * \return \e true in case the initialization succeeded.
     *
     * \sa isValid()
     */
    bool init(
        const QString& title,
        const QString& parentId = QString(),
        const QString& id = QString());

    /*!
     * \brief Indicates if the object is appropriately initialized and ready to be used.
     *
     * \return \e true when the object is appropriately initialized and
     * ready to be used.
     */
    bool isInitialized() const;

    /*!
     * \brief Sets the ID of this object.
     *
     * \param arg specifies the ID.
     */
    void setId(const QString& arg);

    /*!
     * Adds a resource to this object.
     *
     * \param resource specifies the resource to be added.
     */
    void addResource(const HResource& resource);

    /*!
     * Enables or disables the specified property.
     *
     * \param property specifies the name of the property.
     *
     * \param enable specifies whether the property is enabled.
     *
     * \return \e true in case the operation was successfully done.
     */
    bool enableCdsProperty(const QString& property, bool enable);

    /*!
     * Validates the state of this object.
     *
     * If a descendant class specifies mandatory class invariants, it should
     * override this and do the checking in here. This method is called by
     * HObject when serializing content from a stream into the class and when
     * a user invokes isValid().
     *
     * \return \e true in case all the mandatory CDS properties are appropriately
     * defined.
     *
     * \sa isValid(), serialize()
     */
    virtual bool validate() const;

    /*!
     * Serializes the specified property from the stream reader or from the
     * variant to this object.
     *
     * \param propertyName specifies the name of the property.
     *
     * \param var specifies a pointer to a \c QVariant, which contains the value
     * of the property, if the pointer is non-null.
     *
     * \param reader specifies the stream reader from which the value of the
     * property is read when \a var is not defined.
     *
     * \return \e true if the value of the property was successfully read.
     */
    virtual bool serialize(
        const QString& propertyName, QVariant* var, QXmlStreamReader* reader);

    /*!
     * Serializes the specified property to the specified stream writer.
     *
     * \param propertyName specifies the name of the property to be serialized.
     *
     * \param var specifies the value of the property to be serialized. If
     * \a var is valid the class should serialize the contents of it, rather than
     * read the value of the specified property from this object
     * and write that to the stream writer.
     *
     * \param writer specifies the stream writer to which the value is written.
     *
     * \return \e true if the value of the property was successfully written.
     */
    virtual bool serialize(
        const QString& propertyName, const QVariant& var,
        QXmlStreamWriter& writer) const;

    // Documented in HClonable
    virtual void doClone(HClonable* target) const;

public:

    /*!
     * \brief Destroys the instance.
     */
    virtual ~HObject() = 0;

    // Documented in HClonable
    virtual HObject* clone() const;

    /*!
     * \brief Returns all the CDS properties of this object.
     *
     * \return all the CDS properties of this object.
     *
     * \sa hasCdsProperty(), isCdsPropertySet(), setCdsProperty(),
     * getCdsProperty(), isCdsPropertyActive()
     */
    HCdsPropertyMap cdsProperties() const;

    /*!
     * \brief Indicates if the object contains the specified CDS property.
     *
     * \param property specifies the CDS property.
     *
     * \return \e true if the object contains the specified CDS property.
     */
    bool hasCdsProperty(const QString& property) const;

    /*!
     * \brief Indicates if the object contains the specified CDS property.
     *
     * \param property specifies the CDS property.
     *
     * \return \e true if the object contains the specified CDS property.
     */
    bool hasCdsProperty(HCdsProperties::Property property) const;

    /*!
     * \brief Indicates if the object contains the specified CDS property and it has
     * its value set.
     *
     * \param property specifies the CDS property.
     *
     * \return \e true if the object contains the specified CDS property and it
     * has its value set.
     */
    bool isCdsPropertySet(const QString& property) const;

    /*!
     * \brief Indicates if the object contains the specified CDS property and it has
     * its value set.
     *
     * \param property specifies the CDS property.
     *
     * \return \e true if the object contains the specified CDS property and it
     * has its value set.
     */
    bool isCdsPropertySet(HCdsProperties::Property property) const;

    /*!
     * \brief Sets the value of the specified CDS property.
     *
     * \param property specifies the name of the CDS property.
     *
     * \param value specifies the new value of the property.
     *
     * \return \e true if the value was successfully set.
     *
     * \remarks the value is \b not set if the object does not contain a property
     * with the specified name.
     */
    bool setCdsProperty(const QString& property, const QVariant& value);

    /*!
     * \brief Sets the value of the specified CDS property.
     *
     * \param property specifies the CDS property.
     *
     * \param value specifies the new value of the property.
     *
     * \return \e true if the value was successfully set.
     *
     * \remarks the value is \b not set if the object does not contain a property
     * with the specified name.
     */
    bool setCdsProperty(HCdsProperties::Property property, const QVariant& value);

    /*!
     * \brief Retrieves the value of the specified CDS property.
     *
     * \param property specifies the name of the CDS property.
     *
     * \param value specifies a pointer to \c QVariant to which the value
     * will be stored.
     *
     * \return \e true if the value was successfully retrieved and stored to
     * \a value.
     */
    bool getCdsProperty(const QString& property, QVariant* value) const;

    /*!
     * \brief Retrieves the value of the specified CDS property.
     *
     * \param property specifies the CDS property.
     *
     * \param value specifies a pointer to \c QVariant to which the value
     * will be stored.
     *
     * \return \e true if the value was successfully retrieved and stored to
     * \a value.
     */
    bool getCdsProperty(HCdsProperties::Property property, QVariant* value) const;

    /*!
     * \brief Indicates if the specified property is found and active.
     *
     * \param property specifies the name of the property.
     *
     * \return \e true if the specified property is found and active.
     */
    bool isCdsPropertyActive(const QString& property) const;

    /*!
     * \brief Indicates if the specified property is found and active.
     *
     * \param property specifies the name of the property.
     *
     * \return \e true if the specified property is found and active.
     */
    bool isCdsPropertyActive(HCdsProperties::Property property) const;

    /*!
     * \brief Indicates if the object has the mandatory properties set.
     *
     * \return \e true in case the object has the mandatory properties set.
     */
    bool isValid() const;

    /*!
     * \brief Returns the ID of this object.
     *
     * \return The ID of this object.
     */
    QString id() const;

    /*!
     * \brief Indicates if the object might ever contain playable content.
     *
     * \return \e false if the object might contain playable content.
     * In a container object this applies recursively to all child items and
     * containers.
     *
     * \sa setNeverPlayable()
     */
    bool neverPlayable() const;

    /*!
     * \brief Returns the ID of the object that contains this object.
     *
     * \return The ID of the object that contains this
     * object.
     *
     * \sa setParentId()
     */
    QString parentId() const;

    /*!
     * \brief Indicates whether the object is modifiable.
     *
     * \return \e true if the object is \b not modifiable.
     *
     * \sa setRestricted()
     */
    bool isRestricted() const;

    /*!
     * \brief Returns the object update ID.
     *
     * \return The object update ID, which is the value of the state variable
     * \c SystemUpdateID at the time of the most recent \e Object \e Modification
     * of this instance.
     *
     * \remarks This is always zero only if isTrackChangesOptionEnabled()
     * returns \c false. Note, however, zero is a valid value when the
     * Track Changes Option is enabled as well.
     *
     * \sa setObjectUpdateId()
     */
    quint32 objectUpdateId() const;

    /*!
     * \brief Returns the title of this object.
     *
     * \return The title of this object.
     *
     * \sa setTitle()
     */
    QString title() const;

    /*!
     * \brief Returns the modifiability of the resources of this object.
     *
     * \return The modifiability of the resources of this object.
     *
     * \sa setWriteStatus()
     */
    WriteStatus writeStatus() const;

    /*!
     * \brief Returns the creator of this object.
     *
     * \return The creator of this object.
     *
     * \sa setCreator()
     */
    QString creator() const;

    /*!
     * \brief Returns the resources associated with this object.
     *
     * \return The resources associated with this object.
     *
     * \sa setResources()
     */
    HResources resources() const;

    /*!
     * \brief Returns the "CDS class" of the object.
     *
     * \return The "CDS class" of the object.
     *
     * \remarks If the type is defined by HUPnPAv, the return value will be
     * a value defined by the AV Working Committee. Otherwise the CDS class
     * is vendor defined and the only requirement for it is that it starts
     * with the identifier "object." and is followed by a non-empty string.
     */
    QString clazz() const;

    /*!
     * \brief Returns the CDS type of this object.
     *
     * \return The CDS type of this object.
     */
    CdsType type() const;

    /*!
     * \brief Indicates if the object is derived HItem.
     *
     * \return \e true if the object is derived from HItem.
     *
     * \sa isContainer()
     */
    bool isItem() const;

    /*!
     * \brief Indicates if the object is derived from HContainer.
     *
     * \return \e true if the object is derived from HContainer.
     *
     * \sa isItem()
     */
    bool isContainer() const;

    /*!
     * \brief Indicates if the Track Changes Option (TCO) is enabled.
     *
     * \return \e true if the Track Changes Option (TCO) is enabled.
     */
    bool isTrackChangesOptionEnabled() const;

    /*!
     * \brief Specifies whether the object might ever contain playable content.
     *
     * \param arg specifies whether the object might ever contain playable
     * content. If \a arg is \e true, the object may \b never contain
     * playable content.
     *
     * \sa neverPlayable()
     */
    void setNeverPlayable(bool arg);

    /*!
     * \brief Sets the ID of the object that contains this object.
     *
     * \param arg specifies the ID of the object that contains this object.
     *
     * \sa parentId()
     */
    void setParentId(const QString& arg);

    /*!
     * \brief Specifies whether the object is modifiable.
     *
     * \param restricted specifies whether the object is modifiable.
     *
     * \sa restricted()
     */
    void setRestricted(bool restricted);

    /*!
     * \brief Sets the title.
     *
     * \param arg specifies the title.
     *
     * \sa title()
     */
    void setTitle(const QString& arg);

    /*!
     * \brief Sets the creator of this object.
     *
     * \param arg specifies the creator of this object.
     *
     * \sa creator()
     */
    void setCreator(const QString& arg);

    /*!
     * \brief Specifies the object update ID.
     *
     * \param arg specifies the object update ID.
     *
     * \remarks This will be set if and only if isTrackChangesOptionEnabled()
     * returns \e true.
     *
     * \sa objectUpdateId()
     */
    void setObjectUpdateId(quint32 arg);

    /*!
     * \brief Sets the resources associated with this object.
     *
     * \param arg specifies the resources associated with this object.
     *
     * \sa resources()
     */
    void setResources(const QList<HResource>& arg);

    /*!
     * Enables or disables the Track Changes Option (TCO).
     *
     * \param arg specifies whether the TCO is enabled.
     */
    void setTrackChangesOption(bool arg);

    /*!
     * \brief Sets the modifiability of the resources of this object.
     *
     * \param arg specifies the modifiability of the resources of this object.
     *
     * \sa writeStatus()
     */
    void setWriteStatus(WriteStatus arg);

    /*!
     * Statically casts the object to HContainer, if it is derived from it.
     *
     * \return a pointer to this object statically cast to HContainer, if this
     * is derived from it. Otherwise a null pointer is returned.
     *
     * \sa asItem()
     */
    HContainer* asContainer();

    /*!
     * Statically casts the object to HContainer, if it is derived from it.
     *
     * \return a pointer to this object statically cast to HContainer, if this
     * is derived from it. Otherwise a null pointer is returned.
     *
     * \sa asItem()
     */
    const HContainer* asContainer() const;

    /*!
     * Statically casts the object to HItem, if it is derived from it.
     *
     * \return a pointer to this object statically cast to HItem, if this
     * is derived from it. Otherwise a null pointer is returned.
     *
     * \sa asContainer()
     */
    HItem* asItem();

    /*!
     * Statically casts the object to HItem, if it is derived from it.
     *
     * \return a pointer to this object statically cast to HItem, if this
     * is derived from it. Otherwise a null pointer is returned.
     *
     * \sa asContainer()
     */
    const HItem* asItem() const;

    /*!
     * \brief Indicates if the specified CdsType is a derivative of \c object.item.
     *
     * \return \e true if the specified CdsType is a derivative of \c object.item.
     *
     * \sa isContainer()
     */
    inline static bool isItem(CdsType type)
    {
        return type < Container;
    }

    /*!
     * \brief Indicates if the specified CdsType is a derivative of \c object.container.
     *
     * \return \e true if the specified CdsType is a derivative of
     * \c object.container.
     *
     * \sa isItem()
     */
    inline static bool isContainer(CdsType type)
    {
        return type >= Container;
    }

    /*!
     * \brief Returns the class identifier specified by the AV Working Committee.
     *
     * \return The class identifier specified by the AV Working Committee.
     */
    inline static QString sClass() { return "object"; }

    /*!
     * Creates a copy of the specified HObjects.
     *
     * \return a copy of the specified HObjects.
     *
     * \note The ownership of the returned objects is transferred to the caller.
     */
    template<typename T>
    static QList<T> clone(const QList<T>& objects)
    {
        QList<T> retVal;
        foreach(HObject* obj, objects)
        {
            retVal.append(qobject_cast<T>(obj->clone()));
        }
        return retVal;
    }

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when the object has been modified.
     *
     * \param source specifies the HObject that sent the event.
     *
     * \param eventInfo specifies information of the \e Object \e Modification that
     * occurred.
     */
    void objectModified(
        Herqq::Upnp::Av::HObject* source,
        const Herqq::Upnp::Av::HObjectEventInfo& eventInfo);
};

}
}
}

Q_DECLARE_METATYPE(Herqq::Upnp::Av::HObject::WriteStatus)

#endif /* HOBJECT_H_ */
