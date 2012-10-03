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
  File:      actions.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   08-Dec-03, ahu: created
             30-Apr-06, Roger Larsson: Print filename if processing multiple files
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: actions.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#ifndef EXV_HAVE_TIMEGM
# include "timegm.h"
#endif
#include "actions.hpp"
#include "exiv2app.hpp"
#include "image.hpp"
#include "jpgimage.hpp"
#include "xmpsidecar.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "exif.hpp"
#include "easyaccess.hpp"
#include "iptc.hpp"
#include "xmp.hpp"
#include "preview.hpp"
#include "futils.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cassert>
#include <sys/types.h>                  // for stat()
#include <sys/stat.h>                   // for stat()
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                    // for stat()
#endif
#ifdef _MSC_VER
# include <sys/utime.h>
#else
# include <utime.h>
#endif

// *****************************************************************************
// local declarations
namespace {

    //! Helper class to set the timestamp of a file to that of another file
    class Timestamp {
    public:
        //! C'tor
        Timestamp() : actime_(0), modtime_(0) {}
        //! Read the timestamp of a file
        int read(const std::string& path);
        //! Read the timestamp from a broken-down time in buffer \em tm.
        int read(struct tm* tm);
        //! Set the timestamp of a file
        int touch(const std::string& path);

    private:
        time_t actime_;
        time_t modtime_;
    };

    /*!
      @brief Convert a string "YYYY:MM:DD HH:MI:SS" to a struct tm type,
             returns 0 if successful
     */
    int str2Tm(const std::string& timeStr, struct tm* tm);

    //! Convert a localtime to a string "YYYY:MM:DD HH:MI:SS", "" on error
    std::string time2Str(time_t time);

    //! Convert a tm structure to a string "YYYY:MM:DD HH:MI:SS", "" on error
    std::string tm2Str(const struct tm* tm);

    /*!
      @brief Copy metadata from source to target according to Params::copyXyz

      @param source Source file path
      @param target Target file path. An *.exv file is created if target doesn't
                    exist.
      @param targetType Image type for the target image in case it needs to be
                    created.
      @param preserve Indicates if existing metadata in the target file should
                    be kept.
      @return 0 if successful, else an error code
    */
    int metacopy(const std::string& source,
                 const std::string& target,
                 int targetType,
                 bool preserve);

    /*!
      @brief Rename a file according to a timestamp value.

      @param path The original file path. Contains the new path on exit.
      @param tm   Pointer to a buffer with the broken-down time to rename
                  the file to.
      @return 0 if successful, -1 if the file was skipped, 1 on error.
    */
    int renameFile(std::string& path, const struct tm* tm);

    /*!
      @brief Make a file path from the current file path, destination
             directory (if any) and the filename extension passed in.

      @param path Path of the existing file
      @param ext  New filename extension (incl. the dot '.' if required)
      @return 0 if successful, 1 if the new file exists and the user
             chose not to overwrite it.
     */
    std::string newFilePath(const std::string& path, const std::string& ext);

    /*!
      @brief Check if file \em path exists and whether it should be
             overwritten. Ask user if necessary. Return 1 if the file
             exists and shouldn't be overwritten, else 0.
     */
    int dontOverwrite(const std::string& path);
}

// *****************************************************************************
// class member definitions
namespace Action {

    Task::~Task()
    {
    }

    Task::AutoPtr Task::clone() const
    {
        return AutoPtr(clone_());
    }

    TaskFactory* TaskFactory::instance_ = 0;

    TaskFactory& TaskFactory::instance()
    {
        if (0 == instance_) {
            instance_ = new TaskFactory;
        }
        return *instance_;
    } // TaskFactory::instance

    void TaskFactory::cleanup()
    {
        if (instance_ != 0) {
            Registry::iterator e = registry_.end();
            for (Registry::iterator i = registry_.begin(); i != e; ++i) {
                delete i->second;
            }
            delete instance_;
            instance_ = 0;
        }
    } //TaskFactory::cleanup

    void TaskFactory::registerTask(TaskType type, Task::AutoPtr task)
    {
        Registry::iterator i = registry_.find(type);
        if (i != registry_.end()) {
            delete i->second;
        }
        registry_[type] = task.release();
    } // TaskFactory::registerTask

    TaskFactory::TaskFactory()
    {
        // Register a prototype of each known task
        registerTask(adjust,  Task::AutoPtr(new Adjust));
        registerTask(print,   Task::AutoPtr(new Print));
        registerTask(rename,  Task::AutoPtr(new Rename));
        registerTask(erase,   Task::AutoPtr(new Erase));
        registerTask(extract, Task::AutoPtr(new Extract));
        registerTask(insert,  Task::AutoPtr(new Insert));
        registerTask(modify,  Task::AutoPtr(new Modify));
        registerTask(fixiso,  Task::AutoPtr(new FixIso));
        registerTask(fixcom,  Task::AutoPtr(new FixCom));
    } // TaskFactory c'tor

    Task::AutoPtr TaskFactory::create(TaskType type)
    {
        Registry::const_iterator i = registry_.find(type);
        if (i != registry_.end() && i->second != 0) {
            Task* t = i->second;
            return t->clone();
        }
        return Task::AutoPtr(0);
    } // TaskFactory::create

    Print::~Print()
    {
    }

    int Print::run(const std::string& path)
    try {
        path_ = path;
        int rc = 0;
        switch (Params::instance().printMode_) {
        case Params::pmSummary: rc = printSummary();     break;
        case Params::pmList:    rc = printList();        break;
        case Params::pmComment: rc = printComment();     break;
        case Params::pmPreview: rc = printPreviewList(); break;
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e) {
        std::cerr << "Exiv2 exception in print action for file "
                  << path << ":\n" << e << "\n";
        return 1;
    } // Print::run

    int Print::printSummary()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": "
                      << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        align_ = 16;

        // Filename
        printLabel(_("File name"));
        std::cout << path_ << std::endl;

        // Filesize
        struct stat buf;
        if (0 == stat(path_.c_str(), &buf)) {
            printLabel(_("File size"));
            std::cout << buf.st_size << " " << _("Bytes") << std::endl;
        }

        // MIME type
        printLabel(_("MIME type"));
        std::cout << image->mimeType() << std::endl;

        // Image size
        printLabel(_("Image size"));
        std::cout << image->pixelWidth() << " x " << image->pixelHeight() << std::endl;

        if (exifData.empty()) {
            std::cerr << path_ << ": "
                      << _("No Exif data found in the file\n");
            return -3;
        }

        // Camera make
        printTag(exifData, "Exif.Image.Make", _("Camera make"));

        // Camera model
        printTag(exifData, "Exif.Image.Model", _("Camera model"));

        // Image Timestamp
        printTag(exifData, "Exif.Photo.DateTimeOriginal", _("Image timestamp"));

        // Image number
        // Todo: Image number for cameras other than Canon
        printTag(exifData, "Exif.Canon.FileNumber", _("Image number"));

        // Exposure time
        // From ExposureTime, failing that, try ShutterSpeedValue
        bool done = false;
        printLabel(_("Exposure time"));
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ExposureTime");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ShutterSpeedValue");
        }
        std::cout << std::endl;

        // Aperture
        // Get if from FNumber and, failing that, try ApertureValue
        done = false;
        printLabel(_("Aperture"));
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.FNumber");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ApertureValue");
        }
        std::cout << std::endl;

        // Exposure bias
        printTag(exifData, "Exif.Photo.ExposureBiasValue", _("Exposure bias"));

        // Flash
        printTag(exifData, "Exif.Photo.Flash", _("Flash"));

        // Flash bias
        printTag(exifData, Exiv2::flashBias, _("Flash bias"));

        // Actual focal length and 35 mm equivalent
        // Todo: Calculate 35 mm equivalent a la jhead
        Exiv2::ExifData::const_iterator md;
        printLabel(_("Focal length"));
        if (1 == printTag(exifData, "Exif.Photo.FocalLength")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Photo.FocalLengthIn35mmFilm"));
            if (md != exifData.end()) {
                std::cout << " ("<< _("35 mm equivalent") << ": "
                          << md->print(&exifData) << ")";
            }
        }
        else {
            printTag(exifData, "Exif.Canon.FocalLength");
        }
        std::cout << std::endl;

        // Subject distance
        printLabel(_("Subject distance"));
        done = false;
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.SubjectDistance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonSi.SubjectDistance");
        }
        std::cout << std::endl;

        // ISO speed
        printTag(exifData, Exiv2::isoSpeed, _("ISO speed"));

        // Exposure mode
        printTag(exifData, Exiv2::exposureMode, _("Exposure mode"));

        // Metering mode
        printTag(exifData, "Exif.Photo.MeteringMode", _("Metering mode"));

        // Macro mode
        printTag(exifData, Exiv2::macroMode, _("Macro mode"));

        // Image quality setting (compression)
        printTag(exifData, Exiv2::imageQuality, _("Image quality"));

        // Exif Resolution
        printLabel(_("Exif Resolution"));
        long xdim = 0;
        long ydim = 0;
        if (image->mimeType() == "image/tiff") {
            xdim = image->pixelWidth();
            ydim = image->pixelHeight();
        }
        else {
            md = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
            if (md == exifData.end()) {
                md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
            }
            if (md != exifData.end() && md->count() > 0) {
                xdim = md->toLong();
            }
            md = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
            if (md == exifData.end()) {
                md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
            }
            if (md != exifData.end() && md->count() > 0) {
                ydim = md->toLong();
            }
        }
        if (xdim != 0 && ydim != 0) {
            std::cout << xdim << " x " << ydim;
        }
        std::cout << std::endl;

        // White balance
        printTag(exifData, Exiv2::whiteBalance, _("White balance"));

        // Thumbnail
        printLabel(_("Thumbnail"));
        Exiv2::ExifThumbC exifThumb(exifData);
        std::string thumbExt = exifThumb.extension();
        if (thumbExt.empty()) {
            std::cout << _("None");
        }
        else {
            Exiv2::DataBuf buf = exifThumb.copy();
            if (buf.size_ == 0) {
                std::cout << _("None");
            }
            else {
                std::cout << exifThumb.mimeType() << ", "
                          << buf.size_ << " " << _("Bytes");
            }
        }
        std::cout << std::endl;

        // Copyright
        printTag(exifData, "Exif.Image.Copyright", _("Copyright"));

        // Exif Comment
        printTag(exifData, "Exif.Photo.UserComment", _("Exif comment"));
        std::cout << std::endl;

        return 0;
    } // Print::printSummary

    void Print::printLabel(const std::string& label) const
    {
        std::cout << std::setfill(' ') << std::left;
        if (Params::instance().files_.size() > 1) {
            std::cout << std::setw(20) << path_ << " ";
        }
        std::cout << std::setw(align_)
                  << label << ": ";
    }

    int Print::printTag(const Exiv2::ExifData& exifData,
                        const std::string& key,
                        const std::string& label) const
    {
        int rc = 0;
        if (!label.empty()) {
            printLabel(label);
        }
        Exiv2::ExifKey ek(key);
        Exiv2::ExifData::const_iterator md = exifData.findKey(ek);
        if (md != exifData.end()) {
            md->write(std::cout, &exifData);
            rc = 1;
        }
        if (!label.empty()) std::cout << std::endl;
        return rc;
    } // Print::printTag

    int Print::printTag(const Exiv2::ExifData& exifData,
                        EasyAccessFct easyAccessFct,
                        const std::string& label) const
    {
        int rc = 0;
        if (!label.empty()) {
            printLabel(label);
        }
        Exiv2::ExifData::const_iterator md = easyAccessFct(exifData);
        if (md != exifData.end()) {
            md->write(std::cout, &exifData);
            rc = 1;
        }
        if (!label.empty()) std::cout << std::endl;
        return rc;
    } // Print::printTag

    int Print::printList()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        // Set defaults for metadata types and data columns
        if (Params::instance().printTags_ == Exiv2::mdNone) {
            Params::instance().printTags_ = Exiv2::mdExif | Exiv2::mdIptc | Exiv2::mdXmp;
        }
        if (Params::instance().printItems_ == 0) {
            Params::instance().printItems_ = Params::prKey | Params::prType | Params::prCount | Params::prTrans;
        }
        return printMetadata(image.get());
    } // Print::printList

    int Print::printMetadata(const Exiv2::Image* image)
    {
        int rc = 0;
        if (Params::instance().printTags_ & Exiv2::mdExif) {
            const Exiv2::ExifData& exifData = image->exifData();
            for (Exiv2::ExifData::const_iterator md = exifData.begin();
                 md != exifData.end(); ++md) {
                printMetadatum(*md, image);
            }
            if (exifData.empty()) {
                if (Params::instance().verbose_) {
                    std::cerr << path_ << ": " << _("No Exif data found in the file\n");
                }
                rc = -3;
            }
        }
        if (Params::instance().printTags_ & Exiv2::mdIptc) {
            const Exiv2::IptcData& iptcData = image->iptcData();
            for (Exiv2::IptcData::const_iterator md = iptcData.begin();
                 md != iptcData.end(); ++md) {
                printMetadatum(*md, image);
            }
            if (iptcData.empty()) {
                if (Params::instance().verbose_) {
                    std::cerr << path_ << ": " << _("No IPTC data found in the file\n");
                }
                rc = -3;
            }
        }
        if (Params::instance().printTags_ & Exiv2::mdXmp) {
            const Exiv2::XmpData& xmpData = image->xmpData();
            for (Exiv2::XmpData::const_iterator md = xmpData.begin();
                 md != xmpData.end(); ++md) {
                printMetadatum(*md, image);
            }
            if (xmpData.empty()) {
                if (Params::instance().verbose_) {
                    std::cerr << path_ << ": " << _("No XMP data found in the file\n");
                }
                rc = -3;
            }
        }
        return rc;
    } // Print::printMetadata

    bool Print::grepTag(const std::string& key)
    {
        if (Params::instance().keys_.empty()) return true;
        for (Params::Keys::const_iterator k = Params::instance().keys_.begin();
             k != Params::instance().keys_.end(); ++k) {
            if (*k == key) return true;
        }
        return false;
    }

    void Print::printMetadatum(const Exiv2::Metadatum& md, const Exiv2::Image* pImage)
    {
        if (!grepTag(md.key())) return;

        if (   Params::instance().unknown_
            && md.tagName().substr(0, 2) == "0x") {
            return;
        }
        bool const manyFiles = Params::instance().files_.size() > 1;
        if (manyFiles) {
            std::cout << std::setfill(' ') << std::left << std::setw(20)
                      << path_ << "  ";
        }
        bool first = true;
        if (Params::instance().printItems_ & Params::prTag) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << "0x" << std::setw(4) << std::setfill('0')
                      << std::right << std::hex
                      << md.tag();
        }
        if (Params::instance().printItems_ & Params::prGroup) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << std::setw(12) << std::setfill(' ') << std::left
                      << md.groupName();
        }
        if (Params::instance().printItems_ & Params::prKey) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << std::setfill(' ') << std::left << std::setw(44)
                      << md.key();
        }
        if (Params::instance().printItems_ & Params::prName) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << std::setw(27) << std::setfill(' ') << std::left
                      << md.tagName();
        }
        if (Params::instance().printItems_ & Params::prLabel) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << std::setw(30) << std::setfill(' ') << std::left
                      << md.tagLabel();
        }
        if (Params::instance().printItems_ & Params::prType) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << std::setw(9) << std::setfill(' ') << std::left;
            const char* tn = md.typeName();
            if (tn) {
                std::cout << tn;
            }
            else {
                std::ostringstream os;
                os << "0x" << std::setw(4) << std::setfill('0') << std::hex << md.typeId();
                std::cout << os.str();
            }
        }
        if (Params::instance().printItems_ & Params::prCount) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << std::dec << std::setw(3)
                      << std::setfill(' ') << std::right
                      << md.count();
        }
        if (Params::instance().printItems_ & Params::prSize) {
            if (!first) std::cout << " ";
            first = false;
            std::cout << std::dec << std::setw(3)
                      << std::setfill(' ') << std::right
                      << md.size();
        }
        if (Params::instance().printItems_ & Params::prValue) {
            if (!first) std::cout << "  ";
            first = false;
            if (   Params::instance().binary_
                && (   md.typeId() == Exiv2::undefined
                    || md.typeId() == Exiv2::unsignedByte
                    || md.typeId() == Exiv2::signedByte)
                && md.size() > 128) {
                std::cout << _("(Binary value suppressed)") << std::endl;
                return;
            }
            bool done = false;
            if (0 == strcmp(md.key().c_str(), "Exif.Photo.UserComment")) {
                const Exiv2::CommentValue* pcv = dynamic_cast<const Exiv2::CommentValue*>(&md.value());
                if (pcv) {
                    Exiv2::CommentValue::CharsetId csId = pcv->charsetId();
                    if (csId != Exiv2::CommentValue::undefined) {
                        std::cout << "charset=\"" << Exiv2::CommentValue::CharsetInfo::name(csId) << "\" ";
                    }
                    std::cout << pcv->comment(Params::instance().charset_.c_str());
                    done = true;
                }
            }
            if (!done) std::cout << std::dec << md.value();
        }
        if (Params::instance().printItems_ & Params::prTrans) {
            if (!first) std::cout << "  ";
            first = false;
            if (   Params::instance().binary_
                && (   md.typeId() == Exiv2::undefined
                    || md.typeId() == Exiv2::unsignedByte
                    || md.typeId() == Exiv2::signedByte)
                && md.size() > 128) {
                std::cout << _("(Binary value suppressed)") << std::endl;
                return;
            }
            bool done = false;
            if (0 == strcmp(md.key().c_str(), "Exif.Photo.UserComment")) {
                const Exiv2::CommentValue* pcv = dynamic_cast<const Exiv2::CommentValue*>(&md.value());
                if (pcv) {
                    std::cout << pcv->comment(Params::instance().charset_.c_str());
                    done = true;
                }
            }
            if (!done) std::cout << std::dec << md.print(&pImage->exifData());
        }
        if (Params::instance().printItems_ & Params::prHex) {
            if (!first) std::cout << std::endl;
            first = false;
            if (   Params::instance().binary_
                && (   md.typeId() == Exiv2::undefined
                    || md.typeId() == Exiv2::unsignedByte
                    || md.typeId() == Exiv2::signedByte)
                && md.size() > 128) {
                std::cout << _("(Binary value suppressed)") << std::endl;
                return;
            }
            Exiv2::DataBuf buf(md.size());
            md.copy(buf.pData_, pImage->byteOrder());
            Exiv2::hexdump(std::cout, buf.pData_, buf.size_);
        }
        std::cout << std::endl;
    } // Print::printMetadatum

    int Print::printComment()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        if (Params::instance().verbose_) {
            std::cout << _("JPEG comment") << ": ";
        }
        std::cout << image->comment() << std::endl;
        return 0;
    } // Print::printComment

    int Print::printPreviewList()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        bool const manyFiles = Params::instance().files_.size() > 1;
        int cnt = 0;
        Exiv2::PreviewManager pm(*image);
        Exiv2::PreviewPropertiesList list = pm.getPreviewProperties();
        for (Exiv2::PreviewPropertiesList::const_iterator pos = list.begin(); pos != list.end(); ++pos) {
            if (manyFiles) {
                std::cout << std::setfill(' ') << std::left << std::setw(20)
                          << path_ << "  ";
            }
            std::cout << _("Preview") << " " << ++cnt << ": "
                      << pos->mimeType_ << ", ";
            if (pos->width_ != 0 && pos->height_ != 0) {
                std::cout << pos->width_ << "x" << pos->height_ << " "
                          << _("pixels") << ", ";
            }
            std::cout << pos->size_ << " " << _("bytes") << "\n";
        }
        return 0;
    } // Print::printPreviewList

    Print::AutoPtr Print::clone() const
    {
        return AutoPtr(clone_());
    }

    Print* Print::clone_() const
    {
        return new Print(*this);
    }

    Rename::~Rename()
    {
    }

    int Rename::run(const std::string& path)
    {
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
        Exiv2::ExifData::iterator md = exifData.findKey(key);
        if (md == exifData.end()) {
            key = Exiv2::ExifKey("Exif.Image.DateTime");
            md = exifData.findKey(key);
        }
        if (md == exifData.end()) {
            std::cerr << _("Neither tag") << " `Exif.Photo.DateTimeOriginal' "
                      << _("nor") << " `Exif.Image.DateTime' "
                      << _("found in the file") << " " << path << "\n";
            return 1;
        }
        std::string v = md->toString();
        if (v.length() == 0 || v[0] == ' ') {
            std::cerr << _("Image file creation timestamp not set in the file")
                      << " " << path << "\n";
            return 1;
        }
        struct tm tm;
        if (str2Tm(v, &tm) != 0) {
            std::cerr << _("Failed to parse timestamp") << " `" << v
                      << "' " << _("in the file") << " " << path << "\n";
            return 1;
        }
        if (   Params::instance().timestamp_
            || Params::instance().timestampOnly_) {
            ts.read(&tm);
        }
        int rc = 0;
        std::string newPath = path;
        if (Params::instance().timestampOnly_) {
            if (Params::instance().verbose_) {
                std::cout << _("Updating timestamp to") << " " << v << std::endl;
            }
        }
        else {
            rc = renameFile(newPath, &tm);
            if (rc == -1) return 0; // skip
        }
        if (   0 == rc
            && (   Params::instance().preserve_
                || Params::instance().timestamp_
                || Params::instance().timestampOnly_)) {
            ts.touch(newPath);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in rename action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    }} // Rename::run

    Rename::AutoPtr Rename::clone() const
    {
        return AutoPtr(clone_());
    }

    Rename* Rename::clone_() const
    {
        return new Rename(*this);
    }

    Erase::~Erase()
    {
    }

    int Erase::run(const std::string& path)
    try {
        path_ = path;

        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        // Thumbnail must be before Exif
        int rc = 0;
        if (Params::instance().target_ & Params::ctThumb) {
            rc = eraseThumbnail(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctExif) {
            rc = eraseExifData(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctIptc) {
            rc = eraseIptcData(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctComment) {
            rc = eraseComment(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctXmp) {
            rc = eraseXmpData(image.get());
        }
        if (0 == rc) {
            image->writeMetadata();
        }
        if (Params::instance().preserve_) {
            ts.touch(path);
        }

        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in erase action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Erase::run

    int Erase::eraseThumbnail(Exiv2::Image* image) const
    {
        Exiv2::ExifThumb exifThumb(image->exifData());
        std::string thumbExt = exifThumb.extension();
        if (thumbExt.empty()) {
            return 0;
        }
        exifThumb.erase();
        if (Params::instance().verbose_) {
            std::cout << _("Erasing thumbnail data") << std::endl;
        }
        return 0;
    }

    int Erase::eraseExifData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->exifData().count() > 0) {
            std::cout << _("Erasing Exif data from the file") << std::endl;
        }
        image->clearExifData();
        return 0;
    }

    int Erase::eraseIptcData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->iptcData().count() > 0) {
            std::cout << _("Erasing IPTC data from the file") << std::endl;
        }
        image->clearIptcData();
        return 0;
    }

    int Erase::eraseComment(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->comment().size() > 0) {
            std::cout << _("Erasing JPEG comment from the file") << std::endl;
        }
        image->clearComment();
        return 0;
    }

    int Erase::eraseXmpData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->xmpData().count() > 0) {
            std::cout << _("Erasing XMP data from the file") << std::endl;
        }
        image->clearXmpData();                  // Quick fix for bug #612
        image->clearXmpPacket();
        return 0;
    }

    Erase::AutoPtr Erase::clone() const
    {
        return AutoPtr(clone_());
    }

    Erase* Erase::clone_() const
    {
        return new Erase(*this);
    }

    Extract::~Extract()
    {
    }

    int Extract::run(const std::string& path)
    try {
        path_ = path;
        int rc = 0;
        if (Params::instance().target_ & Params::ctThumb) {
            rc = writeThumbnail();
        }
        if (Params::instance().target_ & Params::ctXmpSidecar) {
            std::string xmpPath = newFilePath(path_, ".xmp");
            if (dontOverwrite(xmpPath)) return 0;
            rc = metacopy(path_, xmpPath, Exiv2::ImageType::xmp, false);
        }
        if (Params::instance().target_ & Params::ctPreview) {
            rc = writePreviews();
        }
        if (   !(Params::instance().target_ & Params::ctXmpSidecar)
            && !(Params::instance().target_ & Params::ctThumb)
            && !(Params::instance().target_ & Params::ctPreview)) {
            std::string exvPath = newFilePath(path_, ".exv");
            if (dontOverwrite(exvPath)) return 0;
            rc = metacopy(path_, exvPath, Exiv2::ImageType::exv, false);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in extract action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Extract::run

    int Extract::writeThumbnail() const
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path_
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        int rc = 0;
        Exiv2::ExifThumb exifThumb(exifData);
        std::string thumbExt = exifThumb.extension();
        if (thumbExt.empty()) {
            std::cerr << path_ << ": " << _("Image does not contain an Exif thumbnail\n");
        }
        else {
            std::string thumb = newFilePath(path_, "-thumb");
            std::string thumbPath = thumb + thumbExt;
            if (dontOverwrite(thumbPath)) return 0;
            if (Params::instance().verbose_) {
                Exiv2::DataBuf buf = exifThumb.copy();
                if (buf.size_ != 0) {
                    std::cout << _("Writing thumbnail") << " (" << exifThumb.mimeType() << ", "
                              << buf.size_ << " " << _("Bytes") << ") " << _("to file") << " "
                              << thumbPath << std::endl;
                }
            }
            rc = exifThumb.writeFile(thumb);
            if (rc == 0) {
                std::cerr << path_ << ": " << _("Exif data doesn't contain a thumbnail\n");
            }
        }
        return rc;
    } // Extract::writeThumbnail

    int Extract::writePreviews() const
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();

        Exiv2::PreviewManager pvMgr(*image);
        Exiv2::PreviewPropertiesList pvList = pvMgr.getPreviewProperties();

        const Params::PreviewNumbers& numbers = Params::instance().previewNumbers_;
        for (Params::PreviewNumbers::const_iterator n = numbers.begin(); n != numbers.end(); ++n) {
            if (*n == 0) {
                // Write all previews
                for (int num = 0; num < static_cast<int>(pvList.size()); ++num) {
                    writePreviewFile(pvMgr.getPreviewImage(pvList[num]), num + 1);
                }
                break;
            }
            if (*n > static_cast<int>(pvList.size())) {
                std::cerr << path_ << ": " << _("Image does not have preview")
                          << " " << *n << "\n";
                continue;
            }
            writePreviewFile(pvMgr.getPreviewImage(pvList[*n - 1]), *n);
        }
        return 0;
    } // Extract::writePreviews

    void Extract::writePreviewFile(const Exiv2::PreviewImage& pvImg, int num) const
    {
        std::string pvFile = newFilePath(path_, "-preview") + Exiv2::toString(num);
        std::string pvPath = pvFile + pvImg.extension();
        if (dontOverwrite(pvPath)) return;
        if (Params::instance().verbose_) {
            std::cout << _("Writing preview") << " " << num << " ("
                      << pvImg.mimeType() << ", ";
            if (pvImg.width() != 0 && pvImg.height() != 0) {
                std::cout << pvImg.width() << "x" << pvImg.height() << " "
                          << _("pixels") << ", ";
            }
            std::cout << pvImg.size() << " " << _("bytes") << ") "
                      << _("to file") << " " << pvPath << std::endl;
        }
        long rc = pvImg.writeFile(pvFile);
        if (rc == 0) {
            std::cerr << path_ << ": " << _("Image does not have preview")
                      << " " << num << "\n";
        }
    } // Extract::writePreviewFile

    Extract::AutoPtr Extract::clone() const
    {
        return AutoPtr(clone_());
    }

    Extract* Extract::clone_() const
    {
        return new Extract(*this);
    }

    Insert::~Insert()
    {
    }

    int Insert::run(const std::string& path)
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        int rc = 0;
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        if (Params::instance().target_ & Params::ctThumb) {
            rc = insertThumbnail(path);
        }
        if (   rc == 0
            && (   Params::instance().target_ & Params::ctExif
                || Params::instance().target_ & Params::ctIptc
                || Params::instance().target_ & Params::ctComment
                || Params::instance().target_ & Params::ctXmp)) {
            std::string suffix = Params::instance().suffix_;
            if (suffix.empty()) suffix = ".exv";
            if (Params::instance().target_ & Params::ctXmpSidecar) suffix = ".xmp";
            std::string exvPath = newFilePath(path, suffix);
            rc = metacopy(exvPath, path, Exiv2::ImageType::none, true);
        }
        if (0 == rc && Params::instance().target_ & Params::ctXmpSidecar) {
            rc = insertXmpPacket(path);
        }
        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in insert action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Insert::run

    int Insert::insertXmpPacket(const std::string& path) const
    {
        std::string xmpPath = newFilePath(path, ".xmp");
        if (!Exiv2::fileExists(xmpPath, true)) {
            std::cerr << xmpPath
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::DataBuf buf = Exiv2::readFile(xmpPath);
        std::string xmpPacket;
        xmpPacket.assign(reinterpret_cast<char*>(buf.pData_), buf.size_);
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        image->setXmpPacket(xmpPacket);
        image->writeMetadata();

        return 0;
    }

    int Insert::insertThumbnail(const std::string& path) const
    {
        std::string thumbPath = newFilePath(path, "-thumb.jpg");
        if (!Exiv2::fileExists(thumbPath, true)) {
            std::cerr << thumbPath
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifThumb exifThumb(image->exifData());
        exifThumb.setJpegThumbnail(thumbPath);
        image->writeMetadata();

        return 0;
    } // Insert::insertThumbnail

    Insert::AutoPtr Insert::clone() const
    {
        return AutoPtr(clone_());
    }

    Insert* Insert::clone_() const
    {
        return new Insert(*this);
    }

    Modify::~Modify()
    {
    }

    int Modify::run(const std::string& path)
    {
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();

        int rc = applyCommands(image.get());

        // Save both exif and iptc metadata
        image->writeMetadata();

        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in modify action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    }
    } // Modify::run

    int Modify::applyCommands(Exiv2::Image* pImage)
    {
        if (!Params::instance().jpegComment_.empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Setting JPEG comment") << " '"
                          << Params::instance().jpegComment_
                          << "'"
                          << std::endl;
            }
            pImage->setComment(Params::instance().jpegComment_);
        }

        // loop through command table and apply each command
        ModifyCmds& modifyCmds = Params::instance().modifyCmds_;
        ModifyCmds::const_iterator i = modifyCmds.begin();
        ModifyCmds::const_iterator end = modifyCmds.end();
        int rc = 0;
        int ret = 0;
        for (; i != end; ++i) {
            switch (i->cmdId_) {
            case add:
                ret = addMetadatum(pImage, *i);
                if (rc == 0) rc = ret;
                break;
            case set:
                ret = setMetadatum(pImage, *i);
                if (rc == 0) rc = ret;
                break;
            case del:
                delMetadatum(pImage, *i);
                break;
            case reg:
                regNamespace(*i);
                break;
            case invalidCmdId:
                assert(invalidCmdId == i->cmdId_);
                break;
            }
        }
        return rc;
    } // Modify::applyCommands

    int Modify::addMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Add") << " " << modifyCmd.key_ << " \""
                      << modifyCmd.value_ << "\" ("
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_)
                      << ")" << std::endl;
        }
        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData&  xmpData  = pImage->xmpData();
        Exiv2::Value::AutoPtr value = Exiv2::Value::create(modifyCmd.typeId_);
        int rc = value->read(modifyCmd.value_);
        if (0 == rc) {
            if (modifyCmd.metadataId_ == exif) {
                exifData.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
            }
            if (modifyCmd.metadataId_ == iptc) {
                iptcData.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
            }
            if (modifyCmd.metadataId_ == xmp) {
                xmpData.add(Exiv2::XmpKey(modifyCmd.key_), value.get());
            }
        }
        else {
            std::cerr << _("Warning") << ": " << modifyCmd.key_ << ": "
                      << _("Failed to read") << " "
                      << Exiv2::TypeInfo::typeName(value->typeId())
                      << " " << _("value")
                      << " \"" << modifyCmd.value_ << "\"\n";
        }
        return rc;
    }

    // This function looks rather complex because we try to avoid adding an
    // empty metadatum if reading the value fails
    int Modify::setMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Set") << " " << modifyCmd.key_ << " \""
                      << modifyCmd.value_ << "\" ("
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_)
                      << ")" << std::endl;
        }
        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData&  xmpData  = pImage->xmpData();
        Exiv2::Metadatum* metadatum = 0;
        if (modifyCmd.metadataId_ == exif) {
            Exiv2::ExifData::iterator pos =
                exifData.findKey(Exiv2::ExifKey(modifyCmd.key_));
            if (pos != exifData.end()) {
                metadatum = &(*pos);
            }
        }
        if (modifyCmd.metadataId_ == iptc) {
            Exiv2::IptcData::iterator pos =
                iptcData.findKey(Exiv2::IptcKey(modifyCmd.key_));
            if (pos != iptcData.end()) {
                metadatum = &(*pos);
            }
        }
        if (modifyCmd.metadataId_ == xmp) {
            Exiv2::XmpData::iterator pos =
                xmpData.findKey(Exiv2::XmpKey(modifyCmd.key_));
            if (pos != xmpData.end()) {
                metadatum = &(*pos);
            }
        }
        // If a type was explicitly requested, use it; else
        // use the current type of the metadatum, if any;
        // or the default type
        Exiv2::Value::AutoPtr value;
        if (metadatum) {
            value = metadatum->getValue();
        }
        if (   value.get() == 0
            || (   modifyCmd.explicitType_
                && modifyCmd.typeId_ != value->typeId())) {
            value = Exiv2::Value::create(modifyCmd.typeId_);
        }
        int rc = value->read(modifyCmd.value_);
        if (0 == rc) {
            if (metadatum) {
                metadatum->setValue(value.get());
            }
            else {
                if (modifyCmd.metadataId_ == exif) {
                    exifData.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
                }
                if (modifyCmd.metadataId_ == iptc) {
                    iptcData.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
                }
                if (modifyCmd.metadataId_ == xmp) {
                    xmpData.add(Exiv2::XmpKey(modifyCmd.key_), value.get());
                }
            }
        }
        else {
            std::cerr << _("Warning") << ": " << modifyCmd.key_ << ": "
                      << _("Failed to read") << " "
                      << Exiv2::TypeInfo::typeName(value->typeId())
                      << " " << _("value")
                      << " \"" << modifyCmd.value_ << "\"\n";
        }
        return rc;
    }

    void Modify::delMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Del") << " " << modifyCmd.key_ << std::endl;
        }

        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData&  xmpData  = pImage->xmpData();
        if (modifyCmd.metadataId_ == exif) {
            Exiv2::ExifData::iterator pos;
            Exiv2::ExifKey exifKey = Exiv2::ExifKey(modifyCmd.key_);
            while((pos = exifData.findKey(exifKey)) != exifData.end()) {
                exifData.erase(pos);
            }
        }
        if (modifyCmd.metadataId_ == iptc) {
            Exiv2::IptcData::iterator pos;
            Exiv2::IptcKey iptcKey = Exiv2::IptcKey(modifyCmd.key_);
            while((pos = iptcData.findKey(iptcKey)) != iptcData.end()) {
                iptcData.erase(pos);
            }
        }
        if (modifyCmd.metadataId_ == xmp) {
            Exiv2::XmpData::iterator pos;
            Exiv2::XmpKey xmpKey = Exiv2::XmpKey(modifyCmd.key_);
            while((pos = xmpData.findKey(xmpKey)) != xmpData.end()) {
                xmpData.erase(pos);
            }
        }
    }

    void Modify::regNamespace(const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Reg ") << modifyCmd.key_ << "=\""
                      << modifyCmd.value_ << "\"" << std::endl;
        }
        Exiv2::XmpProperties::registerNs(modifyCmd.value_, modifyCmd.key_);
    }

    Modify::AutoPtr Modify::clone() const
    {
        return AutoPtr(clone_());
    }

    Modify* Modify::clone_() const
    {
        return new Modify(*this);
    }

    Adjust::~Adjust()
    {
    }

    int Adjust::run(const std::string& path)
    try {
        adjustment_      = Params::instance().adjustment_;
        yearAdjustment_  = Params::instance().yodAdjust_[Params::yodYear].adjustment_;
        monthAdjustment_ = Params::instance().yodAdjust_[Params::yodMonth].adjustment_;
        dayAdjustment_   = Params::instance().yodAdjust_[Params::yodDay].adjustment_;

        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        int rc = adjustDateTime(exifData, "Exif.Image.DateTime", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeOriginal", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeDigitized", path);
        if (rc) return 1;
        image->writeMetadata();
        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in adjust action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Adjust::run

    Adjust::AutoPtr Adjust::clone() const
    {
        return AutoPtr(clone_());
    }

    Adjust* Adjust::clone_() const
    {
        return new Adjust(*this);
    }

    int Adjust::adjustDateTime(Exiv2::ExifData& exifData,
                               const std::string& key,
                               const std::string& path) const
    {
        Exiv2::ExifKey ek(key);
        Exiv2::ExifData::iterator md = exifData.findKey(ek);
        if (md == exifData.end()) {
            // Key not found. That's ok, we do nothing.
            return 0;
        }
        std::string timeStr = md->toString();
        if (timeStr == "" || timeStr[0] == ' ') {
            std::cerr << path << ": " << _("Timestamp of metadatum with key") << " `"
                      << ek << "' " << _("not set\n");
            return 1;
        }
        if (Params::instance().verbose_) {
            bool comma = false;
            std::cout << _("Adjusting") << " `" << ek << "' " << _("by");
            if (yearAdjustment_ != 0) {
                std::cout << (yearAdjustment_ < 0 ? " " : " +") << yearAdjustment_ << " ";
                if (yearAdjustment_ < -1 || yearAdjustment_ > 1) {
                    std::cout << _("years");
                }
                else {
                    std::cout << _("year");
                }
                comma = true;
            }
            if (monthAdjustment_ != 0) {
                if (comma) std::cout << ",";
                std::cout << (monthAdjustment_ < 0 ? " " : " +") << monthAdjustment_ << " ";
                if (monthAdjustment_ < -1 || monthAdjustment_ > 1) {
                    std::cout << _("months");
                }
                else {
                    std::cout << _("month");
                }
                comma = true;
            }
            if (dayAdjustment_ != 0) {
                if (comma) std::cout << ",";
                std::cout << (dayAdjustment_ < 0 ? " " : " +") << dayAdjustment_ << " ";
                if (dayAdjustment_ < -1 || dayAdjustment_ > 1) {
                    std::cout << _("days");
                }
                else {
                    std::cout << _("day");
                }
                comma = true;
            }
            if (adjustment_ != 0) {
                if (comma) std::cout << ",";
                std::cout << " " << adjustment_ << _("s");
            }
        }
        struct tm tm;
        if (str2Tm(timeStr, &tm) != 0) {
            if (Params::instance().verbose_) std::cout << std::endl;
            std::cerr << path << ": " << _("Failed to parse timestamp") << " `"
                      << timeStr << "'\n";
            return 1;
        }
        const long monOverflow = (tm.tm_mon + monthAdjustment_) / 12;
        tm.tm_mon = (tm.tm_mon + monthAdjustment_) % 12;
        tm.tm_year += yearAdjustment_ + monOverflow;
        // Let's not create files with non-4-digit years, we can't read them.
        if (tm.tm_year > 9999 - 1900 || tm.tm_year < 1000 - 1900) {
            if (Params::instance().verbose_) std::cout << std::endl;
            std::cerr << path << ": " << _("Can't adjust timestamp by") << " "
                      << yearAdjustment_ + monOverflow
                      << " " << _("years") << "\n";
            return 1;
        }
        time_t time = mktime(&tm);
        time += adjustment_ + dayAdjustment_ * 86400;
        timeStr = time2Str(time);
        if (Params::instance().verbose_) {
            std::cout << " " << _("to") << " " << timeStr << std::endl;
        }
        md->setValue(timeStr);
        return 0;
    } // Adjust::adjustDateTime

    FixIso::~FixIso()
    {
    }

    int FixIso::run(const std::string& path)
    {
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " <<_("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        Exiv2::ExifData::const_iterator md = Exiv2::isoSpeed(exifData);
        if (md != exifData.end()) {
            if (strcmp(md->key().c_str(), "Exif.Photo.ISOSpeedRatings") == 0) {
                if (Params::instance().verbose_) {
                    std::cout << _("Standard Exif ISO tag exists; not modified\n");
                }
                return 0;
            }
            // Copy the proprietary tag to the standard place
            std::ostringstream os;
            md->write(os, &exifData);
            if (Params::instance().verbose_) {
                std::cout << _("Setting Exif ISO value to") << " " << os.str() << "\n";
            }
            exifData["Exif.Photo.ISOSpeedRatings"] = os.str();
        }
        image->writeMetadata();
        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return 0;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in fixiso action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    }
    } // FixIso::run

    FixIso::AutoPtr FixIso::clone() const
    {
        return AutoPtr(clone_());
    }

    FixIso* FixIso::clone_() const
    {
        return new FixIso(*this);
    }

    FixCom::~FixCom()
    {
    }

    int FixCom::run(const std::string& path)
    {
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " <<_("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        Exiv2::ExifData::iterator pos = exifData.findKey(Exiv2::ExifKey("Exif.Photo.UserComment"));
        if (pos == exifData.end()) {
            if (Params::instance().verbose_) {
                std::cout << _("No Exif user comment found") << "\n";
            }
            return 0;
        }
        Exiv2::Value::AutoPtr v = pos->getValue();
        const Exiv2::CommentValue* pcv = dynamic_cast<const Exiv2::CommentValue*>(v.get());
        if (!pcv) {
            if (Params::instance().verbose_) {
                std::cout << _("Found Exif user comment with unexpected value type") << "\n";
            }
            return 0;
        }
        Exiv2::CommentValue::CharsetId csId = pcv->charsetId();
        if (csId != Exiv2::CommentValue::unicode) {
            if (Params::instance().verbose_) {
                std::cout << _("No Exif UNICODE user comment found") << "\n";
            }
            return 0;
        }
        std::string comment = pcv->comment(Params::instance().charset_.c_str());
        if (Params::instance().verbose_) {
            std::cout << _("Setting Exif UNICODE user comment to") << " \"" << comment << "\"\n";
        }
        comment = std::string("charset=\"") + Exiv2::CommentValue::CharsetInfo::name(csId) + "\" " + comment;
        // Remove BOM and convert value from source charset to UCS-2, but keep byte order
        pos->setValue(comment);
        image->writeMetadata();
        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return 0;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in fixcom action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    }
    } // FixCom::run

    FixCom::AutoPtr FixCom::clone() const
    {
        return AutoPtr(clone_());
    }

    FixCom* FixCom::clone_() const
    {
        return new FixCom(*this);
    }

}                                       // namespace Action

// *****************************************************************************
// local definitions
namespace {

    //! @cond IGNORE
    int Timestamp::read(const std::string& path)
    {
        struct stat buf;
        int rc = stat(path.c_str(), &buf);
        if (0 == rc) {
            actime_  = buf.st_atime;
            modtime_ = buf.st_mtime;
        }
        return rc;
    }

    int Timestamp::read(struct tm* tm)
    {
        int rc = 1;
        time_t t = mktime(tm); // interpret tm according to current timezone settings
        if (t != (time_t)-1) {
            rc = 0;
            actime_  = t;
            modtime_ = t;
        }
        return rc;
    }

    int Timestamp::touch(const std::string& path)
    {
        if (0 == actime_) return 1;
        struct utimbuf buf;
        buf.actime = actime_;
        buf.modtime = modtime_;
        return utime(path.c_str(), &buf);
    }
    //! @endcond

    int str2Tm(const std::string& timeStr, struct tm* tm)
    {
        if (timeStr.length() == 0 || timeStr[0] == ' ') return 1;
        if (timeStr.length() < 19) return 2;
        if (   timeStr[4]  != ':' || timeStr[7]  != ':' || timeStr[10] != ' '
            || timeStr[13] != ':' || timeStr[16] != ':') return 3;
        if (0 == tm) return 4;
        std::memset(tm, 0x0, sizeof(struct tm));
        tm->tm_isdst = -1;

        long tmp;
        if (!Util::strtol(timeStr.substr(0,4).c_str(), tmp)) return 5;
        tm->tm_year = tmp - 1900;
        if (!Util::strtol(timeStr.substr(5,2).c_str(), tmp)) return 6;
        tm->tm_mon = tmp - 1;
        if (!Util::strtol(timeStr.substr(8,2).c_str(), tmp)) return 7;
        tm->tm_mday = tmp;
        if (!Util::strtol(timeStr.substr(11,2).c_str(), tmp)) return 8;
        tm->tm_hour = tmp;
        if (!Util::strtol(timeStr.substr(14,2).c_str(), tmp)) return 9;
        tm->tm_min = tmp;
        if (!Util::strtol(timeStr.substr(17,2).c_str(), tmp)) return 10;
        tm->tm_sec = tmp;

        // Conversions to set remaining fields of the tm structure
        if (mktime(tm) == (time_t)-1) return 11;

        return 0;
    } // str2Tm

    std::string time2Str(time_t time)
    {
        struct tm* tm = localtime(&time);
        return tm2Str(tm);
    } // time2Str

    std::string tm2Str(const struct tm* tm)
    {
        if (0 == tm) return "";

        std::ostringstream os;
        os << std::setfill('0')
           << tm->tm_year + 1900 << ":"
           << std::setw(2) << tm->tm_mon + 1 << ":"
           << std::setw(2) << tm->tm_mday << " "
           << std::setw(2) << tm->tm_hour << ":"
           << std::setw(2) << tm->tm_min << ":"
           << std::setw(2) << tm->tm_sec;

        return os.str();
    } // tm2Str

    int metacopy(const std::string& source,
                 const std::string& target,
                 int targetType,
                 bool preserve)
    {
        if (!Exiv2::fileExists(source, true)) {
            std::cerr << source
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr sourceImage = Exiv2::ImageFactory::open(source);
        assert(sourceImage.get() != 0);
        sourceImage->readMetadata();

        // Apply any modification commands to the source image on-the-fly
        Action::Modify::applyCommands(sourceImage.get());

        Exiv2::Image::AutoPtr targetImage;
        if (Exiv2::fileExists(target)) {
            targetImage = Exiv2::ImageFactory::open(target);
            assert(targetImage.get() != 0);
            if (preserve) targetImage->readMetadata();
        }
        else {
            targetImage = Exiv2::ImageFactory::create(targetType, target);
            assert(targetImage.get() != 0);
        }
        if (   Params::instance().target_ & Params::ctExif
            && !sourceImage->exifData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing Exif data from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            targetImage->setExifData(sourceImage->exifData());
        }
        if (   Params::instance().target_ & Params::ctIptc
            && !sourceImage->iptcData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing IPTC data from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            targetImage->setIptcData(sourceImage->iptcData());
        }
        if (   Params::instance().target_ & Params::ctXmp
            && !sourceImage->xmpData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing XMP data from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            // Todo: Should use XMP packet if there are no XMP modification commands
            targetImage->setXmpData(sourceImage->xmpData());
        }
        if (   Params::instance().target_ & Params::ctComment
            && !sourceImage->comment().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing JPEG comment from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            targetImage->setComment(sourceImage->comment());
        }
        try {
            targetImage->writeMetadata();
        }
        catch (const Exiv2::AnyError& e) {
            std::cerr << target <<
                ": " << _("Could not write metadata to file") << ": " << e << "\n";
            return 1;
        }

        return 0;
    } // metacopy

    // Defined outside of the function so that Exiv2::find() can see it
    struct String {
        const char* s_;
        bool operator==(const char* s) const {
            return 0 == strcmp(s_, s);
        }
    };

    int renameFile(std::string& newPath, const struct tm* tm)
    {
        std::string path = newPath;
        std::string format = Params::instance().format_;
        Util::replace(format, ":basename:",   Util::basename(path, true));
        Util::replace(format, ":dirname:",    Util::basename(Util::dirname(path)));
        Util::replace(format, ":parentname:", Util::basename(Util::dirname(Util::dirname(path))));

        const size_t max = 1024;
        char basename[max];
        std::memset(basename, 0x0, max);
        if (strftime(basename, max, format.c_str(), tm) == 0) {
            std::cerr << _("Filename format yields empty filename for the file") << " "
                      << path << "\n";
            return 1;
        }
        newPath =   Util::dirname(path) + EXV_SEPERATOR_STR
                  + basename + Util::suffix(path);
        if (   Util::dirname(newPath)  == Util::dirname(path)
            && Util::basename(newPath) == Util::basename(path)) {
            if (Params::instance().verbose_) {
                std::cout << _("This file already has the correct name") << std::endl;
            }
            return -1;
        }

        bool go = true;
        int seq = 1;
        std::string s;
        Params::FileExistsPolicy fileExistsPolicy
            = Params::instance().fileExistsPolicy_;
        while (go) {
            if (Exiv2::fileExists(newPath)) {
                switch (fileExistsPolicy) {
                case Params::overwritePolicy:
                    go = false;
                    break;
                case Params::renamePolicy:
                    newPath = Util::dirname(path)
                        + EXV_SEPERATOR_STR + basename
                        + "_" + Exiv2::toString(seq++)
                        + Util::suffix(path);
                    break;
                case Params::askPolicy:
                    std::cout << Params::instance().progname()
                              << ": " << _("File") << " `" << newPath
                              << "' " << _("exists. [O]verwrite, [r]ename or [s]kip?")
                              << " ";
                    std::cin >> s;
                    switch (s[0]) {
                    case 'o':
                    case 'O':
                        go = false;
                        break;
                    case 'r':
                    case 'R':
                        fileExistsPolicy = Params::renamePolicy;
                        newPath = Util::dirname(path)
                            + EXV_SEPERATOR_STR + basename
                            + "_" + Exiv2::toString(seq++)
                            + Util::suffix(path);
                        break;
                    default: // skip
                        return -1;
                        break;
                    }
                }
            }
            else {
                go = false;
            }
        }

        if (Params::instance().verbose_) {
            std::cout << _("Renaming file to") << " " << newPath;
            if (Params::instance().timestamp_) {
                std::cout << ", " << _("updating timestamp");
            }
            std::cout << std::endl;
        }

        // Workaround for MinGW rename which does not overwrite existing files
        remove(newPath.c_str());
        if (std::rename(path.c_str(), newPath.c_str()) == -1) {
            std::cerr << Params::instance().progname()
                      << ": " << _("Failed to rename") << " "
                      << path << " " << _("to") << " " << newPath << ": "
                      << Exiv2::strError() << "\n";
            return 1;
        }

        return 0;
    } // renameFile

    std::string newFilePath(const std::string& path, const std::string& ext)
    {
        std::string directory = Params::instance().directory_;
        if (directory.empty()) directory = Util::dirname(path);
        std::string newPath =   directory + EXV_SEPERATOR_STR
                              + Util::basename(path, true) + ext;
        return newPath;
    }

    int dontOverwrite(const std::string& path)
    {
        if (!Params::instance().force_ && Exiv2::fileExists(path)) {
            std::cout << Params::instance().progname()
                      << ": " << _("Overwrite") << " `" << path << "'? ";
            std::string s;
            std::cin >> s;
            if (s[0] != 'y' && s[0] != 'Y') return 1;
        }
        return 0;
    }

}
