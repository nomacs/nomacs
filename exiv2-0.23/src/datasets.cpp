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
/*
  File:      datasets.cpp
  Version:   $Rev: 2681 $
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
             Gilles Caulier (gc) <caulier dot gilles at gmail dot com>
  History:   24-Jul-04, brad: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: datasets.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "datasets.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"
#include "metadatum.hpp"
#include "i18n.h"                // NLS support.

#include <iostream>
#include <iomanip>
#include <sstream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    DataSet::DataSet(
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
    )
        : number_(number), name_(name), title_(title), desc_(desc),
          mandatory_(mandatory), repeatable_(repeatable), minbytes_(minbytes),
          maxbytes_(maxbytes), type_(type), recordId_(recordId),
          photoshop_(photoshop)
    {
    }

    RecordInfo::RecordInfo(
        uint16_t recordId,
        const char* name,
        const char* desc
    )
        : recordId_(recordId), name_(name), desc_(desc)
    {
    }

    const RecordInfo IptcDataSets::recordInfo_[] = {
        RecordInfo(IptcDataSets::invalidRecord, "(invalid)", N_("(invalid)")),
        RecordInfo(IptcDataSets::envelope, "Envelope", N_("IIM envelope record")),
        RecordInfo(IptcDataSets::application2, "Application2", N_("IIM application record 2")),
    };

    static const DataSet envelopeRecord[] = {
        DataSet(IptcDataSets::ModelVersion, "ModelVersion", N_("Model Version"),
                N_("A binary number identifying the version of the Information "
                "Interchange Model, Part I, utilised by the provider. Version "
                "numbers are assigned by IPTC and NAA organizations."),
                true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::Destination, "Destination", N_("Destination"),
                N_("This DataSet is to accommodate some providers who require "
                "routing information above the appropriate OSI layers."),
                false, true, 0, 1024, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::FileFormat, "FileFormat", N_("File Format"),
                N_("A binary number representing the file format. The file format "
                "must be registered with IPTC or NAA with a unique number "
                "assigned to it. The information is used to route "
                "the data to the appropriate system and to allow the receiving "
                "system to perform the appropriate actions there to."),
                true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::FileVersion, "FileVersion", N_("File Version"),
                N_("A binary number representing the particular version of the File "
                "Format specified by <FileFormat> tag."),
                true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ServiceId, "ServiceId", N_("Service Id"),
                N_("Identifies the provider and product"),
                true, false, 0, 10, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::EnvelopeNumber, "EnvelopeNumber", N_("Envelope Number"),
                N_("The characters form a number that will be unique for the date "
                "specified in <DateSent> tag and for the Service Identifier "
                "specified by <ServiceIdentifier> tag. "
                "If identical envelope numbers appear with the same date and "
                "with the same Service Identifier, records 2-9 must be unchanged "
                "from the original. This is not intended to be a sequential serial "
                "number reception check."),
                true, false, 8, 8, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ProductId, "ProductId", N_("Product Id"),
                N_("Allows a provider to identify subsets of its overall service. Used "
                "to provide receiving organisation data on which to select, route, "
                "or otherwise handle data."),
                false, true, 0, 32, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::EnvelopePriority, "EnvelopePriority", N_("Envelope Priority"),
                N_("Specifies the envelope handling priority and not the editorial "
                "urgency (see <Urgency> tag). \"1\" indicates the most urgent, \"5\" "
                "the normal urgency, and \"8\" the least urgent copy. The numeral "
                "\"9\" indicates a User Defined Priority. The numeral \"0\" is reserved "
                "for future use."),
                false, false, 1, 1, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::DateSent, "DateSent", N_("Date Sent"),
                N_("Uses the format CCYYMMDD (century, year, month, day) as de-fined "
                "in ISO 8601 to indicate year, month and day the service sent the material."),
                true, false, 8, 8, Exiv2::date, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::TimeSent, "TimeSent", N_("Time Sent"),
                N_("Uses the format HHMMSS:HHMM where HHMMSS refers to "
                "local hour, minute and seconds and HHMM refers to hours and "
                "minutes ahead (+) or behind (-) Universal Coordinated Time as "
                "described in ISO 8601. This is the time the service sent the material."),
                false, false, 11, 11, Exiv2::time, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::CharacterSet, "CharacterSet", N_("Character Set"),
                N_("This tag consisting of one or more control functions used for the announcement, "
                "invocation or designation of coded character sets. The control functions follow "
                "the ISO 2022 standard and may consist of the escape control "
                "character and one or more graphic characters."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::UNO, "UNO", N_("Unique Name Object"),
                N_("This tag provide a globally unique "
                "identification for objects as specified in the IIM, independent of "
                "provider and for any media form. The provider must ensure the "
                "UNO is unique. Objects with the same UNO are identical."),
                false, false, 14, 80, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ARMId, "ARMId", N_("ARM Identifier"),
                N_("The DataSet identifies the Abstract Relationship Method identifier (ARM) "
                "which is described in a document registered by the originator of "
                "the ARM with the IPTC and NAA organizations."),
                false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ARMVersion, "ARMVersion", N_("ARM Version"),
                N_("This tag consisting of a binary number representing the particular "
                "version of the ARM specified by tag <ARMId>."),
                false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(0xffff, "(Invalid)", "(Invalid)",
                "(Invalid)", false, false, 0, 0, Exiv2::unsignedShort, IptcDataSets::envelope, "")
    };

    const DataSet* IptcDataSets::envelopeRecordList()
    {
        return envelopeRecord;
    }

    static const DataSet application2Record[] = {
        DataSet(IptcDataSets::RecordVersion, "RecordVersion", N_("Record Version"),
                N_("A binary number identifying the version of the Information "
                "Interchange Model, Part II, utilised by the provider. "
                "Version numbers are assigned by IPTC and NAA organizations."),
                true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectType, "ObjectType", N_("Object Type"),
                N_("The Object Type is used to distinguish between different types "
                "of objects within the IIM. The first part is a number representing "
                "a language independent international reference to an Object Type "
                "followed by a colon separator. The second part, if used, is a text "
                "representation of the Object Type Number consisting of graphic "
                "characters plus spaces either in English or in the language of the "
                "service as indicated in tag <LanguageIdentifier>"),
                false, false, 3, 67, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectAttribute, "ObjectAttribute", N_("Object Attribute"),
                N_("The Object Attribute defines the nature of the object "
                "independent of the Subject. The first part is a number representing "
                "a language independent international reference to an Object Attribute "
                "followed by a colon separator. The second part, if used, is a text "
                "representation of the Object Attribute Number consisting of graphic "
                "characters plus spaces either in English, or in the language of the "
                "service as indicated in tag <LanguageIdentifier>"),
                false, true, 4, 68, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectName, "ObjectName", N_("Object Name"),
                N_("Used as a shorthand reference for the object. Changes to exist-ing "
                "data, such as updated stories or new crops on photos, should be "
                "identified in tag <EditStatus>."),
                false, false, 0, 64, Exiv2::string, IptcDataSets::application2,
                N_("Document Title")),
        DataSet(IptcDataSets::EditStatus, "EditStatus", N_("Edit Status"),
                N_("Status of the object data, according to the practice of the provider."),
                false, false, 0, 64, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::EditorialUpdate, "EditorialUpdate", N_("Editorial Update"),
                N_("Indicates the type of update that this object provides to a "
                "previous object. The link to the previous object is made using "
                "the tags <ARMIdentifier> and <ARMVersion>, according to the practices of the provider."),
                false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Urgency, "Urgency", N_("Urgency"),
                N_("Specifies the editorial urgency of content and not necessarily the "
                "envelope handling priority (see tag <EnvelopePriority>). The \"1\" "
                "is most urgent, \"5\" normal and \"8\" denotes the least-urgent copy."),
                false, false, 1, 1, Exiv2::string, IptcDataSets::application2,
                N_("Urgency")),
        DataSet(IptcDataSets::Subject, "Subject", N_("Subject"),
                N_("The Subject Reference is a structured definition of the subject matter."),
                false, true, 13, 236, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Category, "Category", N_("Category"),
                N_("Identifies the subject of the object data in the opinion of the provider. "
                "A list of categories will be maintained by a regional registry, "
                "where available, otherwise by the provider."),
                false, false, 0, 3, Exiv2::string, IptcDataSets::application2,
                N_("Category")),
        DataSet(IptcDataSets::SuppCategory, "SuppCategory", N_("Supplemental Category"),
                N_("Supplemental categories further refine the subject of an "
                "object data. A supplemental category may include "
                "any of the recognised categories as used in tag <Category>. Otherwise, "
                "selection of supplemental categories are left to the provider."),
                false, true, 0, 32, Exiv2::string, IptcDataSets::application2,
                N_("Supplemental Categories")),
        DataSet(IptcDataSets::FixtureId, "FixtureId", N_("Fixture Id"),
                N_("Identifies object data that recurs often and predictably. Enables "
                "users to immediately find or recall such an object."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Keywords, "Keywords", N_("Keywords"),
                N_("Used to indicate specific information retrieval words. "
                "It is expected that a provider of various types of data that are related "
                "in subject matter uses the same keyword, enabling the receiving system "
                "or subsystems to search across all types of data for related material."),
                false, true, 0, 64, Exiv2::string, IptcDataSets::application2,
                N_("Keywords")),
        DataSet(IptcDataSets::LocationCode, "LocationCode", N_("Location Code"),
                N_("Indicates the code of a country/geographical location referenced "
                "by the content of the object. Where ISO has established an appropriate "
                "country code under ISO 3166, that code will be used. When ISO 3166 does not "
                "adequately provide for identification of a location or a country, "
                "e.g. ships at sea, space, IPTC will assign an appropriate three-character "
                "code under the provisions of ISO 3166 to avoid conflicts."),
                false, true, 3, 3, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::LocationName, "LocationName", N_("Location Name"),
                N_("Provides a full, publishable name of a country/geographical "
                "location referenced by the content of the object, according to "
                "guidelines of the provider."),
                false, true, 0, 64, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReleaseDate, "ReleaseDate", N_("Release Date"),
                N_("Designates in the form CCYYMMDD the earliest date the "
                "provider intends the object to be used. Follows ISO 8601 standard."),
                false, false, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReleaseTime, "ReleaseTime", N_("Release Time"),
                N_("Designates in the form HHMMSS:HHMM the earliest time the "
                "provider intends the object to be used. Follows ISO 8601 standard."),
                false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ExpirationDate, "ExpirationDate", N_("Expiration Date"),
                N_("Designates in the form CCYYMMDD the latest date the provider "
                "or owner intends the object data to be used. Follows ISO 8601 standard."),
                false, false, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ExpirationTime, "ExpirationTime", N_("ExpirationTime"),
                N_("Designates in the form HHMMSS:HHMM the latest time the "
                "provider or owner intends the object data to be used. Follows ISO 8601 standard."),
                false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::SpecialInstructions, "SpecialInstructions", N_("Special Instructions"),
                N_("Other editorial instructions concerning the use of the object data, "
                "such as embargoes and warnings."),
                false, false, 0, 256, Exiv2::string, IptcDataSets::application2,
                N_("Instructions")),
        DataSet(IptcDataSets::ActionAdvised, "ActionAdvised", N_("Action Advised"),
                N_("Indicates the type of action that this object provides to a "
                "previous object. The link to the previous object is made using "
                "tags <ARMIdentifier> and <ARMVersion>, according to the practices of the provider."),
                false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReferenceService, "ReferenceService", N_("Reference Service"),
                N_("Identifies the Service Identifier of a prior envelope to which the "
                "current object refers."),
                false, true, 0, 10, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReferenceDate, "ReferenceDate", N_("Reference Date"),
                N_("Identifies the date of a prior envelope to which the current object refers."),
                false, true, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReferenceNumber, "ReferenceNumber", N_("Reference Number"),
                N_("Identifies the Envelope Number of a prior envelope to which the current object refers."),
                false, true, 8, 8, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::DateCreated, "DateCreated", N_("Date Created"),
                N_("Represented in the form CCYYMMDD to designate the date the "
                "intellectual content of the object data was created rather than the "
                "date of the creation of the physical representation. Follows ISO 8601 standard."),
                false, false, 8, 8, Exiv2::date, IptcDataSets::application2,
                N_("Date Created")),
        DataSet(IptcDataSets::TimeCreated, "TimeCreated", N_("Time Created"),
                N_("Represented in the form HHMMSS:HHMM to designate the "
                "time the intellectual content of the object data current source "
                "material was created rather than the creation of the physical "
                "representation. Follows ISO 8601 standard."),
                false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::DigitizationDate, "DigitizationDate", N_("Digitization Date"),
                N_("Represented in the form CCYYMMDD to designate the date the "
                "digital representation of the object data was created. Follows ISO 8601 standard."),
                false, false, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::DigitizationTime, "DigitizationTime", N_("Digitization Time"),
                N_("Represented in the form HHMMSS:HHMM to designate the "
                "time the digital representation of the object data was created. "
                "Follows ISO 8601 standard."),
                false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Program, "Program", N_("Program"),
                N_("Identifies the type of program used to originate the object data."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ProgramVersion, "ProgramVersion", N_("Program Version"),
                N_("Used to identify the version of the program mentioned in tag <Program>."),
                false, false, 0, 10, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectCycle, "ObjectCycle", N_("Object Cycle"),
                N_("Used to identify the editorial cycle of object data."),
                false, false, 1, 1, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Byline, "Byline", N_("By-line"),
                N_("Contains name of the creator of the object data, e.g. writer, photographer "
                "or graphic artist."),
                false, true, 0, 32, Exiv2::string, IptcDataSets::application2,
                N_("Author")),
        DataSet(IptcDataSets::BylineTitle, "BylineTitle", N_("By-line Title"),
                N_("A by-line title is the title of the creator or creators of an "
                "object data. Where used, a by-line title should follow the by-line it modifies."),
                false, true, 0, 32, Exiv2::string, IptcDataSets::application2, "Authors Position"),
        DataSet(IptcDataSets::City, "City", N_("City"),
                N_("Identifies city of object data origin according to guidelines established "
                "by the provider."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2,
                N_("City")),
        DataSet(IptcDataSets::SubLocation, "SubLocation", N_("Sub Location"),
                N_("Identifies the location within a city from which the object data "
                "originates, according to guidelines established by the provider."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ProvinceState, "ProvinceState", N_("Province State"),
                N_("Identifies Province/State of origin according to guidelines "
                "established by the provider."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2,
                N_("State/Province")),
        DataSet(IptcDataSets::CountryCode, "CountryCode", N_("Country Code"),
                N_("Indicates the code of the country/primary location where the "
                "intellectual property of the object data was created, e.g. a photo "
                "was taken, an event occurred. Where ISO has established an appropriate "
                "country code under ISO 3166, that code will be used. When ISO 3166 does not "
                "adequately provide for identification of a location or a new "
                "country, e.g. ships at sea, space, IPTC will assign an "
                "appropriate three-character code under the provisions of "
                "ISO 3166 to avoid conflicts."),
                false, false, 3, 3, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::CountryName, "CountryName", N_("Country Name"),
                N_("Provides full, publishable, name of the country/primary location "
                "where the intellectual property of the object data was created, "
                "according to guidelines of the provider."),
                false, false, 0, 64, Exiv2::string, IptcDataSets::application2,
                N_("Country")),
        DataSet(IptcDataSets::TransmissionReference, "TransmissionReference", N_("Transmission Reference"),
                N_("A code representing the location of original transmission according "
                "to practices of the provider."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2,
                N_("Transmission Reference")),
        DataSet(IptcDataSets::Headline, "Headline", N_("Headline"),
                N_("A publishable entry providing a synopsis of the contents of the object data."),
                false, false, 0, 256, Exiv2::string, IptcDataSets::application2,
                N_("Headline")),
        DataSet(IptcDataSets::Credit, "Credit", N_("Credit"),
                N_("Identifies the provider of the object data, not necessarily the owner/creator."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2,
                N_("Credit")),
        DataSet(IptcDataSets::Source, "Source", N_("Source"),
                N_("Identifies the original owner of the intellectual content of the "
                "object data. This could be an agency, a member of an agency or an individual."),
                false, false, 0, 32, Exiv2::string, IptcDataSets::application2,
                N_("Source")),
        DataSet(IptcDataSets::Copyright, "Copyright", N_("Copyright"),
                N_("Contains any necessary copyright notice."),
                false, false, 0, 128, Exiv2::string, IptcDataSets::application2,
                N_("Copyright Notice")),
        DataSet(IptcDataSets::Contact, "Contact", N_("Contact"),
                N_("Identifies the person or organisation which can provide further "
                "background information on the object data."),
                false, true, 0, 128, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Caption, "Caption", N_("Caption"),
                N_("A textual description of the object data."),
                false, false, 0, 2000, Exiv2::string, IptcDataSets::application2,
                N_("Description")),
        DataSet(IptcDataSets::Writer, "Writer", N_("Writer"),
                N_("Identification of the name of the person involved in the writing, "
                "editing or correcting the object data or caption/abstract."),
                false, true, 0, 32, Exiv2::string, IptcDataSets::application2, "Description writer"),
        DataSet(IptcDataSets::RasterizedCaption, "RasterizedCaption", N_("Rasterized Caption"),
                N_("Contains the rasterized object data description and is used "
                "where characters that have not been coded are required for the caption."),
                false, false, 7360, 7360, Exiv2::undefined, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ImageType, "ImageType", N_("Image Type"),
                N_("Indicates the color components of an image."),
                false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ImageOrientation, "ImageOrientation", N_("Image Orientation"),
                N_("Indicates the layout of an image."),
                false, false, 1, 1, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Language, "Language", N_("Language"),
                N_("Describes the major national language of the object, according "
                "to the 2-letter codes of ISO 639:1988. Does not define or imply "
                "any coded character set, but is used for internal routing, e.g. to "
                "various editorial desks."),
                false, false, 2, 3, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioType, "AudioType", N_("Audio Type"),
                N_("Indicates the type of an audio content."),
                false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioRate, "AudioRate", N_("Audio Rate"),
                N_("Indicates the sampling rate in Hertz of an audio content."),
                false, false, 6, 6, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioResolution, "AudioResolution", N_("Audio Resolution"),
                N_("Indicates the sampling resolution of an audio content."),
                false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioDuration, "AudioDuration", N_("Audio Duration"),
                N_("Indicates the duration of an audio content."),
                false, false, 6, 6, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioOutcue, "AudioOutcue", N_("Audio Outcue"),
                N_("Identifies the content of the end of an audio object data, "
                "according to guidelines established by the provider."),
                false, false, 0, 64, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::PreviewFormat, "PreviewFormat", N_("Preview Format"),
                N_("A binary number representing the file format of the object data "
                "preview. The file format must be registered with IPTC or NAA organizations "
                "with a unique number assigned to it."),
                false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::PreviewVersion, "PreviewVersion", N_("Preview Version"),
                N_("A binary number representing the particular version of the "
                "object data preview file format specified in tag <PreviewFormat>."),
                false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Preview, "Preview", N_("Preview Data"),
                N_("Binary image preview data."),
                false, false, 0, 256000, Exiv2::undefined, IptcDataSets::application2, ""),
        DataSet(0xffff, "(Invalid)", N_("(Invalid)"),
                N_("(Invalid)"),
                false, false, 0, 0, Exiv2::unsignedShort, IptcDataSets::application2, "")
    };

    const DataSet* IptcDataSets::application2RecordList()
    {
        return application2Record;
    }

    static const DataSet unknownDataSet(0xffff, "Unknown dataset", N_("Unknown dataset"),
                                        N_("Unknown dataset"),
                                        false, true, 0, 0xffffffff, Exiv2::string,
                                        IptcDataSets::invalidRecord,
                                        N_("Unknown dataset"));

    // Dataset lookup lists.This is an array with pointers to one list per IIM4 Record.
    // The record id is used as the index into the array.
    const DataSet* IptcDataSets::records_[] = {
        0,
        envelopeRecord, application2Record,
        0
    };

    int IptcDataSets::dataSetIdx(uint16_t number, uint16_t recordId)
    {
        if( recordId != envelope && recordId != application2 ) return -1;
        const DataSet* dataSet = records_[recordId];
        if (dataSet == 0) return -1;
        int idx;
        for (idx = 0; dataSet[idx].number_ != number; ++idx) {
            if (dataSet[idx].number_ == 0xffff) return -1;
        }
        return idx;
    }

    int IptcDataSets::dataSetIdx(const std::string& dataSetName, uint16_t recordId)
    {
        if( recordId != envelope && recordId != application2 ) return -1;
        const DataSet* dataSet = records_[recordId];
        if (dataSet == 0) return -1;
        int idx;
        for (idx = 0; dataSet[idx].name_ != dataSetName; ++idx) {
            if (dataSet[idx].number_ == 0xffff) return -1;
        }
        return idx;
    }

    TypeId IptcDataSets::dataSetType(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) return unknownDataSet.type_;
        return records_[recordId][idx].type_;
    }

    std::string IptcDataSets::dataSetName(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx != -1) return records_[recordId][idx].name_;

        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << number;
        return os.str();
    }

    const char* IptcDataSets::dataSetTitle(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) return unknownDataSet.title_;
        return records_[recordId][idx].title_;
    }

    const char* IptcDataSets::dataSetDesc(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) return unknownDataSet.desc_;
        return records_[recordId][idx].desc_;
    }

    const char* IptcDataSets::dataSetPsName(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) return unknownDataSet.photoshop_;
        return records_[recordId][idx].photoshop_;
    }

    bool IptcDataSets::dataSetRepeatable(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) return unknownDataSet.repeatable_;
        return records_[recordId][idx].repeatable_;
    }

    uint16_t IptcDataSets::dataSet(const std::string& dataSetName,
                                   uint16_t recordId)
    {
        uint16_t dataSet;
        int idx = dataSetIdx(dataSetName, recordId);
        if (idx != -1) {
            // dataSetIdx checks the range of recordId
            dataSet = records_[recordId][idx].number_;
        }
        else {
            if (!isHex(dataSetName, 4, "0x")) throw Error(4, dataSetName);
            std::istringstream is(dataSetName);
            is >> std::hex >> dataSet;
        }
        return dataSet;
    }

    std::string IptcDataSets::recordName(uint16_t recordId)
    {
        if (recordId == envelope || recordId == application2) {
            return recordInfo_[recordId].name_;
        }

        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << recordId;
        return os.str();
    }

    const char* IptcDataSets::recordDesc(uint16_t recordId)
    {
        if (recordId != envelope && recordId != application2) {
            return unknownDataSet.desc_;
        }
        return recordInfo_[recordId].desc_;
    }

    uint16_t IptcDataSets::recordId(const std::string& recordName)
    {
        uint16_t i;
        for (i = application2; i > 0; --i) {
            if (recordInfo_[i].name_ == recordName) break;
        }
        if (i == 0) {
            if (!isHex(recordName, 4, "0x")) throw Error(5, recordName);
            std::istringstream is(recordName);
            is >> std::hex >> i;
        }
        return i;
    }

    void IptcDataSets::dataSetList(std::ostream& os)
    {
        const int count = sizeof(records_)/sizeof(records_[0]);
        for (int i=0; i < count; ++i) {
            const DataSet *record = records_[i];
            for (int j=0; record != 0 && record[j].number_ != 0xffff; ++j) {
                os << record[j] << "\n";
            }
        }
    } // IptcDataSets::dataSetList

    const char* IptcKey::familyName_ = "Iptc";

    IptcKey::IptcKey(const std::string& key)
        : key_(key)
    {
        decomposeKey();
    }

    IptcKey::IptcKey(uint16_t tag, uint16_t record)
        : tag_(tag), record_(record)
    {
        makeKey();
    }

    IptcKey::IptcKey(const IptcKey& rhs)
        : Key(rhs), tag_(rhs.tag_), record_(rhs.record_), key_(rhs.key_)
    {
    }

    IptcKey::~IptcKey()
    {
    }

    IptcKey& IptcKey::operator=(const IptcKey& rhs)
    {
        if (this == &rhs) return *this;
        Key::operator=(rhs);
        tag_ = rhs.tag_;
        record_ = rhs.record_;
        key_ = rhs.key_;
        return *this;
    }

    std::string IptcKey::key() const
    {
        return key_;
    }

    const char* IptcKey::familyName() const
    {
        return familyName_;
    }

    std::string IptcKey::groupName() const
    {
        return recordName();
    }

    std::string IptcKey::tagName() const
    {
        return IptcDataSets::dataSetName(tag_, record_);
    }

    std::string IptcKey::tagLabel() const
    {
        return IptcDataSets::dataSetTitle(tag_, record_);
    }

    uint16_t IptcKey::tag() const
    {
        return tag_;
    }

    std::string IptcKey::recordName() const
    {
        return IptcDataSets::recordName(record_);
    }

    uint16_t IptcKey::record() const
    {
        return record_;
    }

    IptcKey::AutoPtr IptcKey::clone() const
    {
        return AutoPtr(clone_());
    }

    IptcKey* IptcKey::clone_() const
    {
        return new IptcKey(*this);
    }

    void IptcKey::decomposeKey()
    {
        // Get the family name, record name and dataSet name parts of the key
        std::string::size_type pos1 = key_.find('.');
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string familyName = key_.substr(0, pos1);
        if (0 != strcmp(familyName.c_str(), familyName_)) {
            throw Error(6, key_);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key_.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string recordName = key_.substr(pos0, pos1 - pos0);
        if (recordName == "") throw Error(6, key_);
        std::string dataSetName = key_.substr(pos1 + 1);
        if (dataSetName == "") throw Error(6, key_);

        // Use the parts of the key to find dataSet and recordId
        uint16_t recId = IptcDataSets::recordId(recordName);
        uint16_t dataSet = IptcDataSets::dataSet(dataSetName, recId);

        // Possibly translate hex name parts (0xabcd) to real names
        recordName = IptcDataSets::recordName(recId);
        dataSetName = IptcDataSets::dataSetName(dataSet, recId);

        tag_ = dataSet;
        record_ = recId;
        key_ = familyName + "." + recordName + "." + dataSetName;
    } // IptcKey::decomposeKey

    void IptcKey::makeKey()
    {
        key_ = std::string(familyName_)
            + "." + IptcDataSets::recordName(record_)
            + "." + IptcDataSets::dataSetName(tag_, record_);
    }

    // *************************************************************************
    // free functions

    std::ostream& operator<<(std::ostream& os, const DataSet& dataSet)
    {
        IptcKey iptcKey(dataSet.number_, dataSet.recordId_);
        return os << dataSet.name_ << ", "
                  << std::dec << dataSet.number_ << ", "
                  << "0x" << std::setw(4) << std::setfill('0')
                  << std::right << std::hex << dataSet.number_ << ", "
                  << IptcDataSets::recordName(dataSet.recordId_) << ", "
                  << std::boolalpha << dataSet.mandatory_ << ", "
                  << dataSet.repeatable_ << ", "
                  << std::dec << dataSet.minbytes_ << ", "
                  << dataSet.maxbytes_ << ", "
                  << iptcKey.key() << ", "
                  << TypeInfo::typeName(
                      IptcDataSets::dataSetType(dataSet.number_,
                                                dataSet.recordId_)) << ", "
                  << dataSet.desc_;
    }

}                                       // namespace Exiv2
