// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
  Version:   $Rev: 3831 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: properties.cpp 3831 2015-05-20 01:27:32Z asp $")

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
    extern const XmpPropertyInfo xmpMWGKeywordInfo[];
    extern const XmpPropertyInfo xmpVideoInfo[];
    extern const XmpPropertyInfo xmpAudioInfo[];
    extern const XmpPropertyInfo xmpDwCInfo[];
    extern const XmpPropertyInfo xmpDctermsInfo[];
    extern const XmpPropertyInfo xmpLrInfo[];
    extern const XmpPropertyInfo xmpAcdseeInfo[];
    extern const XmpPropertyInfo xmpGPanoInfo[];    

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
        { "http://ns.adobe.com/lightroom/1.0/",           "lr",             xmpLrInfo,        N_("Adobe Lightroom schema")                    },
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
        { "http://ns.microsoft.com/expressionmedia/1.0/", "expressionmedia", xmpExpressionMediaInfo, N_("Expression Media schema")            },
        { "http://ns.microsoft.com/photo/1.2/",           "MP",             xmpMicrosoftPhotoInfo,           N_("Microsoft Photo 1.2 schema")        },
        { "http://ns.microsoft.com/photo/1.2/t/RegionInfo#", "MPRI",        xmpMicrosoftPhotoRegionInfoInfo, N_("Microsoft Photo RegionInfo schema") },
        { "http://ns.microsoft.com/photo/1.2/t/Region#",     "MPReg",       xmpMicrosoftPhotoRegionInfo,     N_("Microsoft Photo Region schema")     },
        { "http://www.metadataworkinggroup.com/schemas/regions/", "mwg-rs", xmpMWGRegionsInfo, N_("Metadata Working Group Regions schema")      },
        { "http://www.metadataworkinggroup.com/schemas/keywords/","mwg-kw", xmpMWGKeywordInfo, N_("Metadata Working Group Keywords schema")     },
        { "http://www.video",                             "video",          xmpVideoInfo,     N_("XMP Extended Video schema")                   },
        { "http://www.audio",                             "audio",          xmpAudioInfo,     N_("XMP Extended Audio schema")                   },
        { "http://rs.tdwg.org/dwc/index.htm",             "dwc",            xmpDwCInfo,       N_("XMP Darwin Core schema")     		            },
        { "http://purl.org/dc/terms/",                    "dcterms",        xmpDctermsInfo,   N_("Qualified Dublin Core schema")                }, // Note: used as properties under dwc:record
        { "http://ns.acdsee.com/iptc/1.0/",               "acdsee",         xmpAcdseeInfo,    N_("ACDSee XMP schema")                           },
        { "http://ns.google.com/photos/1.0/panorama/",    "GPano",          xmpGPanoInfo,     N_("Google Photo Sphere XMP schema")              },


        // Structures
        { "http://ns.adobe.com/xap/1.0/g/",                   "xapG",    0, N_("Colorant structure")           },
        { "http://ns.adobe.com/xap/1.0/sType/Dimensions#",    "stDim",   0, N_("Dimensions structure")         },
        { "http://ns.adobe.com/xap/1.0/sType/Font#",          "stFnt",   0, N_("Font structure")               },
        { "http://ns.adobe.com/xap/1.0/g/img/",               "xmpGImg", 0, N_("Thumbnail structure")          },
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
        { "ImageHistory",           N_("Image History"),             "Text",     xmpText, xmpExternal, N_("An XML based content to list all action processed on this image with image editor (as crop, rotate, color corrections, adjustments, etc.).") },
        { "LensCorrectionSettings", N_("Lens Correction Settings"),  "Text",     xmpText, xmpExternal, N_("The list of Lens Correction tools settings used to fix lens distortion. This include Batch Queue Manager and Image editor tools based on LensFun library.") },
        { "ColorLabel",             N_("Color Label"),               "Text",     xmpText, xmpExternal, N_("The color label assigned to this item. Possible values are \"0\": no label; \"1\": Red; \"2\": Orange; \"3\": Yellow; \"4\": Green; \"5\": Blue; \"6\": Magenta; \"7\": Gray; \"8\": Black; \"9\": White.") },
        { "PickLabel",              N_("Pick Label"),                "Text",     xmpText, xmpExternal, N_("The pick label assigned to this item. Possible values are \"0\": no label; \"1\": item rejected; \"2\": item in pending validation; \"3\": item accepted.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpKipiInfo[] = {
        { "PanoramaInputFiles",     N_("Panorama Input Files"),      "Text",     xmpText, xmpExternal, N_("The list of files processed with Hugin program through Panorama tool.") },
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
        { "loop",                         N_("Loop"),                             "Boolean",               xmpText, xmpInternal, N_("When true, the clip can be looped seamlessly.") },
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

    extern const XmpPropertyInfo xmpLrInfo[] = {
        { "hierarchicalSubject",    N_("Hierarchical Subject"),    "bag Text",  xmpBag,      xmpExternal, N_("Adobe Lightroom hierarchical keywords.")   },
        { "privateRTKInfo",         N_("Private RTK Info"),        "Text",      xmpText,     xmpExternal, N_("Adobe Lightroom private RTK info.")        },
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
        { "ColorNoiseReduction",  N_("Color Noise Reduction"),     "Integer",                          xmpText, xmpExternal, N_("\"Color Noise Reduction\" setting. Range 0 to +100.") },
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
        { "GreenHue",             N_("Green Hue"),                 "Integer",                          xmpText, xmpExternal, N_("\"Green Hue\" setting. Range -100 to +100.") },
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
        { "GPSDestDistanceRef",       N_("GPS Destination Distance Reference"),  "Closed Choice  of Text",       xmpText, xmpInternal, N_("GPS tag 25, 0x19. Units used for speed measurement.") },
        { "GPSDestDistance",          N_("GPS Destination Distance"),            "Rational",                     xmpText, xmpInternal, N_("GPS tag 26, 0x1A. Distance to destination.") },
        { "GPSProcessingMethod",      N_("GPS Processing Method"),               "Text",                         xmpText, xmpInternal, N_("GPS tag 27, 0x1B. A character string recording the name of the method used for location finding.") },
        { "GPSAreaInformation",       N_("GPS Area Information"),                "Text",                         xmpText, xmpInternal, N_("GPS tag 28, 0x1C. A character string recording the name of the GPS area.") },
        { "GPSDifferential",          N_("GPS Differential"),                    "Closed Choice of Integer",     xmpText, xmpInternal, N_("GPS tag 30, 0x1E. Indicates whether differential correction is applied to the GPS receiver.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpAuxInfo[] = {
        { "Lens",             N_("Lens"),             "Text",        xmpText,          xmpInternal, N_("A description of the lens used to take the photograph. For example, \"70-200 mm f/2.8-4.0\".") },
        { "SerialNumber",     N_("Serial Number"),     "Text",       xmpText,          xmpInternal, N_("The serial number of the camera or camera body used to take the photograph.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpIptcInfo[] = {
        { "CreatorContactInfo", N_("Creator's Contact Info"),      "ContactInfo",               xmpText, xmpExternal, N_("The creator's contact information provides all necessary information to get in contact "
                                                                                                                         "with the creator of this news object and comprises a set of sub-properties for proper addressing.") },
        { "CiAdrExtadr",        N_("Contact Info-Address"),        "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: address. Comprises an optional company name and all required "
                                                                                                                         "information to locate the building or postbox to which mail should be sent.") },
        { "CiAdrCity",          N_("Contact Info-City"),           "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: city.") },
        { "CiAdrRegion",        N_("Contact Info-State/Province"), "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: state or province.") },
        { "CiAdrPcode",         N_("Contact Info-Postal Code"),    "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: local postal code.") },
        { "CiAdrCtry",          N_("Contact Info-Country"),        "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: country.") },
        { "CiEmailWork",        N_("Contact Info-Email"),          "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: email address.") },
        { "CiTelWork",          N_("Contact Info-Phone"),          "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: phone number.") },
        { "CiUrlWork",          N_("Contact Info-Web URL"),        "Text",                      xmpText, xmpExternal, N_("sub-key Creator Contact Info: web address.") },
        { "IntellectualGenre",  N_("Intellectual Genre"),          "Text",                      xmpText, xmpExternal, N_("Describes the nature, intellectual or journalistic characteristic of a news object, not "
                                                                                                                         "specifically its content.") },
        { "Scene",              N_("IPTC Scene"),                  "bag closed Choice of Text", xmpBag, xmpExternal, N_("Describes the scene of a photo content. Specifies one or more terms from the IPTC "
                                                                                                                        "\"Scene-NewsCodes\". Each Scene is represented as a string of 6 digits in an unordered list.") },
        { "SubjectCode",        N_("IPTC Subject Code"),           "bag closed Choice of Text", xmpBag, xmpExternal, N_("Specifies one or more Subjects from the IPTC \"Subject-NewsCodes\" taxonomy to "
                                                                                                                        "categorize the content. Each Subject is represented as a string of 8 digits in an unordered list.") },
        { "Location",           N_("Location"),                    "Text",                      xmpText, xmpExternal, N_("(legacy) Name of a location the content is focussing on -- either the location shown in visual "
                                                                                                                         "media or referenced by text or audio media. This location name could either be the name "
                                                                                                                         "of a sublocation to a city or the name of a well known location or (natural) monument "
                                                                                                                         "outside a city. In the sense of a sublocation to a city this element is at the fourth "
                                                                                                                         "level of a top-down geographical hierarchy.") },
        { "CountryCode",        N_("Country Code"),                "closed Choice of Text",     xmpText, xmpExternal, N_("(legacy) Code of the country the content is focussing on -- either the country shown in visual "
                                                                                                                         "media or referenced in text or audio media. This element is at the top/first level of "
                                                                                                                         "a top-down geographical hierarchy. The code should be taken from ISO 3166 two or three "
                                                                                                                         "letter code. The full name of a country should go to the \"Country\" element.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpIptcExtInfo[] = {
        { "AddlModelInfo",           N_("Additional model info"),           "Text",                       xmpText, xmpExternal, N_("Information about the ethnicity and other facts of the model(s) in a model-released image.") },
        { "OrganisationInImageCode", N_("Code of featured Organisation"),   "bag Text",                   xmpBag,  xmpExternal, N_("Code from controlled vocabulary for identifying the organisation or company which is featured in the image.") },
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
        { "CatalogSets", N_("Catalog Sets"), "bag Text", xmpBag,  xmpExternal, N_("Descriptive markers of catalog items by content") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpExpressionMediaInfo[] = {
        { "Event",       N_("Event"),       "Text",     xmpText, xmpExternal, N_("Fixture Identification")                          },
        { "Status",      N_("Status"),      "Text",     xmpText, xmpExternal, N_("A notation making the image unique")              },
        { "People",      N_("People"),      "bag Text", xmpBag,  xmpExternal, N_("Contact")                                         },
        { "CatalogSets", N_("Catalog Sets"), "bag Text", xmpBag,  xmpExternal, N_("Descriptive markers of catalog items by content") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMicrosoftPhotoInfo[] = {
        { "RegionInfo", N_("RegionInfo"), "RegionInfo", xmpText, xmpInternal, N_("Microsoft Photo people-tagging metadata root") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfoInfo[] = {
        { "Regions",          N_("Regions"),            "bag Region", xmpBag,  xmpExternal, N_("Contains Regions/person tags") },
        { "DateRegionsValid", N_("Date Regions Valid"), "Date",       xmpText, xmpExternal, N_("Date the last region was created")  },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfo[] = {
        { "PersonDisplayName", N_("Person Display Name"),   "Text", xmpText, xmpExternal, N_("Name of the person (in the given rectangle)")                               },
        { "Rectangle",         N_("Rectangle"),             "Text", xmpText, xmpExternal, N_("Rectangle that identifies the person within the photo")                     },
        { "PersonEmailDigest", N_("Person Email Digest"),   "Text", xmpText, xmpExternal, N_("SHA-1 encrypted message hash of the person's Windows Live e-mail address"), },
        { "PersonLiveIdCID",   N_("Person LiveId CID"),     "Text", xmpText, xmpExternal, N_("Signed decimal representation of the person's Windows Live CID")            },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMWGRegionsInfo[] = {
        { "Regions",             N_("Regions"),               "RegionInfo",       xmpText, xmpInternal,        N_("Main structure containing region based information")   },
        { "AppliedToDimensions", N_("Applied To Dimensions"), "Dimensions",       xmpText, xmpExternal,        N_("Width and height of image when storing region data")   },
        { "RegionList",          N_("Region List"),           "bag RegionStruct", xmpBag,  xmpExternal,        N_("List of Region structures")                            },
        { "Area",                N_("Area"),                  "Area",             xmpText, xmpExternal,        N_("Descriptive markers of catalog items by content")      },
        { "Type",                N_("Type"),                  "closed Choice of Text", xmpText, xmpExternal,   N_("Type purpose of region (Face|Pet|Focus|BarCode)")      },
        { "Name",                N_("Name"),                  "Text",             xmpText, xmpExternal,        N_("Name/ short description of content in image region")   },
        { "Description",         N_("Description"),           "Text",             xmpText, xmpExternal,        N_("Usage scenario for a given focus area (EvaluatedUsed|EvaluatedNotUsed|NotEvaluatedNotUsed)") },
        { "FocusUsage",          N_("Focus Usage"),           "closed Choice of Text", xmpText, xmpExternal,   N_("Descriptive markers of catalog items by content")      },
        { "BarCodeValue",        N_("Bar Code Value"),        "Text",             xmpText, xmpExternal,        N_("Decoded BarCode value string")                         },
        { "Extensions",          N_("Extensions"),            "Text",             xmpText, xmpInternal,        N_("Any top level XMP property to describe the region content") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpMWGKeywordInfo[] = {
        { "Keywords",       N_("Keywords"),     "KeywordInfo",          xmpText, xmpInternal,   N_("Main structure containing keyword based information")   },
        { "Hierarchy",      N_("Hierarchy"),    "bag KeywordStruct",    xmpBag,  xmpExternal,   N_("List of root keyword structures")   },
        { "Keyword",        N_("Keyword"),      "Text",                 xmpText, xmpExternal,   N_("Name of keyword (-node)")   },
        { "Applied",        N_("Applied"),      "Boolean",              xmpText, xmpExternal,   N_("True if this keyword has been applied, False otherwise. If missing, mwg-kw:Applied is presumed True for leaf nodes and False for ancestor nodes")   },
        { "Children",       N_("Children"),     "bag KeywordStruct",    xmpBag,  xmpExternal,   N_("List of children keyword structures")   },

        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpGPanoInfo[] = {
        { "UsePanoramaViewer",              N_("Use Panorama Viewer"),              "Boolean",              xmpText, xmpExternal,   N_("Whether to show this image in a panorama viewer rather than as a normal flat image. This may be specified based on user preferences or by the stitching software. The application displaying or ingesting the image may choose to ignore this.")   },
        { "CaptureSoftware",                N_("Capture Software"),                 "Text",                 xmpText, xmpExternal,   N_("If capture was done using an application on a mobile device, such as an Android phone, the name of the application that was used (such as \"Photo Sphere\"). This should be left blank if source images were captured manually, such as by using a DSLR on a tripod.")   },
        { "StitchingSoftware",              N_("Stitching Software"),               "Text",                 xmpText, xmpExternal,   N_("The software that was used to create the final panorama. This may sometimes be the same value as that of  GPano:CaptureSoftware.")   },
        { "ProjectionType",                 N_("Projection Type"),                  "Open Choice of Text",  xmpText, xmpExternal,   N_("Projection type used in the image file. Google products currently support the value equirectangular.")   },
        { "PoseHeadingDegrees",             N_("Pose Heading Degrees"),             "Real",                 xmpText, xmpExternal,   N_("Compass heading, measured in degrees, for the center the image. Value must be >= 0 and < 360.")   },
        { "PosePitchDegrees",               N_("Pose Pitch Degrees"),               "Real",                 xmpText, xmpExternal,   N_("Pitch, measured in degrees, for the center in the image. Value must be >= -90 and <= 90.")   },
        { "PoseRollDegrees",                N_("Pose Roll Degrees"),                "Real",                 xmpText, xmpExternal,   N_("Roll, measured in degrees, of the image where level with the horizon is 0. Value must be > -180 and <= 180.")   },
        { "InitialViewHeadingDegrees",      N_("Initial View Heading Degrees"),     "Integer",              xmpText, xmpExternal,   N_("The heading angle of the initial view in degrees.")   },
        { "InitialViewPitchDegrees",        N_("Initial View Pitch Degrees"),       "Integer",              xmpText, xmpExternal,   N_("The pitch angle of the initial view in degrees.")   },
        { "InitialViewRollDegrees",         N_("Initial View Roll Degrees"),        "Integer",              xmpText, xmpExternal,   N_("The roll angle of the initial view in degrees.")   },
        { "InitialHorizontalFOVDegrees",    N_("Initial Horizontal FOV Degrees"),   "Real",                 xmpText, xmpExternal,   N_("The initial horizontal field of view that the viewer should display (in degrees). This is similar to a zoom level.")   },
        { "FirstPhotoDate",                 N_("First Photo Date"),                 "Date",                 xmpText, xmpExternal,   N_("Date and time for the first image created in the panorama.")   },
        { "LastPhotoDate",                  N_("Last Photo Date"),                  "Date",                 xmpText, xmpExternal,   N_("Date and time for the last image created in the panorama.")   },
        { "SourcePhotosCount",              N_("Source Photos Count"),              "Integer",              xmpText, xmpExternal,   N_("Number of source images used to create the panorama")   },
        { "ExposureLockUsed",               N_("Exposure Lock Used"),               "Boolean",              xmpText, xmpExternal,   N_("When individual source photographs were captured, whether or not the camera's exposure setting was locked. ")   },
        { "CroppedAreaImageWidthPixels",    N_("Cropped Area Image Width Pixels"),  "Integer",              xmpText, xmpExternal,   N_("Original width in pixels of the image (equal to the actual image's width for unedited images).")   },
        { "CroppedAreaImageHeightPixels",   N_("Cropped Area Image Height Pixels"), "Integer",              xmpText, xmpExternal,   N_("Original height in pixels of the image (equal to the actual image's height for unedited images).")   },
        { "FullPanoWidthPixels",            N_("Full Pano Width Pixels"),           "Integer",              xmpText, xmpExternal,   N_("Original full panorama width from which the image was cropped. Or, if only a partial panorama was captured, this specifies the width of what the full panorama would have been.")   },
        { "FullPanoHeightPixels",           N_("Full Pano Height Pixels"),          "Integer",              xmpText, xmpExternal,   N_("Original full panorama height from which the image was cropped. Or, if only a partial panorama was captured, this specifies the height of what the full panorama would have been.")   },
        { "CroppedAreaLeftPixels",          N_("Cropped Area Left Pixels"),         "Integer",              xmpText, xmpExternal,   N_("Column where the left edge of the image was cropped from the full sized panorama.")   },
        { "CroppedAreaTopPixels",           N_("Cropped Area Top Pixels"),          "Integer",              xmpText, xmpExternal,   N_("Row where the top edge of the image was cropped from the full sized panorama.")   },
        { "InitialCameraDolly",             N_("Initial Camera Dolly"),             "Real",                 xmpText, xmpExternal,   N_("This optional parameter moves the virtual camera position along the line of sight, away from the center of the photo sphere. A rear surface position is represented by the value -1.0, while a front surface position is represented by 1.0. For normal viewing, this parameter should be set to 0.")   },

        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpVideoInfo[] = {
        { "Album",                  N_("Album"),                            "Text",                     xmpText, xmpExternal, N_("The name of the album.")   },
        { "ArchivalLocation",       N_("Archival Location"),                "Text",                     xmpText, xmpExternal, N_("Information about the Archival Location.")   },
        { "Arranger",               N_("Arranger"),                         "Text",                     xmpText, xmpExternal, N_("Information about the Arranger.")   },
        { "ArrangerKeywords",       N_("Arranger Keywords"),                "Text",                     xmpText, xmpExternal, N_("Information about the Arranger Keywords.")   },
        { "Artist",                 N_("Artist"),                           "Text",                     xmpText, xmpExternal, N_("The name of the artist or artists.")   },
        { "AspectRatio",            N_("Video Aspect Ratio"),               "Ratio",                    xmpText, xmpExternal, N_("Ratio of Width:Height, helps to determine how a video would be displayed on a screen")   },
        { "AspectRatioType",        N_("Video Aspect Ratio Type"),          "Text",                     xmpText, xmpExternal, N_("Aspect Ratio Type. Eg - Free-Resizing or Fixed")   },
        { "AttachFileData",         N_("Attached File Data"),               "Text",                     xmpText, xmpExternal, N_("Attached File Data")   },
        { "AttachFileDesc",         N_("Attached File Description"),        "Text",                     xmpText, xmpExternal, N_("Attached File Description")   },
        { "AttachFileMIME",         N_("Attached File MIME Type"),          "Text",                     xmpText, xmpExternal, N_("Attached File MIME Type")   },
        { "AttachFileName",         N_("Attached File Name"),               "Text",                     xmpText, xmpExternal, N_("Attached File Name")   },
        { "AttachFileUID",          N_("Attached File UID"),                "Integer",                  xmpText, xmpExternal, N_("Attached File Universal ID")   },
        { "BaseURL",                N_("Base URL"),                         "Text",                     xmpText, xmpExternal, N_("A C string that specifies a Base URL.")   },
        { "BitDepth",               N_("Bit Depth"),                        "Integer",                  xmpText, xmpExternal, N_("A 16-bit integer that indicates the pixel depth of the compressed image. Values of 1, 2, 4, 8 , 16, 24, and 32 indicate the depth of color images")   },
        { "Brightness",             N_("Brightness"),                       "Integer",                  xmpText, xmpExternal, N_("Brightness setting.") },
        { "CameraByteOrder",        N_("Camera Byte Order"),                "Text",                     xmpText, xmpExternal, N_("Byte Order used by the Video Capturing device.")   },
        { "Cinematographer",        N_("Video Cinematographer"),            "Text",                     xmpText, xmpExternal, N_("The video Cinematographer information.")   },
        { "CleanApertureWidth",     N_("Clean Aperture Width"),             "Integer",                  xmpText, xmpExternal, N_("Clean aperture width in pixels")   },
        { "CleanApertureHeight",    N_("Clean Aperture Height"),            "Integer",                  xmpText, xmpExternal, N_("Clean aperture height in pixels")   },
        { "Codec",                  N_("Video Codec"),                      "Text",                     xmpText, xmpExternal, N_("The video codec information. Informs about the encoding algorithm of video. Codec Info is required for video playback.")   },
        { "CodecDecodeAll",         N_("Video Codec Decode Info"),          "Text",                     xmpText, xmpExternal, N_("Contains information the video Codec Decode All, i.e. Enabled/Disabled")   },
        { "CodecDescription",       N_("Video Codec Description"),          "Text",                     xmpText, xmpExternal, N_("Contains description the codec.")   },
        { "CodecInfo",              N_("Video Codec Information"),          "Text",                     xmpText, xmpExternal, N_("Contains information the codec needs before decoding can be started.")   },
        { "CodecDownloadUrl",       N_("Video Codec Download URL"),         "Text",                     xmpText, xmpExternal, N_("Video Codec Download URL.")   },
        { "CodecSettings",          N_("Video Codec Settings"),             "Text",                     xmpText, xmpExternal, N_("Contains settings the codec needs before decoding can be started.")   },
        { "ColorMode",              N_("Color Mode"),                       "Text",                     xmpText, xmpExternal, N_("Color Mode")   },
        { "ColorNoiseReduction",    N_("Color Noise Reduction"),            "Integer",                  xmpText, xmpExternal, N_("\"Color Noise Reduction\" setting. Range 0 to +100.") },
        { "ColorSpace",             N_("Video Color Space"),                "closed Choice of Text",    xmpText, xmpInternal, N_("The color space. One of: sRGB (used by Photoshop), CCIR-601 (used for NTSC), "
                                                                                                                                 "CCIR-709 (used for HD).") },
        { "Comment",                N_("Comment"),                          "Text",                     xmpText, xmpExternal, N_("Information about the Comment.")   },
        { "Commissioned",           N_("Commissioned"),                     "Text",                     xmpText, xmpExternal, N_("Commissioned.")   },
        { "CompatibleBrands",       N_("QTime Compatible FileType Brand"),  "Text",                     xmpText, xmpExternal, N_("Other QuickTime Compatible FileType Brand")   },
        { "Composer",               N_("Composer"),                         "Text",                     xmpText, xmpExternal, N_("Information about the Composer.")   },
        { "ComposerKeywords",       N_("Composer Keywords"),                "Text",                     xmpText, xmpExternal, N_("Information about the Composer Keywords.")   },
        { "Compressor",             N_("Compressor"),                       "Text",                     xmpText, xmpExternal, N_("Video Compression Library Used")   },
        { "CompressorID",           N_("Video Compressor ID"),              "Text",                     xmpText, xmpExternal, N_("Video Compression ID of Technology/Codec Used")   },
        { "CompressorVersion",      N_("Compressor Version"),               "Text",                     xmpText, xmpExternal, N_("Information about the Compressor Version.")   },
        { "Container",              N_("Container Type"),                   "Text",                     xmpText, xmpExternal, N_("Primary Metadata Container")   },
        { "ContentCompressAlgo",    N_("Content Compression Algorithm"),    "Text",                     xmpText, xmpExternal, N_("Content Compression Algorithm. Eg: zlib")   },
        { "ContentEncodingType",    N_("Content Encoding Type"),            "Text",                     xmpText, xmpExternal, N_("Content Encoding Type. Eg: Encryption or Compression")   },
        { "ContentEncryptAlgo",     N_("Content Encryption Algorithm"),     "Text",                     xmpText, xmpExternal, N_("Content Encryption Algorithm. Eg: Blowfish")   },
        { "ContentSignAlgo",        N_("Content Signature Algorithm"),      "Text",                     xmpText, xmpExternal, N_("Content Signature Algorithm. Eg: RSA")   },
        { "ContentSignHashAlgo",    N_("Content Sign Hash Algorithm"),      "Text",                     xmpText, xmpExternal, N_("Content Signature Hash Algorithm. Eg: SHA1-160 or MD5")   },
        { "Contrast",               N_("Contrast"),                         "Closed Choice of Integer", xmpText, xmpInternal, N_("Indicates the direction of contrast processing applied by the camera.") },
        { "Copyright",              N_("Copyright"),                        "Text",                     xmpText, xmpExternal, N_("Copyright, can be name of an organization or an individual.")   },
        { "CostumeDesigner",        N_("Costume Designer"),                 "Text",                     xmpText, xmpExternal, N_("Costume Designer associated with the video.")   },
        { "Country",                N_("Country"),                          "Text",                     xmpText, xmpExternal, N_("Name of the country where the video was created.")   },
        { "CreationDate",           N_("Creation Date"),                    "Integer",                  xmpText, xmpExternal, N_("Specifies the date and time of the initial creation of the file. The value is given as the "
                                                                                                                                 "number of 100-nanosecond intervals since January 1, 1601, according to Coordinated Universal Time (Greenwich Mean Time).")   },
        { "CropBottom",             N_("Pixel Crop Bottom"),                "Integer",                  xmpText, xmpExternal, N_("Number of Pixels to be cropped from the bottom.")   },
        { "CropLeft",               N_("Pixel Crop Left"),                  "Integer",                  xmpText, xmpExternal, N_("Number of Pixels to be cropped from the left.")   },
        { "CropRight",              N_("Pixel Crop Right"),                 "Integer",                  xmpText, xmpExternal, N_("Number of Pixels to be cropped from the right.")   },
        { "Cropped",                N_("Cropped"),                          "Integer",                  xmpText, xmpExternal, N_("Field that indicates if a video is cropped.")   },
        { "CropTop",                N_("Pixel Crop Top"),                   "Integer",                  xmpText, xmpExternal, N_("Number of Pixels to be cropped from the top.")   },
        { "CurrentTime",            N_("Current Time"),                     "Integer",                  xmpText, xmpExternal, N_("The time value for current time position within the movie.") },
        { "DataPackets",            N_("Data Packets"),                     "Integer",                  xmpText, xmpExternal, N_("Specifies the number of Data Packet entries that exist within the Data Object.")   },
        { "DateTimeOriginal",       N_("Date and Time Original"),           "Date",                     xmpText, xmpInternal, N_("Date and time when original video was generated, in ISO 8601 format. ") },
        { "DateTimeDigitized",      N_("Date and Time Digitized"),          "Date",                     xmpText, xmpInternal, N_("Date and time when video was stored as digital data, can be the same "
                                                                                                                                 "as DateTimeOriginal if originally stored in digital form. Stored in ISO 8601 format.") },
        { "DateUTC",                N_("Date-Time Original"),               "Text",                     xmpText, xmpExternal, N_("Contains the production date")   },
        { "DefaultOn",              N_("Video Track Default On"),           "Text",                     xmpText, xmpExternal, N_("Video Track Default On , i.e. Enabled/Disabled")   },
        { "DigitalZoomRatio",       N_("Digital Zoom Ratio"),               "Rational",                 xmpText, xmpInternal, N_("Indicates the digital zoom ratio when the video was shot.") },
        { "Dimensions",             N_("Dimensions"),                       "Text",                     xmpText, xmpExternal, N_("Information about the Dimensions of the video frame.")   },
        { "Director",               N_("Director"),                         "Text",                     xmpText, xmpExternal, N_("Information about the Director.")   },
        { "DisplayUnit",            N_("Video Display Unit"),               "Text",                     xmpText, xmpExternal, N_("Video display unit. Eg - cm, pixels, inch")   },
        { "DistributedBy",          N_("Distributed By"),                   "Text",                     xmpText, xmpExternal, N_("Distributed By, i.e. name of person or organization.")   },
        { "DocType",                N_("Doc Type"),                         "Text",                     xmpText, xmpExternal, N_("Describes the contents of the file. In the case of a MATROSKA file, its value is 'matroska'")   },
        { "DocTypeReadVersion",     N_("Doc Type Read Version"),            "Integer",                  xmpText, xmpExternal, N_("A Matroska video specific property, helps in determining the compatibility of file with a particular version of a video player")   },
        { "DocTypeVersion",         N_("Doc Type Version"),                 "Integer",                  xmpText, xmpExternal, N_("A Matroska video specific property, indicated the version of filetype, helps in determining the compatibility")   },
        { "DotsPerInch",            N_("Dots Per Inch"),                    "Integer",                  xmpText, xmpExternal, N_("Dots Per Inch")   },
        { "duration",               N_("Duration"),                         "Integer",                  xmpText, xmpExternal, N_("The duration of the media file. Measured in milli-seconds.")   },
        { "EBMLReadVersion",        N_("EBML Read Version"),                "Integer",                  xmpText, xmpExternal, N_("Extensible Binary Meta Language Read Version")   },
        { "EBMLVersion",            N_("EBML Version"),                     "Integer",                  xmpText, xmpExternal, N_("Extensible Binary Meta Language Version")   },
        { "Edit1",                  N_("Edit Block 1 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit2",                  N_("Edit Block 2 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit3",                  N_("Edit Block 3 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit4",                  N_("Edit Block 4 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit5",                  N_("Edit Block 5 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit6",                  N_("Edit Block 6 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit7",                  N_("Edit Block 7 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit8",                  N_("Edit Block 8 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "Edit9",                  N_("Edit Block 9 / Language"),          "Text",                     xmpText, xmpExternal, N_("Information about the Edit / Language.")   },
        { "EditedBy",               N_("Edited By"),                        "Text",                     xmpText, xmpExternal, N_("Edited By, i.e. name of person or organization.")   },
        { "Enabled",                N_("Video Track Enabled"),              "Text",                     xmpText, xmpExternal, N_("Status of Video Track, i.e. Enabled/Disabled")   },
        { "EncodedBy",              N_("Encoded By"),                       "Text",                     xmpText, xmpExternal, N_("Encoded By, i.e. name of person or organization.")   },
        { "EncodedPixelsWidth",     N_("Encoded Pixels Width"),             "Integer",                  xmpText, xmpExternal, N_("Encoded Pixels width in pixels")   },
        { "EncodedPixelsHeight",    N_("Encoded Pixels Height"),            "Integer",                  xmpText, xmpExternal, N_("Encoded Pixels height in pixels")   },
        { "Encoder",                N_("Encoder"),                          "Text",                     xmpText, xmpExternal, N_("Information about the Encoder.")   },
        { "EndTimecode",            N_("End Timecode"),                     "Integer",                  xmpText, xmpExternal, N_("End Timecode")   },
        { "Engineer",               N_("Engineer"),                         "Text",                     xmpText, xmpExternal, N_("Engineer, in most cases name of person.")   },
        { "Equipment",              N_("Equipment"),                        "Text",                     xmpText, xmpExternal, N_("Information about the Equipment used for recording Video.")   },
        { "ExposureCompensation",   N_("Exposure Compensation"),            "Text",                     xmpText, xmpExternal, N_("Exposure Compensation Information.")   },
        { "ExposureProgram",        N_("Exposure Program"),                 "Text",                     xmpText, xmpExternal, N_("Exposure Program Information.")   },
        { "ExposureTime",           N_("Exposure Time"),                    "Rational",                 xmpText, xmpInternal, N_("Exposure time in seconds.") },
        { "ExtendedContentDescription",N_("Extended Content Description"),  "Text",                     xmpSeq,  xmpExternal, N_("Extended Content Description, usually found in ASF type files.")   },
        { "FileDataRate",           N_("File Data Rate"),                   "Rational",                 xmpText, xmpExternal, N_("The file data rate in megabytes per second. For example: \"36/10\" = 3.6 MB/sec") },
        { "FileID",                 N_("File ID"),                          "Text",                     xmpText, xmpExternal, N_("File ID.")   },
        { "FileLength",             N_("File Length"),                      "Integer",                  xmpText, xmpInternal, N_("File length.") },
        { "FileName",               N_("File Name"),                        "Text",                     xmpText, xmpExternal, N_("File Name or Absolute File Path")   },
        { "FileSize",               N_("File Size"),                        "Integer",                  xmpText, xmpExternal, N_("File Size, in MB")   },
        { "FileType",               N_("File Type"),                        "Text",                     xmpText, xmpExternal, N_("Extension of File or Type of File")   },
        { "FilterEffect",           N_("Filter Effect"),                    "Text",                     xmpText, xmpExternal, N_("Filter Effect Settings Applied.")   },
        { "FirmwareVersion",        N_("Firmware Version"),                 "Text",                     xmpText, xmpExternal, N_("Firmware Version of the Camera/Video device.")   },
        { "FNumber",                N_("F Number"),                         "Rational",                 xmpText, xmpInternal, N_("F number. Camera Lens specific data.") },
        { "FocalLength",            N_("Focal Length"),                     "Rational",                 xmpText, xmpInternal, N_("Focal length of the lens, in millimeters.") },
        { "FocusMode",              N_("Focus Mode"),                       "Text",                     xmpText, xmpExternal, N_("Focus Mode of the Lens. Eg - AF for Auto Focus")   },
        { "Format",                 N_("Format"),                           "Text",                     xmpText, xmpExternal, N_("Indication of movie format (computer-generated, digitized, and so on).")   },
        { "FrameCount",             N_("Frame Count"),                      "Integer",                  xmpText, xmpExternal, N_("Total number of frames in a video")   },
        { "FrameHeight",            N_("Frame Height"),                     "Integer",                  xmpText, xmpExternal, N_("Height of frames in a video")   },
        { "FrameRate",              N_("Video Frame Rate"),                 "Frames per Second",        xmpText, xmpExternal, N_("Rate at which frames are presented in a video (Expressed in fps(Frames per Second))")   },
        { "FrameSize",              N_("Video Frame Size"),                 "Dimensions",               xmpText, xmpExternal, N_("The frame size. For example: w:720, h: 480, unit:pixels") },
        { "FrameWidth",             N_("Frame Width"),                      "Integer",                  xmpText, xmpExternal, N_("Width of frames in a video")   },
        { "Genre",                  N_("Genre"),                            "Text",                     xmpText, xmpExternal, N_("The name of the genre.")   },
        { "GPSAltitude",            N_("GPS Altitude"),                     "Rational",                 xmpText, xmpInternal, N_("GPS tag 6, 0x06. Indicates altitude in meters.") },
        { "GPSAltitudeRef",         N_("GPS Altitude Reference"),           "Closed Choice of Integer", xmpText, xmpInternal, N_("GPS tag 5, 0x05. Indicates whether the altitude is above or below sea level.") },
        { "GPSCoordinates",         N_("GPS Coordinates"),                  "Text",                     xmpText, xmpExternal, N_("Information about the GPS Coordinates.")   },
        { "GPSDateStamp",           N_("GPS Time Stamp"),                   "Date",                     xmpText, xmpInternal, N_("Date stamp of GPS data, ") },
        { "GPSImgDirection",        N_("GPS Image Direction"),              "Rational",                 xmpText, xmpInternal, N_("Direction of image when captured, values range from 0 to 359.99.") },
        { "GPSImgDirectionRef",     N_("GPS Image Direction Reference"),    "Closed Choice of Text",    xmpText, xmpInternal, N_("Reference for image direction.") },
        { "GPSLatitude",            N_("GPS Latitude"),                     "GPSCoordinate",            xmpText, xmpInternal, N_("(North/South). Indicates latitude.") },
        { "GPSLongitude",           N_("GPS Longitude"),                    "GPSCoordinate",            xmpText, xmpInternal, N_("(East/West). Indicates longitude.") },
        { "GPSMapDatum",            N_("GPS Map Datum"),                    "Text",                     xmpText, xmpInternal, N_("Geodetic survey data.") },
        { "GPSSatellites",          N_("GPS Satellites"),                   "Text",                     xmpText, xmpInternal, N_("Satellite information, format is unspecified.") },
        { "GPSTimeStamp",           N_("GPS Time Stamp"),                   "Date",                     xmpText, xmpInternal, N_("Time stamp of GPS data, ") },
        { "GPSVersionID",           N_("GPS Version ID"),                   "Text",                     xmpText, xmpInternal, N_("A decimal encoding with period separators. ") },
        { "GraphicsMode",           N_("Graphics Mode"),                    "Text",                     xmpText, xmpExternal, N_("A 16-bit integer that specifies the transfer mode. The transfer mode specifies which Boolean"
                                                                                                                                 "operation QuickDraw should perform when drawing or transferring an image from one location to another.")   },
        { "Grouping",               N_("Grouping"),                         "Text",                     xmpText, xmpExternal, N_("Information about the Grouping.")   },
        { "HandlerClass",           N_("Handler Class"),                    "Text",                     xmpText, xmpExternal, N_("A four-character code that identifies the type of the handler. Only two values are valid for this field: 'mhlr' for media handlers and 'dhlr' for data handlers.")   },
        { "HandlerDescription",     N_("Handler Description"),              "Text",                     xmpText, xmpExternal, N_("A (counted) string that specifies the name of the component-that is, the media handler used when this media was created..")   },
        { "HandlerType",            N_("Handler Type"),                     "Text",                     xmpText, xmpExternal, N_("A four-character code that identifies the type of the media handler or data handler.")   },
        { "HandlerVendorID",        N_("Handler Vendor ID"),                "Text",                     xmpText, xmpExternal, N_("Component manufacturer.")   },
        { "Height",                 N_("Video Height"),                     "Integer",                  xmpText, xmpExternal, N_("Video height in pixels")   },
        { "HueAdjustment",          N_("Hue Adjustment"),                   "Integer",                  xmpText, xmpExternal, N_("Hue Adjustment Settings Information.")   },
        { "ImageLength",            N_("Image Length"),                     "Integer",                  xmpText, xmpExternal, N_("Image Length, a property inherited from BitMap format") },
        { "InfoBannerImage",        N_("Info Banner Image"),                "Text",                     xmpText, xmpExternal, N_("Information Banner Image.")   },
        { "InfoBannerURL",          N_("Info Banner URL"),                  "Text",                     xmpText, xmpExternal, N_("Information Banner URL.")   },
        { "Information",            N_("Information"),                      "Text",                     xmpText, xmpExternal, N_("Additional Movie Information.")   },
        { "InfoText",               N_("Info Text"),                        "Text",                     xmpText, xmpExternal, N_("Information Text.")   },
        { "InfoURL",                N_("Info URL"),                         "Text",                     xmpText, xmpExternal, N_("Information URL.")   },
        { "ISO",                    N_("ISO Setting"),                      "Integer",                  xmpText, xmpExternal, N_("Information about the ISO Setting.")   },
        { "ISRCCode",               N_("ISRC Code"),                        "Text",                     xmpText, xmpExternal, N_("Information about the ISRC Code.")   },
        { "Junk",                   N_("Junk Data"),                        "Text",                     xmpText, xmpExternal, N_("Video Junk data")   },
        { "Language",               N_("Language"),                         "Text",                     xmpText, xmpExternal, N_("Language.")  },
        { "Length",                 N_("Length"),                           "Integer",                  xmpText, xmpExternal, N_("The length of the media file.")   },
        { "LensModel",              N_("Lens Model"),                       "Text",                     xmpText, xmpExternal, N_("Lens Model.")  },
        { "LensType",               N_("Lens Type"),                        "Text",                     xmpText, xmpExternal, N_("Lens Type.")  },
        { "Lightness",              N_("Lightness"),                        "Text",                     xmpText, xmpExternal, N_("Lightness.")  },
        { "LocationInfo",           N_("Location Information"),             "Text",                     xmpText, xmpExternal, N_("Location Information.")  },
        { "LogoIconURL",            N_("Logo Icon URL"),                    "Text",                     xmpText, xmpExternal, N_("A C string that specifies Logo Icon URL.")   },
        { "LogoURL",                N_("Logo URL"),                         "Text",                     xmpText, xmpExternal, N_("A C string that specifies a Logo URL.")   },
        { "Lyrics",                 N_("Lyrics"),                           "Text",                     xmpText, xmpExternal, N_("Lyrics of a Song/Video.")   },
        { "MajorBrand",             N_("QTime Major FileType Brand"),       "Text",                     xmpText, xmpExternal, N_("QuickTime Major File Type Brand")   },
        { "Make",                   N_("Equipment Make"),                   "Text",                     xmpText, xmpExternal, N_("Manufacturer of recording equipment")   },
        { "MakerNoteType",          N_("Camera Maker Note Type"),           "Text",                     xmpText, xmpExternal, N_("Maker Note Type of the camera.")   },
        { "MakerNoteVersion",       N_("Camera Maker Note Version"),        "Text",                     xmpText, xmpExternal, N_("Maker Note Version of the camera.")   },
        { "MakerURL",               N_("Maker URL"),                        "Text",                     xmpText, xmpExternal, N_("Camera Manufacturer's URL.")   },
        { "MaxApertureValue",       N_("Maximum Aperture Value"),           "Rational",                 xmpText, xmpInternal, N_("Smallest F number of lens, in APEX.") },
        { "MaxBitRate",             N_("Maximum Bit Rate"),                 "Integer",                  xmpText, xmpExternal, N_("Specifies the maximum instantaneous bit rate in bits per second for the entire file. This shall equal the sum of the bit rates of the individual digital media streams.")   },
        { "MaxDataRate",            N_("Maximum Data Rate"),                "kiloBytes per Second",     xmpText, xmpExternal, N_("Peak rate at which data is presented in a video (Expressed in kB/s(kiloBytes per Second))")   },
        { "MediaCreateDate",        N_("Media Track Create Date"),          "Integer",                  xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was created.")   },
        { "MediaDuration",          N_("Media Track Duration"),             "Integer",                  xmpText, xmpExternal, N_("A time value that indicates the duration of this media (in the movie's time coordinate system).")   },
        { "MediaHeaderVersion",     N_("Media Header Version"),             "Text",                     xmpText, xmpExternal, N_("A 1-byte specification of the version of this media header")   },
        { "MediaLangCode",          N_("Media Language Code"),              "Integer",                  xmpText, xmpExternal, N_("A 16-bit integer that specifies the language code for this media.")   },
        { "MediaModifyDate",        N_("Media Track Modify Date"),          "Integer",                  xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was last modified.")   },
        { "MediaTimeScale",         N_("Media Time Scale"),                 "Integer",                  xmpText, xmpExternal, N_("A time value that indicates the time scale for this media-that is, the number of time units that pass per second in its time coordinate system." )   },
        { "Medium",                 N_("Medium"),                           "Text",                     xmpSeq,  xmpExternal, N_("Medium.")   },
        { "Metadata",               N_("Metadata"),                         "Text",                     xmpSeq,  xmpExternal, N_("An array of Unknown / Unregistered Metadata Tags and their values.")   },
        { "MetadataLibrary",        N_("Metadata Library"),                 "Text",                     xmpSeq,  xmpExternal, N_("An array of Unregistered Metadata Library Tags and their values.")   },
        { "MeteringMode",           N_("Metering Mode"),                    "Closed Choice of Integer", xmpText, xmpInternal, N_("Metering mode.") },
        { "MicroSecPerFrame",       N_("Micro Seconds Per Frame"),          "Integer",                  xmpText, xmpExternal, N_("Number of micro seconds per frame, or frame rate")   },
        { "MimeType",               N_("Mime Type"),                        "Text",                     xmpText, xmpExternal, N_("Tells about the video format")   },
        { "MinorVersion",           N_("QTime Minor FileType Version"),     "Text",                     xmpText, xmpExternal, N_("QuickTime Minor File Type Version")   },
        { "Model",                  N_("Equipment Model"),                  "Text",                     xmpText, xmpExternal, N_("Model name or number of equipment.")   },
        { "ModificationDate",       N_("Modification Date-Time"),           "Text",                     xmpText, xmpExternal, N_("Contains the modification date of the video")   },
        { "MovieHeaderVersion",     N_("Movie Header Version"),             "Integer",                  xmpText, xmpExternal, N_("Movie Header Version")   },
        { "MusicBy",                N_("Music By"),                         "Text",                     xmpText, xmpExternal, N_("Music By, i.e. name of person or organization.")   },
        { "MuxingApp",              N_("Muxing App"),                       "Text",                     xmpText, xmpExternal, N_("Contains the name of the library that has been used to create the file (like \"libmatroska 0.7.0\")")   },
        { "Name",                   N_("Name"),                             "Text",                     xmpText, xmpExternal, N_("Name of song or the event.")   },
        { "NextTrackID",            N_("Next Track ID"),                    "Integer",                  xmpText, xmpExternal, N_("A 32-bit integer that indicates a value to use for the track ID number of the next track added to this movie. Note that 0 is not a valid track ID value.")   },
        { "NumOfColours",           N_("Number Of Colours"),                "Integer/Text",             xmpText, xmpExternal, N_("Total number of colours used") },
        { "NumOfImpColours",        N_("Number Of Important Colours"),      "Integer/Text",             xmpText, xmpExternal, N_("Number Of Important Colours, a property inherited from BitMap format")   },
        { "NumOfParts",             N_("Number Of Parts"),                  "Integer",                  xmpText, xmpExternal, N_("Total number of parts in the video.") },
        { "OpColor",                N_("Operation Colours"),                "Integer/Text",             xmpText, xmpExternal, N_("Three 16-bit values that specify the red, green, and blue colors for the transfer mode operation indicated in the graphics mode field.")   },
        { "Organization",           N_("Organization"),                     "Text",                     xmpText, xmpExternal, N_("Name of organization associated with the video.")   },
        { "Orientation",            N_("Orientation"),                      "Closed Choice of Integer", xmpText, xmpInternal, N_("Video Orientation:"
                                                                                                                                  "1 = Horizontal (normal) "
                                                                                                                                  "2 = Mirror horizontal "
                                                                                                                                  "3 = Rotate 180 "
                                                                                                                                  "4 = Mirror vertical "
                                                                                                                                  "5 = Mirror horizontal and rotate 270 CW "
                                                                                                                                  "6 = Rotate 90 CW "
                                                                                                                                  "7 = Mirror horizontal and rotate 90 CW "
                                                                                                                                  "8 = Rotate 270 CW") },
        { "Part",                   N_("Part"),                             "Text",                     xmpText, xmpExternal, N_("Part.")   },
        { "Performers",             N_("Performers"),                       "Text",                     xmpText, xmpExternal, N_("Performers involved in the video.")   },
        { "PerformerKeywords",      N_("Performer Keywords"),               "Text",                     xmpText, xmpExternal, N_("Performer Keywords.")   },
        { "PerformerURL",           N_("Performer URL"),                    "Text",                     xmpText, xmpExternal, N_("Performer's dedicated URL.")   },
        { "PictureControlData",     N_("Picture Control Data"),             "Text",                     xmpText, xmpExternal, N_("Picture Control Data.")   },
        { "PictureControlVersion",  N_("Picture Control Version"),          "Text",                     xmpText, xmpExternal, N_("Picture Control Data Version.")   },
        { "PictureControlName",     N_("Picture Control Name"),             "Text",                     xmpText, xmpExternal, N_("Picture Control Name.")   },
        { "PictureControlBase",     N_("Picture Control Base"),             "Text",                     xmpText, xmpExternal, N_("Picture Control Data Base.")   },
        { "PictureControlAdjust",   N_("Picture Control Adjust"),           "Text",                     xmpText, xmpExternal, N_("Picture Control Adjust Information.")   },
        { "PictureControlQuickAdjust",N_("Picture Control Quick Adjust"),   "Text",                     xmpText, xmpExternal, N_("Picture Control Quick Adjustment Settings.")   },
        { "PlaySelection",          N_("Play Selection"),                   "Text",                     xmpText, xmpExternal, N_("Play Selection.")   },
        { "PlayMode",               N_("Play Mode"),                        "Text",                     xmpText, xmpExternal, N_("Information about the Play Mode.")   },
        { "PhysicalEquivalent",     N_("Chapter Physical Equivalent"),      "Text",                     xmpText, xmpExternal, N_("Contains the information of External media.")   },
        { "PixelDepth",             N_("Video Pixel Depth"),                "closed Choice of Text",    xmpText, xmpExternal, N_("The size in bits of each color component of a pixel. Standard Windows 32-bit "
                                                                                                                                 "pixels have 8 bits per component. One of: 8Int, 16Int, 32Int, 32Float.") },
        { "PixelPerMeterX",         N_("Pixels Per Meter X"),               "Integer",                  xmpText, xmpExternal, N_("Pixels Per Meter X, a property inherited from BitMap format")   },
        { "PixelPerMeterY",         N_("Pixels Per Meter Y"),               "Integer",                  xmpText, xmpExternal, N_("Pixels Per Meter Y, a property inherited from BitMap format")   },
        { "Planes",                 N_("Planes"),                           "Integer",                  xmpText, xmpExternal, N_("The number of Image Planes in the video") },
        { "PosterTime",             N_("Poster Time"),                      "Integer",                  xmpText, xmpExternal, N_("The time value of the time of the movie poster.") },
        { "PreferredRate",          N_("Preferred Rate"),                   "Rational",                 xmpText, xmpExternal, N_("A 32-bit fixed-point number that specifies the rate at which to play this movie. A value of 1.0 indicates normal rate.") },
        { "PreferredVolume",        N_("Preferred Volume"),                 "Rational",                 xmpText, xmpExternal, N_("A 16-bit fixed-point number that specifies how loud to play this movie's sound. A value of 1.0 indicates full volume.") },
        { "Preroll",                N_("Preroll"),                          "Integer",                  xmpText, xmpExternal, N_("Specifies the amount of time to buffer data before starting to play the file, in millisecond units. If this value is nonzero,"
                                                                                                                                 "the Play Duration field and all of the payload Presentation Time fields have been offset by this amount. Therefore, player software "
                                                                                                                                 "must subtract the value in the preroll field from the play duration and presentation times to calculate their actual values.") },
        { "PreviewAtomType",        N_("Preview Atom Type"),                "Text",                     xmpText, xmpExternal, N_("Indicates the type of atom that contains the preview data") },
        { "PreviewDate",            N_("Preview Date"),                     "Integer",                  xmpText, xmpExternal, N_("The date of the movie preview in local time converted from UTC") },
        { "PreviewDuration",        N_("Preview Duration"),                 "Integer",                  xmpText, xmpExternal, N_("The duration of the movie preview in movie time scale units") },
        { "PreviewTime",            N_("Preview Time"),                     "Integer",                  xmpText, xmpExternal, N_("The time value in the movie at which the preview begins.") },
        { "PreviewVersion",         N_("Preview Version"),                  "Integer",                  xmpText, xmpExternal, N_("The version of the movie preview ") },
        { "ProducedBy",             N_("Produced By"),                      "Text",                     xmpText, xmpExternal, N_("Produced By, i.e. name of person or organization.")   },
        { "Producer",               N_("Producer"),                         "Text",                     xmpText, xmpExternal, N_("Producer involved with the video.")   },
        { "ProducerKeywords",       N_("Producer Keywords"),                "Text",                     xmpText, xmpExternal, N_("Information about the Producer Keywords.")   },
        { "ProductionApertureWidth",N_("Production Aperture Width"),        "Integer",                  xmpText, xmpExternal, N_("Production aperture width in pixels")   },
        { "ProductionApertureHeight",N_("Production Aperture Height"),      "Integer",                  xmpText, xmpExternal, N_("Production aperture height in pixels")   },
        { "ProductionDesigner",     N_("Production Designer"),              "Text",                     xmpText, xmpExternal, N_("Information about the Production Designer.")   },
        { "ProductionStudio",       N_("Production Studio"),                "Text",                     xmpText, xmpExternal, N_("Information about the Production Studio.")   },
        { "Product",                N_("Product"),                          "Text",                     xmpText, xmpExternal, N_("Product.")   },
        { "ProjectRef",             N_("Project Reference"),                "ProjectLink",              xmpText, xmpExternal, N_("A reference to the project that created this file.") },
        { "Rate",                   N_("Rate"),                             "Integer",                  xmpText, xmpExternal, N_("Rate.")   },
        { "Rated",                  N_("Rated"),                            "Text",                     xmpText, xmpExternal, N_("The age circle required for viewing the video.")   },
        { "Rating",                 N_("Rating"),                           "Text",                     xmpText, xmpExternal, N_("Rating, eg. 7  or 8 (generally out of 10).")   },
        { "RecordLabelName",        N_("Record Label Name"),                "Text",                     xmpText, xmpExternal, N_("Record Label Name, or the name of the organization recording the video.")   },
        { "RecordLabelURL",         N_("Record Label URL"),                 "Text",                     xmpText, xmpExternal, N_("Record Label URL.")   },
        { "RecordingCopyright",     N_("Recording Copyright"),              "Text",                     xmpText, xmpExternal, N_("Recording Copyright.")   },
        { "Requirements",           N_("Requirements"),                     "Text",                     xmpText, xmpExternal, N_("Information about the Requirements.")   },
        { "ResolutionUnit",         N_("Resolution Unit"),                  "Closed Choice of Integer", xmpText, xmpInternal, N_("Unit used for XResolution and YResolution. Value is one of: 2 = inches; 3 = centimeters.") },
        { "RippedBy",               N_("Ripped By"),                        "Text",                     xmpText, xmpExternal, N_("Ripped By, i.e. name of person or organization.")   },
        { "Saturation",             N_("Saturation"),                       "Closed Choice of Integer", xmpText, xmpInternal, N_("Indicates the direction of saturation processing applied by the camera.") },
        { "SecondaryGenre",         N_("Secondary Genre"),                  "Text",                     xmpText, xmpExternal, N_("The name of the secondary genre..")   },
        { "SelectionTime",          N_("Selection Time"),                   "Integer",                  xmpText, xmpExternal, N_("The time value for the start time of the current selection.") },
        { "SelectionDuration",      N_("Selection Duration"),               "Integer",                  xmpText, xmpExternal, N_("The duration of the current selection in movie time scale units.") },
        { "SendDuration",           N_("Send Duration"),                    "Integer",                  xmpText, xmpExternal, N_("Specifies the time needed to send the file in 100-nanosecond units. This value should "
                                                                                                                                 "include the duration of the last packet in the content.")   },
        { "Sharpness",              N_("Sharpness"),                        "Integer",                  xmpText, xmpExternal, N_("\"Sharpness\" setting. Range 0 to +100.") },
        { "Software",               N_("Software"),                         "Text",                     xmpText, xmpExternal, N_("Software used to generate / create Video data.")   },
        { "SoftwareVersion",        N_("Software Version"),                 "Text",                     xmpText, xmpExternal, N_("The Version of the software used.")   },
        { "SongWriter",             N_("Song Writer"),                      "Text",                     xmpText, xmpExternal, N_("The name of the song writer.")   },
        { "SongWriterKeywords",     N_("Song Writer Keywords"),             "Text",                     xmpText, xmpExternal, N_("Song Writer Keywords.")   },
        { "Source",                 N_("Source"),                           "Text",                     xmpText, xmpExternal, N_("Source.")   },
        { "SourceCredits",          N_("Source Credits"),                   "Text",                     xmpText, xmpExternal, N_("Source Credits.")   },
        { "SourceForm",             N_("Source Form"),                      "Text",                     xmpText, xmpExternal, N_("Source Form.")   },
        { "SourceImageHeight",      N_("Source Image Height"),              "Integer",                  xmpText, xmpExternal, N_("Video height in pixels")   },
        { "SourceImageWidth",       N_("Source Image Width"),               "Integer",                  xmpText, xmpExternal, N_("Video width in pixels")   },
        { "Starring",               N_("Starring"),                         "Text",                     xmpText, xmpExternal, N_("Starring, name of famous people appearing in the video.")   },
        { "StartTimecode",          N_("Start Timecode"),                   "Integer",                  xmpText, xmpExternal, N_("Start Timecode")   },
        { "Statistics",             N_("Statistics"),                       "Text",                     xmpText, xmpExternal, N_("Statistics.")   },
        { "StreamCount",            N_("Stream Count"),                     "Integer",                  xmpText, xmpExternal, N_("Total Number Of Streams")   },
        { "StreamName",             N_("Stream Name"),                      "Text",                     xmpText, xmpExternal, N_("Describes the Stream Name. Eg - FUJIFILM AVI STREAM 0100")   },
        { "StreamQuality",          N_("Stream Quality"),                   "Integer",                  xmpText, xmpExternal, N_("General Stream Quality")   },
        { "StreamSampleRate",       N_("Stream Sample Rate"),               "Rational",                 xmpText, xmpExternal, N_("Stream Sample Rate")   },
        { "StreamSampleCount",      N_("Stream Sample Count"),              "Integer",                  xmpText, xmpExternal, N_("Stream Sample Count")   },
        { "StreamSampleSize",       N_("Stream Sample Size"),               "Integer",                  xmpText, xmpExternal, N_("General Stream Sample Size")   },
        { "StreamType",             N_("Stream Type"),                      "Text",                     xmpText, xmpExternal, N_("Describes the Stream Type. Eg - Video, Audio or Subtitles")   },
        { "SubTCodec",              N_("Subtitles Codec"),                  "Text",                     xmpText, xmpExternal, N_("Subtitles stream codec, for general purpose")   },
        { "SubTCodecDecodeAll",     N_("Subtitle Codec Decode Info"),       "Text",                     xmpText, xmpExternal, N_("Contains information the Subtitles codec decode all, i.e. Enabled/Disabled")   },
        { "SubTCodecInfo",          N_("Subtitles Codec Information"),      "Text",                     xmpText, xmpExternal, N_("Contains additional information about subtitles.")   },
        { "SubTCodecDownloadUrl",   N_("Subtitle Codec Download URL"),      "Text",                     xmpText, xmpExternal, N_("Video Subtitle Codec Download URL.")   },
        { "SubTCodecSettings",      N_("Subtitle Codec Settings"),          "Text",                     xmpText, xmpExternal, N_("Contains settings the codec needs before decoding can be started.")   },
        { "SubTDefaultOn",          N_("Subtitle Track Default On"),        "Text",                     xmpText, xmpExternal, N_("Subtitles Track Default On , i.e. Enabled/Disabled")   },
        { "SubTEnabled",            N_("Subtitle Track Enabled"),           "Text",                     xmpText, xmpExternal, N_("Status of Subtitles Track, i.e. Enabled/Disabled")   },
        { "Subtitle",               N_("Subtitle"),                         "Text",                     xmpText, xmpExternal, N_("Subtitle of the video.")   },
        { "SubtitleKeywords",       N_("Subtitle Keywords"),                "Text",                     xmpText, xmpExternal, N_("Subtitle Keywords.")   },
        { "SubTLang",               N_("Subtitles Language"),               "Text",                     xmpText, xmpExternal, N_("The Language in which the subtitles is recorded in.")   },
        { "SubTTrackForced",        N_("Subtitle Track Forced"),            "Text",                     xmpText, xmpExternal, N_("Subtitles Track Forced , i.e. Enabled/Disabled")   },
        { "SubTTrackLacing",        N_("Subtitle Track Lacing"),            "Text",                     xmpText, xmpExternal, N_("Subtitles Track Lacing , i.e. Enabled/Disabled")   },
        { "Subject",                N_("Subject"),                          "Text",                     xmpText, xmpExternal, N_("Subject. ")   },
        { "TapeName",               N_("Tape Name"),                        "Text",                     xmpText, xmpExternal, N_("TapeName.")   },
        { "TagDefault",             N_("Tag Default Setting"),              "Text",                     xmpText, xmpExternal, N_("If Tag is Default enabled, this value is Yes, else No ")   },
        { "TagLanguage",            N_("Tag Language"),                     "Text",                     xmpText, xmpExternal, N_("Language that has been used to define tags")   },
        { "TagName",                N_("Tag Name"),                         "Text",                     xmpText, xmpExternal, N_("Tags could be used to define several titles for a segment.")   },
        { "TagString",              N_("Tag String"),                       "Text",                     xmpText, xmpExternal, N_("Information contained in a Tags")   },
        { "TargetType",             N_("Target Type"),                      "Text",                     xmpText, xmpExternal, N_("A string describing the logical level of the object the Tag is referring to.")   },
        { "Technician",             N_("Technician"),                       "Text",                     xmpText, xmpExternal, N_("Technician, in most cases name of person.")   },
        { "ThumbnailHeight",        N_("Thumbnail Height"),                 "Integer",                  xmpText, xmpExternal, N_("Preview Image Thumbnail Height.")   },
        { "ThumbnailLength",        N_("Thumbnail Length"),                 "Integer",                  xmpText, xmpExternal, N_("Preview Image Thumbnail Length.")   },
        { "ThumbnailWidth",         N_("Thumbnail Width"),                  "Integer",                  xmpText, xmpExternal, N_("Preview Image Thumbnail Width.")   },
        { "TimecodeScale",          N_("Timecode Scale"),                   "Rational",                 xmpText, xmpExternal, N_("Multiplying factor which is helpful in calculation of a particular timecode")   },
        { "TimeOffset",             N_("Time Offset"),                      "Integer",                  xmpText, xmpExternal, N_("Specifies the presentation time offset of the stream in 100-nanosecond units. This value shall be equal to the send time of the first interleaved packet in the data section.")   },
        { "TimeScale",              N_("Time Scale"),                       "Integer",                  xmpText, xmpExternal, N_("A time value that indicates the time scale for this movie-that is, the number of time units that"
                                                                                                                                  "pass per second in its time coordinate system. A time coordinate system that measures time"
                                                                                                                                  "in sixtieths of a second, for example, has a time scale of 60.")   },
        { "Title",                  N_("Title"),                            "Text",                     xmpText, xmpExternal, N_("Contains a general name of the SEGMENT, like 'Lord of the Rings - The Two Towers', however, Tags could be used to define several titles for a segment.")   },
        { "ToningEffect",           N_("Toning Effect"),                    "Text",                     xmpText, xmpExternal, N_("Toning Effect Settings Applied.")   },
        { "TotalFrameCount",        N_("Total Frame Count"),                "Integer",                  xmpText, xmpExternal, N_("Total number of frames in a video")   },
        { "TotalStream",            N_("Number Of Streams"),                "Integer",                  xmpText, xmpExternal, N_("Total number of streams present in a video. Eg - Video, Audio or Subtitles")   },
        { "Track",                  N_("Track"),                            "Text",                     xmpText, xmpExternal, N_("Information about the Track.")   },
        { "TrackCreateDate",        N_("Video Track Create Date"),          "Integer",                  xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was created.")   },
        { "TrackDuration",          N_("Video Track Duration"),             "Integer",                  xmpText, xmpExternal, N_("A time value that indicates the duration of this track (in the movie's time coordinate system).")   },
        { "TrackForced",            N_("Video Track Forced"),               "Text",                     xmpText, xmpExternal, N_("Video Track Forced , i.e. Enabled/Disabled")   },
        { "TrackID",                N_("Track ID"),                         "Integer",                  xmpText, xmpExternal, N_("A 32-bit integer that uniquely identifies the track. The value 0 cannot be used.")   },
        { "TrackHeaderVersion",     N_("Track Header Version"),             "Text",                     xmpText, xmpExternal, N_("A 1-byte specification of the version of this track header")   },
        { "TrackLacing",            N_("Video Track Lacing"),               "Text",                     xmpText, xmpExternal, N_("Video Track Lacing , i.e. Enabled/Disabled")   },
        { "TrackLang",              N_("Track Language"),                   "Text",                     xmpText, xmpExternal, N_("The Language in which a particular stream is recorded in.")   },
        { "TrackLayer",             N_("Video Track Layer"),                "Integer",                  xmpText, xmpExternal, N_("A 16-bit integer that indicates this track's spatial priority in its movie. The QuickTime Movie"
                                                                                                                                  "Toolbox uses this value to determine how tracks overlay one another. Tracks with lower layer"
                                                                                                                                  "values are displayed in front of tracks with higher layer values.")   },
        { "TrackModifyDate",        N_("Video Track Modify Date"),          "Integer",                  xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was last modified.")   },
        { "TrackName",              N_("Track Name"),                       "Text",                     xmpText, xmpExternal, N_("Track Name could be used to define titles for a segment.")   },
        { "TrackNumber",            N_("Track Number"),                     "Integer",                  xmpText, xmpExternal, N_("Track Number.")   },
        { "TrackVolume",            N_("Track Volume"),                     "Rational",                 xmpText, xmpExternal, N_("A 16-bit fixed-point number that specifies how loud to play this track's sound. A value of 1.0 indicates full volume.") },
        { "TranslateCodec",         N_("Chapter Translate Codec"),          "Text",                     xmpText, xmpExternal, N_("Chapter Translate Codec information. Usually used in Matroska file type.")   },
        { "UnknownInfo",            N_("Unknown Information"),              "Text",                     xmpText, xmpExternal, N_("Unknown / Unregistered Metadata Tags and their values.")   },
        { "UnknownInfo2",           N_("Unknown Information"),              "Text",                     xmpText, xmpExternal, N_("Unknown / Unregistered Metadata Tags and their values.")   },
        { "URL",                    N_("Video URL"),                        "Text",                     xmpText, xmpExternal, N_("A C string that specifies a URL. There may be additional data after the C string.")   },
        { "URN",                    N_("Video URN"),                        "Text",                     xmpText, xmpExternal, N_("A C string that specifies a URN. There may be additional data after the C string.")   },
        { "VariProgram",            N_("Vari Program"),                     "Text",                     xmpText, xmpExternal, N_("Software settings used to generate / create Video data.")   },
        { "VegasVersionMajor",      N_("Vegas Version Major"),              "Text",                     xmpText, xmpExternal, N_("Vegas Version Major.")   },
        { "VegasVersionMinor",      N_("Vegas Version Minor"),              "Text",                     xmpText, xmpExternal, N_("Vegas Version Minor.")   },
        { "Vendor",                 N_("Vendor"),                           "Text",                     xmpText, xmpExternal, N_("The developer of the compressor that generated the compressed data.")   },
        { "VendorID",               N_("Vendor ID"),                        "Text",                     xmpText, xmpExternal, N_("A 32-bit integer that specifies the developer of the compressor that generated the compressed data. Often this field contains 'appl' to indicate Apple Computer, Inc.")   },
        { "VideoQuality",           N_("Video Quality"),                    "Integer",                  xmpText, xmpExternal, N_("Video Stream Quality")   },
        { "VideoSampleSize",        N_("Video Sample Size"),                "Integer",                  xmpText, xmpExternal, N_("Video Stream Sample Size")   },
        { "VideoScanType",          N_("Video Scan Type"),                  "Text",                     xmpText, xmpExternal, N_("Video Scan Type, it can be Progressive or Interlaced")   },
        { "WatermarkURL",           N_("Watermark URL"),                    "Text",                     xmpText, xmpExternal, N_("A C string that specifies a Watermark URL.")   },
        { "WhiteBalance",           N_("White Balance"),                    "Closed Choice Text",       xmpText, xmpExternal, N_("\"White Balance\" setting. One of: As Shot, Auto, Daylight, Cloudy, Shade, Tungsten, "
                                                                                                                                 "Fluorescent, Flash, Custom") },
        { "WhiteBalanceFineTune",   N_("White Balance Fine Tune"),          "Integer",                  xmpText, xmpExternal, N_("White Balance Fine Tune.") },
        { "Width",                  N_("Video Width"),                      "Integer",                  xmpText, xmpExternal, N_("Video width in pixels")   },
        { "WindowLocation",         N_("Window Location"),                  "Text",                     xmpText, xmpExternal, N_("Information about the Window Location.")   },
        { "WorldTime",              N_("World Time"),                       "Integer",                  xmpText, xmpExternal, N_("World Time")   },
        { "WrittenBy",              N_("Written By"),                       "Text",                     xmpText, xmpExternal, N_("Written By, i.e. name of person or organization.")   },
        { "WritingApp",             N_("Writing App"),                      "Text",                     xmpText, xmpExternal, N_("Contains the name of the application used to create the file (like \"mkvmerge 0.8.1\")")   },
        { "XResolution",            N_("X Resolution"),                     "Rational",                 xmpText, xmpInternal, N_("Horizontal resolution in pixels per unit.") },
        { "Year",                   N_("Year"),                             "Integer",                  xmpText, xmpExternal, N_("Year in which the video was made.")   },
        { "YResolution",            N_("Y Resolution"),                     "Rational",                 xmpText, xmpInternal, N_("Vertical resolution in pixels per unit.") },
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpAudioInfo[] = {
        { "AvgBytePerSec",      N_("Average Bytes Per Second"),     "Integer",               xmpText, xmpExternal, N_("Average Bytes Per Second found in audio stream")   },
        { "Balance",            N_("Balance"),                      "Integer",               xmpText, xmpExternal, N_("Indicates the left-right balance of the audio")   },
        { "BitsPerSample",      N_("Bits Per Sample/ Bit Rate"),    "Integer",               xmpText, xmpExternal, N_("Bits per test sample")   },
        { "ChannelType",        N_("Audio Channel Type"),           "Integers",              xmpText, xmpExternal, N_("The audio channel type. One of: Mono, Stereo, 5.1, 7.1.") },
        { "Codec",              N_("Audio Codec"),                  "Text",                  xmpText, xmpExternal, N_("Codec used for Audio Encoding/Decoding")   },
        { "CodecDecodeAll",     N_("Audio Codec Decode Info"),      "Text",                  xmpText, xmpExternal, N_("Contains information the audio codec decode all, i.e. Enabled/Disabled")   },
        { "CodecDescription",   N_("Audio Codec Description"),      "Text",                  xmpText, xmpExternal, N_("Contains description the codec.")   },
        { "CodecDownloadUrl",   N_("Audio Codec Download URL"),     "Text",                  xmpText, xmpExternal, N_("Audio Codec Download URL.")   },
        { "CodecInfo",          N_("Audio Codec Information"),      "Text",                  xmpText, xmpExternal, N_("Contains information the codec needs before decoding can be started. An example is the Vorbis initialization packets for Vorbis audio.")   },
        { "CodecSettings",      N_("Audio Codec Settings"),         "Text",                  xmpText, xmpExternal, N_("Contains settings the codec needs before decoding can be started.")   },
        { "Compressor",         N_("Audio Compressor"),             "Text",                  xmpText, xmpExternal, N_("The audio compression used. For example, MP3.") },
        { "DefaultDuration",    N_("Audio Default Duration"),       "Text",                  xmpText, xmpExternal, N_("The number of micro seconds an audio chunk plays.")   },
        { "DefaultStream",      N_("Audio Default Stream"),         "Text",                  xmpText, xmpExternal, N_("Audio Stream that would be played by default.")   },
        { "DefaultOn",          N_("Audio Track Default On"),       "Text",                  xmpText, xmpExternal, N_("Audio Track Default On , i.e. Enabled/Disabled")   },
        { "Enabled",            N_("Audio Track Enabled"),          "Text",                  xmpText, xmpExternal, N_("Status of Audio Track, i.e. Enabled/Disabled")   },
        { "Format",             N_("Audio Format"),                 "Text",                  xmpText, xmpExternal, N_("A four-character code that identifies the format of the audio.")   },
        { "HandlerClass",       N_("Handler Class"),                "Text",                  xmpText, xmpExternal, N_("A four-character code that identifies the type of the handler. Only two values are valid for this field: 'mhlr' for media handlers and 'dhlr' for data handlers.")   },
        { "HandlerDescription", N_("Handler Description"),          "Text",                  xmpText, xmpExternal, N_("A (counted) string that specifies the name of the component-that is, the media handler used when this media was created..")   },
        { "HandlerType",        N_("Handler Type"),                 "Text",                  xmpText, xmpExternal, N_("A four-character code that identifies the type of the media handler or data handler.")   },
        { "HandlerVendorID",    N_("Handler Vendor ID"),            "Text",                  xmpText, xmpExternal, N_("Component manufacturer.")   },                                        
        { "MediaCreateDate",    N_("Media Track Create Date"),      "Integer",               xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was created.")   },
        { "MediaDuration",      N_("Media Track Duration"),         "Integer",               xmpText, xmpExternal, N_("A time value that indicates the duration of this media (in the movie's time coordinate system).")   },
        { "MediaHeaderVersion", N_("Media Header Version"),         "Text",                  xmpText, xmpExternal, N_("A 1-byte specification of the version of this media header")   },
        { "MediaLangCode",      N_("Media Language Code"),          "Integer",               xmpText, xmpExternal, N_("A 16-bit integer that specifies the language code for this media.")   },
        { "MediaModifyDate",    N_("Media Track Modify Date"),      "Integer",               xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was last modified.")   },
        { "MediaTimeScale",     N_("Media Time Scale"),             "Integer",               xmpText, xmpExternal, N_("A time value that indicates the time scale for this media-that is, the number of time units that pass per second in its time coordinate system." )   },
        { "OutputSampleRate",   N_("Output Audio Sample Rate"),     "Integer",               xmpText, xmpExternal, N_("The output audio sample rate. Can be any value, but commonly 32000, 41100, or 48000.") },
        { "SampleCount",        N_("Audio Sample Count"),           "Integer",               xmpText, xmpExternal, N_("Sample taken for Analyzing Audio Stream")   },
        { "SampleRate",         N_("Audio Sample Rate"),            "Integer",               xmpText, xmpExternal, N_("The audio sample rate. Can be any value, but commonly 32000, 41100, or 48000.") },
        { "SampleType",         N_("Audio Sample Type"),            "closed Choice of Text", xmpText, xmpExternal, N_("The audio sample type. One of: 8Int, 16Int, 32Int, 32Float.") },
        { "SchemeTitle",        N_("Sound Scheme Title"),           "Text",                  xmpText, xmpExternal, N_("Sound Scheme Title.")   },
        { "TimeOffset",         N_("Time Offset"),                  "Integer",               xmpText, xmpExternal, N_("Specifies the presentation time offset of the stream in 100-nanosecond units. This value shall be equal to the send time of the first interleaved packet in the data section.")   },
        { "TrackCreateDate",    N_("Audio Track Create Date"),      "Integer",               xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was created.")   },
        { "TrackDuration",      N_("Audio Track Duration"),         "Integer",               xmpText, xmpExternal, N_("A time value that indicates the duration of this track (in the movie's time coordinate system).")   },
        { "TrackForced",        N_("Audio Track Forced"),           "Text",                  xmpText, xmpExternal, N_("Audio Track Forced , i.e. Enabled/Disabled")   },
        { "TrackID",            N_("Track ID"),                     "Integer",               xmpText, xmpExternal, N_("A 32-bit integer that uniquely identifies the track. The value 0 cannot be used.")   },
        { "TrackHeaderVersion", N_("Track Header Version"),         "Text",                  xmpText, xmpExternal, N_("A 1-byte specification of the version of this track header")   },
        { "TrackLacing",        N_("Audio Track Lacing"),           "Text",                  xmpText, xmpExternal, N_("Audio Track Lacing , i.e. Enabled/Disabled")   },
        { "TrackLang",          N_("Track Language"),               "Text",                  xmpText, xmpExternal, N_("The Language in which a particular stream is recorded in.")   },
        { "TrackLayer",         N_("Audio Track Layer"),            "Integer",               xmpText, xmpExternal, N_("A 16-bit integer that indicates this track's spatial priority in its movie. The QuickTime Movie"
                                                                                                                      "Toolbox uses this value to determine how tracks overlay one another. Tracks with lower layer"
                                                                                                                      "values are displayed in front of tracks with higher layer values.")   },
        { "TrackModifyDate",    N_("Audio Track Modify Date"),      "Integer",               xmpText, xmpExternal, N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was last modified.")   },
        { "TrackVolume",        N_("Track Volume"),                 "Rational",              xmpText, xmpExternal, N_("A 16-bit fixed-point number that specifies how loud to play this track's sound. A value of 1.0 indicates full volume.") },
        { "URL",                N_("Audio URL"),                    "Text",                  xmpText, xmpExternal, N_("A C string that specifies a URL. There may be additional data after the C string.")   },
        { "URN",                N_("Audio URN"),                    "Text",                  xmpText, xmpExternal, N_("A C string that specifies a URN. There may be additional data after the C string.")   },
        { "VendorID",           N_("Vendor ID"),                    "Text",                  xmpText, xmpExternal, N_("A 32-bit integer that specifies the developer of the compressor that generated the compressed data. Often this field contains 'appl' to indicate Apple Computer, Inc.")   },
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpDctermsInfo[] = {
            // dcterms properties used by DwC under the Record Class.
            { "type",                   N_("Type"),                     "bag open Choice", xmpBag,       xmpExternal, N_("The nature or genre of the resource.")                         },
            { "modified",               N_("Date Modified"),            "seq Date",        xmpSeq,       xmpExternal, N_("Date on which the resource was changed.")                         },
            { "language",               N_("Language"),                 "bag Locale",      xmpBag,       xmpExternal, N_("A language of the resource.")                    },
            { "license",                N_("License"),                  "Lang Alt",        langAlt,      xmpExternal, N_("A legal document giving official permission to do something with the resource.")                                              },
            { "rightsHolder",           N_("Rights Holder"),            "seq ProperName",  xmpSeq,       xmpExternal, N_("A person or organization owning or managing rights over the resource.")         },
            { "accessRights",           N_("Access Rights"),            "Lang Alt",        langAlt,      xmpExternal, N_("Information about who can access the resource or an indication of its security status.")                                              },
            { "bibliographicCitation",  N_("Bibliographic Citation"),   "Text",            xmpText,      xmpExternal, N_("A bibliographic reference for the resource.")              },
            { "references",             N_("References"),               "bag Text",        xmpBag,       xmpExternal, N_("A related resource that is referenced, cited, or otherwise pointed to by the described resource.")      },
        // Location Level Class
        { "Location",               N_("Location"),                 "Location",        xmpText,      xmpInternal, 
                                    N_("*Main structure* containing Darwin Core location based information."),
        },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

   extern const XmpPropertyInfo xmpDwCInfo[] = {

        // Record Level Class
        { "Record",                         N_("Record"),                               "Record",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing record based information."),
        },
            // Record Level Terms
            { "institutionID",                  N_("Institution ID"),                       "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the institution having custody of the object(s) or information referred to in the record.")
            },
            { "collectionID",                   N_("Collection ID"),                        "Text",     xmpText,    xmpExternal, 
                                                N_("An identifier for the collection or dataset from which the record was derived. For physical specimens, the recommended best practice is to use the identifier in a collections registry such as the Biodiversity Collections Index (http://www.biodiversitycollectionsindex.org/).")
            },
            { "institutionCode",                N_("Institution Code"),                     "Text",      xmpText,   xmpExternal,      
                                                N_("The name (or acronym) in use by the institution having custody of the object(s) or information referred to in the record.")
            },
            { "datasetID",                      N_("Dataset ID"),                           "Text",     xmpText,    xmpExternal, 
                                                N_("An identifier for the set of data. May be a global unique identifier or an identifier specific to a collection or institution.")
            },
            { "collectionCode",                 N_("Collection Code"),                      "Text",     xmpText,    xmpExternal, 
                                                N_("The name, acronym, coden, or initialism identifying the collection or data set from which the record was derived.")
            },
            { "datasetName",                    N_("Dataset Name"),                         "Text",     xmpText,    xmpExternal, 
                                                N_("The name identifying the data set from which the record was derived.")
            },
            { "ownerInstitutionCode",           N_("Owner Institution Code"),               "Text",      xmpText,   xmpExternal,      
                                                N_("The name (or acronym) in use by the institution having ownership of the object(s) or information referred to in the record.")
            },
            { "basisOfRecord",                  N_("Basis Of Record"),                      "Text",     xmpText,    xmpExternal, 
                                                N_("The specific nature of the data record - a subtype of the type. Recommended best practice is to use a controlled vocabulary such as the Darwin Core Type Vocabulary (http://rs.tdwg.org/dwc/terms/type-vocabulary/index.htm).")
            },
            { "informationWithheld",            N_("Information Withheld"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("Additional information that exists, but that has not been shared in the given record.")
            },
            { "dataGeneralizations",            N_("Data Generalizations"),                 "Text",     xmpText,    xmpExternal, 
                                                N_("Actions taken to make the shared data less specific or complete than in its original form. Suggests that alternative data of higher quality may be available on request.")
            },
            { "dynamicProperties",              N_("Dynamic Properties"),                   "bag Text",     xmpBag,    xmpExternal, 
                                                N_("A list (concatenated and separated) of additional measurements, facts, characteristics, or assertions about the record. Meant to provide a mechanism for structured content such as key-value pairs.")
            },

        // Occurrence Level Class
        { "Occurrence",                     N_("Occurrence"),                         "Occurrence",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing occurrence based information."),
        },
            // Occurrence Level Terms
            { "occurrenceID",                   N_("Occurrence ID"),                        "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the Occurrence (as opposed to a particular digital record of the occurrence). In the absence of a persistent global unique identifier, construct one from a combination of identifiers in the record that will most closely make the occurrenceID globally unique.")
            },
            { "catalogNumber",                  N_("Catalog Number"),                       "Text",     xmpText,    xmpExternal, 
                                                N_("An identifier (preferably unique) for the record within the data set or collection.")
            },
            { "occurrenceDetails",              N_("Occurrence Details"),                   "Text",      xmpText,   xmpExternal,      
                                                N_("Depreciated. Details about the Occurrence.")
            },
            { "occurrenceRemarks",              N_("Occurrence Remarks"),                   "Text",      xmpText,   xmpExternal,      
                                                N_("Comments or notes about the Occurrence.")
            },
            { "recordNumber",                   N_("Record Number"),                        "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier given to the Occurrence at the time it was recorded. Often serves as a link between field notes and an Occurrence record, such as a specimen collector's number.")
            },
            { "recordedBy",                     N_("Recorded By"),                          "bag Text",     xmpBag,   xmpExternal,      
                                                N_("A list (concatenated and separated) of names of people, groups, or organizations responsible for recording the original Occurrence. The primary collector or observer, especially one who applies a personal identifier (recordNumber), should be listed first.")
            },
            { "individualID",                   N_("Individual ID"),                        "Text",      xmpText,   xmpExternal,      
                                                N_("Depreciated. An identifier for an individual or named group of individual organisms represented in the Occurrence. Meant to accommodate resampling of the same individual or group for monitoring purposes. May be a global unique identifier or an identifier specific to a data set.")
            },
            { "individualCount",                N_("Individual Count"),                     "Integer",      xmpText,   xmpExternal,      
                                                N_("The number of individuals represented present at the time of the Occurrence.")
            },
            { "organismQuantity",               N_("Organism Quantity"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("A number or enumeration value for the quantity of organisms.")
            },
            { "organismQuantityType",           N_("Organism Quantity Type"),               "Text",      xmpText,   xmpExternal,      
                                                N_("The type of quantification system used for the quantity of organisms.")
            },
            { "sex",                            N_("Sex"),                                  "Text",      xmpText,   xmpExternal,      
                                                N_("The sex of the biological individual(s) represented in the Occurrence. Recommended best practice is to use a controlled vocabulary.")
            },
            { "lifeStage",                      N_("Life Stage"),                           "Text",      xmpText,   xmpExternal,      
                                                N_("The age class or life stage of the biological individual(s) at the time the Occurrence was recorded. Recommended best practice is to use a controlled vocabulary.")
            },
            { "reproductiveCondition",          N_("Reproductive Condition"),               "Text",      xmpText,   xmpExternal,      
                                                N_("The reproductive condition of the biological individual(s) represented in the Occurrence. Recommended best practice is to use a controlled vocabulary.")
            },
            { "behavior",                       N_("Behavior"),                             "Text",     xmpText,    xmpExternal, 
                                                N_("A description of the behavior shown by the subject at the time the Occurrence was recorded. Recommended best practice is to use a controlled vocabulary.")
            },
            { "establishmentMeans",             N_("Establishment Means"),                  "Text",      xmpText,    xmpExternal,      
                                                N_("The process by which the biological individual(s) represented in the Occurrence became established at the location. Recommended best practice is to use a controlled vocabulary.")
            },
            { "occurrenceStatus",               N_("Occurrence Status"),                    "Text",      xmpText,   xmpExternal,     
                                                N_("A statement about the presence or absence of a Taxon at a Location. Recommended best practice is to use a controlled vocabulary.")
            },
            { "preparations",                   N_("Preparations"),                         "bag Text",     xmpBag,   xmpExternal,      
                                                N_("A list (concatenated and separated) of preparations and preservation methods for a specimen.")
            },
            { "disposition",                    N_("Disposition"),                          "Text",     xmpText,    xmpExternal, 
                                                N_("The current state of a specimen with respect to the collection identified in collectionCode or collectionID. Recommended best practice is to use a controlled vocabulary.")
            },
            { "otherCatalogNumbers",            N_("Other Catalog Numbers"),                "bag Text",     xmpBag,   xmpExternal,      
                                                N_("A list (concatenated and separated) of previous or alternate fully qualified catalog numbers or other human-used identifiers for the same Occurrence, whether in the current or any other data set or collection.")
            },
            { "previousIdentifications",        N_("Previous Identifications"),             "bag Text",     xmpBag,   xmpExternal,      
                                                N_("Depreciated. A list (concatenated and separated) of previous assignments of names to the Occurrence.")
            },
            { "associatedMedia",                N_("Associated Media"),                     "bag Text",     xmpBag,    xmpExternal, 
                                                N_("A list (concatenated and separated) of identifiers (publication, global unique identifier, URI) of media associated with the Occurrence.")
            },
            { "associatedReferences",           N_("Associated References"),                "bag Text",     xmpBag,    xmpExternal, 
                                                N_("A list (concatenated and separated) of identifiers (publication, bibliographic reference, global unique identifier, URI) of literature associated with the Occurrence.")
            },
            { "associatedOccurrences",          N_("Associated Occurrences"),               "bag Text",     xmpBag,    xmpExternal, 
                                                N_("Depreciated. A list (concatenated and separated) of identifiers of other Occurrence records and their associations to this Occurrence.")
            },
            { "associatedSequences",            N_("Associated Sequences"),                 "bag Text",     xmpBag,    xmpExternal, 
                                                N_("A list (concatenated and separated) of identifiers (publication, global unique identifier, URI) of genetic sequence information associated with the Occurrence.")
            },
            { "associatedTaxa",                 N_("Associated Taxa"),                      "bag Text",     xmpBag,    xmpExternal, 
                                                N_("A list (concatenated and separated) of identifiers or names of taxa and their associations with the Occurrence.")
            },

        // Organism Level Class
        { "Organism",                 N_("Organism"),                     "Organism",    xmpText,   xmpInternal, 
                                      N_("*Main structure* containing organism based information."),
        },
            // Organism Level Terms
            { "organismID",               N_("Organism ID"),                    "Text",      xmpText,    xmpExternal,      
                                          N_("An identifier for the Organism instance (as opposed to a particular digital record of the Organism). May be a globally unique identifier or an identifier specific to the data set.")
            },
            { "organismName",             N_("Organism Name"),                        "Text",      xmpText,   xmpExternal,      
                                          N_("A textual name or label assigned to an Organism instance.")
            },
            { "organismScope",            N_("Organism Scope"),                      "Text",      xmpText,    xmpExternal, 
                                          N_("A description of the kind of Organism instance. Can be used to indicate whether the Organism instance represents a discrete organism or if it represents a particular type of aggregation. Recommended best practice is to use a controlled vocabulary.")
            },
            { "associatedOccurrences",    N_("Organism Associated Occurrences"),      "bag Text",     xmpBag,    xmpExternal,      
                                          N_("A list (concatenated and separated with a vertical bar ' | ') of identifiers of other Occurrence records and their associations to this Occurrence.")
            },
            { "associatedOrganisms",      N_("Associated Organisms"),   "bag Text",     xmpBag,   xmpExternal,      
                                          N_("A list (concatenated and separated with a vertical bar ' | ' ) of identifiers of other Organisms and their associations to this Organism.")
            },
            { "previousIdentifications",  N_("Previous Identifications"),         "bag Text",     xmpBag,    xmpExternal,      
                                          N_("A list (concatenated and separated with a vertical bar ' | ' ) of previous assignments of names to the Organism.")
            },
            { "organismRemarks",          N_("Organism Remarks"),                   "Text",      xmpText,    xmpExternal,      
                                          N_("Comments or notes about the Organism instance.")
            },

        // Material Sample Level Class
        { "MaterialSample",                 N_("Material Sample"),                         "MaterialSample",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing material sample based information."),
        },
        { "LivingSpecimen",                 N_("Living Specimen"),                         "LivingSpecimen",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing living specimen based information. A specimen that is alive."),
        },
        { "PreservedSpecimen",              N_("Preserved Specimen"),                      "PreservedSpecimen",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing preserved specimen based information. A specimen that has been preserved."),
        },
        { "FossilSpecimen",                 N_("Fossil Specimen"),                         "FossilSpecimen",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing fossil specimen based information. A preserved specimen that is a fossil."),
        },
            // Material Sample Level Terms
            { "materialSampleID",               N_("Material Sample ID"),                    "Text",      xmpText,    xmpExternal,      
                                                N_("An identifier for the MaterialSample (as opposed to a particular digital record of the material sample). In the absence of a persistent global unique identifier, construct one from a combination of identifiers in the record that will most closely make the materialSampleID globally unique.")
            },

        // Event Level Class
        { "Event",                          N_("Event"),                            "Event",    xmpText,  xmpInternal, 
                                            N_("*Main structure* containing event based information."),
        },
        { "HumanObservation",               N_("Human Observation"),                "HumanObservation",    xmpText,  xmpInternal, 
                                            N_("*Main structure* containing human observation based information."),
        },
        { "MachineObservation",             N_("Machine Observation"),              "MachineObservation",    xmpText,  xmpInternal, 
                                            N_("*Main structure* containing machine observation based information."),
        },
            // Event Level Terms
            { "eventID",                        N_("Event ID"),                             "Text",      xmpText,    xmpExternal,      
                                                N_("An identifier for the set of information associated with an Event (something that occurs at a place and time). May be a global unique identifier or an identifier specific to the data set.")
            },
            { "parentEventID",                  N_("Parent Event ID"),                             "Text",      xmpText,    xmpExternal,      
                                                N_("An identifier for the broader Event that groups this and potentially other Events.")
            },
            { "eventDate",                      N_("Event Date"),                  "Date",      xmpText,    xmpExternal,      
                                                N_("The date-time or interval during which an Event occurred. For occurrences, this is the date-time when the event was recorded. Not suitable for a time in a geological context. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "earliestDate",                   N_("Event Earliest Date"),                  "Date",      xmpText,    xmpExternal,      
                                                N_("Depreciated. (Child of Xmp.dwc.Event) The date-time or interval during which an Event started. For occurrences, this is the date-time when the event was recorded. Not suitable for a time in a geological context. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "latestDate",                     N_("Event Latest Date"),                    "Date",      xmpText,    xmpExternal,      
                                                N_("Depreciated. (Child of Xmp.dwc.Event) The date-time or interval during which an Event ended. For occurrences, this is the date-time when the event was recorded. Not suitable for a time in a geological context. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "eventTime",                      N_("Event Time"),                           "Date",      xmpText,    xmpExternal,      
                                                N_("The time or interval during which an Event occurred. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "startDayOfYear",                 N_("Start Day Of Year"),                    "Integer",      xmpText,    xmpExternal,      
                                                N_("The earliest ordinal day of the year on which the Event occurred (1 for January 1, 365 for December 31, except in a leap year, in which case it is 366).")
            },
            { "endDayOfYear",                   N_("End Day Of Year"),                      "Integer",      xmpText,    xmpExternal,      
                                                N_("The latest ordinal day of the year on which the Event occurred (1 for January 1, 365 for December 31, except in a leap year, in which case it is 366).")
            },
            { "year",                           N_("Year"),                                 "Integer",      xmpText,    xmpExternal,      
                                                N_("The four-digit year in which the Event occurred, according to the Common Era Calendar.")
            },
            { "month",                          N_("Month"),                                "Integer",      xmpText,    xmpExternal,      
                                                N_("The ordinal month in which the Event occurred.")
            },
            { "day",                            N_("Day"),                                  "Integer",      xmpText,    xmpExternal, 
                                                N_("The integer day of the month on which the Event occurred.")
            },
            { "verbatimEventDate",              N_("Verbatim Event Date"),                  "Text",      xmpText,   xmpExternal,      
                                                N_("The verbatim original representation of the date and time information for an Event.")
            },
            { "habitat",                        N_("Habitat"),                              "Text",      xmpText,    xmpExternal,      
                                                N_("A category or description of the habitat in which the Event occurred.")
            },
            { "samplingProtocol",               N_("Sampling Protocol"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("The name of, reference to, or description of the method or protocol used during an Event.")
            },
            { "samplingEffort",                 N_("Sampling Effort"),                      "Text",      xmpText,   xmpExternal,      
                                                N_("The amount of effort expended during an Event.")
            },               
            { "sampleSizeValue",                N_("Sampling Size Value"),                      "Text",      xmpText,   xmpExternal,      
                                                N_("A numeric value for a measurement of the size (time duration, length, area, or volume) of a sample in a sampling event.")
            },
            { "sampleSizeUnit",                 N_("Sampling Size Unit"),                      "Text",      xmpText,   xmpExternal,      
                                                N_("The unit of measurement of the size (time duration, length, area, or volume) of a sample in a sampling event.")
            },
            { "fieldNumber",                    N_("Field Number"),                         "Text",      xmpText,    xmpExternal,      
                                                N_("An identifier given to the event in the field. Often serves as a link between field notes and the Event.")
            },
            { "fieldNotes",                     N_("Field Notes"),                          "Text",      xmpText,    xmpExternal,      
                                                N_("One of (a) an indicator of the existence of, (b) a reference to (publication, URI), or (c) the text of notes taken in the field about the Event.")
            },
            { "eventRemarks",                   N_("Event Remarks"),                        "Text",      xmpText,    xmpExternal,      
                                                N_("Comments or notes about the Event.")
            },

        //Location Level Class
        { "dctermsLocation",                N_("Location Class"),                       "Location",    xmpText,   xmpInternal, 
                                            N_("Depreciated. Use Xmp.dcterms.Location instead. *Main structure* containing location based information."),
        },
            //Location Level Terms
            { "locationID",                     N_("Location ID"),                          "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the set of location information (data associated with Location). May be a global unique identifier or an identifier specific to the data set.")
            },
            { "higherGeographyID",              N_("Higher Geography ID"),                   "Text",      xmpText,    xmpExternal,      
                                                N_("An identifier for the geographic region within which the Location occurred. Recommended best practice is to use an persistent identifier from a controlled vocabulary such as the Getty Thesaurus of Geographic Names.")
            },
            { "higherGeography",                N_("Higher Geography"),                     "bag Text",     xmpBag,    xmpExternal,      
                                                N_("A list (concatenated and separated) of geographic names less specific than the information captured in the locality term.")
            },
            { "continent",                      N_("Continent"),                            "Text",     xmpText,    xmpExternal, 
                                                N_("The name of the continent in which the Location occurs. Recommended best practice is to use a controlled vocabulary such as the Getty Thesaurus of Geographic Names or the ISO 3166 Continent code.")
            },
            { "waterBody",                      N_("Water Body"),                           "Text",      xmpText,   xmpExternal,      
                                                N_("The name of the water body in which the Location occurs. Recommended best practice is to use a controlled vocabulary such as the Getty Thesaurus of Geographic Names.")
            },
            { "islandGroup",                    N_("Island Group"),                         "Text",      xmpText,   xmpExternal,      
                                                N_("The name of the island group in which the Location occurs. Recommended best practice is to use a controlled vocabulary such as the Getty Thesaurus of Geographic Names.")
            },
            { "island",                         N_("Island"),                               "Text",      xmpText,   xmpExternal,      
                                                N_("The name of the island on or near which the Location occurs. Recommended best practice is to use a controlled vocabulary such as the Getty Thesaurus of Geographic Names.")
            },
            { "country",                        N_("Country"),                              "Text",     xmpText,    xmpExternal, 
                                                N_("The name of the country or major administrative unit in which the Location occurs. Recommended best practice is to use a controlled vocabulary such as the Getty Thesaurus of Geographic Names.")
            },
            { "countryCode",                    N_("Country Code"),                         "Text",     xmpText,    xmpExternal, 
                                                N_("The standard code for the country in which the Location occurs. Recommended best practice is to use ISO 3166-1-alpha-2 country codes.")
            },
            { "stateProvince",                  N_("State Province"),                       "Text",      xmpText,   xmpExternal,      
                                                N_("The name of the next smaller administrative region than country (state, province, canton, department, region, etc.) in which the Location occurs.")
            },
            { "county",                         N_("County"),                               "Text",     xmpText,    xmpExternal, 
                                                N_("The full, unabbreviated name of the next smaller administrative region than stateProvince (county, shire, department, etc.) in which the Location occurs.")
            },
            { "municipality",                   N_("Municipality"),                         "Text",      xmpText,   xmpExternal,      
                                                N_("The full, unabbreviated name of the next smaller administrative region than county (city, municipality, etc.) in which the Location occurs. Do not use this term for a nearby named place that does not contain the actual location.")
            },
            { "locality",                       N_("Locality"),                             "Text",      xmpText,   xmpExternal,      
                                                N_("The specific description of the place. Less specific geographic information can be provided in other geographic terms (higherGeography, continent, country, stateProvince, county, municipality, waterBody, island, islandGroup). This term may contain information modified from the original to correct perceived errors or standardize the description.")
            },
            { "verbatimLocality",               N_("Verbatim Locality"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("The original textual description of the place.")
            },
            { "verbatimElevation",              N_("Verbatim Elevation"),                   "Text",      xmpText,   xmpExternal,      
                                                N_("The original description of the elevation (altitude, usually above sea level) of the Location.")
            },
            { "minimumElevationInMeters",       N_("Minimum Elevation In Meters"),          "Real",      xmpText,   xmpExternal,      
                                                N_("The lower limit of the range of elevation (altitude, usually above sea level), in meters.")
            },
            { "maximumElevationInMeters",       N_("Maximum Elevation In Meters"),          "Real",      xmpText,   xmpExternal,      
                                                N_("The upper limit of the range of elevation (altitude, usually above sea level), in meters.")
            },
            { "verbatimDepth",                  N_("Verbatim Depth"),                       "Text",      xmpText,   xmpExternal,      
                                                N_("The original description of the depth below the local surface.")
            },
            { "minimumDepthInMeters",           N_("Minimum Depth In Meters"),              "Real",      xmpText,   xmpExternal,      
                                                N_("The lesser depth of a range of depth below the local surface, in meters.")
            },
            { "maximumDepthInMeters",           N_("Maximum Depth In Meters"),              "Real",      xmpText,   xmpExternal,      
                                                N_("The greater depth of a range of depth below the local surface, in meters.")
            },
            { "minimumDistanceAboveSurfaceInMeters",      N_("Minimum Distance Above Surface In Meters"),   "Real", xmpText, xmpExternal,      
                                                N_("The lesser distance in a range of distance from a reference surface in the vertical direction, in meters. Use positive values for locations above the surface, negative values for locations below. If depth measures are given, the reference surface is the location given by the depth, otherwise the reference surface is the location given by the elevation.")
            },
            { "maximumDistanceAboveSurfaceInMeters",    N_("Maximum Distance Above Surface In Meters"), "Real", xmpText,    xmpExternal,      
                                                        N_("The greater distance in a range of distance from a reference surface in the vertical direction, in meters. Use positive values for locations above the surface, negative values for locations below. If depth measures are given, the reference surface is the location given by the depth, otherwise the reference surface is the location given by the elevation.")
            },
            { "locationAccordingTo",            N_("Location According To"),                "Text",      xmpText,   xmpExternal,      
                                                N_("Information about the source of this Location information. Could be a publication (gazetteer), institution, or team of individuals.")
            },
            { "locationRemarks",                N_("Location Remarks"),                     "Text",      xmpText,   xmpExternal,      
                                                N_("Comments or notes about the Location.")
            },
            { "verbatimCoordinates",            N_("Verbatim Coordinates"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("The verbatim original spatial coordinates of the Location. The coordinate ellipsoid, geodeticDatum, or full Spatial Reference System (SRS) for these coordinates should be stored in verbatimSRS and the coordinate system should be stored in verbatimCoordinateSystem.")
            },
            { "verbatimLatitude",               N_("Verbatim Latitude"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("The verbatim original latitude of the Location. The coordinate ellipsoid, geodeticDatum, or full Spatial Reference System (SRS) for these coordinates should be stored in verbatimSRS and the coordinate system should be stored in verbatimCoordinateSystem.")
            },
            { "verbatimLongitude",              N_("Verbatim Longitude"),                   "Text",      xmpText,   xmpExternal,      
                                                N_("The verbatim original longitude of the Location. The coordinate ellipsoid, geodeticDatum, or full Spatial Reference System (SRS) for these coordinates should be stored in verbatimSRS and the coordinate system should be stored in verbatimCoordinateSystem.")
            },
            { "verbatimCoordinateSystem",       N_("Verbatim Coordinate System"),           "Text",      xmpText,   xmpExternal,      
                                                N_("The spatial coordinate system for the verbatimLatitude and verbatimLongitude or the verbatimCoordinates of the Location. Recommended best practice is to use a controlled vocabulary.")
            },
            { "verbatimSRS",                    N_("Verbatim SRS"),                         "Text",      xmpText,   xmpExternal,      
                                                N_("The ellipsoid, geodetic datum, or spatial reference system (SRS) upon which coordinates given in verbatimLatitude and verbatimLongitude, or verbatimCoordinates are based. Recommended best practice is use the EPSG code as a controlled vocabulary to provide an SRS, if known. Otherwise use a controlled vocabulary for the name or code of the geodetic datum, if known. Otherwise use a controlled vocabulary for the name or code of the ellipsoid, if known. If none of these is known, use the value \"unknown\".")
            },
            { "decimalLatitude",                N_("Decimal Latitude"),                     "Real",     xmpText,    xmpExternal, 
                                                N_("The geographic latitude (in decimal degrees, using the spatial reference system given in geodeticDatum) of the geographic center of a Location. Positive values are north of the Equator, negative values are south of it. Legal values lie between -90 and 90, inclusive.")
            },
            { "decimalLongitude",               N_("Decimal Longitude"),                    "Real",     xmpText,    xmpExternal, 
                                                N_("The geographic longitude (in decimal degrees, using the spatial reference system given in geodeticDatum) of the geographic center of a Location. Positive values are east of the Greenwich Meridian, negative values are west of it. Legal values lie between -180 and 180, inclusive.")
            },
            { "geodeticDatum",                  N_("Geodetic Datum"),                       "Text",      xmpText,    xmpExternal,      
                                                N_("The ellipsoid, geodetic datum, or spatial reference system (SRS) upon which the geographic coordinates given in decimalLatitude and decimalLongitude as based. Recommended best practice is use the EPSG code as a controlled vocabulary to provide an SRS, if known. Otherwise use a controlled vocabulary for the name or code of the geodetic datum, if known. Otherwise use a controlled vocabulary for the name or code of the ellipsoid, if known. If none of these is known, use the value \"unknown\".")
            },
            { "coordinateUncertaintyInMeters",  N_("Coordinate Uncertainty In Meters"),     "Real",     xmpText,    xmpExternal, 
                                                N_("The horizontal distance (in meters) from the given decimalLatitude and decimalLongitude describing the smallest circle containing the whole of the Location. Leave the value empty if the uncertainty is unknown, cannot be estimated, or is not applicable (because there are no coordinates). Zero is not a valid value for this term.")
            },
            { "coordinatePrecision",            N_("Coordinate Precision"),                 "Text",     xmpText,    xmpExternal, 
                                                N_("A decimal representation of the precision of the coordinates given in the decimalLatitude and decimalLongitude.")
            },
            { "pointRadiusSpatialFit",          N_("Point Radius Spatial Fit"),              "Real",      xmpText,   xmpExternal,      
                                                N_("The ratio of the area of the point-radius (decimalLatitude, decimalLongitude, coordinateUncertaintyInMeters) to the area of the true (original, or most specific) spatial representation of the Location. Legal values are 0, greater than or equal to 1, or undefined. A value of 1 is an exact match or 100% overlap. A value of 0 should be used if the given point-radius does not completely contain the original representation. The pointRadiusSpatialFit is undefined (and should be left blank) if the original representation is a point without uncertainty and the given georeference is not that same point (without uncertainty). If both the original and the given georeference are the same point, the pointRadiusSpatialFit is 1.")
            },
            { "footprintWKT",                   N_("Footprint WKT"),                         "Text",      xmpText,    xmpExternal,      
                                                N_("A Well-Known Text (WKT) representation of the shape (footprint, geometry) that defines the Location. A Location may have both a point-radius representation (see decimalLatitude) and a footprint representation, and they may differ from each other.")
            },
            { "footprintSRS",                   N_("Footprint SRS"),                        "Text",      xmpText,    xmpExternal,      
                                                N_("A Well-Known Text (WKT) representation of the Spatial Reference System (SRS) for the footprintWKT of the Location. Do not use this term to describe the SRS of the decimalLatitude and decimalLongitude, even if it is the same as for the footprintWKT - use the geodeticDatum instead.")
            },
            { "footprintSpatialFit",            N_("Footprint Spatial Fit"),                "Real",      xmpText,    xmpExternal,      
                                                N_("The ratio of the area of the footprint (footprintWKT) to the area of the true (original, or most specific) spatial representation of the Location. Legal values are 0, greater than or equal to 1, or undefined. A value of 1 is an exact match or 100% overlap. A value of 0 should be used if the given footprint does not completely contain the original representation. The footprintSpatialFit is undefined (and should be left blank) if the original representation is a point and the given georeference is not that same point. If both the original and the given georeference are the same point, the footprintSpatialFit is 1.")
            },
            { "georeferencedBy",                N_("Georeferenced By"),                     "bag Text",     xmpBag,   xmpExternal,      
                                                N_("A list (concatenated and separated) of names of people, groups, or organizations who determined the georeference (spatial representation) for the Location.")
            },
            { "georeferencedDate",              N_("Georeferenced Date"),                   "Date",      xmpText,    xmpExternal,      
                                                N_("The date on which the Location was georeferenced. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "georeferenceProtocol",           N_("Georeference Protocol"),                "Text",      xmpText,    xmpExternal,      
                                                N_("A description or reference to the methods used to determine the spatial footprint, coordinates, and uncertainties.")
            },
            { "georeferenceSources",            N_("Georeference Sources"),                 "bag Text",     xmpBag,    xmpExternal,      
                                                N_("A list (concatenated and separated) of maps, gazetteers, or other resources used to georeference the Location, described specifically enough to allow anyone in the future to use the same resources.")
            },
            { "georeferenceVerificationStatus", N_("Georeference Verification Status"),     "Text",      xmpText,    xmpExternal,      
                                                N_("A categorical description of the extent to which the georeference has been verified to represent the best possible spatial description. Recommended best practice is to use a controlled vocabulary.")
            },
            { "georeferenceRemarks",            N_("Georeference Remarks"),                 "Text",      xmpText,    xmpExternal,      
                                                N_("Notes or comments about the spatial description determination, explaining assumptions made in addition or opposition to the those formalized in the method referred to in georeferenceProtocol.")
            },

        // Geological Context Level Class
        { "GeologicalContext",              N_("Geological Context"),                     "GeologicalContext",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing geological context based information."),
        },
            // Geological Context Level Terms
            { "geologicalContextID",            N_("Geological Context ID"),                "Text",      xmpText,    xmpExternal,      
                                                N_("An identifier for the set of information associated with a GeologicalContext (the location within a geological context, such as stratigraphy). May be a global unique identifier or an identifier specific to the data set.")
            },
            { "earliestEonOrLowestEonothem",    N_("Earliest Eon Or Lowest Eonothem"),      "Text",     xmpText,    xmpExternal, 
                                                N_("The full name of the earliest possible geochronologic eon or lowest chrono-stratigraphic eonothem or the informal name (\"Precambrian\") attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "latestEonOrHighestEonothem",     N_("Latest Eon Or Highest Eonothem"),       "Text",      xmpText,   xmpExternal,      
                                                N_("The full name of the latest possible geochronologic eon or highest chrono-stratigraphic eonothem or the informal name (\"Precambrian\") attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "earliestEraOrLowestErathem",     N_("Earliest Era Or Lowest Erathem"),       "Text",     xmpText,    xmpExternal, 
                                                N_("The full name of the earliest possible geochronologic era or lowest chronostratigraphic erathem attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "latestEraOrHighestErathem",      N_("Latest Era Or Highest Erathem"),        "Text",      xmpText,   xmpExternal,      
                                                N_("The full name of the latest possible geochronologic era or highest chronostratigraphic erathem attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "earliestPeriodOrLowestSystem",   N_("Earliest Period Or Lowest System"),     "Text",     xmpText,    xmpExternal, 
                                                N_("The full name of the earliest possible geochronologic period or lowest chronostratigraphic system attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "latestPeriodOrHighestSystem",    N_("Latest Period Or Highest System"),      "Text",      xmpText,   xmpExternal,      
                                                N_("The full name of the latest possible geochronologic period or highest chronostratigraphic system attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "earliestEpochOrLowestSeries",    N_("Earliest Epoch Or Lowest Series"),      "Text",     xmpText,    xmpExternal, 
                                                N_("The full name of the earliest possible geochronologic epoch or lowest chronostratigraphic series attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "latestEpochOrHighestSeries",     N_("Latest Epoch Or Highest Series"),       "Text",      xmpText,   xmpExternal,      
                                                N_("The full name of the latest possible geochronologic epoch or highest chronostratigraphic series attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "earliestAgeOrLowestStage",       N_("Earliest Age Or Lowest Stage"),         "Text",     xmpText,    xmpExternal, 
                                                N_("The full name of the earliest possible geochronologic age or lowest chronostratigraphic stage attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "latestAgeOrHighestStage",        N_("Latest Age Or Highest Stage"),          "Text",      xmpText,   xmpExternal,      
                                                N_("The full name of the latest possible geochronologic age or highest chronostratigraphic stage attributable to the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "lowestBiostratigraphicZone",     N_("Lowest Biostratigraphic Zone"),         "Text",      xmpText,   xmpExternal,      
                                                N_("The full name of the lowest possible geological biostratigraphic zone of the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "highestBiostratigraphicZone",    N_("Highest Biostratigraphic Zone"),        "Text",      xmpText,    xmpExternal,      
                                                N_("The full name of the highest possible geological biostratigraphic zone of the stratigraphic horizon from which the cataloged item was collected.")
            },
            { "lithostratigraphicTerms",        N_("Lithostratigraphic Terms"),             "Text",      xmpText,   xmpExternal,      
                                                N_("The combination of all litho-stratigraphic names for the rock from which the cataloged item was collected.")
            },
            { "group",                          N_("Group"),                                "Text",      xmpText,    xmpExternal,      
                                                N_("The full name of the lithostratigraphic group from which the cataloged item was collected.")
            },
            { "formation",                      N_("Formation"),                            "Text",      xmpText,    xmpExternal,      
                                                N_("The full name of the lithostratigraphic formation from which the cataloged item was collected.")
            },
            { "member",                         N_("Member"),                               "Text",      xmpText,   xmpExternal,      
                                                N_("The full name of the lithostratigraphic member from which the cataloged item was collected.")
            },
            { "bed",                            N_("Bed"),                                  "Text",     xmpText,    xmpExternal, 
                                                N_("The full name of the lithostratigraphic bed from which the cataloged item was collected.")
            },

        // Identification Level Class
        { "Identification",                 N_("Identification"),                     "Identification",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing identification based information."),
        },
            // Identification Level Terms
            { "identificationID",               N_("Identification ID"),                    "Text",      xmpText,    xmpExternal,      
                                                N_("An identifier for the Identification (the body of information associated with the assignment of a scientific name). May be a global unique identifier or an identifier specific to the data set.")
            },
            { "identifiedBy",                   N_("Identified By"),                        "bag Text",     xmpBag,   xmpExternal,      
                                                N_("A list (concatenated and separated) of names of people, groups, or organizations who assigned the Taxon to the subject.")
            },
            { "dateIdentified",                 N_("Date Identified"),                      "Date",      xmpText,    xmpExternal, 
                                                N_("The date on which the subject was identified as representing the Taxon. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "identificationReferences",       N_("Identification References"),            "bag Text",     xmpBag,    xmpExternal,      
                                                N_("A list (concatenated and separated) of references (publication, global unique identifier, URI) used in the Identification.")
            },
            { "identificationVerificationStatus",   N_("Identification Verification Status"),   "Text",  xmpText,   xmpExternal,      
                                                    N_("A categorical indicator of the extent to which the taxonomic identification has been verified to be correct. Recommended best practice is to use a controlled vocabulary such as that used in HISPID/ABCD.")
            },
            { "identificationRemarks",          N_("Identification Remarks"),               "Text",      xmpText,    xmpExternal,      
                                                N_("Comments or notes about the Identification.")
            },
            { "identificationQualifier",        N_("Identification Qualifier"),             "Text",      xmpText,    xmpExternal,      
                                                N_("A brief phrase or a standard term (\"cf.\" \"aff.\") to express the determiner's doubts about the Identification.")
            },
            { "typeStatus",                     N_("Type Status"),                          "bag Text",     xmpBag,   xmpExternal,      
                                                N_("A list (concatenated and separated) of nomenclatural types (type status, typified scientific name, publication) applied to the subject.")
            },

        // Taxon Level Class
        { "Taxon",                          N_("Taxon"),                                "Taxon",    xmpBag,   xmpInternal, 
                                            N_("*Main structure* containing taxonomic based information."),
        },
            // Taxon Level Terms
            { "taxonID",                        N_("Taxon ID"),                             "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the set of taxon information (data associated with the Taxon class). May be a global unique identifier or an identifier specific to the data set.")
            },
            { "scientificNameID",               N_("Scientific Name ID"),                   "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the nomenclatural (not taxonomic) details of a scientific name.")
            },
            { "acceptedNameUsageID",            N_("Accepted Name Usage ID"),               "Text",     xmpText,    xmpExternal, 
                                                N_("An identifier for the name usage (documented meaning of the name according to a source) of the currently valid (zoological) or accepted (botanical) taxon.")
            },
            { "parentNameUsageID",              N_("Parent Name Usage ID"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the name usage (documented meaning of the name according to a source) of the direct, most proximate higher-rank parent taxon (in a classification) of the most specific element of the scientificName.")
            },
            { "originalNameUsageID",            N_("Original Name Usage ID"),               "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the name usage (documented meaning of the name according to a source) in which the terminal element of the scientificName was originally established under the rules of the associated nomenclaturalCode.")
            },
            { "nameAccordingToID",              N_("Name According To ID"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the source in which the specific taxon concept circumscription is defined or implied. See nameAccordingTo.")
            },
            { "namePublishedInID",              N_("Name Published In ID"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the publication in which the scientificName was originally established under the rules of the associated nomenclaturalCode.")
            },
            { "taxonConceptID",                 N_("Taxon Concept ID"),                     "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the taxonomic concept to which the record refers - not for the nomenclatural details of a taxon.")
            },
            { "scientificName",                 N_("Scientific Name"),                      "Text",      xmpText,   xmpExternal,      
                                                N_("The full scientific name, with authorship and date information if known. When forming part of an Identification, this should be the name in lowest level taxonomic rank that can be determined. This term should not contain identification qualifications, which should instead be supplied in the IdentificationQualifier term.")
            },
            { "acceptedNameUsage",              N_("Accepted Name Usage"),                  "Text",     xmpText,    xmpExternal,
                                                N_("The full name, with authorship and date information if known, of the currently valid (zoological) or accepted (botanical) taxon.")
            },
            { "parentNameUsage",                N_("Parent Name Usage"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("The full name, with authorship and date information if known, of the direct, most proximate higher-rank parent taxon (in a classification) of the most specific element of the scientificName.")
            },
            { "originalNameUsage",              N_("Original Name Usage"),                  "Text",      xmpText,   xmpExternal,      
                                                N_("The taxon name, with authorship and date information if known, as it originally appeared when first established under the rules of the associated nomenclaturalCode. The basionym (botany) or basonym (bacteriology) of the scientificName or the senior/earlier homonym for replaced names.")
            },
            { "nameAccordingTo",                N_("Name According To"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("The reference to the source in which the specific taxon concept circumscription is defined or implied - traditionally signified by the Latin \"sensu\" or \"sec.\" (from secundum, meaning \"according to\"). For taxa that result from identifications, a reference to the keys, monographs, experts and other sources should be given.")
            },
            { "namePublishedIn",                N_("Name Published In"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("A reference for the publication in which the scientificName was originally established under the rules of the associated nomenclaturalCode.")
            },
            { "namePublishedInYear",            N_("Name Published In Year"),               "Text",      xmpText,   xmpExternal,      
                                                N_("The four-digit year in which the scientificName was published.")
            },
            { "higherClassification",           N_("Higher Classification"),                "bag Text",     xmpBag,    xmpExternal,      
                                                N_("A list (concatenated and separated) of taxa names terminating at the rank immediately superior to the taxon referenced in the taxon record. Recommended best practice is to order the list starting with the highest rank and separating the names for each rank with a semi-colon ;")
            },
            { "kingdom",                        N_("Kingdom"),                              "Text",      xmpText,   xmpExternal,      
                                                N_("The full scientific name of the kingdom in which the taxon is classified.")
            },
            { "phylum",                         N_("Phylum"),                               "Text",      xmpText,   xmpExternal,      
                                                N_("The full scientific name of the phylum or division in which the taxon is classified.")
            },
            { "class",                          N_("Class"),                                "Text",     xmpText,    xmpExternal, 
                                                N_("The full scientific name of the class in which the taxon is classified.")
            },
            { "order",                          N_("Order"),                                "Text",      xmpText,   xmpExternal,      
                                                N_("The full scientific name of the order in which the taxon is classified.")
            },
            { "family",                         N_("Family"),                               "Text",      xmpText,    xmpExternal,      
                                                N_("The full scientific name of the family in which the taxon is classified.")
            },
            { "genus",                          N_("Genus"),                                "Text",      xmpText,    xmpExternal,      
                                                N_("The full scientific name of the genus in which the taxon is classified.")
            },
            { "subgenus",                       N_("Subgenus"),                             "Text",      xmpText,   xmpExternal,      
                                                N_("The full scientific name of the subgenus in which the taxon is classified. Values should include the genus to avoid homonym confusion.")
            },
            { "specificEpithet",                N_("Specific Epithet"),                     "Text",      xmpText,   xmpExternal,      
                                                N_("The name of the first or species epithet of the scientificName.")
            },
            { "infraspecificEpithet",           N_("Infraspecific Epithet"),                "Text",      xmpText,   xmpExternal,      
                                                N_("The name of the lowest or terminal infraspecific epithet of the scientificName, excluding any rank designation.")
            },
            { "taxonRank",                      N_("Taxon Rank"),                           "Text",      xmpText,   xmpExternal,      
                                                N_("The taxonomic rank of the most specific name in the scientificName. Recommended best practice is to use a controlled vocabulary.")
            },
            { "verbatimTaxonRank",              N_("Verbatim Taxon Rank"),                  "Text",      xmpText,   xmpExternal,      
                                                N_("The taxonomic rank of the most specific name in the scientificName as it appears in the original record.")
            },
            { "scientificNameAuthorship",       N_("Scientific Name Authorship"),           "Text",      xmpText,   xmpExternal,      
                                                N_("The authorship information for the scientificName formatted according to the conventions of the applicable nomenclaturalCode.")
            },
            { "vernacularName",                 N_("Vernacular Name"),                      "Lang Alt", langAlt,   xmpExternal,      
                                                N_("A common or vernacular name.")
            },
            { "nomenclaturalCode",              N_("Nomenclatural Code"),                   "Text",      xmpText,   xmpExternal,      
                                                N_("The nomenclatural code (or codes in the case of an ambiregnal name) under which the scientificName is constructed. Recommended best practice is to use a controlled vocabulary.")
            },
            { "taxonomicStatus",                N_("Taxonomic Status"),                     "Text",      xmpText,   xmpExternal,      
                                                N_("The status of the use of the scientificName as a label for a taxon. Requires taxonomic opinion to define the scope of a taxon. Rules of priority then are used to define the taxonomic status of the nomenclature contained in that scope, combined with the experts opinion. It must be linked to a specific taxonomic reference that defines the concept. Recommended best practice is to use a controlled vocabulary.")
            },
            { "nomenclaturalStatus",            N_("Nomenclatural Status"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("The status related to the original publication of the name and its conformance to the relevant rules of nomenclature. It is based essentially on an algorithm according to the business rules of the code. It requires no taxonomic opinion.")
            },
            { "taxonRemarks",                   N_("Taxon Remarks"),                        "Text",      xmpText,   xmpExternal,      
                                                N_("Comments or notes about the taxon or name.")
            },

        // Resource Relationship Level Class
        { "ResourceRelationship",           N_("Resource Relationship"),                 "ResourceRelationship",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing relationships between resources based information."),
        },
            // Resource Relationship Level Terms
            { "resourceRelationshipID",         N_("Resource Relationship ID"),             "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for an instance of relationship between one resource (the subject) and another (relatedResource, the object).")
            },
            { "resourceID",                     N_("Resource ID"),                          "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the resource that is the subject of the relationship.")
            },
            { "relatedResourceID",              N_("Related Resource ID"),                  "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for a related resource (the object, rather than the subject of the relationship).")
            },
            { "relationshipOfResource",         N_("Relationship Of Resource"),             "Text",      xmpText,   xmpExternal,      
                                                N_("The relationship of the resource identified by relatedResourceID to the subject (optionally identified by the resourceID). Recommended best practice is to use a controlled vocabulary.")
            },
            { "relationshipAccordingTo",        N_("Relationship According To"),            "Text",      xmpText,   xmpExternal,      
                                                N_("The source (person, organization, publication, reference) establishing the relationship between the two resources.")
            },
            { "relationshipEstablishedDate",    N_("Relationship Established Date"),        "Date",      xmpText,    xmpExternal,      
                                                N_("The date-time on which the relationship between the two resources was established. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "relationshipRemarks",            N_("Relationship Remarks"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("Comments or notes about the relationship between the two resources.")
            },

        // Measurement Or Fact Level Class
        { "MeasurementOrFact",              N_("Measurement Or Fact"),                 "MeasurementOrFact",    xmpText,   xmpInternal, 
                                            N_("*Main structure* containing measurement based information."),
        },
            // Measurement Or Fact Level Terms
            { "measurementID",                  N_("Measurement ID"),                       "Text",      xmpText,   xmpExternal,      
                                                N_("An identifier for the MeasurementOrFact (information pertaining to measurements, facts, characteristics, or assertions). May be a global unique identifier or an identifier specific to the data set.")
            },
            { "measurementType",                N_("Measurement Type"),                     "Text",      xmpText,   xmpExternal,      
                                                N_("The nature of the measurement, fact, characteristic, or assertion. Recommended best practice is to use a controlled vocabulary.")
            },
            { "measurementValue",               N_("Measurement Value"),                    "Text",      xmpText,   xmpExternal,      
                                                N_("The value of the measurement, fact, characteristic, or assertion.")
            },
            { "measurementAccuracy",            N_("Measurement Accuracy"),                 "Text",      xmpText,   xmpExternal,      
                                                N_("The description of the potential error associated with the measurementValue.")
            },
            { "measurementUnit",                N_("Measurement Unit"),                     "Text",      xmpText,   xmpExternal,      
                                                N_("The units associated with the measurementValue. Recommended best practice is to use the International System of Units (SI).")
            },
            { "measurementDeterminedDate",      N_("Measurement Determined Date"),          "Date",      xmpText,    xmpExternal,      
                                                N_("The date on which the MeasurementOrFact was made. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")
            },
            { "measurementDeterminedBy",        N_("Measurement Determined By"),            "bag Text",     xmpBag,   xmpExternal,      
                                                N_("A list (concatenated and separated) of names of people, groups, or organizations who determined the value of the MeasurementOrFact.")
            },
            { "measurementMethod",              N_("Measurement Method"),                   "Text",      xmpText,   xmpExternal,      
                                                N_("A description of or reference to (publication, URI) the method or protocol used to determine the measurement, fact, characteristic, or assertion.")
            },
            { "measurementRemarks",             N_("Measurement Remarks"),                  "Text",      xmpText,   xmpExternal,      
                                                N_("Comments or notes accompanying the MeasurementOrFact.")
            },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpAcdseeInfo[] = {
        { "caption",       N_("Caption"),     "Text",                 xmpText, xmpExternal,   N_("A brief description of the file")   },
        { "datetime",      N_("Date Time"),   "Text",                 xmpText, xmpExternal,   N_("Date and Time")   },
        { "author",        N_("Author"),      "Text",                 xmpText, xmpExternal,   N_("The name of the author or photographer")   },
        { "rating",        N_("Rating"),      "Text",                 xmpText, xmpExternal,   N_("Numerical rating from 1 to 5")   },
        { "notes",         N_("Notes"),       "Text",                 xmpText, xmpExternal,   N_("Any descriptive or additional free-form text up to 4,095 characters")   },
        { "tagged",        N_("Tagged"),      "Text",                 xmpText, xmpExternal,   N_("True or False")   },
        { "categories",    N_("Categories"),  "Text",                 xmpText, xmpExternal,   N_("Catalog of hierarchical keywords and groups")   },

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
