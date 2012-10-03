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
  @file    xmp.hpp
  @brief   Encoding and decoding of XMP data
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    13-Jul-07, ahu: created
 */
#ifndef XMP_HPP_
#define XMP_HPP_

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "properties.hpp"
#include "value.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;

// *****************************************************************************
// class definitions

    /*!
      @brief Information related to an XMP property. An XMP metadatum consists
             of an XmpKey and a Value and provides methods to manipulate these.
     */
    class EXIV2API Xmpdatum : public Metadatum {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for new tags created by an application. The
                 %Xmpdatum is created from a key / value pair. %Xmpdatum
                 copies (clones) the value if one is provided. Alternatively, a
                 program can create an 'empty' %Xmpdatum with only a key and
                 set the value using setValue().

          @param key The key of the %Xmpdatum.
          @param pValue Pointer to a %Xmpdatum value.
          @throw Error if the key cannot be parsed and converted
                 to a known schema namespace prefix and property name.
         */
        explicit Xmpdatum(const XmpKey& key,
                          const Value* pValue =0);
        //! Copy constructor
        Xmpdatum(const Xmpdatum& rhs);
        //! Destructor
        virtual ~Xmpdatum();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Xmpdatum& operator=(const Xmpdatum& rhs);
        /*!
          @brief Assign std::string \em value to the %Xmpdatum.
                 Calls setValue(const std::string&).
         */
        Xmpdatum& operator=(const std::string& value);
        /*!
          @brief Assign const char* \em value to the %Xmpdatum.
                 Calls operator=(const std::string&).
         */
        Xmpdatum& operator=(const char* value);
        /*!
          @brief Assign a boolean \em value to the %Xmpdatum.
                 Translates the value to a string "true" or "false".
         */
        Xmpdatum& operator=(const bool& value);
        /*!
          @brief Assign a \em value of any type with an output operator
                 to the %Xmpdatum. Calls operator=(const std::string&).
         */
        template<typename T>
        Xmpdatum& operator=(const T& value);
        /*!
          @brief Assign Value \em value to the %Xmpdatum.
                 Calls setValue(const Value*).
         */
        Xmpdatum& operator=(const Value& value);
        void setValue(const Value* pValue);
        /*!
          @brief Set the value to the string \em value. Uses Value::read(const
                 std::string&).  If the %Xmpdatum does not have a Value yet,
                 then a %Value of the correct type for this %Xmpdatum is
                 created. If the key is unknown, a XmpTextValue is used as
                 default. Return 0 if the value was read successfully.
         */
        int setValue(const std::string& value);
        //@}

        //! @name Accessors
        //@{
        //! Not implemented. Calling this method will raise an exception.
        long copy(byte* buf, ByteOrder byteOrder) const;
        std::ostream& write(std::ostream& os, const ExifData* pMetadata =0) const;
        /*!
          @brief Return the key of the Xmpdatum. The key is of the form
                 '<b>Xmp</b>.prefix.property'. Note however that the
                 key is not necessarily unique, i.e., an XmpData object may
                 contain multiple metadata with the same key.
         */
        std::string key() const;
        const char* familyName() const;
        //! Return the (preferred) schema namespace prefix.
        std::string groupName() const;
        //! Return the property name.
        std::string tagName() const;
        std::string tagLabel() const;
        //! Properties don't have a tag number. Return 0.
        uint16_t tag() const;
        TypeId typeId() const;
        const char* typeName() const;
        // Todo: Remove this method from the baseclass
        //! The Exif typeSize doesn't make sense here. Return 0.
        long typeSize() const;
        long count() const;
        long size() const;
        std::string toString() const;
        std::string toString(long n) const;
        long toLong(long n =0) const;
        float toFloat(long n =0) const;
        Rational toRational(long n =0) const;
        Value::AutoPtr getValue() const;
        const Value& value() const;
        //@}

    private:
        // Pimpl idiom
        struct Impl;
        Impl* p_;

    }; // class Xmpdatum

    //! Container type to hold all metadata
    typedef std::vector<Xmpdatum> XmpMetadata;

    /*!
      @brief A container for XMP data. This is a top-level class of
             the %Exiv2 library.

      Provide high-level access to the XMP data of an image:
      - read XMP information from an XML block
      - access metadata through keys and standard C++ iterators
      - add, modify and delete metadata
      - serialize XMP data to an XML block
    */
    class EXIV2API XmpData {
    public:
        //! XmpMetadata iterator type
        typedef XmpMetadata::iterator iterator;
        //! XmpMetadata const iterator type
        typedef XmpMetadata::const_iterator const_iterator;

        //! @name Manipulators
        //@{
        /*!
          @brief Returns a reference to the %Xmpdatum that is associated with a
                 particular \em key. If %XmpData does not already contain such
                 an %Xmpdatum, operator[] adds object \em Xmpdatum(key).

          @note  Since operator[] might insert a new element, it can't be a const
                 member function.
         */
        Xmpdatum& operator[](const std::string& key);
        /*!
          @brief Add an %Xmpdatum from the supplied key and value pair. This
                 method copies (clones) the value.
          @return 0 if successful.
         */
        int add(const XmpKey& key, const Value* value);
        /*!
          @brief Add a copy of the Xmpdatum to the XMP metadata.
          @return 0 if successful.
         */
        int add(const Xmpdatum& xmpdatum);
        /*!
          @brief Delete the Xmpdatum at iterator position pos, return the
                 position of the next Xmpdatum.

          @note  Iterators into the metadata, including pos, are potentially
                 invalidated by this call.
         */
        iterator erase(iterator pos);
        //! Delete all Xmpdatum instances resulting in an empty container.
        void clear();
        //! Sort metadata by key
        void sortByKey();
        //! Begin of the metadata
        iterator begin();
        //! End of the metadata
        iterator end();
        /*!
          @brief Find the first Xmpdatum with the given key, return an iterator
                 to it.
         */
        iterator findKey(const XmpKey& key);
        //@}

        //! @name Accessors
        //@{
        //! Begin of the metadata
        const_iterator begin() const;
        //! End of the metadata
        const_iterator end() const;
        /*!
          @brief Find the first Xmpdatum with the given key, return a const
                 iterator to it.
         */
        const_iterator findKey(const XmpKey& key) const;
        //! Return true if there is no XMP metadata
        bool empty() const;
        //! Get the number of metadata entries
        long count() const;
        //@}

    private:
        // DATA
        XmpMetadata xmpMetadata_;
    }; // class XmpData

    /*!
      @brief Stateless parser class for XMP packets. Images use this
             class to parse and serialize XMP packets. The parser uses
             the XMP toolkit to do the job.
     */
    class EXIV2API XmpParser {
    public:
        //! Options to control the format of the serialized XMP packet.
        enum XmpFormatFlags {
            omitPacketWrapper   = 0x0010UL,  //!< Omit the XML packet wrapper.
            readOnlyPacket      = 0x0020UL,  //!< Default is a writeable packet.
            useCompactFormat    = 0x0040UL,  //!< Use a compact form of RDF.
            includeThumbnailPad = 0x0100UL,  //!< Include a padding allowance for a thumbnail image.
            exactPacketLength   = 0x0200UL,  //!< The padding parameter is the overall packet length.
            writeAliasComments  = 0x0400UL,  //!< Show aliases as XML comments.
            omitAllFormatting   = 0x0800UL   //!< Omit all formatting whitespace.
        };
        /*!
          @brief Decode XMP metadata from an XMP packet \em xmpPacket into
                 \em xmpData. The format of the XMP packet must follow the
                 XMP specification. This method clears any previous contents
                 of \em xmpData.

          @param xmpData   Container for the decoded XMP properties
          @param xmpPacket The raw XMP packet to decode
          @return 0 if successful;<BR>
                  1 if XMP support has not been compiled-in;<BR>
                  2 if the XMP toolkit failed to initialize;<BR>
                  3 if the XMP toolkit failed and raised an XMP_Error
        */
        static int decode(      XmpData&     xmpData,
                          const std::string& xmpPacket);
        /*!
          @brief Encode (serialize) XMP metadata from \em xmpData into a
                 string xmpPacket. The XMP packet returned in the string
                 follows the XMP specification. This method only modifies
                 \em xmpPacket if the operations succeeds (return code 0).

          @param xmpPacket   Reference to a string to hold the encoded XMP
                             packet.
          @param xmpData     XMP properties to encode.
          @param formatFlags Flags that control the format of the XMP packet,
                             see enum XmpFormatFlags.
          @param padding     Padding length.
          @return 0 if successful;<BR>
                  1 if XMP support has not been compiled-in;<BR>
                  2 if the XMP toolkit failed to initialize;<BR>
                  3 if the XMP toolkit failed and raised an XMP_Error
        */
        static int encode(      std::string& xmpPacket,
                          const XmpData&     xmpData,
                                uint16_t     formatFlags =useCompactFormat,
                                uint32_t     padding =0);
        /*!
          @brief Lock/unlock function type

          A function of this type can be passed to initialize() to
          make subsequent registration of XMP namespaces thread-safe.
          See the initialize() function for more information.

          @param pLockData Pointer to the pLockData passed to initialize()
          @param lockUnlock Indicates whether to lock (true) or unlock (false)
         */
        typedef void (*XmpLockFct)(void* pLockData, bool lockUnlock);

        /*!
          @brief Initialize the XMP Toolkit.

          Calling this method is usually not needed, as encode() and
          decode() will initialize the XMP Toolkit if necessary.

          The function takes optional pointers to a callback function
          \em xmpLockFct and related data \em pLockData that the parser
          uses when XMP namespaces are subsequently registered.

          The initialize() function itself still is not thread-safe and
          needs to be called in a thread-safe manner (e.g., on program
          startup), but if used with suitable additional locking
          parameters, any subsequent registration of namespaces will be
          thread-safe.

          Example usage on Windows using a critical section:

          @code
          void main()
          {
              struct XmpLock
              {
                  CRITICAL_SECTION cs;
                  XmpLock()  { InitializeCriticalSection(&cs); }
                  ~XmpLock() { DeleteCriticalSection(&cs); }

                  static void LockUnlock(void* pData, bool fLock)
                  {
                      XmpLock* pThis = reinterpret_cast<XmpLock*>(pData);
                      if (pThis)
                      {
                          (fLock) ? EnterCriticalSection(&pThis->cs)
                                  : LeaveCriticalSection(&pThis->cs);
                      }
                  }
              } xmpLock;

              // Pass the locking mechanism to the XMP parser on initialization.
              // Note however that this call itself is still not thread-safe.
              Exiv2::XmpParser::initialize(XmpLock::LockUnlock, &xmpLock);

              // Program continues here, subsequent registrations of XMP 
              // namespaces are serialized using xmpLock.

          }
          @endcode

          @return True if the initialization was successful, else false.
         */
        static bool initialize(XmpParser::XmpLockFct xmpLockFct =0, void* pLockData =0);
        /*!
          @brief Terminate the XMP Toolkit and unregister custom namespaces.

          Call this method when the XmpParser is no longer needed to
          allow the XMP Toolkit to cleanly shutdown.
         */
        static void terminate();

    private:
        /*!
          @brief Register a namespace with the XMP Toolkit.
         */
        static void registerNs(const std::string& ns,
                               const std::string& prefix);
        /*!
          @brief Delete a namespace from the XMP Toolkit.

          XmpProperties::unregisterNs calls this to synchronize namespaces.
        */
        static void unregisterNs(const std::string& ns);

        // DATA
        static bool initialized_; //! Indicates if the XMP Toolkit has been initialized
        static XmpLockFct xmpLockFct_;
        static void* pLockData_;

    }; // class XmpParser

// *****************************************************************************
// free functions, template and inline definitions

    inline Xmpdatum& Xmpdatum::operator=(const char* value)
    {
        return Xmpdatum::operator=(std::string(value));
    }

    inline Xmpdatum& Xmpdatum::operator=(const bool& value)
    {
        return operator=(value ? "True" : "False");
    }

    template<typename T>
    Xmpdatum& Xmpdatum::operator=(const T& value)
    {
        setValue(Exiv2::toString(value));
        return *this;
    }

}                                       // namespace Exiv2

#endif                                  // #ifndef XMP_HPP_
