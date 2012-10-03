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
  @file    datasets.hpp
  @brief   IPTC dataset and type information
  @version $Rev: 2681 $
  @author  Brad Schick (brad) <brad@robotbattle.com>
  @date    24-Jul-04, brad: created
 */
#ifndef DATASETS_HPP_
#define DATASETS_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "metadatum.hpp"

// + standard includes
#include <string>
#include <utility>                              // for std::pair
#include <iosfwd>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    //! Details of an IPTC record.
    struct EXIV2API RecordInfo {
        //! Constructor
        RecordInfo(uint16_t recordId, const char* name, const char* desc);
        uint16_t recordId_;                     //!< Record id
        const char* name_;                      //!< Record name (one word)
        const char* desc_;                      //!< Record description
    };

    //! Details of an IPTC dataset.
    struct EXIV2API DataSet {
        //! Constructor
        DataSet(
            uint16_t number,
            const char* name,
            const char* title,
            const char* desc,
            bool mandatory,
            bool repeatable,
            uint32_t minbytes,
            uint32_t maxbytes,
            TypeId type,
            uint16_t recordId,
            const char* photoshop
        );
        uint16_t number_;                       //!< Dataset number
        const char* name_;                      //!< Dataset name
        const char* title_;                     //!< Dataset title or label
        const char* desc_;                      //!< Dataset description
        bool mandatory_;                        //!< True if dataset is mandatory
        bool repeatable_;                       //!< True if dataset is repeatable
        uint32_t minbytes_;                     //!< Minimum number of bytes
        uint32_t maxbytes_;                     //!< Maximum number of bytes
        TypeId type_;                           //!< Exiv2 default type
        uint16_t recordId_;                     //!< Record id
        const char* photoshop_;                 //!< Photoshop string
    }; // struct DataSet

    //! IPTC dataset reference, implemented as a static class.
    class EXIV2API IptcDataSets {
    public:
        /*!
          @name Record identifiers
          @brief Record identifiers to logically group dataSets. There are other
                 possible record types, but they are not standardized by the IPTC
                 IIM4 standard (and not commonly used in images).
         */
        //@{
        static const uint16_t invalidRecord = 0;
        static const uint16_t envelope = 1;
        static const uint16_t application2 = 2;
        //@}

        //! @name Dataset identifiers
        //@{
        static const uint16_t ModelVersion           = 0;
        static const uint16_t Destination            = 5;
        static const uint16_t FileFormat             = 20;
        static const uint16_t FileVersion            = 22;
        static const uint16_t ServiceId              = 30;
        static const uint16_t EnvelopeNumber         = 40;
        static const uint16_t ProductId              = 50;
        static const uint16_t EnvelopePriority       = 60;
        static const uint16_t DateSent               = 70;
        static const uint16_t TimeSent               = 80;
        static const uint16_t CharacterSet           = 90;
        static const uint16_t UNO                    = 100;
        static const uint16_t ARMId                  = 120;
        static const uint16_t ARMVersion             = 122;
        static const uint16_t RecordVersion          = 0;
        static const uint16_t ObjectType             = 3;
        static const uint16_t ObjectAttribute        = 4;
        static const uint16_t ObjectName             = 5;
        static const uint16_t EditStatus             = 7;
        static const uint16_t EditorialUpdate        = 8;
        static const uint16_t Urgency                = 10;
        static const uint16_t Subject                = 12;
        static const uint16_t Category               = 15;
        static const uint16_t SuppCategory           = 20;
        static const uint16_t FixtureId              = 22;
        static const uint16_t Keywords               = 25;
        static const uint16_t LocationCode           = 26;
        static const uint16_t LocationName           = 27;
        static const uint16_t ReleaseDate            = 30;
        static const uint16_t ReleaseTime            = 35;
        static const uint16_t ExpirationDate         = 37;
        static const uint16_t ExpirationTime         = 38;
        static const uint16_t SpecialInstructions    = 40;
        static const uint16_t ActionAdvised          = 42;
        static const uint16_t ReferenceService       = 45;
        static const uint16_t ReferenceDate          = 47;
        static const uint16_t ReferenceNumber        = 50;
        static const uint16_t DateCreated            = 55;
        static const uint16_t TimeCreated            = 60;
        static const uint16_t DigitizationDate       = 62;
        static const uint16_t DigitizationTime       = 63;
        static const uint16_t Program                = 65;
        static const uint16_t ProgramVersion         = 70;
        static const uint16_t ObjectCycle            = 75;
        static const uint16_t Byline                 = 80;
        static const uint16_t BylineTitle            = 85;
        static const uint16_t City                   = 90;
        static const uint16_t SubLocation            = 92;
        static const uint16_t ProvinceState          = 95;
        static const uint16_t CountryCode            = 100;
        static const uint16_t CountryName            = 101;
        static const uint16_t TransmissionReference  = 103;
        static const uint16_t Headline               = 105;
        static const uint16_t Credit                 = 110;
        static const uint16_t Source                 = 115;
        static const uint16_t Copyright              = 116;
        static const uint16_t Contact                = 118;
        static const uint16_t Caption                = 120;
        static const uint16_t Writer                 = 122;
        static const uint16_t RasterizedCaption      = 125;
        static const uint16_t ImageType              = 130;
        static const uint16_t ImageOrientation       = 131;
        static const uint16_t Language               = 135;
        static const uint16_t AudioType              = 150;
        static const uint16_t AudioRate              = 151;
        static const uint16_t AudioResolution        = 152;
        static const uint16_t AudioDuration          = 153;
        static const uint16_t AudioOutcue            = 154;
        static const uint16_t PreviewFormat          = 200;
        static const uint16_t PreviewVersion         = 201;
        static const uint16_t Preview                = 202;
        //@}

    private:
        //! Prevent construction: not implemented.
        IptcDataSets() {}
        //! Prevent copy-construction: not implemented.
        IptcDataSets(const IptcDataSets& rhs);
        //! Prevent assignment: not implemented.
        IptcDataSets& operator=(const IptcDataSets& rhs);

    public:
        /*!
          @brief Return the name of the dataset.
          @param number The dataset number
          @param recordId The IPTC record Id
          @return The name of the dataset or a string containing the hexadecimal
                  value of the dataset in the form "0x01ff", if this is an unknown
                  dataset.
         */
        static std::string dataSetName(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the title (label) of the dataset.
          @param number The dataset number
          @param recordId The IPTC record Id
          @return The title (label) of the dataset
         */
        static const char* dataSetTitle(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the description of the dataset.
          @param number The dataset number
          @param recordId The IPTC record Id
          @return The description of the dataset
         */
        static const char* dataSetDesc(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the photohsop name of a given dataset.
          @param number The dataset number
          @param recordId The IPTC record Id
          @return The name used by photoshop for a dataset or an empty
                 string if photoshop does not use the dataset.
         */
        static const char* dataSetPsName(uint16_t number, uint16_t recordId);
        /*!
          @brief Check if a given dataset is repeatable
          @param number The dataset number
          @param recordId The IPTC record Id
          @return true if the given dataset is repeatable otherwise false
         */
        static bool dataSetRepeatable(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the dataSet number for dataset name and record id

          @param dataSetName dataSet name
          @param recordId recordId

          @return dataSet number

          @throw Error if the \em dataSetName or \em recordId are invalid
         */
        static uint16_t dataSet(const std::string& dataSetName, uint16_t recordId);
        //! Return the type for dataSet number and Record id
        static TypeId dataSetType(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the name of the Record
          @param recordId The record id
          @return The name of the record or a string containing the hexadecimal
                  value of the record in the form "0x01ff", if this is an
                  unknown record.
         */
        static std::string recordName(uint16_t recordId);
        /*!
           @brief Return the description of a record
           @param recordId Record Id number
           @return the description of the Record
         */
        static const char* recordDesc(uint16_t recordId);
        /*!
           @brief Return the Id number of a record
           @param recordName Name of a record type
           @return the Id number of a Record
           @throw Error if the record is not known;
         */
        static uint16_t recordId(const std::string& recordName);
        //! Return read-only list of built-in Envelope Record datasets
        static const DataSet* envelopeRecordList();
        //! Return read-only list of built-in Application2 Record datasets
        static const DataSet* application2RecordList();
        //! Print a list of all dataSets to output stream
        static void dataSetList(std::ostream& os);

    private:
        static int dataSetIdx(uint16_t number, uint16_t recordId);
        static int dataSetIdx(const std::string& dataSetName, uint16_t recordId);

        static const DataSet* records_[];
        static const RecordInfo recordInfo_[];

    }; // class IptcDataSets

    /*!
      @brief Concrete keys for IPTC metadata.
     */
    class EXIV2API IptcKey : public Key {
    public:
        //! Shortcut for an %IptcKey auto pointer.
        typedef std::auto_ptr<IptcKey> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor to create an IPTC key from a key string.

          @param key The key string.
          @throw Error if the first part of the key is not '<b>Iptc</b>' or
                 the remaining parts of the key cannot be parsed and
                 converted to a record name and a dataset name.
        */
        explicit IptcKey(const std::string& key);
        /*!
          @brief Constructor to create an IPTC key from dataset and record ids.
          @param tag Dataset id
          @param record Record id
         */
        IptcKey(uint16_t tag, uint16_t record);
        //! Copy constructor
        IptcKey(const IptcKey& rhs);
        //! Destructor
        virtual ~IptcKey();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator.
         */
        IptcKey& operator=(const IptcKey& rhs);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const;
        virtual const char* familyName() const;
        /*!
          @brief Return the name of the group (the second part of the key).
                 For IPTC keys, the group name is the record name.
        */
        virtual std::string groupName() const;
        virtual std::string tagName() const;
        virtual std::string tagLabel() const;
        virtual uint16_t tag() const;
        AutoPtr clone() const;
        //! Return the name of the record
        std::string recordName() const;
        //! Return the record id
        uint16_t record() const;
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Set the key corresponding to the dataset and record id.
                 The key is of the form '<b>Iptc</b>.recordName.dataSetName'.
         */
        void makeKey();
        /*!
          @brief Parse and convert the key string into dataset and record id.
                 Updates data members if the string can be decomposed, or throws
                 \em Error.

          @throw Error if the key cannot be decomposed.
         */
        void decomposeKey();
        //@}

    private:
        //! Internal virtual copy constructor.
        EXV_DLLLOCAL virtual IptcKey* clone_() const;

        // DATA
        static const char* familyName_;

        uint16_t tag_;                 //!< Tag value
        uint16_t record_;              //!< Record value
        std::string key_;              //!< Key

    }; // class IptcKey

// *****************************************************************************
// free functions

    //! Output operator for dataSet
    EXIV2API std::ostream& operator<<(std::ostream& os, const DataSet& dataSet);

}                                       // namespace Exiv2

#endif                                  // #ifndef DATASETS_HPP_
