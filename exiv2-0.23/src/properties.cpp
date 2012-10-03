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
  File:      properties.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: properties.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#include "properties.hpp"
#include "tags_int.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"
#include "metadatum.hpp"
#include "i18n.h"                // NLS support.
#include "xmp.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cctype>

// *****************************************************************************
namespace {

    //! Struct used in the lookup table for pretty print functions
    struct XmpPrintInfo {
        //! Comparison operator for key
        bool operator==(const std::string& key) const
        {
            return 0 == strcmp(key_, key.c_str());
        }

        const char* key_;                      //!< XMP key
        Exiv2::PrintFct printFct_;             //!< Print function
    };

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    //! @cond IGNORE
    extern const XmpPropertyInfo xmpDcInfo[];
    extern const XmpPropertyInfo xmpDigikamInfo[];
    extern const XmpPropertyInfo xmpKipiInfo[];
    extern const XmpPropertyInfo xmpXmpInfo[];
    extern const XmpPropertyInfo xmpXmpRightsInfo[];
    extern const XmpPropertyInfo xmpXmpMMInfo[];
    extern const XmpPropertyInfo xmpXmpBJInfo[];
    extern const XmpPropertyInfo xmpXmpTPgInfo[];
    extern const XmpPropertyInfo xmpXmpDMInfo[];
    extern const XmpPropertyInfo xmpMicrosoftInfo[];
    extern const XmpPropertyInfo xmpPdfInfo[];
    extern const XmpPropertyInfo xmpPhotoshopInfo[];
    extern const XmpPropertyInfo xmpCrsInfo[];
    extern const XmpPropertyInfo xmpTiffInfo[];
    extern const XmpPropertyInfo xmpExifInfo[];
    extern const XmpPropertyInfo xmpAuxInfo[];
    extern const XmpPropertyInfo xmpIptcInfo[];
    extern const XmpPropertyInfo xmpIptcExtInfo[];
    extern const XmpPropertyInfo xmpPlusInfo[];
    extern const XmpPropertyInfo xmpMediaProInfo[];
    extern const XmpPropertyInfo xmpExpressionMediaInfo[];
    extern const XmpPropertyInfo xmpMicrosoftPhotoInfo[];
    extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfoInfo[];
    extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfo[];
    extern const XmpPropertyInfo xmpMWGRegionsInfo[];

    extern const XmpNsInfo xmpNsInfo[] = {
        // Schemas   -   NOTE: Schemas which the XMP-SDK doesn't know must be registered in XmpParser::initialize - Todo: Automate this
        { "http://purl.org/dc/elements/1.1/",             "dc",             xmpDcInfo,        N_("Dublin Core schema")                        },
        { "http://www.digikam.org/ns/1.0/",               "digiKam",        xmpDigikamInfo,   N_("digiKam Photo Management schema")           },
        { "http://www.digikam.org/ns/kipi/1.0/",          "kipi",           xmpKipiInfo,      N_("KDE Image Program Interface schema")        },
        { "http://ns.adobe.com/xap/1.0/",                 "xmp",            xmpXmpInfo,       N_("XMP Basic schema")                          },
        { "http://ns.adobe.com/xap/1.0/rights/",          "xmpRights",      xmpXmpRightsInfo, N_("XMP Rights Management schema")              },
        { "http://ns.adobe.com/xap/1.0/mm/",              "xmpMM",          xmpXmpMMInfo,     N_("XMP Media Management schema")               },
        { "http://ns.adobe.com/xap/1.0/bj/",              "xmpBJ",          xmpXmpBJInfo,     N_("XMP Basic Job Ticket schema")               },
        { "http://ns.adobe.com/xap/1.0/t/pg/",            "xmpTPg",         xmpXmpTPgInfo,    N_("XMP Paged-Text schema")                     },
        { "http://ns.adobe.com/xmp/1.0/DynamicMedia/",    "xmpDM",          xmpXmpDMInfo,     N_("XMP Dynamic Media schema")                  },
        { "http://ns.microsoft.com/photo/1.0/",           "MicrosoftPhoto", xmpMicrosoftInfo, N_("Microsoft Photo schema")                    },
        { "http://ns.adobe.com/pdf/1.3/",                 "pdf",            xmpPdfInfo,       N_("Adobe PDF schema")                          },
        { "http://ns.adobe.com/photoshop/1.0/",           "photoshop",      xmpPhotoshopInfo, N_("Adobe photoshop schema")                    },
        { "http://ns.adobe.com/camera-raw-settings/1.0/", "crs",            xmpCrsInfo,       N_("Camera Raw schema")                         },
        { "http://ns.adobe.com/tiff/1.0/",                "tiff",           xmpTiffInfo,      N_("Exif Schema for TIFF Properties")           },
        { "http://ns.adobe.com/exif/1.0/",                "exif",           xmpExifInfo,      N_("Exif schema for Exif-specific Properties")  },
        { "http://ns.adobe.com/exif/1.0/aux/",            "aux",            xmpAuxInfo,       N_("Exif schema for Additional Exif Properties")},
        { "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/",  "iptc",           xmpIptcInfo,      N_("IPTC Core schema")                          }, // NOTE: 'Iptc4xmpCore' is just too long, so make 'iptc'
        { "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/",  "Iptc4xmpCore",   xmpIptcInfo,      N_("IPTC Core schema")                          }, // the default prefix. But provide the official one too.
        { "http://iptc.org/std/Iptc4xmpExt/2008-02-29/",  "iptcExt",        xmpIptcExtInfo,   N_("IPTC Extension schema")                     }, // NOTE: It really should be 'Iptc4xmpExt' but following
        { "http://iptc.org/std/Iptc4xmpExt/2008-02-29/",  "Iptc4xmpExt",    xmpIptcExtInfo,   N_("IPTC Extension schema")                     }, // example above, 'iptcExt' is the default, Iptc4xmpExt works too.
        { "http://ns.useplus.org/ldf/xmp/1.0/",           "plus",           xmpPlusInfo,      N_("PLUS License Data Format schema")           },
        { "http://ns.iview-multimedia.com/mediapro/1.0/", "mediapro",       xmpMediaProInfo,  N_("iView Media Pro schema")                    },
        { "http://ns.microsoft.com/expressionmedia/1.0/", "expressionmedia",xmpExpressionMediaInfo, N_("Expression Media schema")             },
        { "http://ns.microsoft.com/photo/1.2/",              "MP",    xmpMicrosoftPhotoInfo,           N_("Microsoft Photo 1.2 schema")       },
        { "http://ns.microsoft.com/photo/1.2/t/RegionInfo#", "MPRI",  xmpMicrosoftPhotoRegionInfoInfo, N_("Microsoft Photo RegionInfo schema")},
        { "http://ns.microsoft.com/photo/1.2/t/Region#",     "MPReg", xmpMicrosoftPhotoRegionInfo,     N_("Microsoft Photo Region schema")    },
        { "http://www.metadataworkinggroup.com/schemas/regions/", "mwg-rs", xmpMWGRegionsInfo,N_("Metadata Working Group Regions schema")     },

        // Structures
        { "http://ns.adobe.com/xap/1.0/g/",                   "xapG",    0, N_("Colorant structure")           },
        { "http://ns.adobe.com/xap/1.0/sType/Dimensions#",    "stDim",   0, N_("Dimensions structure")         },
        { "http://ns.adobe.com/xap/1.0/sType/Font#",          "stFnt",   0, N_("Font structure")               },
        { "http://ns.adobe.com/xap/1.0/g/img/",               "xapGImg", 0, N_("Thumbnail structure")          },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceEvent#", "stEvt",   0, N_("Resource Event structure")     },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceRef#",   "stRef",   0, N_("ResourceRef structure")        },
        { "http://ns.adobe.com/xap/1.0/sType/Version#",       "stVer",   0, N_("Version structure")            },
        { "http://ns.adobe.com/xap/1.0/sType/Job#",           "stJob",   0, N_("Basic Job/Workflow structure") },
        { "http://ns.adobe.com/xmp/sType/Area#",              "stArea",  0, N_("Area structure")               },

        // Qualifiers
        { "http://ns.adobe.com/xmp/Identifier/qual/1.0/", "xmpidq", 0, N_("Qualifier for xmp:Identifier") }
    };

    extern const XmpPropertyInfo xmpDcInfo[] = {
        { "contributor",      N_("Contributor"),      "bag ProperName",  xmpBag,       xmpExternal, N_("Contributors to the resource (other than the authors).")                               },
        { "coverage",         N_("Coverage"),         "Text",            xmpText,      xmpExternal, N_("The spatial or temporal topic of the resource, the spatial applicability of the "
                                                                                                       "resource, or the jurisdiction under which the resource is relevant.")                      },
        { "creator",          N_("Creator"),          "seq ProperName",  xmpSeq,       xmpExternal, N_("The authors of the resource (listed in order of precedence, if significant).")         },
        { "date",             N_("Date"),             "seq Date",        xmpSeq,       xmpExternal, N_("Date(s) that something interesting happened to the resource.")                         },
        { "description",      N_("Description"),      "Lang Alt",        langAlt,      xmpExternal, N_("A textual description of the content of the resource. Multiple values may be "
                                                                                                       "present for different languages.")                                                     },
        { "format",           N_("Format"),           "MIMEType",        xmpText,      xmpInternal, N_("The file format used when saving the resource. Tools and applications should set "
                                                                                                       "this property to the save format of the data. It may include appropriate qualifiers.") },
        { "identifier",       N_("Identifier"),       "Text",            xmpText,      xmpExternal, N_("Unique identifier of the resource. Recommended best practice is to identify the "
                                                                                                       "resource by means of a string conforming to a formal identification system.")              },
        { "language",         N_("Language"),         "bag Locale",      xmpBag,       xmpInternal, N_("An unordered array specifying the languages used in the resource.")                    },
        { "publisher",        N_("Publisher"),        "bag ProperName",  xmpBag,       xmpExternal, N_("An entity responsible for making the resource available. Examples of a Publisher "
                                                                                                       "include a person, an organization, or a service. Typically, the name of a Publisher "
                                                                                                       "should be used to indicate the entity.")                                               },
        { "relation",         N_("Relation"),         "bag Text",        xmpBag,       xmpInternal, N_("Relationships to other documents. Recommended best practice is to identify the "
                                                                                                       "related resource by means of a string conforming to a formal identification system.")      },
        { "rights",           N_("Rights"),           "Lang Alt",        langAlt,      xmpExternal, N_("Informal rights statement, selected by language. Typically, rights information "
                                                                                                       "includes a statement about various property rights associated with the resource, "
                                                                                                       "including intellectual property rights.")                                              },
        { "source",           N_("Source"),           "Text",            xmpText,      xmpExternal, N_("Unique identifier of the work from which this resource was derived.")                  },
        { "subject",          N_("Subject"),          "bag Text",        xmpBag,       xmpExternal, N_("An unordered array of descriptive phrases or keywords that specify the topic of the "
                                                                                                       "content of the resource.")                                                             },
        { "title",            N_("Title"),            "Lang Alt",        langAlt,      xmpExternal, N_("The title of the document, or the name given to the resource. Typically, it will be "
                                                                                                       "a name by which the resource is formally known.")                                      },
        { "type",             N_("Type"),             "bag open Choice", xmpBag,       xmpExternal, N_("A document type; for example, novel, poem, or working paper.")                         },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpDigikamInfo[] = {
        { "TagsList",               N_("Tags List"),                 "seq Text", xmpSeq,  xmpExternal, N_("The list of complete tags path as string. The path hierarchy is separated by '/' character (ex.: \"City/Paris/Monument/Eiffel Tower\".") },
        { "CaptionsAuthorNames",    N_("Captions Author Names"),     "Lang Alt", langAlt, xmpExternal, N_("The list of all captions author names for each language alternative captions set in standard XMP tags.") },
        { "CaptionsDateTimeStamps", N_("Captions Date Time Stamps"), "Lang Alt", langAlt, xmpExternal, N_("The list of all captions date time stamps for each language alternative captions set in standard XMP tags.") },
        { "ImageHistory",           N_("Image History"),             "Text",     xmpText, xmpExternal, N_("An XML based content to list all action processed on this image with image editor (as crop, rotate, color corrections, adjustements, etc.).") },
        { "LensCorrectionSettings", N_("Lens Correction Settings"),  "Text",     xmpText, xmpExternal, N_("The list of Lens Correction tools settings used to fix lens distorsion. This include Batch Queue Manager and Image editor tools based on LensFun library.") },
        { "ColorLabel",             N_("Color Label"),               "Text",     xmpText, xmpExternal, N_("The color label assigned to this item. Possible values are \"0\": no label; \"1\": Red; \"2\": Orange; \"3\": Yellow; \"4\": Green; \"5\": Blue; \"6\": Magenta; \"7\": Gray; \"8\": Black; \"9\": White.") },
        { "PickLabel",              N_("Pick Label"),                "Text",     xmpText, xmpExternal, N_("The pick label assigned to this item. Possible values are \"0\": no label; \"1\": item rejected; \"2\": item in pending validation; \"3\": item accepted.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpKipiInfo[] = {
        { "EnfuseInputFiles",       N_("Enfuse Input Files"),        "Text",     xmpText, xmpExternal, N_("The list of files processed with Enfuse program through ExpoBlending tool.") },
        { "EnfuseSettings",         N_("Enfuse Settings"),           "Text",     xmpText, xmpExternal, N_("The list of Enfuse settings used to blend image stack with ExpoBlending tool.") },
        { "picasawebGPhotoId",      N_("PicasaWeb Item ID"),         "Text",     xmpText, xmpExternal, N_("Item ID from PicasaWeb web service.") },
        { "yandexGPhotoId",         N_("Yandex Fotki Item ID"),      "Text",     xmpText, xmpExternal, N_("Item ID from Yandex Fotki web service.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpInfo[] = {
        { "Advisory",         N_("Advisory"),         "bag XPath",                xmpBag,    xmpExternal, N_("An unordered array specifying properties that were edited outside the authoring "
                                                                                                             "application. Each item should contain a single namespace and XPath separated by "
                                                                                                             "one ASCII space (U+0020).") },
        { "BaseURL",          N_("Base URL"),         "URL",                      xmpText,   xmpInternal, N_("The base URL for relative URLs in the document content. If this document contains "
                                                                                                             "Internet links, and those links are relative, they are relative to this base URL. "
                                                                                                             "This property provides a standard way for embedded relative URLs to be interpreted "
                                                                                                             "by tools. Web authoring tools should set the value based on their notion of where "
                                                                                                             "URLs will be interpreted.") },
        { "CreateDate",       N_("Create Date"),      "Date",                     xmpText,   xmpInternal, N_("The date and time the resource was originally created.") },
        { "CreatorTool",      N_("Creator Tool"),     "AgentName",                xmpText,   xmpInternal, N_("The name of the first known tool used to create the resource. If history is "
                                                                                                             "present in the metadata, this value should be equivalent to that of "
                                                                                                             "xmpMM:History's softwareAgent property.") },
        { "Identifier",       N_("Identifier"),       "bag Text",                 xmpBag,    xmpExternal, N_("An unordered array of text strings that unambiguously identify the resource within "
                                                                                                             "a given context. An array item may be qualified with xmpidq:Scheme to denote the "
                                                                                                             "formal identification system to which that identifier conforms. Note: The "
                                                                                                             "dc:identifier property is not used because it lacks a defined scheme qualifier and "
                                                                                                             "has been defined in the XMP Specification as a simple (single-valued) property.") },
        { "Label",            N_("Label"),            "Text",                     xmpText,   xmpExternal, N_("A word or short phrase that identifies a document as a member of a user-defined "
                                                                                                             "collection. Used to organize documents in a file browser.") },
        { "MetadataDate",     N_("Metadata Date"),    "Date",                     xmpText,   xmpInternal, N_("The date and time that any metadata for this resource was last changed. It should "
                                                                                                             "be the same as or more recent than xmp:ModifyDate.") },
        { "ModifyDate",       N_("Modify Date"),      "Date",                     xmpText,   xmpInternal, N_("The date and time the resource was last modified. Note: The value of this property "
                                                                                                             "is not necessarily the same as the file's system modification date because it is "
                                                                                                             "set before the file is saved.") },
        { "Nickname",         N_("Nickname"),         "Text",                     xmpText,   xmpExternal, N_("A short informal name for the resource.") },
        { "Rating",           N_("Rating"),           "Closed Choice of Integer", xmpText,   xmpExternal, N_("A number that indicates a document's status relative to other documents, "
                                                                                                             "used to organize documents in a file browser. Values are user-defined within an "
                                                                                                             "application-defined range.") },
        { "Thumbnails",       N_("Thumbnails"),       "alt Thumbnail",            xmpText, xmpInternal, N_("An alternative array of thumbnail images for a file, which can differ in "
                                                                                                             "characteristics such as size or image encoding.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpRightsInfo[] = {
        { "Certificate",      N_("Certificate"),   "URL",            xmpText,       xmpExternal, N_("Online rights management certificate.") },
        { "Marked",           N_("Marked"),        "Boolean",        xmpText,       xmpExternal, N_("Indicates that this is a rights-managed resource.") },
        { "Owner",            N_("Owner"),         "bag ProperName", xmpBag,        xmpExternal, N_("An unordered array specifying the legal owner(s) of a resource.") },
        { "UsageTerms",       N_("Usage Terms"),   "Lang Alt",       langAlt,       xmpExternal, N_("Text instructions on how a resource can be legally used.") },
        { "WebStatement",     N_("Web Statement"), "URL",            xmpText,       xmpExternal, N_("The location of a web page describing the owner and/or rights statement for this resource.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpMMInfo[] = {
        { "DerivedFrom",      N_("Derived From"),      "ResourceRef",       xmpText,    xmpInternal, N_("A reference to the original document from which this one is derived. It is a "
                                                                                                        "minimal reference; missing components can be assumed to be unchanged. For example, "
                                                                                                        "a new version might only need to specify the instance ID and version number of the "
                                                                                                        "previous version, or a rendition might only need to specify the instance ID and "
                                                                                                        "rendition class of the original.") },
        { "DocumentID",       N_("Document ID"),       "URI",               xmpText,    xmpInternal, N_("The common identifier for all versions and renditions of a document. It should be "
                                                                                                        "based on a UUID; see Document and Instance IDs below.") },
        { "History",          N_("History"),           "seq ResourceEvent", xmpText,    xmpInternal, N_("An ordered array of high-level user actions that resulted in this resource. It is "
                                                                                                        "intended to give human readers a general indication of the steps taken to make the "
                                                                                                        "changes from the previous version to this one. The list should be at an abstract "
                                                                                                        "level; it is not intended to be an exhaustive keystroke or other detailed history.") },
        { "InstanceID",       N_("Instance ID"),       "URI",               xmpText,    xmpInternal, N_("An identifier for a specific incarnation of a document, updated each time a file "
                                                                                                        "is saved. It should be based on a UUID; see Document and Instance IDs below.") },
        { "ManagedFrom",      N_("Managed From"),      "ResourceRef",       xmpText,    xmpInternal, N_("A reference to the document as it was prior to becoming managed. It is set when a "
                                                                                                        "managed document is introduced to an asset management system that does not "
                                                                                                        "currently own it. It may or may not include references to different management systems.") },
        { "Manager",          N_("Manager"),           "AgentName",         xmpText,    xmpInternal, N_("The name of the asset management system that manages this resource. Along with "
                                                                                                        "xmpMM: ManagerVariant, it tells applications which asset management system to "
                                                                                                        "contact concerning this document.") },
        { "ManageTo",         N_("Manage To"),         "URI",               xmpText,    xmpInternal, N_("A URI identifying the managed resource to the asset management system; the presence "
                                                                                                        "of this property is the formal indication that this resource is managed. The form "
                                                                                                        "and content of this URI is private to the asset management system.") },
        { "ManageUI",         N_("Manage UI"),         "URI",               xmpText,    xmpInternal, N_("A URI that can be used to access information about the managed resource through a "
                                                                                                        "web browser. It might require a custom browser plug-in.") },
        { "ManagerVariant",   N_("Manager Variant"),   "Text",              xmpText,    xmpInternal, N_("Specifies a particular variant of the asset management system. The format of this "
                                                                                                        "property is private to the specific asset management system.") },
        { "RenditionClass",   N_("Rendition Class"),   "RenditionClass",    xmpText,    xmpInternal, N_("The rendition class name for this resource. This property should be absent or set "
                                                                                                        "to default for a document version that is not a derived rendition.") },
        { "RenditionParams",  N_("Rendition Params"),  "Text",              xmpText,    xmpInternal, N_("Can be used to provide additional rendition parameters that are too complex or "
                                                                                                        "verbose to encode in xmpMM: RenditionClass.") },
        { "VersionID",        N_("Version ID"),        "Text",              xmpText,    xmpInternal, N_("The document version identifier for this resource. Each version of a document gets "
                                                                                                        "a new identifier, usually simply by incrementing integers 1, 2, 3 . . . and so on. "
                                                                                                        "Media management systems can have other conventions or support branching which "
                                                                                                        "requires a more complex scheme.") },
        { "Versions",         N_("Versions"),          "seq Version",       xmpText,    xmpInternal, N_("The version history associated with this resource. Entry [1] is the oldest known "
                                                                                                        "version for this document, entry [last()] is the most recent version. Typically, a "
                                                                                                        "media management system would fill in the version information in the metadata on "
                                                                                                        "check-in. It is not guaranteed that a complete history  versions from the first to "
                                                                                                        "this one will be present in the xmpMM:Versions property. Interior version information "
                                                                                                        "can be compressed or eliminated and the version history can be truncated at some point.") },
        { "LastURL",          N_("Last URL"),          "URL",               xmpText,    xmpInternal, N_("Deprecated for privacy protection.") },
        { "RenditionOf",      N_("Rendition Of"),      "ResourceRef",       xmpText,    xmpInternal, N_("Deprecated in favor of xmpMM:DerivedFrom. A reference to the document of which this is "
                                                                                                        "a rendition.") },
        { "SaveID",           N_("Save ID"),           "Integer",           xmpText,    xmpInternal, N_("Deprecated. Previously used only to support the xmpMM:LastURL property.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpBJInfo[] = {
        { "JobRef",           N_("Job Reference"),     "bag Job",   xmpText,    xmpExternal, N_("References an external job management file for a job process in which the document is being used. Use of job "
                                                                                                "names is under user control. Typical use would be to identify all documents that are part of a particular job or contract. "
                                                                                                "There are multiple values because there can be more than one job using a particular document at any time, and it can "
                                                                                                "also be useful to keep historical information about what jobs a document was part of previously.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpTPgInfo[] = {
        { "MaxPageSize",      N_("Maximum Page Size"), "Dimensions",   xmpText,    xmpInternal, N_("The size of the largest page in the document (including any in contained documents).") },
        { "NPages",           N_("Number of Pages"),   "Integer",      xmpText,    xmpInternal, N_("The number of pages in the document (including any in contained documents).") },
        { "Fonts",            N_("Fonts"),             "bag Font",     xmpText,    xmpInternal, N_("An unordered array of fonts that are used in the document (including any in contained documents).") },
        { "Colorants",        N_("Colorants"),         "seq Colorant", xmpText,    xmpInternal, N_("An ordered array of colorants (swatches) that are used in the document (including any in contained documents).") },
        { "PlateNames",       N_("Plate Names"),       "seq Text",     xmpSeq,     xmpInternal, N_("An ordered array of plate names that are needed to print the document (including any in contained documents).") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpDMInfo[] = {
        { "projectRef",                   N_("Project Reference"),                "ProjectLink",           xmpText, xmpInternal, N_("A reference to the project that created this file.") },
        { "videoFrameRate",               N_("Video Frame Rate"),                 "open Choice of Text",   xmpText, xmpInternal, N_("The video frame rate. One of: 24, NTSC, PAL.") },
        { "videoFrameSize",               N_("Video Frame Size"),                 "Dimensions",            xmpText, xmpInternal, N_("The frame size. For example: w:720, h: 480, unit:pixels") },
        { "videoPixelAspectRatio",        N_("Video Pixel Aspect Ratio"),         "Rational",              xmpText, xmpInternal, N_("The aspect ratio, expressed as ht/wd. For example: \"648/720\" = 0.9") },
        { "videoPixelDepth",              N_("Video Pixel Depth"),                "closed Choice of Text", xmpText, xmpInternal, N_("The size in bits of each color component of a pixel. Standard Windows 32-bit "
                                                                                                                                    "pixels have 8 bits per component. One of: 8Int, 16Int, 32Int, 32Float.") },
        { "videoColorSpace",              N_("Video Color Space"),                "closed Choice of Text", xmpText, xmpInternal, N_("The color space. One of: sRGB (used by Photoshop), CCIR-601 (used for NTSC), "
                                                                                                                                    "CCIR-709 (used for HD).") },
        { "videoAlphaMode",               N_("Video Alpha Mode"),                 "closed Choice of Text", xmpText, xmpExternal, N_("The alpha mode. One of: straight, pre-multiplied.") },
        { "videoAlphaPremultipleColor",   N_("Video Alpha Premultiple Color"),    "Colorant",              xmpText, xmpExternal, N_("A color in CMYK or RGB to be used as the pre-multiple color when "
                                                                                                                                    "alpha mode is pre-multiplied.") },
        { "videoAlphaUnityIsTransparent", N_("Video Alpha Unity Is Transparent"), "Boolean",               xmpText, xmpInternal, N_("When true, unity is clear, when false, it is opaque.") },
        { "videoCompressor",              N_("Video Compressor"),                 "Text",                  xmpText, xmpInternal, N_("Video compression used. For example, jpeg.") },
        { "videoFieldOrder",              N_("Video Field Order"),                "closed Choice of Text", xmpText, xmpInternal, N_("The field order for video. One of: Upper, Lower, Progressive.") },
        { "pullDown",                     N_("Pull Down"),                        "closed Choice of Text", xmpText, xmpInternal, N_("The sampling phase of film to be converted to video (pull-down). One of: "
                                                                                                                                    "WSSWW, SSWWW, SWWWS, WWWSS, WWSSW, WSSWW_24p, SSWWW_24p, SWWWS_24p, WWWSS_24p, WWSSW_24p.") },
        { "audioSampleRate",              N_("Audio Sample Rate"),                "Integer",               xmpText, xmpInternal, N_("The audio sample rate. Can be any value, but commonly 32000, 41100, or 48000.") },
        { "audioSampleType",              N_("Audio Sample Type"),                "closed Choice of Text", xmpText, xmpInternal, N_("The audio sample type. One of: 8Int, 16Int, 32Int, 32Float.") },
        { "audioChannelType",             N_("Audio Channel Type"),               "closed Choice of Text", xmpText, xmpInternal, N_("The audio channel type. One of: Mono, Stereo, 5.1, 7.1.") },
        { "audioCompressor",              N_("Audio Compressor"),                 "Text",                  xmpText, xmpInternal, N_("The audio compression used. For example, MP3.") },
        { "speakerPlacement",             N_("Speaker Placement"),                "Text",                  xmpText, xmpExternal, N_("A description of the speaker angles from center front in degrees. For example: "
                                                                                                                                    "\"Left = -30, Right = 30, Center = 0, LFE = 45, Left Surround = -110, Right Surround = 110\"") },
        { "fileDataRate",                 N_("File Data Rate"),                   "Rational",              xmpText, xmpInternal, N_("The file data rate in megabytes per second. For example: \"36/10\" = 3.6 MB/sec") },
        { "tapeName",                     N_("Tape Name"),                        "Text",                  xmpText, xmpExternal, N_("The name of the tape from which the clip was captured, as set during the capture process.") },
        { "altTapeName",                  N_("Alternative Tape Name"),            "Text",                  xmpText, xmpExternal, N_("An alternative tape name, set via the project window or timecode dialog in Premiere. "
                                                                                                                                    "If an alternative name has been set and has not been reverted, that name is displayed.") },
        { "startTimecode",                N_("Start Time Code"),                  "Timecode",              xmpText, xmpInternal, N_("The timecode of the first frame of video in the file, as obtained from the device control.") },
        { "altTimecode",                  N_("Alternative Time code"),            "Timecode",              xmpText, xmpExternal, N_("A timecode set by the user. When specified, it is used instead of the startTimecode.") },
        { "duration",                     N_("Duration"),                         "Time",                  xmpText, xmpInternal, N_("The duration of the media file.") },
        { "scene",                        N_("Scene"),                            "Text",                  xmpText, xmpExternal, N_("The name of the scene.") },
        { "shotName",                     N_("Shot Name"),                        "Text",                  xmpText, xmpExternal, N_("The name of the shot or take.") },
        { "shotDate",                     N_("Shot Date"),                        "Date",                  xmpText, xmpExternal, N_("The date and time when the video was shot.") },
        { "shotLocation",                 N_("Shot Location"),                    "Text",                  xmpText, xmpExternal, N_("The name of the location where the video was shot. For example: \"Oktoberfest, Munich Germany\" "
                                                                                                                                    "For more accurate positioning, use the EXIF GPS values.") },
        { "logComment",                   N_("Log Comment"),                      "Text",                  xmpText, xmpExternal, N_("User's log comments.") },
        { "markers",                      N_("Markers"),                          "seq Marker",            xmpText, xmpInternal, N_("An ordered list of markers") },
        { "contributedMedia",             N_("Contributed Media"),                "bag Media",             xmpText, xmpInternal, N_("An unordered list of all media used to create this media.") },
        { "absPeakAudioFilePath",         N_("Absolute Peak Audio File Path"),    "URI",                   xmpText, xmpInternal, N_("The absolute path to the file's peak audio file. If empty, no peak file exists.") },
        { "relativePeakAudioFilePath",    N_("Relative Peak Audio File Path"),    "URI",                   xmpText, xmpInternal, N_("The relative path to the file's peak audio file. If empty, no peak file exists.") },
        { "videoModDate",                 N_("Video Modified Date"),              "Date",                  xmpText, xmpInternal, N_("The date and time when the video was last modified.") },
        { "audioModDate",                 N_("Audio Modified Date"),              "Date",                  xmpText, xmpInternal, N_("The date and time when the audio was last modified.") },
        { "metadataModDate",              N_("Metadata Modified Date"),           "Date",                  xmpText, xmpInternal, N_("The date and time when the metadata was last modified.") },
        { "artist",                       N_("Artist"),                           "Text",                  xmpText, xmpExternal, N_("The name of the artist or artists.") },
        { "album",                        N_("Album"),                            "Text",                  xmpText, xmpExternal, N_("The name of the album.") },
        { "trackNumber",                  N_("Track Number"),                     "Integer",               xmpText, xmpExternal, N_("A numeric value indicating the order of the audio file within its original recording.") },
        { "genre",                        N_("Genre"),                            "Text",                  xmpText, xmpExternal, N_("The name of the genre.") },
        { "copyright",                    N_("Copyright"),                        "Text",                  xmpText, xmpExternal, N_("The copyright information.") },
        { "releaseDate",                  N_("Release Date"),                     "Date",                  xmpText, xmpExternal, N_("The date the title was released.") },
        { "composer",                     N_("Composer"),                         "Text",                  xmpText, xmpExternal, N_("The composer's name.") },
        { "engineer",                     N_("Engineer"),                         "Text",                  xmpText, xmpExternal, N_("The engineer's name.") },
        { "tempo",                        N_("Tempo"),                            "Real",                  xmpText, xmpInternal, N_("The audio's tempo.") },
        { "instrument",                   N_("Instrument"),                       "Text",                  xmpText, xmpExternal, N_("The musical instrument.") },
        { "introTime",                    N_("Intro Time"),                       "Time",                  xmpText, xmpInternal, N_("The duration of lead time for queuing music.") },
        { "outCue",                       N_("Out Cue"),                          "Time",                  xmpText, xmpInternal, N_("The time at which to fade out.") },
        { "relativeTimestamp",            N_("Relative Timestamp"),               "Time",                  xmpText, xmpInternal, N_("The start time of the media inside the audio project.") },
        { "loop",                         N_("Loop"),                             "Boolean",               xmpText, xmpInternal, N_("When true, the clip can be looped seemlessly.") },
        { "numberOfBeats",                N_("Number Of Beats"),                  "Real",                  xmpText, xmpInternal, N_("The number of beats.") },
        { "key",                          N_("Key"),                              "closed Choice of Text", xmpText, xmpInternal, N_("The audio's musical key. One of: C, C#, D, D#, E, F, F#, G, G#, A, A#, B.") },
        { "stretchMode",                  N_("Stretch Mode"),                     "closed Choice of Text", xmpText, xmpInternal, N_("The audio stretch mode. One of: Fixed length, Time-Scale, Resample, Beat Splice, Hybrid.") },
        { "timeScaleParams",              N_("Time Scale Parameters"),            "timeScaleStretch",      xmpText, xmpInternal, N_("Additional parameters for Time-Scale stretch mode.") },
        { "resampleParams",               N_("Resample Parameters"),              "resampleStretch",       xmpText, xmpInternal, N_("Additional parameters for Resample stretch mode.") },
        { "beatSpliceParams",             N_("Beat Splice Parameters"),           "beatSpliceStretch",     xmpText, xmpInternal, N_("Additional parameters for Beat Splice stretch mode.") },
        { "timeSignature",                N_("Time Signature"),                   "closed Choice of Text", xmpText, xmpInternal, N_("The time signature of the music. One of: 2/4, 3/4, 4/4, 5/4, 7/4, 6/8, 9/8, 12/8, other.") },
        { "scaleType",                    N_("Scale Type"),                       "closed Choice of Text", xmpText, xmpInternal, N_("The musical scale used in the music. One of: Major, Minor, Both, Neither. "
                                                                                                                                    "Neither is most often used for instruments with no associated scale, such as drums.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMicrosoftInfo[] = {
        { "CameraSerialNumber", N_("Camera Serial Number"), "Text",     xmpText, xmpExternal, N_("Camera Serial Number.") },
        { "DateAcquired",       N_("Date Acquired"),        "Date",     xmpText, xmpExternal, N_("Date Acquired.")        },
        { "FlashManufacturer",  N_("Flash Manufacturer"),   "Text",     xmpText, xmpExternal, N_("Flash Manufacturer.")   },
        { "FlashModel",         N_("Flash Model"),          "Text",     xmpText, xmpExternal, N_("Flash Model.")          },
        { "LastKeywordIPTC",    N_("Last Keyword IPTC"),    "bag Text", xmpBag,  xmpExternal, N_("Last Keyword IPTC.")    },
        { "LastKeywordXMP",     N_("Last Keyword XMP"),     "bag Text", xmpBag,  xmpExternal, N_("Last Keyword XMP.")     },
        { "LensManufacturer",   N_("Lens Manufacturer"),    "Text",     xmpText, xmpExternal, N_("Lens Manufacturer.")    },
        { "LensModel",          N_("Lens Model"),           "Text",     xmpText, xmpExternal, N_("Lens Model.")           },
        { "Rating",             N_("Rating Percent"),       "Text",     xmpText, xmpExternal, N_("Rating Percent.")       },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpPdfInfo[] = {
        { "Keywords",   N_("Keywords"),    "Text",      xmpText, xmpExternal, N_("Keywords.") },
        { "PDFVersion", N_("PDF Version"), "Text",      xmpText, xmpInternal, N_("The PDF file version (for example: 1.0, 1.3, and so on).") },
        { "Producer",   N_("Producer"),    "AgentName", xmpText, xmpInternal, N_("The name of the tool that created the PDF document.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpPhotoshopInfo[] = {
        { "AuthorsPosition",        N_("Authors Position"),        "Text",       xmpText, xmpExternal, N_("By-line title.") },
        { "CaptionWriter",          N_("Caption Writer"),          "ProperName", xmpText, xmpExternal, N_("Writer/editor.") },
        { "Category",               N_("Category"),                "Text",       xmpText, xmpExternal, N_("Category. Limited to 3 7-bit ASCII characters.") },
        { "City",                   N_("City"),                    "Text",       xmpText, xmpExternal, N_("City.") },
        { "Country",                N_("Country"),                 "Text",       xmpText, xmpExternal, N_("Country/primary location.") },
        { "Credit",                 N_("Credit"),                  "Text",       xmpText, xmpExternal, N_("Credit.") },
        { "DateCreated",            N_("Date Created"),            "Date",       xmpText, xmpExternal, N_("The date the intellectual content of the document was created (rather than the creation "
                                                                                                          "date of the physical representation), following IIM conventions. For example, a photo "
                                                                                                          "taken during the American Civil War would have a creation date during that epoch "
                                                                                                          "(1861-1865) rather than the date the photo was digitized for archiving.") },
        { "Headline",               N_("Headline"),                "Text",       xmpText, xmpExternal, N_("Headline.") },
        { "Instructions",           N_("Instructions"),            "Text",       xmpText, xmpExternal, N_("Special instructions.") },
        { "Source",                 N_("Source"),                  "Text",       xmpText, xmpExternal, N_("Source.") },
        { "State",                  N_("State"),                   "Text",       xmpText, xmpExternal, N_("Province/state.") },
        { "SupplementalCategories", N_("Supplemental Categories"), "bag Text",   xmpBag,  xmpExternal, N_("Supplemental category.") },
        { "TransmissionReference",  N_("Transmission Reference"),  "Text",       xmpText, xmpExternal, N_("Original transmission reference.") },
        { "Urgency",                N_("Urgency"),                 "Integer",    xmpText, xmpExternal, N_("Urgency. Valid range is 1-8.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    //! XMP crs:CropUnits
    extern const TagDetails crsCropUnits[] = {
        { 0, N_("pixels") },
        { 1, N_("inches") },
        { 2, N_("cm")     }
    };

    extern const XmpPropertyInfo xmpCrsInfo[] = {
        { "AutoBrightness",       N_("Auto Brightness"),           "Boolean",                          xmpText, xmpExternal, N_("When true, \"Brightness\" is automatically adjusted.") },
        { "AutoContrast",         N_("Auto Contrast"),             "Boolean",                          xmpText, xmpExternal, N_("When true, \"Contrast\" is automatically adjusted.") },
        { "AutoExposure",         N_("Auto Exposure"),             "Boolean",                          xmpText, xmpExternal, N_("When true, \"Exposure\" is automatically adjusted.") },
        { "AutoShadows",          N_("Auto Shadows"),              "Boolean",                          xmpText, xmpExternal, N_("When true,\"Shadows\" is automatically adjusted.") },
        { "BlueHue",              N_("Blue Hue"),                  "Integer",                          xmpText, xmpExternal, N_("\"Blue Hue\" setting. Range -100 to 100.") },
        { "BlueSaturation",       N_("Blue Saturation"),           "Integer",                          xmpText, xmpExternal, N_("\"Blue Saturation\" setting. Range -100 to +100.") },
        { "Brightness",           N_("Brightness"),                "Integer",                          xmpText, xmpExternal, N_("\"Brightness\" setting. Range 0 to +150.") },
        { "CameraProfile",        N_("Camera Profile"),            "Text",                             xmpText, xmpExternal, N_("\"Camera Profile\" setting.") },
        { "ChromaticAberrationB", N_("Chromatic Aberration Blue"), "Integer",                          xmpText, xmpExternal, N_("\"Chromatic Aberration, Fix Blue/Yellow Fringe\" setting. Range -100 to +100.") },
        { "ChromaticAberrationR", N_("Chromatic Aberration Red"),  "Integer",                          xmpText, xmpExternal, N_("\"Chromatic Aberration, Fix Red/Cyan Fringe\" setting. Range -100 to +100.") },
        { "ColorNoiseReduction",  N_("Color Noise Reduction"),     "Integer",                          xmpText, xmpExternal, N_("\"Color Noise Reducton\" setting. Range 0 to +100.") },
        { "Contrast",             N_("Contrast"),                  "Integer",                          xmpText, xmpExternal, N_("\"Contrast\" setting. Range -50 to +100.") },
        { "CropTop",              N_("Crop Top"),                  "Real",                             xmpText, xmpExternal, N_("When \"Has Crop\" is true, top of crop rectangle") },
        { "CropLeft",             N_("Crop Left"),                 "Real",                             xmpText, xmpExternal, N_("When \"Has Crop\" is true, left of crop rectangle.") },
        { "CropBottom",           N_("Crop Bottom"),               "Real",                             xmpText, xmpExternal, N_("When \"Has Crop\" is true, bottom of crop rectangle.") },
        { "CropRight",            N_("Crop Right"),                "Real",                             xmpText, xmpExternal, N_("When \"Has Crop\" is true, right of crop rectangle.") },
        { "CropAngle",            N_("Crop Angle"),                "Real",                             xmpText, xmpExternal, N_("When \"Has Crop\" is true, angle of crop rectangle.") },
        { "CropWidth",            N_("Crop Width"),                "Real",                             xmpText, xmpExternal, N_("Width of resulting cropped image in CropUnits units.") },
        { "CropHeight",           N_("Crop Height"),               "Real",                             xmpText, xmpExternal, N_("Height of resulting cropped image in CropUnits units.") },
        { "CropUnits",            N_("Crop Units"),                "Integer",                          xmpText, xmpExternal, N_("Units for CropWidth and CropHeight. 0=pixels, 1=inches, 2=cm") },
        { "Exposure",             N_("Exposure"),                  "Real",                             xmpText, xmpExternal, N_("\"Exposure\" setting. Range -4.0 to +4.0.") },
        { "GreenHue",             N_("GreenHue"),                  "Integer",                          xmpText, xmpExternal, N_("\"Green Hue\" setting. Range -100 to +100.") },
        { "GreenSaturation",      N_("Green Saturation"),          "Integer",                          xmpText, xmpExternal, N_("\"Green Saturation\" setting. Range -100 to +100.") },
        { "HasCrop",              N_("Has Crop"),                  "Boolean",                          xmpText, xmpExternal, N_("When true, image has a cropping rectangle.") },
        { "HasSettings",          N_("Has Settings"),              "Boolean",                          xmpText, xmpExternal, N_("When true, non-default camera raw settings.") },
        { "LuminanceSmoothing",   N_("Luminance Smoothing"),       "Integer",                          xmpText, xmpExternal, N_("\"Luminance Smoothing\" setting. Range 0 to +100.") },
        { "RawFileName",          N_("Raw File Name"),             "Text",                             xmpText, xmpInternal, N_("File name of raw file (not a complete path).") },
        { "RedHue",               N_("Red Hue"),                   "Integer",                          xmpText, xmpExternal, N_("\"Red Hue\" setting. Range -100 to +100.") },
        { "RedSaturation",        N_("Red Saturation"),            "Integer",                          xmpText, xmpExternal, N_("\"Red Saturation\" setting. Range -100 to +100.") },
        { "Saturation",           N_("Saturation"),                "Integer",                          xmpText, xmpExternal, N_("\"Saturation\" setting. Range -100 to +100.") },
        { "Shadows",              N_("Shadows"),                   "Integer",                          xmpText, xmpExternal, N_("\"Shadows\" setting. Range 0 to +100.") },
        { "ShadowTint",           N_("Shadow Tint"),               "Integer",                          xmpText, xmpExternal, N_("\"Shadow Tint\" setting. Range -100 to +100.") },
        { "Sharpness",            N_("Sharpness"),                 "Integer",                          xmpText, xmpExternal, N_("\"Sharpness\" setting. Range 0 to +100.") },
        { "Temperature",          N_("Temperature"),               "Integer",                          xmpText, xmpExternal, N_("\"Temperature\" setting. Range 2000 to 50000.") },
        { "Tint",                 N_("Tint"),                      "Integer",                          xmpText, xmpExternal, N_("\"Tint\" setting. Range -150 to +150.") },
        { "ToneCurve",            N_("Tone Curve"),                "Seq of points (Integer, Integer)", xmpText, xmpExternal, N_("Array of points (Integer, Integer) defining a \"Tone Curve\".") },
        { "ToneCurveName",        N_("Tone Curve Name"),           "Choice Text",                      xmpText, xmpInternal, N_("The name of the Tone Curve described by ToneCurve. One of: Linear, Medium Contrast, "
                                                                                                                                "Strong Contrast, Custom or a user-defined preset name.") },
        { "Version",              N_("Version"),                   "Text",                             xmpText, xmpInternal, N_("Version of Camera Raw plugin.") },
        { "VignetteAmount",       N_("Vignette Amount"),           "Integer",                          xmpText, xmpExternal, N_("\"Vignetting Amount\" setting. Range -100 to +100.") },
        { "VignetteMidpoint",     N_("Vignette Midpoint"),         "Integer",                          xmpText, xmpExternal, N_("\"Vignetting Midpoint\" setting. Range 0 to +100.") },
        { "WhiteBalance",         N_("White Balance"),             "Closed Choice Text",               xmpText, xmpExternal, N_("\"White Balance\" setting. One of: As Shot, Auto, Daylight, Cloudy, Shade, Tungsten, "
                                                                                                                                "Fluorescent, Flash, Custom") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpTiffInfo[] = {
        { "ImageWidth",                N_("Image Width"),                "Integer",                      xmpText, xmpInternal, N_("TIFF tag 256, 0x100. Image width in pixels.") },
        { "ImageLength",               N_("Image Length"),               "Integer",                      xmpText, xmpInternal, N_("TIFF tag 257, 0x101. Image height in pixels.") },
        { "BitsPerSample",             N_("Bits Per Sample"),            "seq Integer",                  xmpSeq,  xmpInternal, N_("TIFF tag 258, 0x102. Number of bits per component in each channel.") },
        { "Compression",               N_("Compression"),                "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 259, 0x103. Compression scheme: 1 = uncompressed; 6 = JPEG.") },
        { "PhotometricInterpretation", N_("Photometric Interpretation"), "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 262, 0x106. Pixel Composition: 2 = RGB; 6 = YCbCr.") },
        { "Orientation",               N_("Orientation"),                "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 274, 0x112. Orientation:"
                                                                                                                                  "1 = 0th row at top, 0th column at left "
                                                                                                                                  "2 = 0th row at top, 0th column at right "
                                                                                                                                  "3 = 0th row at bottom, 0th column at right "
                                                                                                                                  "4 = 0th row at bottom, 0th column at left "
                                                                                                                                  "5 = 0th row at left, 0th column at top "
                                                                                                                                  "6 = 0th row at right, 0th column at top "
                                                                                                                                  "7 = 0th row at right, 0th column at bottom "
                                                                                                                                  "8 = 0th row at left, 0th column at bottom") },
        { "SamplesPerPixel",           N_("Samples Per Pixel"),          "Integer",                      xmpText, xmpInternal, N_("TIFF tag 277, 0x115. Number of components per pixel.") },
        { "PlanarConfiguration",       N_("Planar Configuration"),       "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 284, 0x11C. Data layout:1 = chunky; 2 = planar.") },
        { "YCbCrSubSampling",          N_("YCbCr Sub Sampling"),         "Closed Choice of seq Integer", xmpSeq,  xmpInternal, N_("TIFF tag 530, 0x212. Sampling ratio of chrominance "
                                                                                                                                  "components: [2, 1] = YCbCr4:2:2; [2, 2] = YCbCr4:2:0") },
        { "YCbCrPositioning",          N_("YCbCr Positioning"),          "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 531, 0x213. Position of chrominance vs. "
                                                                                                                                  "luminance components: 1 = centered; 2 = co-sited.") },
        { "XResolution",               N_("X Resolution"),               "Rational",                     xmpText, xmpInternal, N_("TIFF tag 282, 0x11A. Horizontal resolution in pixels per unit.") },
        { "YResolution",               N_("Y Resolution"),               "Rational",                     xmpText, xmpInternal, N_("TIFF tag 283, 0x11B. Vertical resolution in pixels per unit.") },
        { "ResolutionUnit",            N_("Resolution Unit"),            "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 296, 0x128. Unit used for XResolution and "
                                                                                                                                  "YResolution. Value is one of: 2 = inches; 3 = centimeters.") },
        { "TransferFunction",          N_("Transfer Function"),          "seq Integer",                  xmpSeq,  xmpInternal, N_("TIFF tag 301, 0x12D. Transfer function for image "
                                                                                                                                  "described in tabular style with 3 * 256 entries.") },
        { "WhitePoint",                N_("White Point"),                "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 318, 0x13E. Chromaticity of white point.") },
        { "PrimaryChromaticities",     N_("Primary Chromaticities"),     "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 319, 0x13F. Chromaticity of the three primary colors.") },
        { "YCbCrCoefficients",         N_("YCbCr Coefficients"),         "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 529, 0x211. Matrix coefficients for RGB to YCbCr transformation.") },
        { "ReferenceBlackWhite",       N_("Reference Black White"),      "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 532, 0x214. Reference black and white point values.") },
        { "DateTime",                  N_("Date and Time"),              "Date",                         xmpText, xmpInternal, N_("TIFF tag 306, 0x132 (primary) and EXIF tag 37520, "
                                                                                                                                  "0x9290 (subseconds). Date and time of image creation "
                                                                                                                                  "(no time zone in EXIF), stored in ISO 8601 format, not "
                                                                                                                                  "the original EXIF format. This property includes the "
                                                                                                                                  "value for the EXIF SubSecTime attribute. "
                                                                                                                                  "NOTE: This property is stored in XMP as xmp:ModifyDate.") },
        { "ImageDescription",          N_("Image Description"),          "Lang Alt",                     langAlt, xmpExternal, N_("TIFF tag 270, 0x10E. Description of the image. Note: This property is stored in XMP as dc:description.") },
        { "Make",                      N_("Make"),                       "ProperName",                   xmpText, xmpInternal, N_("TIFF tag 271, 0x10F. Manufacturer of recording equipment.") },
        { "Model",                     N_("Model"),                      "ProperName",                   xmpText, xmpInternal, N_("TIFF tag 272, 0x110. Model name or number of equipment.") },
        { "Software",                  N_("Software"),                   "AgentName",                    xmpText, xmpInternal, N_("TIFF tag 305, 0x131. Software or firmware used to generate image. "
                                                                                                                                  "Note: This property is stored in XMP as xmp:CreatorTool. ") },
        { "Artist",                    N_("Artist"),                     "ProperName",                   xmpText, xmpExternal, N_("TIFF tag 315, 0x13B. Camera owner, photographer or image creator. "
                                                                                                                                  "Note: This property is stored in XMP as the first item in the dc:creator array.") },
        { "Copyright",                 N_("Copyright"),                  "Lang Alt",                     langAlt, xmpExternal, N_("TIFF tag 33432, 0x8298. Copyright information. "
                                                                                                                                  "Note: This property is stored in XMP as dc:rights.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpExifInfo[] = {
        { "ExifVersion",              N_("Exif Version"),                        "Closed Choice of Text",        xmpText, xmpInternal, N_("EXIF tag 36864, 0x9000. EXIF version number.") },
        { "FlashpixVersion",          N_("Flashpix Version"),                    "Closed Choice of Text",        xmpText, xmpInternal, N_("EXIF tag 40960, 0xA000. Version of FlashPix.") },
        { "ColorSpace",               N_("Color Space"),                         "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 40961, 0xA001. Color space information") },
        { "ComponentsConfiguration",  N_("Components Configuration"),            "Closed Choice of seq Integer", xmpSeq,  xmpInternal, N_("EXIF tag 37121, 0x9101. Configuration of components in data: 4 5 6 0 (if RGB compressed data), "
                                                                                                                                          "1 2 3 0 (other cases).") },
        { "CompressedBitsPerPixel",   N_("Compressed Bits Per Pixel"),           "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37122, 0x9102. Compression mode used for a compressed image is indicated "
                                                                                                                                          "in unit bits per pixel.") },
        { "PixelXDimension",          N_("Pixel X Dimension"),                   "Integer",                      xmpText, xmpInternal, N_("EXIF tag 40962, 0xA002. Valid image width, in pixels.") },
        { "PixelYDimension",          N_("Pixel Y Dimension"),                   "Integer",                      xmpText, xmpInternal, N_("EXIF tag 40963, 0xA003. Valid image height, in pixels.") },
        { "UserComment",              N_("User Comment"),                        "Lang Alt",                     langAlt, xmpExternal, N_("EXIF tag 37510, 0x9286. Comments from user.") },
        { "RelatedSoundFile",         N_("Related Sound File"),                  "Text",                         xmpText, xmpInternal, N_("EXIF tag 40964, 0xA004. An \"8.3\" file name for the related sound file.") },
        { "DateTimeOriginal",         N_("Date and Time Original"),              "Date",                         xmpText, xmpInternal, N_("EXIF tags 36867, 0x9003 (primary) and 37521, 0x9291 (subseconds). "
                                                                                                                                          "Date and time when original image was generated, in ISO 8601 format. "
                                                                                                                                          "Includes the EXIF SubSecTimeOriginal data.") },
        { "DateTimeDigitized",        N_("Date and Time Digitized"),             "Date",                         xmpText, xmpInternal, N_("EXIF tag 36868, 0x9004 (primary) and 37522, 0x9292 (subseconds). Date and time when "
                                                                                                                                          "image was stored as digital data, can be the same as DateTimeOriginal if originally "
                                                                                                                                          "stored in digital form. Stored in ISO 8601 format. Includes the EXIF "
                                                                                                                                          "SubSecTimeDigitized data.") },
        { "ExposureTime",             N_("Exposure Time"),                       "Rational",                     xmpText, xmpInternal, N_("EXIF tag 33434, 0x829A. Exposure time in seconds.") },
        { "FNumber",                  N_("F Number"),                            "Rational",                     xmpText, xmpInternal, N_("EXIF tag 33437, 0x829D. F number.") },
        { "ExposureProgram",          N_("Exposure Program"),                    "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 34850, 0x8822. Class of program used for exposure.") },
        { "SpectralSensitivity",      N_("Spectral Sensitivity"),                "Text",                         xmpText, xmpInternal, N_("EXIF tag 34852, 0x8824. Spectral sensitivity of each channel.") },
        { "ISOSpeedRatings",          N_("ISOSpeedRatings"),                     "seq Integer",                  xmpSeq,  xmpInternal, N_("EXIF tag 34855, 0x8827. ISO Speed and ISO Latitude of the input device as "
                                                                                                                                          "specified in ISO 12232.") },
        { "OECF",                     N_("OECF"),                                "OECF/SFR",                     xmpText, xmpInternal, N_("EXIF tag 34856, 0x8828. Opto-Electoric Conversion Function as specified in ISO 14524.") },
        { "ShutterSpeedValue",        N_("Shutter Speed Value"),                 "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37377, 0x9201. Shutter speed, unit is APEX. See Annex C of the EXIF specification.") },
        { "ApertureValue",            N_("Aperture Value"),                      "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37378, 0x9202. Lens aperture, unit is APEX.") },
        { "BrightnessValue",          N_("Brightness Value"),                    "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37379, 0x9203. Brightness, unit is APEX.") },
        { "ExposureBiasValue",        N_("Exposure Bias Value"),                 "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37380, 0x9204. Exposure bias, unit is APEX.") },
        { "MaxApertureValue",         N_("Maximum Aperture Value"),              "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37381, 0x9205. Smallest F number of lens, in APEX.") },
        { "SubjectDistance",          N_("Subject Distance"),                    "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37382, 0x9206. Distance to subject, in meters.") },
        { "MeteringMode",             N_("Metering Mode"),                       "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 37383, 0x9207. Metering mode.") },
        { "LightSource",              N_("Light Source"),                        "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 37384, 0x9208. Light source.") },
        { "Flash",                    N_("Flash"),                               "Flash",                        xmpText, xmpInternal, N_("EXIF tag 37385, 0x9209. Strobe light (flash) source data.") },
        { "FocalLength",              N_("Focal Length"),                        "Rational",                     xmpText, xmpInternal, N_("EXIF tag 37386, 0x920A. Focal length of the lens, in millimeters.") },
        { "SubjectArea",              N_("Subject Area"),                        "seq Integer",                  xmpSeq,  xmpInternal, N_("EXIF tag 37396, 0x9214. The location and area of the main subject in the overall scene.") },
        { "FlashEnergy",              N_("Flash Energy"),                        "Rational",                     xmpText, xmpInternal, N_("EXIF tag 41483, 0xA20B. Strobe energy during image capture.") },
        { "SpatialFrequencyResponse", N_("Spatial Frequency Response"),          "OECF/SFR",                     xmpText, xmpInternal, N_("EXIF tag 41484, 0xA20C. Input device spatial frequency table and SFR values as "
                                                                                                                                          "specified in ISO 12233.") },
        { "FocalPlaneXResolution",    N_("Focal Plane X Resolution"),            "Rational",                     xmpText, xmpInternal, N_("EXIF tag 41486, 0xA20E. Horizontal focal resolution, measured pixels per unit.") },
        { "FocalPlaneYResolution",    N_("Focal Plane Y Resolution"),            "Rational",                     xmpText, xmpInternal, N_("EXIF tag 41487, 0xA20F. Vertical focal resolution, measured in pixels per unit.") },
        { "FocalPlaneResolutionUnit", N_("Focal Plane Resolution Unit"),         "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41488, 0xA210. Unit used for FocalPlaneXResolution and FocalPlaneYResolution.") },
        { "SubjectLocation",          N_("Subject Location"),                    "seq Integer",                  xmpSeq,  xmpInternal, N_("EXIF tag 41492, 0xA214. Location of the main subject of the scene. The first value is the "
                                                                                                                                          "horizontal pixel and the second value is the vertical pixel at which the "
                                                                                                                                          "main subject appears.") },
        { "ExposureIndex",            N_("Exposure Index"),                      "Rational",                     xmpText, xmpInternal, N_("EXIF tag 41493, 0xA215. Exposure index of input device.") },
        { "SensingMethod",            N_("Sensing Method"),                      "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41495, 0xA217. Image sensor type on input device.") },
        { "FileSource",               N_("File Source"),                         "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41728, 0xA300. Indicates image source.") },
        { "SceneType",                N_("Scene Type"),                          "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41729, 0xA301. Indicates the type of scene.") },
        { "CFAPattern",               N_("CFA Pattern"),                         "CFAPattern",                   xmpText, xmpInternal, N_("EXIF tag 41730, 0xA302. Color filter array geometric pattern of the image sense.") },
        { "CustomRendered",           N_("Custom Rendered"),                     "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41985, 0xA401. Indicates the use of special processing on image data.") },
        { "ExposureMode",             N_("Exposure Mode"),                       "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41986, 0xA402. Indicates the exposure mode set when the image was shot.") },
        { "WhiteBalance",             N_("White Balance"),                       "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41987, 0xA403. Indicates the white balance mode set when the image was shot.") },
        { "DigitalZoomRatio",         N_("Digital Zoom Ratio"),                  "Rational",                     xmpText, xmpInternal, N_("EXIF tag 41988, 0xA404. Indicates the digital zoom ratio when the image was shot.") },
        { "FocalLengthIn35mmFilm",    N_("Focal Length In 35mm Film"),           "Integer",                      xmpText, xmpInternal, N_("EXIF tag 41989, 0xA405. Indicates the equivalent focal length assuming a 35mm film "
                                                                                                                                          "camera, in mm. A value of 0 means the focal length is unknown. Note that this tag "
                                                                                                                                          "differs from the FocalLength tag.") },
        { "SceneCaptureType",         N_("Scene Capture Type"),                  "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41990, 0xA406. Indicates the type of scene that was shot.") },
        { "GainControl",              N_("Gain Control"),                        "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41991, 0xA407. Indicates the degree of overall image gain adjustment.") },
        { "Contrast",                 N_("Contrast"),                            "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41992, 0xA408. Indicates the direction of contrast processing applied by the camera.") },
        { "Saturation",               N_("Saturation"),                          "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41993, 0xA409. Indicates the direction of saturation processing applied by the camera.") },
        { "Sharpness",                N_("Sharpness"),                           "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41994, 0xA40A. Indicates the direction of sharpness processing applied by the camera.") },
        { "DeviceSettingDescription", N_("Device Setting Description"),          "DeviceSettings",               xmpText, xmpInternal, N_("EXIF tag 41995, 0xA40B. Indicates information on the picture-taking conditions of a particular camera model.") },
        { "SubjectDistanceRange",     N_("Subject Distance Range"),              "Closed Choice of Integer",     xmpText, xmpInternal, N_("EXIF tag 41996, 0xA40C. Indicates the distance to the subject.") },
        { "ImageUniqueID",            N_("Image Unique ID"),                     "Text",                         xmpText, xmpInternal, N_("EXIF tag 42016, 0xA420. An identifier assigned uniquely to each image. It is recorded as a 32 "
                                                                                                                                          "character ASCII string, equivalent to hexadecimal notation and 128-bit fixed length.") },
        { "GPSVersionID",             N_("GPS Version ID"),                      "Text",                         xmpText, xmpInternal, N_("GPS tag 0, 0x00. A decimal encoding of each of the four EXIF bytes with period separators. "
                                                                                                                                          "The current value is \"2.0.0.0\".") },
        { "GPSLatitude",              N_("GPS Latitude"),                        "GPSCoordinate",                xmpText, xmpInternal, N_("GPS tag 2, 0x02 (position) and 1, 0x01 (North/South). Indicates latitude.") },
        { "GPSLongitude",             N_("GPS Longitude"),                       "GPSCoordinate",                xmpText, xmpInternal, N_("GPS tag 4, 0x04 (position) and 3, 0x03 (East/West). Indicates longitude.") },
        { "GPSAltitudeRef",           N_("GPS Altitude Reference"),              "Closed Choice of Integer",     xmpText, xmpInternal, N_("GPS tag 5, 0x05. Indicates whether the altitude is above or below sea level.") },
        { "GPSAltitude",              N_("GPS Altitude"),                        "Rational",                     xmpText, xmpInternal, N_("GPS tag 6, 0x06. Indicates altitude in meters.") },
        { "GPSTimeStamp",             N_("GPS Time Stamp"),                      "Date",                         xmpText, xmpInternal, N_("GPS tag 29 (date), 0x1D, and, and GPS tag 7 (time), 0x07. Time stamp of GPS data, "
                                                                                                                                          "in Coordinated Universal Time. Note: The GPSDateStamp tag is new in EXIF 2.2. "
                                                                                                                                          "The GPS timestamp in EXIF 2.1 does not include a date. If not present, "
                                                                                                                                          "the date component for the XMP should be taken from exif:DateTimeOriginal, or if that is "
                                                                                                                                          "also lacking from exif:DateTimeDigitized. If no date is available, do not write "
                                                                                                                                          "exif:GPSTimeStamp to XMP.") },
        { "GPSSatellites",            N_("GPS Satellites"),                      "Text",                         xmpText, xmpInternal, N_("GPS tag 8, 0x08. Satellite information, format is unspecified.") },
        { "GPSStatus",                N_("GPS Status"),                          "Closed Choice of Text",        xmpText, xmpInternal, N_("GPS tag 9, 0x09. Status of GPS receiver at image creation time.") },
        { "GPSMeasureMode",           N_("GPS Measure Mode"),                    "Text",                         xmpText, xmpInternal, N_("GPS tag 10, 0x0A. GPS measurement mode, Text type.") },
        { "GPSDOP",                   N_("GPS DOP"),                             "Rational",                     xmpText, xmpInternal, N_("GPS tag 11, 0x0B. Degree of precision for GPS data.") },
        { "GPSSpeedRef",              N_("GPS Speed Reference"),                 "Closed Choice of Text",        xmpText, xmpInternal, N_("GPS tag 12, 0x0C. Units used to speed measurement.") },
        { "GPSSpeed",                 N_("GPS Speed"),                           "Rational",                     xmpText, xmpInternal, N_("GPS tag 13, 0x0D. Speed of GPS receiver movement.") },
        { "GPSTrackRef",              N_("GPS Track Reference"),                 "Closed Choice of Text",        xmpText, xmpInternal, N_("GPS tag 14, 0x0E. Reference for movement direction.") },
        { "GPSTrack",                 N_("GPS Track"),                           "Rational",                     xmpText, xmpInternal, N_("GPS tag 15, 0x0F. Direction of GPS movement, values range from 0 to 359.99.") },
        { "GPSImgDirectionRef",       N_("GPS Image Direction Reference"),       "Closed Choice of Text",        xmpText, xmpInternal, N_("GPS tag 16, 0x10. Reference for image direction.") },
        { "GPSImgDirection",          N_("GPS Image Direction"),                 "Rational",                     xmpText, xmpInternal, N_("GPS tag 17, 0x11. Direction of image when captured, values range from 0 to 359.99.") },
        { "GPSMapDatum",              N_("GPS Map Datum"),                       "Text",                         xmpText, xmpInternal, N_("GPS tag 18, 0x12. Geodetic survey data.") },
        { "GPSDestLatitude",          N_("GPS Destination Latitude"),            "GPSCoordinate",                xmpText, xmpInternal, N_("GPS tag 20, 0x14 (position) and 19, 0x13 (North/South). Indicates destination latitude.") },
        { "GPSDestLongitude",         N_("GPS Destination Longitude"),           "GPSCoordinate",                xmpText, xmpInternal, N_("GPS tag 22, 0x16 (position) and 21, 0x15 (East/West). Indicates destination longitude.") },
        { "GPSDestBearingRef",        N_("GPS Destination Bearing Reference"),   "Closed Choice of Text",        xmpText, xmpInternal, N_("GPS tag 23, 0x17. Reference for movement direction.") },
        { "GPSDestBearing",           N_("GPS Destination Bearing"),             "Rational",                     xmpText, xmpInternal, N_("GPS tag 24, 0x18. Destination bearing, values from 0 to 359.99.") },
        { "GPSDestDistanceRef",       N_("GPS Destination Distance Refefrence"), "Closed Choice  of Text",       xmpText, xmpInternal, N_("GPS tag 25, 0x19. Units used for speed measurement.") },
        { "GPSDestDistance",          N_("GPS Destination Distance"),            "Rational",                     xmpText, xmpInternal, N_("GPS tag 26, 0x1A. Distance to destination.") },
        { "GPSProcessingMethod",      N_("GPS Processing Method"),               "Text",                         xmpText, xmpInternal, N_("GPS tag 27, 0x1B. A character string recording the name of the method used for location finding.") },
        { "GPSAreaInformation",       N_("GPS Area Information"),                "Text",                         xmpText, xmpInternal, N_("GPS tag 28, 0x1C. A character string recording the name of the GPS area.") },
        { "GPSDifferential",          N_("GPS Differential"),                    "Closed Choice of Integer",     xmpText, xmpInternal, N_("GPS tag 30, 0x1E. Indicates whether differential correction is applied to the GPS receiver.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpAuxInfo[] = {
        { "Lens",             N_("Lens"),             "Text",        xmpText,          xmpInternal, N_("A description of the lens used to take the photograph. For example, \"70-200 mm f/2.8-4.0\".") },
        { "SerialNumber",     N_("SerialNumber"),     "Text",        xmpText,          xmpInternal, N_("The serial number of the camera or camera body used to take the photograph.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpIptcInfo[] = {
        { "CiAdrCity",          N_("Contact Info-City"),           "Text",                      xmpText, xmpExternal, N_("The contact information city part.") },
        { "CiAdrCtry",          N_("Contact Info-Country"),        "Text",                      xmpText, xmpExternal, N_("The contact information country part.") },
        { "CiAdrExtadr",        N_("Contact Info-Address"),        "Text",                      xmpText, xmpExternal, N_("The contact information address part. Comprises an optional company name and all required "
                                                                                                                         "information to locate the building or postbox to which mail should be sent.") },
        { "CiAdrPcode",         N_("Contact Info-Postal Code"),    "Text",                      xmpText, xmpExternal, N_("The contact information part denoting the local postal code.") },
        { "CiAdrRegion",        N_("Contact Info-State/Province"), "Text",                      xmpText, xmpExternal, N_("The contact information part denoting regional information like state or province.") },
        { "CiEmailWork",        N_("Contact Info-Email"),          "Text",                      xmpText, xmpExternal, N_("The contact information email address part.") },
        { "CiTelWork",          N_("Contact Info-Phone"),          "Text",                      xmpText, xmpExternal, N_("The contact information phone number part.") },
        { "CiUrlWork",          N_("Contact Info-Web URL"),        "Text",                      xmpText, xmpExternal, N_("The contact information web address part.") },
        { "CountryCode",        N_("Country Code"),                "closed Choice of Text",     xmpText, xmpExternal, N_("Code of the country the content is focussing on -- either the country shown in visual "
                                                                                                                         "media or referenced in text or audio media. This element is at the top/first level of "
                                                                                                                         "a top-down geographical hierarchy. The code should be taken from ISO 3166 two or three "
                                                                                                                         "letter code. The full name of a country should go to the \"Country\" element.") },
        { "CreatorContactInfo", N_("Creator's Contact Info"),      "ContactInfo",               xmpText, xmpExternal, N_("The creator's contact information provides all necessary information to get in contact "
                                                                                                                         "with the creator of this news object and comprises a set of sub-properties for proper addressing.") },
        { "IntellectualGenre",  N_("Intellectual Genre"),          "Text",                      xmpText, xmpExternal, N_("Describes the nature, intellectual or journalistic characteristic of a news object, not "
                                                                                                                         "specifically its content.") },
        { "Location",           N_("Location"),                    "Text",                      xmpText, xmpExternal, N_("Name of a location the content is focussing on -- either the location shown in visual "
                                                                                                                         "media or referenced by text or audio media. This location name could either be the name "
                                                                                                                         "of a sublocation to a city or the name of a well known location or (natural) monument "
                                                                                                                         "outside a city. In the sense of a sublocation to a city this element is at the fourth "
                                                                                                                         "level of a top-down geographical hierarchy.") },
        { "Scene",              N_("IPTC Scene"),                  "bag closed Choice of Text", xmpBag, xmpExternal, N_("Describes the scene of a photo content. Specifies one or more terms from the IPTC "
                                                                                                                        "\"Scene-NewsCodes\". Each Scene is represented as a string of 6 digits in an unordered list.") },
        { "SubjectCode",        N_("IPTC Subject Code"),           "bag closed Choice of Text", xmpBag, xmpExternal, N_("Specifies one or more Subjects from the IPTC \"Subject-NewsCodes\" taxonomy to "
                                                                                                                        "categorize the content. Each Subject is represented as a string of 8 digits in an unordered list.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpIptcExtInfo[] = {
        { "AddlModelInfo",           N_("Additional model info"),           "Text",                       xmpText, xmpExternal, N_("Information about the ethnicity and other facts of the model(s) in a model-released image.") },
        { "OrganisationInImageCode", N_("Code of featured Organisation"),   "bag Text",                   xmpBag,  xmpExternal, N_("Code from controlled vocabulary for identyfing the organisation or company which is featured in the image.") },
        { "CVterm",                  N_("Controlled Vocabulary Term"),      "bag URI",                    xmpBag,  xmpExternal, N_("A term to describe the content of the image by a value from a Controlled Vocabulary.") },
        { "ModelAge",                N_("Model age"),                       "bag Integer",                xmpBag,  xmpExternal, N_("Age of the human model(s) at the time this image was taken in a model released image.") },
        { "OrganisationInImageName", N_("Name of featured Organisation"),   "bag Text",                   xmpBag,  xmpExternal, N_("Name of the organisation or company which is featured in the image.") },
        { "PersonInImage",           N_("Person shown"),                    "bag Text",                   xmpBag,  xmpExternal, N_("Name of a person shown in the image.") },
        { "DigImageGUID",            N_("Digital Image Identifier"),        "Text",                       xmpText, xmpExternal, N_("Globally unique identifier for this digital image. It is created and applied by the creator of the digital image at the time of its creation. this value shall not be changed after that time.") },
        { "DigitalSourcefileType",   N_("Physical type of original photo"), "URI",                        xmpText, xmpExternal, N_("The type of the source digital file.") },
        { "Event",                   N_("Event"),                           "Lang Alt",                   langAlt, xmpExternal, N_("Names or describes the specific event at which the photo was taken.") },
        { "MaxAvailHeight",          N_("Maximum available height"),        "Integer",                    xmpText, xmpExternal, N_("The maximum available height in pixels of the original photo from which this photo has been derived by downsizing.") },
        { "MaxAvailWidth",           N_("Maximum available width"),         "Integer",                    xmpText, xmpExternal, N_("The maximum available width in pixels of the original photo from which this photo has been derived by downsizing.") },
        { "RegistryId",              N_("Registry Entry"),                  "bag RegistryEntryDetails",   xmpBag,  xmpExternal, N_("Both a Registry Item Id and a Registry Organisation Id to record any registration of this digital image with a registry.") },
        { "RegItemId",               N_("Registry Entry-Item Identifier"),  "Text",                       xmpText, xmpExternal, N_("A unique identifier created by a registry and applied by the creator of the digital image. This value shall not be changed after being applied. This identifier is linked to a corresponding Registry Organisation Identifier.") },
        { "RegOrgId",                N_("Registry Entry-Organisation Identifier"), "Text",                xmpText, xmpExternal, N_("An identifier for the registry which issued the corresponding Registry Image Id.") },
        { "IptcLastEdited",          N_("IPTC Fields Last Edited"),         "Date",                       xmpText, xmpExternal, N_("The date and optionally time when any of the IPTC photo metadata fields has been last edited.") },
        { "LocationShown",           N_("Location shown"),                  "bag LocationDetails",        xmpBag,  xmpExternal, N_("A location shown in the image.") },
        { "LocationCreated",         N_("Location Created"),                "bag LocationDetails",        xmpBag,  xmpExternal, N_("The location the photo was taken.") },
        { "City",                    N_("Location-City"),                   "Text",                       xmpText, xmpExternal, N_("Name of the city of a location.") },
        { "CountryCode",             N_("Location-Country ISO-Code"),       "Text",                       xmpText, xmpExternal, N_("The ISO code of a country of a location.") },
        { "CountryName",             N_("Location-Country Name"),           "Text",                       xmpText, xmpExternal, N_("The name of a country of a location.") },
        { "ProvinceState",           N_("Location-Province/State"),         "Text",                       xmpText, xmpExternal, N_("The name of a subregion of a country - a province or state - of a location.") },
        { "Sublocation",             N_("Location-Sublocation"),            "Text",                       xmpText, xmpExternal, N_("Name of a sublocation. This sublocation name could either be the name of a sublocation to a city or the name of a well known location or (natural) monument outside a city.") },
        { "WorldRegion",             N_("Location-World Region"),           "Text",                       xmpText, xmpExternal, N_("The name of a world region of a location.") },
        { "ArtworkOrObject",         N_("Artwork or object in the image"),  "bag ArtworkOrObjectDetails", xmpBag,  xmpExternal, N_("A set of metadata about artwork or an object in the image.") },
        { "AOCopyrightNotice",       N_("Artwork or object-Copyright notice"), "Text",                    xmpText, xmpExternal, N_("Contains any necessary copyright notice for claiming the intellectual property for artwork or an object in the image and should identify the current owner of the copyright of this work with associated intellectual property rights.") },
        { "AOCreator",               N_("Artwork or object-Creator"),       "seq ProperName",             xmpSeq,  xmpExternal, N_("Contains the name of the artist who has created artwork or an object in the image. In cases where the artist could or should not be identified the name of a company or organisation may be appropriate.") },
        { "AODateCreated",           N_("Artwork or object-Date Created"),  "Date",                       xmpText, xmpExternal, N_("Designates the date and optionally the time the artwork or object in the image was created. This relates to artwork or objects with associated intellectual property rights.") },
        { "AOSource",                N_("Artwork or object-Source"),        "Text",                       xmpText, xmpExternal, N_("The organisation or body holding and registering the artwork or object in the image for inventory purposes.") },
        { "AOSourceInvNo",           N_("Artwork or object-Source inventory number"), "Text",             xmpText, xmpExternal, N_("The inventory number issued by the organisation or body holding and registering the artwork or object in the image.") },
        { "AOTitle",                 N_("Artwork or object-Title"),         "Lang Alt",                   langAlt, xmpExternal, N_("A reference for the artwork or object in the image.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    //! XMP iptcExt:DigitalSourcefileType
    extern const TagVocabulary iptcExtDigitalSourcefileType[] = {
        { "scanfilm",         N_("Scan from film")                           },
        { "scantransparency", N_("Scan from transparency (including slide)") },
        { "scanprint",        N_("Scan from print")                          },
        { "cameraraw",        N_("Camera RAW")                               },
        { "cameratiff",       N_("Camera TIFF")                              },
        { "camerajpeg",       N_("Camera JPEG")                              }
    };

    extern const XmpPropertyInfo xmpPlusInfo[] = {
        // PLUS Version 1.2.0
        { "Version", N_("PLUS Version"), "Text", xmpText, xmpExternal, N_("The version number of the PLUS standards in place at the time of the transaction.") },
        { "Licensee", N_("Licensee"), "seq LicenseeDetail", xmpSeq, xmpExternal, N_("Party or parties to whom the license is granted by the Licensor/s under the license transaction.") },
        { "LicenseeID", N_("Licensee ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each Licensee.") },
        { "LicenseeName", N_("Licensee Name"), "ProperName", xmpText, xmpExternal, N_("Name of each Licensee.") },
        { "EndUser", N_("End User"), "seq EndUserDetail", xmpSeq, xmpExternal, N_("Party or parties ultimately making use of the image under the license.") },
        { "EndUserID", N_("End User ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each End User.") },
        { "EndUserName", N_("End User Name"), "ProperName", xmpText, xmpExternal, N_("Name of each End User.") },
        { "Licensor", N_("Licensor"), "seq LicensorDetail", xmpSeq, xmpExternal, N_("Party or parties granting the license to the Licensee.") },
        { "LicensorID", N_("Licensor ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each Licensor.") },
        { "LicensorName", N_("Licensor Name"), "ProperName", xmpText, xmpExternal, N_("Name of each Licensor.") },
        { "LicensorStreetAddress", N_("Licensor Address"), "Text", xmpText, xmpExternal, N_("Licensor street address.") },
        { "LicensorExtendedAddress", N_("Licensor Address Detail"), "Text", xmpText, xmpExternal, N_("Additional Licensor mailing address details.") },
        { "LicensorCity", N_("Licensor City"), "Text", xmpText, xmpExternal, N_("Licensor City name.") },
        { "LicensorRegion", N_("Licensor State or Province"), "Text", xmpText, xmpExternal, N_("Licensor State or Province name.") },
        { "LicensorPostalCode", N_("Licensor Postal Code"), "Text", xmpText, xmpExternal, N_("Licensor Postal Code or Zip Code.") },
        { "LicensorCountry", N_("Licensor Country"), "Text", xmpText, xmpExternal, N_("Licensor Country name.") },
        { "LicensorTelephoneType1", N_("Licensor Telephone Type 1"), "URL", xmpText, xmpExternal, N_("Licensor Telephone Type 1.") },
        { "LicensorTelephone1", N_("Licensor Telephone 1"), "Text", xmpText, xmpExternal, N_("Licensor Telephone number 1.") },
        { "LicensorTelephoneType2", N_("Licensor Telephone Type 2"), "URL", xmpText, xmpExternal, N_("Licensor Telephone Type 2.") },
        { "LicensorTelephone2", N_("Licensor Telephone 2"), "Text", xmpText, xmpExternal, N_("Licensor Telephone number 2.") },
        { "LicensorEmail", N_("Licensor Email"), "Text", xmpText, xmpExternal, N_("Licensor Email address.") },
        { "LicensorURL", N_("Licensor URL"), "URL", xmpText, xmpExternal, N_("Licensor world wide web address.") },
        { "LicensorNotes", N_("Licensor Notes"), "Lang Alt", langAlt, xmpExternal, N_("Supplemental information for use in identifying and contacting the Licensor/s.") },
        { "MediaSummaryCode", N_("PLUS Media Summary Code"), "Text", xmpText, xmpExternal, N_("A PLUS-standardized alphanumeric code string summarizing the media usages included in the license.") },
        { "LicenseStartDate", N_("License Start Date"), "Date", xmpText, xmpExternal, N_("The date on which the license takes effect.") },
        { "LicenseEndDate", N_("License End Date"), "Date", xmpText, xmpExternal, N_("The date on which the license expires.") },
        { "MediaConstraints", N_("Media Constraints"), "Lang Alt", langAlt, xmpExternal, N_("Constraints limiting the scope of PLUS Media Usage/s included in the license to particular named media or to media not yet specifically defined in the PLUS Media Matrix.") },
        { "RegionConstraints", N_("Region Constraints"), "Lang Alt", langAlt, xmpExternal, N_("Constraints limiting the scope of geographic distribution to specific cities, states, provinces or other areas to be included in or excluded from the PLUS Regions specified in the Media Usages specified in the license.") },
        { "ProductOrServiceConstraints", N_("Product or Service Constraints"), "Lang Alt", langAlt, xmpExternal, N_("Constraints limiting usage of the image to promotion of/association with a named product or service.") },
        { "ImageFileConstraints", N_("Image File Constraints"), "bag URL", xmpBag, xmpExternal, N_("Constraints on the changing of the image file name, metadata or file type.") },
        { "ImageAlterationConstraints", N_("Image Alteration Constraints"), "bag URL", xmpBag, xmpExternal, N_("Constraints on alteration of the image by cropping, flipping, retouching, colorization, de-colorization or merging.") },
        { "ImageDuplicationConstraints", N_("Image Duplication Constraints"), "URL", xmpText, xmpExternal, N_("Constraints on the creation of duplicates of the image.") },
        { "ModelReleaseStatus", N_("Model Release Status"), "URL", xmpText, xmpExternal, N_("Summarizes the availability and scope of model releases authorizing usage of the likenesses of persons appearing in the photograph.") },
        { "ModelReleaseID", N_("Model Release ID"), "bag Text", xmpBag, xmpExternal, N_("Optional identifier associated with each Model Release.") },
        { "MinorModelAgeDisclosure", N_("Minor Model Age Disclosure"), "URL", xmpText, xmpExternal, N_("Age of the youngest model pictured in the image, at the time that the image was made.") },
        { "PropertyReleaseStatus", N_("Property Release Status"), "URL", xmpText, xmpExternal, N_("Summarizes the availability and scope of property releases authorizing usage of the properties appearing in the photograph.") },
        { "PropertyReleaseID", N_("Property Release ID"), "bag Text", xmpBag, xmpExternal, N_("Optional identifier associated with each Property Release.") },
        { "OtherConstraints", N_("Other Constraints"), "Lang Alt", langAlt, xmpExternal, N_("Additional constraints on the license.") },
        { "CreditLineRequired", N_("Credit Line Required"), "URL", xmpText, xmpExternal, N_("Attribution requirements, if any.") },
        { "AdultContentWarning", N_("Adult Content Warning"), "URL", xmpText, xmpExternal, N_("Warning indicating the presence of content not suitable for minors.") },
        { "OtherLicenseRequirements", N_("Other License Requirements"), "Lang Alt", langAlt, xmpExternal, N_("Additional license requirements.") },
        { "TermsAndConditionsText", N_("Terms and Conditions Text"), "Lang Alt", langAlt, xmpExternal, N_("Terms and Conditions applying to the license.") },
        { "TermsAndConditionsURL", N_("Terms and Conditions URL"), "URL", xmpText, xmpExternal, N_("URL for Terms and Conditions applying to the license.") },
        { "OtherConditions", N_("Other License Conditions"), "Lang Alt", langAlt, xmpExternal, N_("Additional license conditions.") },
        { "ImageType", N_("Image Type"), "URL", xmpText, xmpExternal, N_("Identifies the type of image delivered.") },
        { "LicensorImageID", N_("Licensor Image ID"), "Text", xmpText, xmpExternal, N_("Optional identifier assigned by the Licensor to the image.") },
        { "FileNameAsDelivered", N_("Image File Name As Delivered"), "Text", xmpText, xmpExternal, N_("Name of the image file delivered to the Licensee for use under the license.") },
        { "ImageFileFormatAsDelivered", N_("Image File Format As Delivered"), "URL", xmpText, xmpExternal, N_("File format of the image file delivered to the Licensee for use under the license.") },
        { "ImageFileSizeAsDelivered", N_("Image File Size As Delivered"), "URL", xmpText, xmpExternal, N_("Size of the image file delivered to the Licensee.") },
        { "CopyrightStatus", N_("Copyright Status"), "URL", xmpText, xmpExternal, N_("Copyright status of the image.") },
        { "CopyrightRegistrationNumber", N_("Copyright Registration Number"), "Text", xmpText, xmpExternal, N_("Copyright Registration Number, if any, applying to the licensed image.") },
        { "FirstPublicationDate", N_("First Publication Date"), "Date", xmpText, xmpExternal, N_("The date on which the image was first published.") },
        { "CopyrightOwner", N_("Copyright Owner"), "seq CopyrightOwnerDetail", xmpSeq, xmpExternal, N_("Owner or owners of the copyright in the licensed image.") },
        { "CopyrightOwnerID", N_("Copyright Owner ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each Copyright Owner.") },
        { "CopyrightOwnerName", N_("Copyright Owner Name"), "ProperName", xmpText, xmpExternal, N_("Name of Copyright Owner.") },
        { "CopyrightOwnerImageID", N_("Copyright Owner Image ID"), "Text", xmpText, xmpExternal, N_("Optional identifier assigned by the Copyright Owner to the image.") },
        { "ImageCreator", N_("Image Creator"), "seq ImageCreatorDetail", xmpSeq, xmpExternal, N_("Creator/s of the image.") },
        { "ImageCreatorID", N_("Image Creator ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each Image Creator.") },
        { "ImageCreatorName", N_("Image Creator Name"), "ProperName", xmpText, xmpExternal, N_("Name of Image Creator.") },
        { "ImageCreatorImageID", N_("Image Creator Image ID"), "Text", xmpText, xmpExternal, N_("Optional identifier assigned by the Image Creator to the image.") },
        { "ImageSupplierID", N_("Image Supplier ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying the Image Supplier.") },
        { "ImageSupplierName", N_("Image Supplier Name"), "ProperName", xmpText, xmpExternal, N_("Name of Image Supplier.") },
        { "ImageSupplierImageID", N_("Image Supplier Image ID"), "Text", xmpText, xmpExternal, N_("Optional identifier assigned by the Image Supplier to the image.") },
        { "LicenseeImageID", N_("Licensee Image ID"), "Text", xmpText, xmpExternal, N_("Optional identifier assigned by the Licensee to the image.") },
        { "LicenseeImageNotes", N_("Licensee Image Notes"), "Lang Alt", langAlt, xmpExternal, N_("Notes added by Licensee.") },
        { "OtherImageInfo", N_("Other Image Info"), "Lang Alt", langAlt, xmpExternal, N_("Additional image information.") },
        { "LicenseID", N_("License ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID assigned by the Licensor to the License.") },
        { "LicensorTransactionID", N_("Licensor Transaction ID"), "bag Text", xmpBag, xmpExternal, N_("Identifier assigned by Licensor for Licensor's reference and internal use.") },
        { "LicenseeTransactionID", N_("Licensee Transaction ID"), "bag Text", xmpBag, xmpExternal, N_("Identifier assigned by Licensee for Licensee's reference and internal use.") },
        { "LicenseeProjectReference", N_("Licensee Project Reference"), "bag Text", xmpBag, xmpExternal, N_("Project reference name or description assigned by Licensee.") },
        { "LicenseTransactionDate", N_("License Transaction Date"), "Date", xmpText, xmpExternal, N_("The date of the License Transaction.") },
        { "Reuse", N_("Reuse"), "URL", xmpText, xmpExternal, N_("Indicates whether a license is a repeat or an initial license.  Reuse may require that licenses stored in files previously delivered to the customer be updated.") },
        { "OtherLicenseDocuments", N_("Other License Documents"), "bag Text", xmpBag, xmpExternal, N_("Reference information for additional documents associated with the license.") },
        { "OtherLicenseInfo", N_("Other License Info"), "Lang Alt", langAlt, xmpExternal, N_("Additional license information.") },
        { "Custom1", N_("Custom 1"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensor's discretion.") },
        { "Custom2", N_("Custom 2"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensor's discretion.") },
        { "Custom3", N_("Custom 3"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensor's discretion.") },
        { "Custom4", N_("Custom 4"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensor's discretion.") },
        { "Custom5", N_("Custom 5"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensor's discretion.") },
        { "Custom6", N_("Custom 6"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensee's discretion.") },
        { "Custom7", N_("Custom 7"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensee's discretion.") },
        { "Custom8", N_("Custom 8"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensee's discretion.") },
        { "Custom9", N_("Custom 9"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensee's discretion.") },
        { "Custom10", N_("Custom 10"), "bag Lang Alt", xmpBag, xmpExternal, N_("Optional field for use at Licensee's discretion.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    //! XMP plus:AdultContentWarning
    extern const TagVocabulary plusAdultContentWarning[] = {
        { "CW-AWR", N_("Adult Content Warning Required") },
        { "CW-NRQ", N_("Not Required")                   },
        { "CW-UNK", N_("Unknown")                        }
    };

    //! XMP plus:CopyrightStatus
    extern const TagVocabulary plusCopyrightStatus[] = {
        { "CS-PRO", N_("Protected")     },
        { "CS-PUB", N_("Public Domain") },
        { "CS-UNK", N_("Unknown")       }
    };

    //! XMP plus:CreditLineRequired
    extern const TagVocabulary plusCreditLineRequired[] = {
        { "CR-CAI", N_("Credit Adjacent To Image") },
        { "CR-CCA", N_("Credit in Credits Area")   },
        { "CR-COI", N_("Credit on Image")          },
        { "CR-NRQ", N_("Not Require")              }
    };

    //! XMP plus:ImageAlterationConstraints
    extern const TagVocabulary plusImageAlterationConstraints[] = {
        { "AL-CLR", N_("No Colorization")    },
        { "AL-CRP", N_("No Cropping")        },
        { "AL-DCL", N_("No De-Colorization") },
        { "AL-FLP", N_("No Flipping")        },
        { "AL-MRG", N_("No Merging")         },
        { "AL-RET", N_("No Retouching")      }
    };

    //! XMP plus:ImageDuplicationConstraints
    extern const TagVocabulary plusImageDuplicationConstraints[] = {
        { "DP-LIC", N_("Duplication Only as Necessary Under License") },
        { "DP-NDC", N_("No Duplication Constraints")                  },
        { "DP-NOD", N_("No Duplication")                              }
    };

    //! XMP plus:ImageFileConstraints
    extern const TagVocabulary plusImageFileConstraints[] = {
        { "IF-MFN", N_("Maintain File Name")       },
        { "IF-MFT", N_("Maintain File Type")       },
        { "IF-MID", N_("Maintain ID in File Name") },
        { "IF-MMD", N_("Maintain Metadata")        }
    };

    //! XMP plus:ImageFileFormatAsDelivered
    extern const TagVocabulary plusImageFileFormatAsDelivered[] = {
        { "FF-BMP", N_("Windows Bitmap (BMP)")                      },
        { "FF-DNG", N_("Digital Negative (DNG)")                    },
        { "FF-EPS", N_("Encapsulated PostScript (EPS)")             },
        { "FF-GIF", N_("Graphics Interchange Format (GIF)")         },
        { "FF-JPG", N_("JPEG Interchange Formats (JPG, JIF, JFIF)") },
        { "FF-OTR", N_("Other")                                     },
        { "FF-PIC", N_("Macintosh Picture (PICT)")                  },
        { "FF-PNG", N_("Portable Network Graphics (PNG)")           },
        { "FF-PSD", N_("Photoshop Document (PSD)")                  },
        { "FF-RAW", N_("Proprietary RAW Image Format")              },
        { "FF-TIF", N_("Tagged Image File Format (TIFF)")           },
        { "FF-WMP", N_("Windows Media Photo (HD Photo)")            }
    };

    //! XMP plus:ImageFileSizeAsDelivered
    extern const TagVocabulary plusImageFileSizeAsDelivered[] = {
        { "SZ-G50", N_("Greater than 50 MB") },
        { "SZ-U01", N_("Up to 1 MB")         },
        { "SZ-U10", N_("Up to 10 MB")        },
        { "SZ-U30", N_("Up to 30 MB")        },
        { "SZ-U50", N_("Up to 50 MB")        }
    };

    //! XMP plus:ImageType
    extern const TagVocabulary plusImageType[] = {
        { "TY-ILL", N_("Illustrated Image")              },
        { "TY-MCI", N_("Multimedia or Composited Image") },
        { "TY-OTR", N_("Other")                          },
        { "TY-PHO", N_("Photographic Image")             },
        { "TY-VID", N_("Video")                          }
    };

    //! XMP plus:LicensorTelephoneType
    extern const TagVocabulary plusLicensorTelephoneType[] = {
        { "cell",  N_("Cell")  },
        { "fax",   N_("FAX")   },
        { "home",  N_("Home")  },
        { "pager", N_("Pager") },
        { "work",  N_("Work")  }
    };

    //! XMP plus:MinorModelAgeDisclosure
    extern const TagVocabulary plusMinorModelAgeDisclosure[] = {
        { "AG-UNK", N_("Age Unknown")     },
        { "AG-A25", N_("Age 25 or Over")  },
        { "AG-A24", N_("Age 24")          },
        { "AG-A23", N_("Age 23")          },
        { "AG-A22", N_("Age 22")          },
        { "AG-A21", N_("Age 21")          },
        { "AG-A20", N_("Age 20")          },
        { "AG-A19", N_("Age 19")          },
        { "AG-A18", N_("Age 18")          },
        { "AG-A17", N_("Age 17")          },
        { "AG-A16", N_("Age 16")          },
        { "AG-A15", N_("Age 15")          },
        { "AG-U14", N_("Age 14 or Under") }
    };

    //! XMP plus:ModelReleaseStatus
    extern const TagVocabulary plusModelReleaseStatus[] = {
        { "MR-NON", N_("None")                                 },
        { "MR-NAP", N_("Not Applicable")                       },
        { "MR-UMR", N_("Unlimited Model Releases")             },
        { "MR-LMR", N_("Limited or Incomplete Model Releases") }
    };

    //! XMP plus:PropertyReleaseStatus
    extern const TagVocabulary plusPropertyReleaseStatus[] = {
        { "PR-NON", N_("None")                                    },
        { "PR-NAP", N_("Not Applicable")                          },
        { "PR-UPR", N_("Unlimited Property Releases")             },
        { "PR-LPR", N_("Limited or Incomplete Property Releases") }
    };

    //! XMP plus:Reuse
    extern const TagVocabulary plusReuse[] = {
        { "RE-NAP", N_("Not Applicable") },
        { "RE-REU", N_("Repeat Use")     }
    };

    extern const XmpPropertyInfo xmpMediaProInfo[] = {
        { "Event",       N_("Event"),       "Text",     xmpText, xmpExternal, N_("Fixture Identification")                          },
        { "Status",      N_("Status"),      "Text",     xmpText, xmpExternal, N_("A notation making the image unique")              },
        { "People",      N_("People"),      "bag Text", xmpBag,  xmpExternal, N_("Contact")                                         },
        { "CatalogSets", N_("CatalogSets"), "bag Text", xmpBag,  xmpExternal, N_("Descriptive markers of catalog items by content") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpExpressionMediaInfo[] = {
        { "Event",       N_("Event"),       "Text",     xmpText, xmpExternal, N_("Fixture Identification")                          },
        { "Status",      N_("Status"),      "Text",     xmpText, xmpExternal, N_("A notation making the image unique")              },
        { "People",      N_("People"),      "bag Text", xmpBag,  xmpExternal, N_("Contact")                                         },
        { "CatalogSets", N_("CatalogSets"), "bag Text", xmpBag,  xmpExternal, N_("Descriptive markers of catalog items by content") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMicrosoftPhotoInfo[] = {
        { "RegionInfo", N_("RegionInfo"), "RegionInfo", xmpText, xmpInternal, N_("Microsoft Photo people-tagging metadata root") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfoInfo[] = {
        { "DateRegionsValid", N_("DateRegionsValid"), "Date",       xmpText, xmpExternal, N_("Date the last region was created")  },
        { "Regions",          N_("Regions"),          "bag Region", xmpBag,  xmpExternal, N_("Contains Regions/person tags") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfo[] = {
        { "PersonDisplayName", N_("PersonDisplayName"), "Text", xmpText, xmpExternal, N_("Name of the person (in the given rectangle)")                               },
        { "Rectangle",         N_("Rectangle"),         "Text", xmpText, xmpExternal, N_("Rectangle that identifies the person within the photo")                     },
        { "PersonEmailDigest", N_("PersonEmailDigest"), "Text", xmpText, xmpExternal, N_("SHA-1 encrypted message hash of the person's Windows Live e-mail address"), },
        { "PersonLiveCID",     N_("PersonLiveCID"),     "Text", xmpText, xmpExternal, N_("Signed decimal representation of the person's Windows Live CID")            },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMWGRegionsInfo[] = {
        { "Regions",             N_("Regions"),             "RegionInfo",       xmpText, xmpInternal,        N_("Main structure containing region based information")   },
        { "AppliedToDimensions", N_("AppliedToDimensions"), "Dimensions",       xmpText, xmpExternal,        N_("Width and height of image when storing region data")   },
        { "RegionList",          N_("RegionList"),          "bag RegionStruct", xmpBag,  xmpExternal,        N_("List of Region structures")                            },
        { "Area",                N_("Area"),                "Area",             xmpText, xmpExternal,        N_("Descriptive markers of catalog items by content")      },
        { "Type",                N_("Type"),                "closed Choice of Text", xmpText, xmpExternal,   N_("Type purpose of region (Face|Pet|Focus|BarCode)")      },
        { "Name",                N_("Name"),                "Text",             xmpText, xmpExternal,        N_("Name/ short description of content in image region")   },
        { "Description",         N_("Description"),         "Text",             xmpText, xmpExternal,        N_("Usage scenario for a given focus area (EvaluatedUsed|EvaluatedNotUsed|NotEvaluatedNotUsed)") },
        { "FocusUsage",          N_("FocusUsage"),          "closed Choice of Text", xmpText, xmpExternal,   N_("Descriptive markers of catalog items by content")      },
        { "BarCodeValue",        N_("BarCodeValue"),        "Text",             xmpText, xmpExternal,        N_("Decoded BarCode value string")                         },
        { "Extensions",          N_("Extensions"),          "Text",             xmpText, xmpInternal, N_("Any top level XMP property to describe the region content") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPrintInfo xmpPrintInfo[] = {
        {"Xmp.crs.CropUnits",                 EXV_PRINT_TAG(crsCropUnits)   },
        {"Xmp.exif.ApertureValue",            print0x9202                   },
        {"Xmp.exif.BrightnessValue",          printFloat                    },
        {"Xmp.exif.ColorSpace",               print0xa001                   },
        {"Xmp.exif.ComponentsConfiguration",  print0x9101                   },
        {"Xmp.exif.Contrast",                 printNormalSoftHard           },
        {"Xmp.exif.CreateDate",               printXmpDate                  },
        {"Xmp.exif.CustomRendered",           print0xa401                   },
        {"Xmp.exif.DateTimeOriginal",         printXmpDate                  },
        {"Xmp.exif.ExifVersion",              printXmpVersion               },
        {"Xmp.exif.ExposureBiasValue",        print0x9204                   },
        {"Xmp.exif.ExposureMode",             print0xa402                   },
        {"Xmp.exif.ExposureProgram",          print0x8822                   },
        {"Xmp.exif.FileSource",               print0xa300                   },
        {"Xmp.exif.FlashpixVersion",          printXmpVersion               },
        {"Xmp.exif.FNumber",                  print0x829d                   },
        {"Xmp.exif.FocalLength",              print0x920a                   },
        {"Xmp.exif.FocalPlaneResolutionUnit", printExifUnit                 },
        {"Xmp.exif.FocalPlaneXResolution",    printFloat                    },
        {"Xmp.exif.FocalPlaneYResolution",    printFloat                    },
        {"Xmp.exif.GainControl",              print0xa407                   },
        {"Xmp.exif.GPSAltitudeRef",           print0x0005                   },
        {"Xmp.exif.GPSDestBearingRef",        printGPSDirRef                },
        {"Xmp.exif.GPSDestDistanceRef",       print0x0019                   },
        {"Xmp.exif.GPSDifferential",          print0x001e                   },
        {"Xmp.exif.GPSImgDirectionRef",       printGPSDirRef                },
        {"Xmp.exif.GPSMeasureMode",           print0x000a                   },
        {"Xmp.exif.GPSSpeedRef",              print0x000c                   },
        {"Xmp.exif.GPSStatus",                print0x0009                   },
        {"Xmp.exif.GPSTimeStamp",             printXmpDate                  },
        {"Xmp.exif.GPSTrackRef",              printGPSDirRef                },
        {"Xmp.exif.LightSource",              print0x9208                   },
        {"Xmp.exif.MeteringMode",             print0x9207                   },
        {"Xmp.exif.ModifyDate",               printXmpDate                  },
        {"Xmp.exif.Saturation",               print0xa409                   },
        {"Xmp.exif.SceneCaptureType",         print0xa406                   },
        {"Xmp.exif.SceneType",                print0xa301                   },
        {"Xmp.exif.SensingMethod",            print0xa217                   },
        {"Xmp.exif.Sharpness",                printNormalSoftHard           },
        {"Xmp.exif.ShutterSpeedValue",        print0x9201                   },
        {"Xmp.exif.SubjectDistanceRange",     print0xa40c                   },
        {"Xmp.exif.WhiteBalance",             print0xa403                   },
        {"Xmp.tiff.Orientation",              print0x0112                   },
        {"Xmp.tiff.ResolutionUnit",           printExifUnit                 },
        {"Xmp.tiff.XResolution",              printLong                     },
        {"Xmp.tiff.YCbCrPositioning",         print0x0213                   },
        {"Xmp.tiff.YResolution",              printLong                     },
        {"Xmp.iptcExt.DigitalSourcefileType",    EXV_PRINT_VOCABULARY(iptcExtDigitalSourcefileType)   },
        {"Xmp.plus.AdultContentWarning",         EXV_PRINT_VOCABULARY(plusAdultContentWarning)        },
        {"Xmp.plus.CopyrightStatus",             EXV_PRINT_VOCABULARY(plusCopyrightStatus)            },
        {"Xmp.plus.CreditLineRequired",          EXV_PRINT_VOCABULARY(plusCreditLineRequired)         },
        {"Xmp.plus.ImageAlterationConstraints",  EXV_PRINT_VOCABULARY(plusImageAlterationConstraints) },
        {"Xmp.plus.ImageDuplicationConstraints", EXV_PRINT_VOCABULARY(plusImageDuplicationConstraints)},
        {"Xmp.plus.ImageFileConstraints",        EXV_PRINT_VOCABULARY(plusImageFileConstraints)       },
        {"Xmp.plus.ImageFileFormatAsDelivered",  EXV_PRINT_VOCABULARY(plusImageFileFormatAsDelivered) },
        {"Xmp.plus.ImageFileSizeAsDelivered",    EXV_PRINT_VOCABULARY(plusImageFileSizeAsDelivered)   },
        {"Xmp.plus.ImageType",                   EXV_PRINT_VOCABULARY(plusImageType)                  },
        {"Xmp.plus.LicensorTelephoneType1",      EXV_PRINT_VOCABULARY(plusLicensorTelephoneType)      },
        {"Xmp.plus.LicensorTelephoneType2",      EXV_PRINT_VOCABULARY(plusLicensorTelephoneType)      },
        {"Xmp.plus.MinorModelAgeDisclosure",     EXV_PRINT_VOCABULARY(plusMinorModelAgeDisclosure)    },
        {"Xmp.plus.ModelReleaseStatus",          EXV_PRINT_VOCABULARY(plusModelReleaseStatus)         },
        {"Xmp.plus.PropertyReleaseStatus",       EXV_PRINT_VOCABULARY(plusPropertyReleaseStatus)      },
        {"Xmp.plus.Reuse",                       EXV_PRINT_VOCABULARY(plusReuse)                      }
    };

    XmpNsInfo::Ns::Ns(const std::string& ns)
        : ns_(ns)
    {
    }

    XmpNsInfo::Prefix::Prefix(const std::string& prefix)
        : prefix_(prefix)
    {
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Ns& ns) const
    {
        std::string n(ns_);
        return n == ns.ns_;
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Prefix& prefix) const
    {
        std::string p(prefix_);
        return p == prefix.prefix_;
    }

    bool XmpPropertyInfo::operator==(const std::string& name) const
    {
        std::string n(name_);
        return n == name;
    }

    XmpProperties::NsRegistry XmpProperties::nsRegistry_;

    const XmpNsInfo* XmpProperties::lookupNsRegistry(const XmpNsInfo::Prefix& prefix)
    {
        for (NsRegistry::const_iterator i = nsRegistry_.begin();
             i != nsRegistry_.end(); ++i) {
            if (i->second == prefix) return &(i->second);
        }
        return 0;
    }

    void XmpProperties::registerNs(const std::string& ns,
                                   const std::string& prefix)
    {
        std::string ns2 = ns;
        if (   ns2.substr(ns2.size() - 1, 1) != "/"
            && ns2.substr(ns2.size() - 1, 1) != "#") ns2 += "/";
        // Check if there is already a registered namespace with this prefix
        const XmpNsInfo* xnp = lookupNsRegistry(XmpNsInfo::Prefix(prefix));
        if (xnp) {
#ifndef SUPPRESS_WARNINGS
            if (strcmp(xnp->ns_, ns2.c_str()) != 0) {
                EXV_WARNING << "Updating namespace URI for " << prefix << " from "
                            << xnp->ns_ << " to " << ns2 << "\n";
            }
#endif
            unregisterNs(xnp->ns_);
        }
        // Allocated memory is freed when the namespace is unregistered.
        // Using malloc/free for better system compatibility in case
        // users don't unregister their namespaces explicitly.
        XmpNsInfo xn;
        char* c = static_cast<char*>(std::malloc(ns2.size() + 1));
        std::strcpy(c, ns2.c_str());
        xn.ns_ = c;
        c = static_cast<char*>(std::malloc(prefix.size() + 1));
        std::strcpy(c, prefix.c_str());
        xn.prefix_ = c;
        xn.xmpPropertyInfo_ = 0;
        xn.desc_ = "";
        nsRegistry_[ns2] = xn;
    }

    void XmpProperties::unregisterNs(const std::string& ns)
    {
        NsRegistry::iterator i = nsRegistry_.find(ns);
        if (i != nsRegistry_.end()) {
            std::free(const_cast<char*>(i->second.prefix_));
            std::free(const_cast<char*>(i->second.ns_));
            nsRegistry_.erase(i);
        }
    }

    void XmpProperties::unregisterNs()
    {
        NsRegistry::iterator i = nsRegistry_.begin();
        while (i != nsRegistry_.end()) {
            NsRegistry::iterator kill = i++;
            unregisterNs(kill->first);
        }
    }

    std::string XmpProperties::prefix(const std::string& ns)
    {
        std::string ns2 = ns;
        if (   ns2.substr(ns2.size() - 1, 1) != "/"
            && ns2.substr(ns2.size() - 1, 1) != "#") ns2 += "/";
        NsRegistry::const_iterator i = nsRegistry_.find(ns2);
        std::string p;
        if (i != nsRegistry_.end()) {
            p = i->second.prefix_;
        }
        else {
            const XmpNsInfo* xn = find(xmpNsInfo, XmpNsInfo::Ns(ns2));
            if (xn) p = std::string(xn->prefix_);
        }
        return p;
    }

    std::string XmpProperties::ns(const std::string& prefix)
    {
        const XmpNsInfo* xn = lookupNsRegistry(XmpNsInfo::Prefix(prefix));
        if (xn != 0) return xn->ns_;
        return nsInfo(prefix)->ns_;
    }

    const char* XmpProperties::propertyTitle(const XmpKey& key)
    {
        const XmpPropertyInfo* pi = propertyInfo(key);
        return pi ? pi->title_ : 0;
    }

    const char* XmpProperties::propertyDesc(const XmpKey& key)
    {
        const XmpPropertyInfo* pi = propertyInfo(key);
        return pi ? pi->desc_ : 0;
    }

    TypeId XmpProperties::propertyType(const XmpKey& key)
    {
        const XmpPropertyInfo* pi = propertyInfo(key);
        return pi ? pi->typeId_ : xmpText;
    }

    const XmpPropertyInfo* XmpProperties::propertyInfo(const XmpKey& key)
    {
        std::string prefix = key.groupName();
        std::string property = key.tagName();
        // If property is a path for a nested property, determines the innermost element
        std::string::size_type i = property.find_last_of('/');
        if (i != std::string::npos) {
            for (; i != std::string::npos && !isalpha(property[i]); ++i) {}
            property = property.substr(i);
            i = property.find_first_of(':');
            if (i != std::string::npos) {
                prefix = property.substr(0, i);
                property = property.substr(i+1);
            }
#ifdef DEBUG
            std::cout << "Nested key: " << key.key() << ", prefix: " << prefix
                      << ", property: " << property << "\n";
#endif
        }
        const XmpPropertyInfo* pl = propertyList(prefix);
        if (!pl) return 0;
        const XmpPropertyInfo* pi = 0;
        for (int i = 0; pl[i].name_ != 0; ++i) {
            if (0 == strcmp(pl[i].name_, property.c_str())) {
                pi = pl + i;
                break;
            }
        }
        return pi;
    }

    const char* XmpProperties::nsDesc(const std::string& prefix)
    {
        return nsInfo(prefix)->desc_;
    }

    const XmpPropertyInfo* XmpProperties::propertyList(const std::string& prefix)
    {
        return nsInfo(prefix)->xmpPropertyInfo_;
    }

    const XmpNsInfo* XmpProperties::nsInfo(const std::string& prefix)
    {
        const XmpNsInfo::Prefix pf(prefix);
        const XmpNsInfo* xn = lookupNsRegistry(pf);
        if (!xn) xn = find(xmpNsInfo, pf);
        if (!xn) throw Error(35, prefix);
        return xn;
    }

    void XmpProperties::printProperties(std::ostream& os, const std::string& prefix)
    {
        const XmpPropertyInfo* pl = propertyList(prefix);
        if (pl) {
            for (int i = 0; pl[i].name_ != 0; ++i) {
                os << pl[i];
            }
        }

    } // XmpProperties::printProperties

    std::ostream& XmpProperties::printProperty(std::ostream& os,
                                               const std::string& key,
                                               const Value& value)
    {
        PrintFct fct = printValue;
        if (value.count() != 0) {
            const XmpPrintInfo* info = find(xmpPrintInfo, key);
            if (info) fct = info->printFct_;
        }
        return fct(os, value, 0);
    }

    //! @cond IGNORE

    //! Internal Pimpl structure with private members and data of class XmpKey.
    struct XmpKey::Impl {
        Impl() {}                       //!< Default constructor
        Impl(const std::string& prefix, const std::string& property); //!< Constructor

        /*!
          @brief Parse and convert the \em key string into property and prefix.
                 Updates data members if the string can be decomposed, or throws
                 \em Error.

          @throw Error if the key cannot be decomposed.
        */
        void decomposeKey(const std::string& key);

        // DATA
        static const char* familyName_; //!< "Xmp"

        std::string prefix_;            //!< Prefix
        std::string property_;          //!< Property name
    };
    //! @endcond

    XmpKey::Impl::Impl(const std::string& prefix, const std::string& property)
    {
        // Validate prefix
        if (XmpProperties::ns(prefix).empty()) throw Error(46, prefix);

        property_ = property;
        prefix_ = prefix;
    }

    const char* XmpKey::Impl::familyName_ = "Xmp";

    XmpKey::XmpKey(const std::string& key)
        : p_(new Impl)
    {
        p_->decomposeKey(key);
    }

    XmpKey::XmpKey(const std::string& prefix, const std::string& property)
        : p_(new Impl(prefix, property))
    {
    }

    XmpKey::~XmpKey()
    {
        delete p_;
    }

    XmpKey::XmpKey(const XmpKey& rhs)
        : Key(rhs), p_(new Impl(*rhs.p_))
    {
    }

    XmpKey& XmpKey::operator=(const XmpKey& rhs)
    {
        if (this == &rhs) return *this;
        Key::operator=(rhs);
        *p_ = *rhs.p_;
        return *this;
    }

    XmpKey::AutoPtr XmpKey::clone() const
    {
        return AutoPtr(clone_());
    }

    XmpKey* XmpKey::clone_() const
    {
        return new XmpKey(*this);
    }

    std::string XmpKey::key() const
    {
        return std::string(p_->familyName_) + "." + p_->prefix_ + "." + p_->property_;
    }

    const char* XmpKey::familyName() const
    {
        return p_->familyName_;
    }

    std::string XmpKey::groupName() const
    {
        return p_->prefix_;
    }

    std::string XmpKey::tagName() const
    {
        return p_->property_;
    }

    std::string XmpKey::tagLabel() const
    {
        const char* pt = XmpProperties::propertyTitle(*this);
        if (!pt) return tagName();
        return pt;
    }

    uint16_t XmpKey::tag() const
    {
        return 0;
    }

    std::string XmpKey::ns() const
    {
        return XmpProperties::ns(p_->prefix_);
    }

    void XmpKey::Impl::decomposeKey(const std::string& key)
    {
        // Get the family name, prefix and property name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string familyName = key.substr(0, pos1);
        if (0 != strcmp(familyName.c_str(), familyName_)) {
            throw Error(6, key);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string prefix = key.substr(pos0, pos1 - pos0);
        if (prefix == "") throw Error(6, key);
        std::string property = key.substr(pos1 + 1);
        if (property == "") throw Error(6, key);

        // Validate prefix
        if (XmpProperties::ns(prefix).empty()) throw Error(46, prefix);

        property_ = property;
        prefix_ = prefix;
    } // XmpKey::Impl::decomposeKey

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const XmpPropertyInfo& property)
    {
        return os << property.name_                       << ",\t"
                  << property.title_                      << ",\t"
                  << property.xmpValueType_               << ",\t"
                  << TypeInfo::typeName(property.typeId_) << ",\t"
                  << ( property.xmpCategory_ == xmpExternal ? "External" : "Internal" ) << ",\t"
                  << property.desc_                       << "\n";
    }
    //! @endcond

}                                       // namespace Exiv2
