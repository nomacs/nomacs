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
  @file    basicio.hpp
  @brief   Simple binary IO abstraction
  @version $Rev: 2681 $
  @author  Brad Schick (brad)
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    04-Dec-04, brad: created
 */
#ifndef BASICIO_HPP_
#define BASICIO_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <memory>                               // for std::auto_ptr

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief An interface for simple binary IO.

      Designed to have semantics and names similar to those of C style FILE*
      operations. Subclasses should all behave the same so that they can be
      interchanged.
     */
    class EXIV2API BasicIo {
    public:
        //! BasicIo auto_ptr type
        typedef std::auto_ptr<BasicIo> AutoPtr;

        //! Seek starting positions
        enum Position { beg, cur, end };

        //! @name Creators
        //@{
        //! Destructor
        virtual ~BasicIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Open the IO source using the default access mode. The
              default mode should allow for reading and writing.

          This method can also be used to "reopen" an IO source which will
          flush any unwritten data and reset the IO position to the start.
          Subclasses may provide custom methods to allow for
          opening IO sources differently.

          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int open() = 0;
        /*!
          @brief Close the IO source. After closing a BasicIo instance can not
              be read or written. Closing flushes any unwritten data. It is
              safe to call close on a closed instance.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int close() = 0;
        /*!
          @brief Write data to the IO source. Current IO position is advanced
              by the number of bytes written.
          @param data Pointer to data. Data must be at least \em wcount
              bytes long
          @param wcount Number of bytes to be written.
          @return Number of bytes written to IO source successfully;<BR>
              0 if failure;
         */
        virtual long write(const byte* data, long wcount) = 0;
        /*!
          @brief Write data that is read from another BasicIo instance to
              the IO source. Current IO position is advanced by the number
              of bytes written.
          @param src Reference to another BasicIo instance. Reading start
              at the source's current IO position
          @return Number of bytes written to IO source successfully;<BR>
              0 if failure;
         */
        virtual long write(BasicIo& src) = 0;
        /*!
          @brief Write one byte to the IO source. Current IO position is
              advanced by one byte.
          @param data The single byte to be written.
          @return The value of the byte written if successful;<BR>
              EOF if failure;
         */
        virtual int putb(byte data) = 0;
        /*!
          @brief Read data from the IO source. Reading starts at the current
              IO position and the position is advanced by the number of bytes
              read.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return DataBuf instance containing the bytes read. Use the
              DataBuf::size_ member to find the number of bytes read.
              DataBuf::size_ will be 0 on failure.
         */
        virtual DataBuf read(long rcount) = 0;
        /*!
          @brief Read data from the IO source. Reading starts at the current
              IO position and the position is advanced by the number of bytes
              read.
          @param buf Pointer to a block of memory into which the read data
              is stored. The memory block must be at least \em rcount bytes
              long.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return Number of bytes read from IO source successfully;<BR>
              0 if failure;
         */
        virtual long read(byte* buf, long rcount) = 0;
        /*!
          @brief Read one byte from the IO source. Current IO position is
              advanced by one byte.
          @return The byte read from the IO source if successful;<BR>
              EOF if failure;
         */
        virtual int getb() = 0;
        /*!
          @brief Remove all data from this object's IO source and then transfer
              data from the \em src BasicIo object into this object.

          The source object is invalidated by this operation and should not be
          used after this method returns. This method exists primarily to
          be used with the BasicIo::temporary() method.

          @param src Reference to another BasicIo instance. The entire contents
              of src are transferred to this object. The \em src object is
              invalidated by the method.
          @throw Error In case of failure
         */
        virtual void transfer(BasicIo& src) = 0;
        /*!
          @brief Move the current IO position.
          @param offset Number of bytes to move the position relative
              to the starting position specified by \em pos
          @param pos Position from which the seek should start
          @return 0 if successful;<BR>
              Nonzero if failure;
         */
        virtual int seek(long offset, Position pos) = 0;
        /*!
          @brief Direct access to the IO data. For files, this is done by
                 mapping the file into the process's address space; for memory
                 blocks, this allows direct access to the memory block.
          @param isWriteable Set to true if the mapped area should be writeable
                 (default is false).
          @return A pointer to the mapped area.
          @throw Error In case of failure.
         */
        virtual byte* mmap(bool isWriteable =false) =0;
        /*!
          @brief Remove a mapping established with mmap(). If the mapped area
                 is writeable, this ensures that changes are written back.
          @return 0 if successful;<BR>
                  Nonzero if failure;
         */
        virtual int munmap() =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Get the current IO position.
          @return Offset from the start of IO if successful;<BR>
                 -1 if failure;
         */
        virtual long tell() const = 0;
        /*!
          @brief Get the current size of the IO source in bytes.
          @return Size of the IO source in bytes;<BR>
                 -1 if failure;
         */
        virtual long size() const = 0;
        //!Returns true if the IO source is open, otherwise false.
        virtual bool isopen() const = 0;
        //!Returns 0 if the IO source is in a valid state, otherwise nonzero.
        virtual int error() const = 0;
        //!Returns true if the IO position has reach the end, otherwise false.
        virtual bool eof() const = 0;
        /*!
          @brief Return the path to the IO resource. Often used to form
              comprehensive error messages where only a BasicIo instance is
              available.
         */
        virtual std::string path() const =0;
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like path() but returns a unicode path in an std::wstring.
          @note This function is only available on Windows.
         */
        virtual std::wstring wpath() const =0;
#endif
        /*!
          @brief Returns a temporary data storage location. This is often
              needed to rewrite an IO source.

          For example, data may be read from the original IO source, modified
          in some way, and then saved to the temporary instance. After the
          operation is complete, the BasicIo::transfer method can be used to
          replace the original IO source with the modified version. Subclasses
          are free to return any class that derives from BasicIo.

          @return An instance of BasicIo on success
          @throw Error In case of failure
         */
        virtual BasicIo::AutoPtr temporary() const = 0;
        //@}

    protected:
        //! @name Creators
        //@{
        //! Default Constructor
        BasicIo() {}
        //@}
    }; // class BasicIo

    /*!
      @brief Utility class that closes a BasicIo instance upon destruction.
          Meant to be used as a stack variable in functions that need to
          ensure BasicIo instances get closed. Useful when functions return
          errors from many locations.
     */
    class EXIV2API IoCloser {
    public:
        //! @name Creators
        //@{
        //! Constructor, takes a BasicIo reference
        IoCloser(BasicIo& bio) : bio_(bio) {}
        //! Destructor, closes the BasicIo reference
        ~IoCloser() { close(); }
        //@}

        //! @name Manipulators
        //@{
        //! Close the BasicIo if it is open
        void close() { if (bio_.isopen()) bio_.close(); }
        //@}

        // DATA
        //! The BasicIo reference
        BasicIo& bio_;

    private:
        // Not implemented
        //! Copy constructor
        IoCloser(const IoCloser&);
        //! Assignment operator
        IoCloser& operator=(const IoCloser&);
    }; // class IoCloser

    /*!
      @brief Provides binary file IO by implementing the BasicIo
          interface.
     */
    class EXIV2API FileIo : public BasicIo {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that accepts the file path on which IO will be
              performed. The constructor does not open the file, and
              therefore never failes.
          @param path The full path of a file
         */
        FileIo(const std::string& path);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like FileIo(const std::string& path) but accepts a
              unicode path in an std::wstring.
          @note This constructor is only available on Windows.
         */
        FileIo(const std::wstring& wpath);
#endif
        //! Destructor. Flushes and closes an open file.
        virtual ~FileIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Open the file using using the specified mode.

          This method can also be used to "reopen" a file which will flush any
          unwritten data and reset the IO position to the start. Although
          files can be opened in binary or text mode, this class has
          only been tested carefully in binary mode.

          @param mode Specified that type of access allowed on the file.
              Valid values match those of the C fopen command exactly.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        int open(const std::string& mode);
        /*!
          @brief Open the file using using the default access mode of "rb".
              This method can also be used to "reopen" a file which will flush
              any unwritten data and reset the IO position to the start.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int open();
        /*!
          @brief Flush and unwritten data and close the file . It is
              safe to call close on an already closed instance.
          @return 0 if successful;<BR>
                 Nonzero if failure;
         */
        virtual int close();
        /*!
          @brief Write data to the file. The file position is advanced
              by the number of bytes written.
          @param data Pointer to data. Data must be at least \em wcount
              bytes long
          @param wcount Number of bytes to be written.
          @return Number of bytes written to the file successfully;<BR>
                 0 if failure;
         */
        virtual long write(const byte* data, long wcount);
        /*!
          @brief Write data that is read from another BasicIo instance to
              the file. The file position is advanced by the number
              of bytes written.
          @param src Reference to another BasicIo instance. Reading start
              at the source's current IO position
          @return Number of bytes written to the file successfully;<BR>
                 0 if failure;
         */
        virtual long write(BasicIo& src);
        /*!
          @brief Write one byte to the file. The file position is
              advanced by one byte.
          @param data The single byte to be written.
          @return The value of the byte written if successful;<BR>
                 EOF if failure;
         */
        virtual int putb(byte data);
        /*!
          @brief Read data from the file. Reading starts at the current
              file position and the position is advanced by the number of
              bytes read.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return DataBuf instance containing the bytes read. Use the
                DataBuf::size_ member to find the number of bytes read.
                DataBuf::size_ will be 0 on failure.
         */
        virtual DataBuf read(long rcount);
        /*!
          @brief Read data from the file. Reading starts at the current
              file position and the position is advanced by the number of
              bytes read.
          @param buf Pointer to a block of memory into which the read data
              is stored. The memory block must be at least \em rcount bytes
              long.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return Number of bytes read from the file successfully;<BR>
                 0 if failure;
         */
        virtual long read(byte* buf, long rcount);
        /*!
          @brief Read one byte from the file. The file position is
              advanced by one byte.
          @return The byte read from the file if successful;<BR>
                 EOF if failure;
         */
        virtual int getb();
        /*!
          @brief Remove the contents of the file and then transfer data from
              the \em src BasicIo object into the empty file.

          This method is optimized to simply rename the source file if the
          source object is another FileIo instance. The source BasicIo object
          is invalidated by this operation and should not be used after this
          method returns. This method exists primarily to be used with
          the BasicIo::temporary() method.

          @note If the caller doesn't have permissions to write to the file,
              an exception is raised and \em src is deleted.

          @param src Reference to another BasicIo instance. The entire contents
              of src are transferred to this object. The \em src object is
              invalidated by the method.
          @throw Error In case of failure
         */
        virtual void transfer(BasicIo& src);
        /*!
          @brief Move the current file position.
          @param offset Number of bytes to move the file position
              relative to the starting position specified by \em pos
          @param pos Position from which the seek should start
          @return 0 if successful;<BR>
                 Nonzero if failure;
         */
        virtual int seek(long offset, Position pos);
        /*!
          @brief Map the file into the process's address space. The file must be
                 open before mmap() is called. If the mapped area is writeable,
                 changes may not be written back to the underlying file until
                 munmap() is called. The pointer is valid only as long as the
                 FileIo object exists.
          @param isWriteable Set to true if the mapped area should be writeable
                 (default is false).
          @return A pointer to the mapped area.
          @throw Error In case of failure.
         */
        virtual byte* mmap(bool isWriteable =false);
        /*!
          @brief Remove a mapping established with mmap(). If the mapped area is
                 writeable, this ensures that changes are written back to the
                 underlying file.
          @return 0 if successful;<BR>
                  Nonzero if failure;
         */
        virtual int munmap();
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Get the current file position.
          @return Offset from the start of the file if successful;<BR>
                 -1 if failure;
         */
        virtual long tell() const;
        /*!
          @brief Flush any buffered writes and get the current file size
              in bytes.
          @return Size of the file in bytes;<BR>
                 -1 if failure;
         */
        virtual long size() const;
        //! Returns true if the file is open, otherwise false.
        virtual bool isopen() const;
        //! Returns 0 if the file is in a valid state, otherwise nonzero.
        virtual int error() const;
        //! Returns true if the file position has reach the end, otherwise false.
        virtual bool eof() const;
        //! Returns the path of the file
        virtual std::string path() const;
#ifdef EXV_UNICODE_PATH
        /*
          @brief Like path() but returns the unicode path of the file in an std::wstring.
          @note This function is only available on Windows.
         */
        virtual std::wstring wpath() const;
#endif
        /*!
          @brief Returns a temporary data storage location. The actual type
              returned depends upon the size of the file represented a FileIo
              object. For small files, a MemIo is returned while for large files
              a FileIo is returned. Callers should not rely on this behavior,
              however, since it may change.
          @return An instance of BasicIo on success
          @throw Error If opening the temporary file fails
         */
        virtual BasicIo::AutoPtr temporary() const;
        //@}

    private:
        // NOT IMPLEMENTED
        //! Copy constructor
        FileIo(FileIo& rhs);
        //! Assignment operator
        FileIo& operator=(const FileIo& rhs);

        // Pimpl idiom
        class Impl;
        Impl* p_;

    }; // class FileIo

    /*!
      @brief Provides binary IO on blocks of memory by implementing the BasicIo
          interface. A copy-on-write implementation ensures that the data passed
          in is only copied when necessary, i.e., as soon as data is written to
          the MemIo. The original data is only used for reading. If writes are
          performed, the changed data can be retrieved using the read methods
          (since the data used in construction is never modified).

      @note If read only usage of this class is common, it might be worth
          creating a specialized readonly class or changing this one to
          have a readonly mode.
     */
    class EXIV2API MemIo : public BasicIo {
    public:
        //! @name Creators
        //@{
        //! Default constructor that results in an empty object
        MemIo();
        /*!
          @brief Constructor that accepts a block of memory. A copy-on-write
              algorithm allows read operations directly from the original data
              and will create a copy of the buffer on the first write operation.
          @param data Pointer to data. Data must be at least \em size
              bytes long
          @param size Number of bytes to copy.
         */
        MemIo(const byte* data, long size);
        //! Destructor. Releases all managed memory
        ~MemIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Memory IO is always open for reading and writing. This method
                 therefore only resets the IO position to the start.

          @return 0
         */
        virtual int open();
        /*!
          @brief Does nothing on MemIo objects.
          @return 0
         */
        virtual int close();
        /*!
          @brief Write data to the memory block. If needed, the size of the
              internal memory block is expanded. The IO position is advanced
              by the number of bytes written.
          @param data Pointer to data. Data must be at least \em wcount
              bytes long
          @param wcount Number of bytes to be written.
          @return Number of bytes written to the memory block successfully;<BR>
                 0 if failure;
         */
        virtual long write(const byte* data, long wcount);
        /*!
          @brief Write data that is read from another BasicIo instance to
              the memory block. If needed, the size of the internal memory
              block is expanded. The IO position is advanced by the number
              of bytes written.
          @param src Reference to another BasicIo instance. Reading start
              at the source's current IO position
          @return Number of bytes written to the memory block successfully;<BR>
                 0 if failure;
         */
        virtual long write(BasicIo& src);
        /*!
          @brief Write one byte to the memory block. The IO position is
              advanced by one byte.
          @param data The single byte to be written.
          @return The value of the byte written if successful;<BR>
                 EOF if failure;
         */
        virtual int putb(byte data);
        /*!
          @brief Read data from the memory block. Reading starts at the current
              IO position and the position is advanced by the number of
              bytes read.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return DataBuf instance containing the bytes read. Use the
                DataBuf::size_ member to find the number of bytes read.
                DataBuf::size_ will be 0 on failure.
         */
        virtual DataBuf read(long rcount);
        /*!
          @brief Read data from the memory block. Reading starts at the current
              IO position and the position is advanced by the number of
              bytes read.
          @param buf Pointer to a block of memory into which the read data
              is stored. The memory block must be at least \em rcount bytes
              long.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return Number of bytes read from the memory block successfully;<BR>
                 0 if failure;
         */
        virtual long read(byte* buf, long rcount);
        /*!
          @brief Read one byte from the memory block. The IO position is
              advanced by one byte.
          @return The byte read from the memory block if successful;<BR>
                 EOF if failure;
         */
        virtual int getb();
        /*!
          @brief Clear the memory block and then transfer data from
              the \em src BasicIo object into a new block of memory.

          This method is optimized to simply swap memory block if the source
          object is another MemIo instance. The source BasicIo instance
          is invalidated by this operation and should not be used after this
          method returns. This method exists primarily to be used with
          the BasicIo::temporary() method.

          @param src Reference to another BasicIo instance. The entire contents
              of src are transferred to this object. The \em src object is
              invalidated by the method.
          @throw Error In case of failure
         */
        virtual void transfer(BasicIo& src);
        /*!
          @brief Move the current IO position.
          @param offset Number of bytes to move the IO position
              relative to the starting position specified by \em pos
          @param pos Position from which the seek should start
          @return 0 if successful;<BR>
                 Nonzero if failure;
         */
        virtual int seek(long offset, Position pos);
        /*!
          @brief Allow direct access to the underlying data buffer. The buffer
                 is not protected against write access in any way, the argument
                 is ignored.
          @note  The application must ensure that the memory pointed to by the
                 returned pointer remains valid and allocated as long as the
                 MemIo object exists.
         */
        virtual byte* mmap(bool /*isWriteable*/ =false);
        virtual int munmap();
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Get the current IO position.
          @return Offset from the start of the memory block
         */
        virtual long tell() const;
        /*!
          @brief Get the current memory buffer size in bytes.
          @return Size of the in memory data in bytes;<BR>
                 -1 if failure;
         */
        virtual long size() const;
        //!Always returns true
        virtual bool isopen() const;
        //!Always returns 0
        virtual int error() const;
        //!Returns true if the IO position has reach the end, otherwise false.
        virtual bool eof() const;
        //! Returns a dummy path, indicating that memory access is used
        virtual std::string path() const;
#ifdef EXV_UNICODE_PATH
        /*
          @brief Like path() but returns a unicode dummy path in an std::wstring.
          @note This function is only available on Windows.
         */
        virtual std::wstring wpath() const;
#endif
        /*!
          @brief Returns a temporary data storage location. Currently returns
              an empty MemIo object, but callers should not rely on this
              behavior since it may change.
          @return An instance of BasicIo
         */
        virtual BasicIo::AutoPtr temporary() const;
        //@}

    private:
        // NOT IMPLEMENTED
        //! Copy constructor
        MemIo(MemIo& rhs);
        //! Assignment operator
        MemIo& operator=(const MemIo& rhs);

        // Pimpl idiom
        class Impl;
        Impl* p_;

    }; // class MemIo

// *****************************************************************************
// template, inline and free functions

    /*!
      @brief Read file \em path into a DataBuf, which is returned.
      @return Buffer containing the file.
      @throw Error In case of failure.
     */
    EXIV2API DataBuf readFile(const std::string& path);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like readFile() but accepts a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API DataBuf readFile(const std::wstring& wpath);
#endif
    /*!
      @brief Write DataBuf \em buf to file \em path.
      @return Return the number of bytes written.
      @throw Error In case of failure.
     */
    EXIV2API long writeFile(const DataBuf& buf, const std::string& path);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like writeFile() but accepts a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API long writeFile(const DataBuf& buf, const std::wstring& wpath);

#endif

}                                       // namespace Exiv2

#endif                                  // #ifndef BASICIO_HPP_
