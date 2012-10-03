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
  @file    actions.hpp
  @brief   Implements base class Task, TaskFactory and the various supported
           actions (derived from Task).
  @version $Rev: 2681 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Dec-03, ahu: created
 */
#ifndef ACTIONS_HPP_
#define ACTIONS_HPP_

// *****************************************************************************
// included header files

// + standard includes
#include <string>
#include <map>

#include "exiv2app.hpp"
#include "image.hpp"
#include "exif.hpp"
#include "iptc.hpp"

// *****************************************************************************
// class declarations

namespace Exiv2 {
    class ExifData;
    class Image;
    class Metadatum;
    class PreviewImage;
}

// *****************************************************************************
// namespace extensions
/*!
  @brief Contains all action classes (task subclasses).
 */
namespace Action {

    //! Enumerates all tasks
    enum TaskType { none, adjust, print, rename, erase, extract, insert,
                    modify, fixiso, fixcom };

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class for all concrete actions.

      Task provides a simple interface that actions must implement and a few
      commonly used helpers.
     */
    class Task {
    public:
        //! Shortcut for an auto pointer.
        typedef std::auto_ptr<Task> AutoPtr;
        //! Virtual destructor.
        virtual ~Task();
        //! Virtual copy construction.
        AutoPtr clone() const;
        /*!
          @brief Application interface to perform a task.

          @param path Path of the file to process.
          @return 0 if successful.
         */
        virtual int run(const std::string& path) =0;

    private:
        //! Internal virtual copy constructor.
        virtual Task* clone_() const =0;

    }; // class Task

    /*!
      @brief Task factory.

      Creates an instance of the task of the requested type.  The factory is
      implemented as a singleton, which can be accessed only through the static
      member function instance().
    */
    class TaskFactory {
    public:
        /*!
          @brief Get access to the task factory.

          Clients access the task factory exclusively through
          this method.
        */
        static TaskFactory& instance();
        //! Destructor
        void cleanup();

        /*!
          @brief  Create a task.

          @param  type Identifies the type of task to create.
          @return An auto pointer that owns a task of the requested type.  If
                  the task type is not supported, the pointer is 0.
          @remark The caller of the function should check the content of the
                  returned auto pointer and take appropriate action (e.g., throw
                  an exception) if it is 0.
         */
        Task::AutoPtr create(TaskType type);

        /*!
          @brief Register a task prototype together with its type.

          The task factory creates new tasks of a given type by cloning its
          associated prototype. Additional tasks can be registered.  If called
          for a type which already exists in the list, the corresponding
          prototype is replaced.

          @param type Task type.
          @param task Pointer to the prototype. Ownership is transfered to the
                 task factory. That's what the auto pointer indicates.
        */
        void registerTask(TaskType type, Task::AutoPtr task);

    private:
        //! Prevent construction other than through instance().
        TaskFactory();
        //! Prevent copy construction: not implemented.
        TaskFactory(const TaskFactory& rhs);

        //! Pointer to the one and only instance of this class.
        static TaskFactory* instance_;
        //! Type used to store Task prototype classes
        typedef std::map<TaskType, Task*> Registry;
        //! List of task types and corresponding prototypes.
        Registry registry_;

    }; // class TaskFactory

    //! %Print the Exif (or other metadata) of a file to stdout
    class Print : public Task {
    public:
        virtual ~Print();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Print> AutoPtr;
        AutoPtr clone() const;

        //! Print the Jpeg comment
        int printComment();
        //! Print list of available preview images
        int printPreviewList();
        //! Print Exif summary information
        int printSummary();
        //! Print Exif, IPTC and XMP metadata in user defined format
        int printList();
        //! Return true if key should be printed, else false
        bool grepTag(const std::string& key);
        //! Print all metadata in a user defined format
        int printMetadata(const Exiv2::Image* image);
        //! Print a metadatum in a user defined format
        void printMetadatum(const Exiv2::Metadatum& md, const Exiv2::Image* image);
        //! Print the label for a summary line
        void printLabel(const std::string& label) const;
        /*!
          @brief Print one summary line with a label (if provided) and requested
                 data. A line break is printed only if a label is provided.
          @return 1 if a line was written, 0 if the key was not found.
         */
        int printTag(const Exiv2::ExifData& exifData,
                     const std::string& key,
                     const std::string& label ="") const;
        //! Type for an Exiv2 Easy access function
        typedef Exiv2::ExifData::const_iterator (*EasyAccessFct)(const Exiv2::ExifData& ed);
        /*!
          @brief Print one summary line with a label (if provided) and requested
                 data. A line break is printed only if a label is provided.
          @return 1 if a line was written, 0 if the information was not found.
         */
        int printTag(const Exiv2::ExifData& exifData,
                     EasyAccessFct easyAccessFct,
                     const std::string& label) const;

    private:
        virtual Print* clone_() const;

        std::string path_;
        int align_;                // for the alignment of the summary output
    }; // class Print

    /*!
      @brief %Rename a file to its metadate creation timestamp,
             in the specified format.
     */
    class Rename : public Task {
    public:
        virtual ~Rename();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Rename> AutoPtr;
        AutoPtr clone() const;

    private:
        virtual Rename* clone_() const;
    }; // class Rename

    //! %Adjust the Exif (or other metadata) timestamps
    class Adjust : public Task {
    public:
        virtual ~Adjust();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Adjust> AutoPtr;
        AutoPtr clone() const;

    private:
        virtual Adjust* clone_() const;
        int adjustDateTime(Exiv2::ExifData& exifData,
                           const std::string& key,
                           const std::string& path) const;

        long adjustment_;
        long yearAdjustment_;
        long monthAdjustment_;
        long dayAdjustment_;

    }; // class Adjust

    /*!
      @brief %Erase the entire exif data or only the thumbnail section.
     */
    class Erase : public Task {
    public:
        virtual ~Erase();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Erase> AutoPtr;
        AutoPtr clone() const;

        /*!
          @brief Delete the thumbnail image, incl IFD1 metadata from the file.
         */
        int eraseThumbnail(Exiv2::Image* image) const;
        /*!
          @brief Erase the complete Exif data block from the file.
         */
        int eraseExifData(Exiv2::Image* image) const;
        /*!
          @brief Erase all Iptc data from the file.
         */
        int eraseIptcData(Exiv2::Image* image) const;
        /*!
          @brief Erase Jpeg comment from the file.
         */
        int eraseComment(Exiv2::Image* image) const;
        /*!
          @brief Erase XMP packet from the file.
         */
        int eraseXmpData(Exiv2::Image* image) const;

    private:
        virtual Erase* clone_() const;
        std::string path_;

    }; // class Erase

    /*!
      @brief %Extract the entire exif data or only the thumbnail section.
     */
    class Extract : public Task {
    public:
        virtual ~Extract();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Extract> AutoPtr;
        AutoPtr clone() const;

        /*!
          @brief Write the thumbnail image to a file. The filename is composed by
                 removing the suffix from the image filename and appending
                 "-thumb" and the appropriate suffix (".jpg" or ".tif"), depending
                 on the format of the Exif thumbnail image.
         */
        int writeThumbnail() const;
        /*!
          @brief Write preview images to files.
         */
        int writePreviews() const;
        /*!
          @brief Write one preview image to a file. The filename is composed by
                 removing the suffix from the image filename and appending
                 "-preview<num>" and the appropriate suffix (".jpg" or ".tif"),
                 depending on the format of the Exif thumbnail image.
         */
        void writePreviewFile(const Exiv2::PreviewImage& pvImg, int num) const;

    private:
        virtual Extract* clone_() const;
        std::string path_;

    }; // class Extract

    /*!
      @brief %Insert the Exif data from corresponding *.exv files.
     */
    class Insert : public Task {
    public:
        virtual ~Insert();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Insert> AutoPtr;
        AutoPtr clone() const;

        /*!
          @brief Insert a Jpeg thumbnail image from a file into file \em path.
                 The filename of the thumbnail is expected to be the image
                 filename (\em path) minus its suffix plus "-thumb.jpg".
         */
        int insertThumbnail(const std::string& path) const;
        /*!
          @brief Insert an XMP packet from a file into file \em path.
                 The filename of the XMP packet is expected to be the image
                 filename (\em path) minus its suffix plus ".xmp".
         */
        int insertXmpPacket(const std::string& path) const;

    private:
        virtual Insert* clone_() const;

    }; // class Insert

    /*!
      @brief %Modify the Exif data according to the commands in the
             modification table.
     */
    class Modify : public Task {
    public:
        virtual ~Modify();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Modify> AutoPtr;
        AutoPtr clone() const;
        Modify() {}
        //! Apply modification commands to the \em pImage, return 0 if successful.
        static int applyCommands(Exiv2::Image* pImage);

    private:
        virtual Modify* clone_() const;
        //! Copy contructor needed because of AutoPtr member
        Modify(const Modify& /*src*/) : Task() {}

        //! Add a metadatum to \em pImage according to \em modifyCmd
        static int addMetadatum(Exiv2::Image* pImage,
                                const ModifyCmd& modifyCmd);
        //! Set a metadatum in \em pImage according to \em modifyCmd
        static int setMetadatum(Exiv2::Image* pImage,
                                const ModifyCmd& modifyCmd);
        //! Delete a metadatum from \em pImage according to \em modifyCmd
        static void delMetadatum(Exiv2::Image* pImage,
                                 const ModifyCmd& modifyCmd);
        //! Register an XMP namespace according to \em modifyCmd
        static void regNamespace(const ModifyCmd& modifyCmd);

    }; // class Modify

    /*!
      @brief %Copy ISO settings from any of the Nikon makernotes to the
             regular Exif tag, Exif.Photo.ISOSpeedRatings.
     */
    class FixIso : public Task {
    public:
        virtual ~FixIso();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<FixIso> AutoPtr;
        AutoPtr clone() const;

    private:
        virtual FixIso* clone_() const;
        std::string path_;

    }; // class FixIso

    /*!
      @brief Fix the character encoding of Exif UNICODE user comments.
             Decodes the comment using the auto-detected or specified
             character encoding and writes it back in UCS-2.
     */
    class FixCom : public Task {
    public:
        virtual ~FixCom();
        virtual int run(const std::string& path);
        typedef std::auto_ptr<FixCom> AutoPtr;
        AutoPtr clone() const;

    private:
        virtual FixCom* clone_() const;
        std::string path_;

    }; // class FixCom

}                                       // namespace Action

#endif                                  // #ifndef ACTIONS_HPP_
