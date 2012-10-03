// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    metadatum.hpp
  @brief   Provides abstract base classes Metadatum and Key
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad)
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    09-Jan-04, ahu: created<BR>
           31-Jul-04, brad: isolated as a component<BR>
           23-Aug-04, ahu: added Key
 */
#ifndef METADATUM_HPP_
#define METADATUM_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class defining the %Key of a metadatum.
             Keys are used to identify and group metadata.
    */
    class EXIV2API Key {
    public:
        //! Shortcut for a %Key auto pointer.
        typedef std::auto_ptr<Key> AutoPtr;

        //! @name Creators
        //@{
        //! Destructor
        virtual ~Key();
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the key of the metadatum as a string. The key is of the
                 form 'familyName.groupName.tagName'. Note however that the
                 key is not necessarily unique, e.g., an ExifData may contain
                 multiple metadata with the same key.
         */
        virtual std::string key() const =0;
        //! Return an identifier for the type of metadata (the first part of the key)
        virtual const char* familyName() const =0;
        //! Return the name of the group (the second part of the key)
        virtual std::string groupName() const =0;
        //! Return the name of the tag (which is also the third part of the key)
        virtual std::string tagName() const =0;
        //! Return a label for the tag
        virtual std::string tagLabel() const =0;
        //! Return the tag number
        virtual uint16_t tag() const =0;
        /*!
          @brief Return an auto-pointer to a copy of itself (deep copy).
                 The caller owns this copy and the auto-pointer ensures that it
                 will be deleted.
         */
        AutoPtr clone() const;
        /*!
          @brief Write the key to an output stream. You do not usually have
                 to use this function; it is used for the implementation of
                 the output operator for %Key,
                 operator<<(std::ostream &os, const Key &key).
        */
        std::ostream& write(std::ostream& os) const { return os << key(); }
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        Key& operator=(const Key& rhs);
        //@}

    private:
        //! Internal virtual copy constructor.
        EXV_DLLLOCAL virtual Key* clone_() const =0;

    }; // class Key

    //! Output operator for Key types
    inline std::ostream& operator<<(std::ostream& os, const Key& key)
    {
        return key.write(os);
    }

    /*!
      @brief Abstract base class defining the interface to access information
             related to one metadata tag.
     */
    class EXIV2API Metadatum {
    public:
        //! @name Creators
        //@{
        //! Default Constructor
        Metadatum();
        //! Copy constructor
        Metadatum(const Metadatum& rhs);
        //! Destructor
        virtual ~Metadatum();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the value. This method copies (clones) the value pointed
                 to by pValue.
         */
        virtual void setValue(const Value* pValue) =0;
        /*!
          @brief Set the value to the string buf.
                 Uses Value::read(const std::string& buf). If the metadatum does
                 not have a value yet, then one is created. See subclasses for
                 more details. Return 0 if the value was read successfully.
         */
        virtual int setValue(const std::string& buf) =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write the interpreted value to a string.

          Implemented in terms of write(), see there.
         */
        std::string print(const ExifData* pMetadata =0) const;
        /*!
          @brief Write value to a data buffer and return the number
                 of bytes written.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        virtual long copy(byte* buf, ByteOrder byteOrder) const =0;
        /*!
          @brief Write the interpreted value to an output stream, return
                 the stream.

          The method takes an optional pointer to a metadata container.
          Pretty-print functions may use that to refer to other metadata as it
          is sometimes not sufficient to know only the value of the metadatum
          that should be interpreted. Thus, it is advisable to always call this
          method with a pointer to the metadata container if possible.

          This functionality is currently only implemented for Exif tags.
          The pointer is ignored when used to write IPTC datasets or XMP
          properties.

          Without the optional metadata pointer, you do not usually have to use
          this function; it is used for the implementation of the output
          operator for %Metadatum,
          operator<<(std::ostream &os, const Metadatum &md).

          See also print(), which prints the interpreted value to a string.
         */
        virtual std::ostream& write(
                  std::ostream& os,
            const ExifData*     pMetadata =0
        ) const =0;
        /*!
          @brief Return the key of the metadatum. The key is of the form
                 'familyName.groupName.tagName'. Note however that the key
                 is not necessarily unique, e.g., an ExifData object may
                 contain multiple metadata with the same key.
         */
        virtual std::string key() const =0;
        //! Return the name of the metadata family (which is also the first part of the key)
        virtual const char* familyName() const =0;
        //! Return the name of the metadata group (which is also the second part of the key)
        virtual std::string groupName() const =0;
        //! Return the name of the tag (which is also the third part of the key)
        virtual std::string tagName() const =0;
        //! Return a label for the tag 	
        virtual std::string tagLabel() const =0;
        //! Return the tag
        virtual uint16_t tag() const =0;
        //! Return the type id of the value
        virtual TypeId typeId() const =0;
        //! Return the name of the type
        virtual const char* typeName() const =0;
        //! Return the size in bytes of one component of this type
        virtual long typeSize() const =0;
        //! Return the number of components in the value
        virtual long count() const =0;
        //! Return the size of the value in bytes
        virtual long size() const =0;
        //! Return the value as a string.
        virtual std::string toString() const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to
                 a string. The behaviour of the method is undefined if there
                 is no <EM>n</EM>-th component.
         */
        virtual std::string toString(long n) const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to long.
                 The return value is -1 if the value is not set and the behaviour
                 of the method is undefined if there is no <EM>n</EM>-th component.
         */
        virtual long toLong(long n =0) const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to float.
                 The return value is -1 if the value is not set and the behaviour
                 of the method is undefined if there is no <EM>n</EM>-th component.
         */
        virtual float toFloat(long n =0) const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to Rational.
                 The return value is -1/1 if the value is not set and the behaviour
                 of the method is undefined if there is no <EM>n</EM>-th component.
         */
        virtual Rational toRational(long n =0) const =0;
        /*!
          @brief Return an auto-pointer to a copy (clone) of the value. The
                 caller owns this copy and the auto-poiner ensures that it will
                 be deleted.

          This method is provided for users who need full control over the
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.

          @return An auto-pointer containing a pointer to a copy (clone) of the
                  value, 0 if the value is not set.
         */
        virtual Value::AutoPtr getValue() const =0;
        /*!
          @brief Return a constant reference to the value.

          This method is provided mostly for convenient and versatile output of
          the value which can (to some extent) be formatted through standard
          stream manipulators.  Do not attempt to write to the value through
          this reference. An Error is thrown if the value is not set; as an
          alternative to catching it, one can use count() to check if there
          is any data before calling this method.

          @return A constant reference to the value.
          @throw Error if the value is not set.
         */
        virtual const Value& value() const =0;
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        Metadatum& operator=(const Metadatum& rhs);
        //@}

    }; // class Metadatum

    /*!
      @brief Output operator for Metadatum types, writing the interpreted
             tag value.
     */
    inline std::ostream& operator<<(std::ostream& os, const Metadatum& md)
    {
        return md.write(os);
    }

    /*!
      @brief Compare two metadata by tag. Return true if the tag of metadatum
             lhs is less than that of rhs.
     */
    EXIV2API bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs);
    /*!
      @brief Compare two metadata by key. Return true if the key of metadatum
             lhs is less than that of rhs.
     */
    EXIV2API bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs);

}                                       // namespace Exiv2

#endif                                  // #ifndef METADATUM_HPP_
