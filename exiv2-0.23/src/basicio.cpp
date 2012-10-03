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
  File:      basicio.cpp
  Version:   $Rev: 2689 $
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   04-Dec-04, brad: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: basicio.cpp 2689 2012-03-24 13:00:00Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "basicio.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <memory>
#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>                       // for remove, rename
#include <cstdlib>                      // for alloc, realloc, free
#include <sys/types.h>                  // for stat, chmod
#include <sys/stat.h>                   // for stat, chmod
#ifdef EXV_HAVE_SYS_MMAN_H
# include <sys/mman.h>                  // for mmap and munmap
#endif
#ifdef EXV_HAVE_PROCESS_H
# include <process.h>
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                    // for getpid, stat
#endif

#if defined WIN32 && !defined __CYGWIN__
// Windows doesn't provide mode_t, nlink_t
typedef unsigned short mode_t;
typedef short nlink_t;

# include <windows.h>
# include <io.h>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    BasicIo::~BasicIo()
    {
    }

    //! Internal Pimpl structure of class FileIo.
    class FileIo::Impl {
    public:
        //! Constructor
        Impl(const std::string& path);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        Impl(const std::wstring& wpath);
#endif
        // Enumerations
        //! Mode of operation
        enum OpMode { opRead, opWrite, opSeek };
#ifdef EXV_UNICODE_PATH
        //! Used to indicate if the path is stored as a standard or unicode string
        enum WpMode { wpStandard, wpUnicode };
#endif
        // DATA
        std::string path_;              //!< (Standard) path
#ifdef EXV_UNICODE_PATH
        std::wstring wpath_;            //!< Unicode path
        WpMode wpMode_;                 //!< Indicates which path is in use
#endif
        std::string openMode_;          //!< File open mode
        FILE *fp_;                      //!< File stream pointer
        OpMode opMode_;                 //!< File open mode

#if defined WIN32 && !defined __CYGWIN__
        HANDLE hFile_;                  //!< Duplicated fd
        HANDLE hMap_;                   //!< Handle from CreateFileMapping
#endif
        byte* pMappedArea_;             //!< Pointer to the memory-mapped area
        size_t mappedLength_;           //!< Size of the memory-mapped area
        bool isMalloced_;               //!< Is the mapped area allocated?
        bool isWriteable_;              //!< Can the mapped area be written to?
        // TYPES
        //! Simple struct stat wrapper for internal use
        struct StructStat {
            StructStat() : st_mode(0), st_size(0), st_nlink(0) {}
            mode_t  st_mode;            //!< Permissions
            off_t   st_size;            //!< Size
            nlink_t st_nlink;           //!< Number of hard links (broken on Windows, see winNumberOfLinks())
        };

        // METHODS
        /*!
          @brief Switch to a new access mode, reopening the file if needed.
              Optimized to only reopen the file when it is really necessary.
          @param opMode The mode to switch to.
          @return 0 if successful
         */
        int switchMode(OpMode opMode);
        //! stat wrapper for internal use
        int stat(StructStat& buf) const;
#if defined WIN32 && !defined __CYGWIN__
        // Windows function to determine the number of hardlinks (on NTFS)
        DWORD winNumberOfLinks() const;
#endif

    private:
        // NOT IMPLEMENTED
        Impl(const Impl& rhs);                         //!< Copy constructor
        Impl& operator=(const Impl& rhs);              //!< Assignment

    }; // class FileIo::Impl

    FileIo::Impl::Impl(const std::string& path)
        : path_(path),
#ifdef EXV_UNICODE_PATH
          wpMode_(wpStandard),
#endif
          fp_(0), opMode_(opSeek),
#if defined WIN32 && !defined __CYGWIN__
          hFile_(0), hMap_(0),
#endif
          pMappedArea_(0), mappedLength_(0), isMalloced_(false), isWriteable_(false)
    {
    }

#ifdef EXV_UNICODE_PATH
    FileIo::Impl::Impl(const std::wstring& wpath)
        : wpath_(wpath),
          wpMode_(wpUnicode),
          fp_(0), opMode_(opSeek),
#if defined WIN32 && !defined __CYGWIN__
          hFile_(0), hMap_(0),
#endif
          pMappedArea_(0), mappedLength_(0), isMalloced_(false), isWriteable_(false)
    {
    }

#endif
    int FileIo::Impl::switchMode(OpMode opMode)
    {
        assert(fp_ != 0);
        if (opMode_ == opMode) return 0;
        OpMode oldOpMode = opMode_;
        opMode_ = opMode;

        bool reopen = true;
        switch(opMode) {
        case opRead:
            // Flush if current mode allows reading, else reopen (in mode "r+b"
            // as in this case we know that we can write to the file)
            if (openMode_[0] == 'r' || openMode_[1] == '+') reopen = false;
            break;
        case opWrite:
            // Flush if current mode allows writing, else reopen
            if (openMode_[0] != 'r' || openMode_[1] == '+') reopen = false;
            break;
        case opSeek:
            reopen = false;
            break;
        }

        if (!reopen) {
            // Don't do anything when switching _from_ opSeek mode; we
            // flush when switching _to_ opSeek.
            if (oldOpMode == opSeek) return 0;

            // Flush. On msvcrt fflush does not do the job
            std::fseek(fp_, 0, SEEK_CUR);
            return 0;
        }

        // Reopen the file
        long offset = std::ftell(fp_);
        if (offset == -1) return -1;
        // 'Manual' open("r+b") to avoid munmap()
        if (fp_ != 0) {
            std::fclose(fp_);
            fp_= 0;
        }
        openMode_ = "r+b";
        opMode_ = opSeek;
#ifdef EXV_UNICODE_PATH
        if (wpMode_ == wpUnicode) {
            fp_ = ::_wfopen(wpath_.c_str(), s2ws(openMode_).c_str());
        }
        else
#endif
        {
            fp_ = std::fopen(path_.c_str(), openMode_.c_str());
        }
        if (!fp_) return 1;
        return std::fseek(fp_, offset, SEEK_SET);
    } // FileIo::Impl::switchMode

    int FileIo::Impl::stat(StructStat& buf) const
    {
        int ret = 0;
#ifdef EXV_UNICODE_PATH
        if (wpMode_ == wpUnicode) {
            struct _stat st;
            ret = ::_wstat(wpath_.c_str(), &st);
            if (0 == ret) {
                buf.st_size = st.st_size;
                buf.st_mode = st.st_mode;
                buf.st_nlink = st.st_nlink;
            }
        }
        else
#endif
        {
            struct stat st;
            ret = ::stat(path_.c_str(), &st);
            if (0 == ret) {
                buf.st_size = st.st_size;
                buf.st_mode = st.st_mode;
                buf.st_nlink = st.st_nlink;
            }
        }
        return ret;
    } // FileIo::Impl::stat

#if defined WIN32 && !defined __CYGWIN__
    DWORD FileIo::Impl::winNumberOfLinks() const
    {
        DWORD nlink = 1;

        HANDLE hFd = (HANDLE)_get_osfhandle(fileno(fp_));
        if (hFd != INVALID_HANDLE_VALUE) {
            typedef BOOL (WINAPI * GetFileInformationByHandle_t)(HANDLE, LPBY_HANDLE_FILE_INFORMATION);
            HMODULE hKernel = LoadLibraryA("kernel32.dll");
            if (hKernel) {
                GetFileInformationByHandle_t pfcn_GetFileInformationByHandle = (GetFileInformationByHandle_t)GetProcAddress(hKernel, "GetFileInformationByHandle");
                if (pfcn_GetFileInformationByHandle) {
                    BY_HANDLE_FILE_INFORMATION fi = {0};
                    if (pfcn_GetFileInformationByHandle(hFd, &fi)) {
                        nlink = fi.nNumberOfLinks;
                    }
#ifdef DEBUG
                    else EXV_DEBUG << "GetFileInformationByHandle failed\n";
#endif
                }
#ifdef DEBUG
                else EXV_DEBUG << "GetProcAddress(hKernel, \"GetFileInformationByHandle\") failed\n";
#endif
                FreeLibrary(hKernel);
            }
#ifdef DEBUG
            else EXV_DEBUG << "LoadLibraryA(\"kernel32.dll\") failed\n";
#endif
        }
#ifdef DEBUG
        else EXV_DEBUG << "_get_osfhandle failed: INVALID_HANDLE_VALUE\n";
#endif

        return nlink;
    } // FileIo::Impl::winNumberOfLinks

#endif // defined WIN32 && !defined __CYGWIN__
    FileIo::FileIo(const std::string& path)
        : p_(new Impl(path))
    {
    }

#ifdef EXV_UNICODE_PATH
    FileIo::FileIo(const std::wstring& wpath)
        : p_(new Impl(wpath))
    {
    }

#endif
    FileIo::~FileIo()
    {
        close();
        delete p_;
    }

    int FileIo::munmap()
    {
        int rc = 0;
        if (p_->pMappedArea_ != 0) {
#if defined EXV_HAVE_MMAP && defined EXV_HAVE_MUNMAP
            if (::munmap(p_->pMappedArea_, p_->mappedLength_) != 0) {
                rc = 1;
            }
#elif defined WIN32 && !defined __CYGWIN__
            UnmapViewOfFile(p_->pMappedArea_);
            CloseHandle(p_->hMap_);
            p_->hMap_ = 0;
            CloseHandle(p_->hFile_);
            p_->hFile_ = 0;
#else
            if (p_->isWriteable_) {
                seek(0, BasicIo::beg);
                write(p_->pMappedArea_, p_->mappedLength_);
            }
            if (p_->isMalloced_) {
                delete[] p_->pMappedArea_;
                p_->isMalloced_ = false;
            }
#endif
        }
        if (p_->isWriteable_) {
            if (p_->fp_ != 0) p_->switchMode(Impl::opRead);
            p_->isWriteable_ = false;
        }
        p_->pMappedArea_ = 0;
        p_->mappedLength_ = 0;
        return rc;
    }

    byte* FileIo::mmap(bool isWriteable)
    {
        assert(p_->fp_ != 0);
        if (munmap() != 0) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), strError().c_str(), "munmap");
            }
            else
#endif
            {
                throw Error(2, path(), strError(), "munmap");
            }
        }
        p_->mappedLength_ = size();
        p_->isWriteable_ = isWriteable;
        if (p_->isWriteable_ && p_->switchMode(Impl::opWrite) != 0) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(16, wpath(), strError().c_str());
            }
            else
#endif
            {
                throw Error(16, path(), strError());
            }
        }
#if defined EXV_HAVE_MMAP && defined EXV_HAVE_MUNMAP
        int prot = PROT_READ;
        if (p_->isWriteable_) {
            prot |= PROT_WRITE;
        }
        void* rc = ::mmap(0, p_->mappedLength_, prot, MAP_SHARED, fileno(p_->fp_), 0);
        if (MAP_FAILED == rc) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), strError().c_str(), "mmap");
            }
            else
#endif
            {
                throw Error(2, path(), strError(), "mmap");
            }
        }
        p_->pMappedArea_ = static_cast<byte*>(rc);

#elif defined WIN32 && !defined __CYGWIN__
        // Windows implementation

        // TODO: An attempt to map a file with a length of 0 (zero) fails with
        // an error code of ERROR_FILE_INVALID.
        // Applications should test for files with a length of 0 (zero) and
        // reject those files.

        DWORD dwAccess = FILE_MAP_READ;
        DWORD flProtect = PAGE_READONLY;
        if (isWriteable) {
            dwAccess = FILE_MAP_WRITE;
            flProtect = PAGE_READWRITE;
        }
        HANDLE hPh = GetCurrentProcess();
        HANDLE hFd = (HANDLE)_get_osfhandle(fileno(p_->fp_));
        if (hFd == INVALID_HANDLE_VALUE) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), "MSG1", "_get_osfhandle");
            }
            else
#endif
            {
                throw Error(2, path(), "MSG1", "_get_osfhandle");
            }
        }
        if (!DuplicateHandle(hPh, hFd, hPh, &p_->hFile_, 0, false, DUPLICATE_SAME_ACCESS)) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), "MSG2", "DuplicateHandle");
            }
            else
#endif
            {
                throw Error(2, path(), "MSG2", "DuplicateHandle");
            }
        }
        p_->hMap_ = CreateFileMapping(p_->hFile_, 0, flProtect, 0, (DWORD) p_->mappedLength_, 0);
        if (p_->hMap_ == 0 ) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), "MSG3", "CreateFileMapping");
            }
            else
#endif
            {
                throw Error(2, path(), "MSG3", "CreateFileMapping");
            }
        }
        void* rc = MapViewOfFile(p_->hMap_, dwAccess, 0, 0, 0);
        if (rc == 0) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), "MSG4", "CreateFileMapping");
            }
            else
#endif
            {
                throw Error(2, path(), "MSG4", "CreateFileMapping");
            }
        }
        p_->pMappedArea_ = static_cast<byte*>(rc);
#else
        // Workaround for platforms without mmap: Read the file into memory
        DataBuf buf(static_cast<long>(p_->mappedLength_));
        if (read(buf.pData_, buf.size_) != buf.size_) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), strError().c_str(), "FileIo::read");
            }
            else
#endif
            {
                throw Error(2, path(), strError(), "FileIo::read");
            }
        }
        if (error() || eof()) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(2, wpath(), strError().c_str(), "FileIo::mmap");
            }
            else
#endif
            {
                throw Error(2, path(), strError(), "FileIo::mmap");
            }
        }
        p_->pMappedArea_ = buf.release().first;
        p_->isMalloced_ = true;
#endif
        return p_->pMappedArea_;
    }

    BasicIo::AutoPtr FileIo::temporary() const
    {
        BasicIo::AutoPtr basicIo;

        Impl::StructStat buf;
        int ret = p_->stat(buf);
#if defined WIN32 && !defined __CYGWIN__
        DWORD nlink = p_->winNumberOfLinks();
#else 
        nlink_t nlink = buf.st_nlink;
#endif

        // If file is > 1MB and doesn't have hard links then use a file, otherwise
        // use a memory buffer. I.e., files with hard links always use a memory
        // buffer, which is a workaround to ensure that the links don't get broken.
        if (ret != 0 || (buf.st_size > 1048576 && nlink == 1)) {
            pid_t pid = ::getpid();
            std::auto_ptr<FileIo> fileIo;
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                std::wstring tmpname = wpath() + s2ws(toString(pid));
                fileIo = std::auto_ptr<FileIo>(new FileIo(tmpname));
            }
            else
#endif
            {
                std::string tmpname = path() + toString(pid);
                fileIo = std::auto_ptr<FileIo>(new FileIo(tmpname));
            }
            if (fileIo->open("w+b") != 0) {
#ifdef EXV_UNICODE_PATH
                if (p_->wpMode_ == Impl::wpUnicode) {
                    throw WError(10, wpath(), "w+b", strError().c_str());
                }
                else
#endif
                {
                    throw Error(10, path(), "w+b", strError());
                }
            }
            basicIo = fileIo;
        }
        else {
            basicIo.reset(new MemIo);
        }

        return basicIo;
    }

    long FileIo::write(const byte* data, long wcount)
    {
        assert(p_->fp_ != 0);
        if (p_->switchMode(Impl::opWrite) != 0) return 0;
        return (long)std::fwrite(data, 1, wcount, p_->fp_);
    }

    long FileIo::write(BasicIo& src)
    {
        assert(p_->fp_ != 0);
        if (static_cast<BasicIo*>(this) == &src) return 0;
        if (!src.isopen()) return 0;
        if (p_->switchMode(Impl::opWrite) != 0) return 0;

        byte buf[4096];
        long readCount = 0;
        long writeCount = 0;
        long writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            writeTotal += writeCount = (long)std::fwrite(buf, 1, readCount, p_->fp_);
            if (writeCount != readCount) {
                // try to reset back to where write stopped
                src.seek(writeCount-readCount, BasicIo::cur);
                break;
            }
        }

        return writeTotal;
    }

    void FileIo::transfer(BasicIo& src)
    {
        const bool wasOpen = (p_->fp_ != 0);
        const std::string lastMode(p_->openMode_);

        FileIo *fileIo = dynamic_cast<FileIo*>(&src);
        if (fileIo) {
            // Optimization if src is another instance of FileIo
            fileIo->close();
            // Check if the file can be written to, if it already exists
            if (open("a+b") != 0) {
                // Remove the (temporary) file
#ifdef EXV_UNICODE_PATH
                if (fileIo->p_->wpMode_ == Impl::wpUnicode) {
                    ::_wremove(fileIo->wpath().c_str());
                }
                else
#endif
                {
                    ::remove(fileIo->path().c_str());
                }
#ifdef EXV_UNICODE_PATH
                if (p_->wpMode_ == Impl::wpUnicode) {
                    throw WError(10, wpath(), "a+b", strError().c_str());
                }
                else
#endif
                {
                    throw Error(10, path(), "a+b", strError());
                }
            }
            close();

            bool statOk = true;
            mode_t origStMode = 0;
            std::string spf;
            char* pf = 0;
#ifdef EXV_UNICODE_PATH
            std::wstring wspf;
            wchar_t* wpf = 0;
            if (p_->wpMode_ == Impl::wpUnicode) {
                wspf = wpath();
                wpf = const_cast<wchar_t*>(wspf.c_str());
            }
            else
#endif
            {
                spf = path();
                pf = const_cast<char*>(spf.c_str());
            }

            // Get the permissions of the file, or linked-to file, on platforms which have lstat
#ifdef EXV_HAVE_LSTAT

# ifdef EXV_UNICODE_PATH
#  error EXV_UNICODE_PATH and EXV_HAVE_LSTAT are not compatible. Stop.
# endif
            struct stat buf1;
            if (::lstat(pf, &buf1) == -1) {
                statOk = false;
#ifndef SUPPRESS_WARNINGS
                EXV_WARNING << Error(2, pf, strError(), "::lstat") << "\n";
#endif
            }
            origStMode = buf1.st_mode;
            DataBuf lbuf; // So that the allocated memory is freed. Must have same scope as pf
            // In case path() is a symlink, get the path of the linked-to file
            if (statOk && S_ISLNK(buf1.st_mode)) {
                lbuf.alloc(buf1.st_size + 1);
                memset(lbuf.pData_, 0x0, lbuf.size_);
                pf = reinterpret_cast<char*>(lbuf.pData_);
                if (::readlink(path().c_str(), pf, lbuf.size_ - 1) == -1) {
                    throw Error(2, path(), strError(), "readlink");
                }
                // We need the permissions of the file, not the symlink
                if (::stat(pf, &buf1) == -1) {
                    statOk = false;
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << Error(2, pf, strError(), "::stat") << "\n";
#endif
                }
                origStMode = buf1.st_mode;
            }
#else // EXV_HAVE_LSTAT
            Impl::StructStat buf1;
            if (p_->stat(buf1) == -1) {
                statOk = false;
            }
            origStMode = buf1.st_mode;
#endif // !EXV_HAVE_LSTAT

            // MSVCRT rename that does not overwrite existing files
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                if (fileExists(wpf) && ::_wremove(wpf) != 0) {
                    throw WError(2, wpf, strError().c_str(), "::_wremove");
                }
                if (::_wrename(fileIo->wpath().c_str(), wpf) == -1) {
                    throw WError(17, fileIo->wpath(), wpf, strError().c_str());
                }
                ::_wremove(fileIo->wpath().c_str());
                // Check permissions of new file
                struct _stat buf2;
                if (statOk && ::_wstat(wpf, &buf2) == -1) {
                    statOk = false;
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << Error(2, wpf, strError(), "::_wstat") << "\n";
#endif
                }
                if (statOk && origStMode != buf2.st_mode) {
                    // Set original file permissions
                    if (::_wchmod(wpf, origStMode) == -1) {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << Error(2, wpf, strError(), "::_wchmod") << "\n";
#endif
                    }
                }
            } // if (p_->wpMode_ == Impl::wpUnicode)
            else
#endif // EXV_UNICODE_PATH
            {
                if (fileExists(pf) && ::remove(pf) != 0) {
                    throw Error(2, pf, strError(), "::remove");
                }
                if (::rename(fileIo->path().c_str(), pf) == -1) {
                    throw Error(17, fileIo->path(), pf, strError());
                }
                ::remove(fileIo->path().c_str());
                // Check permissions of new file
                struct stat buf2;
                if (statOk && ::stat(pf, &buf2) == -1) {
                    statOk = false;
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << Error(2, pf, strError(), "::stat") << "\n";
#endif
                }
                if (statOk && origStMode != buf2.st_mode) {
                    // Set original file permissions
                    if (::chmod(pf, origStMode) == -1) {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << Error(2, pf, strError(), "::chmod") << "\n";
#endif
                    }
                }
            }
        } // if (fileIo)
        else {
            // Generic handling, reopen both to reset to start
            if (open("w+b") != 0) {
#ifdef EXV_UNICODE_PATH
                if (p_->wpMode_ == Impl::wpUnicode) {
                    throw WError(10, wpath(), "w+b", strError().c_str());
                }
                else
#endif
                {
                    throw Error(10, path(), "w+b", strError());
                }
            }
            if (src.open() != 0) {
#ifdef EXV_UNICODE_PATH
                if (p_->wpMode_ == Impl::wpUnicode) {
                    throw WError(9, src.wpath(), strError().c_str());
                }
                else
#endif
                {
                    throw Error(9, src.path(), strError());
                }
            }
            write(src);
            src.close();
        }

        if (wasOpen) {
            if (open(lastMode) != 0) {
#ifdef EXV_UNICODE_PATH
                if (p_->wpMode_ == Impl::wpUnicode) {
                    throw WError(10, wpath(), lastMode.c_str(), strError().c_str());
                }
                else
#endif
                {
                    throw Error(10, path(), lastMode, strError());
                }
            }
        }
        else close();

        if (error() || src.error()) {
#ifdef EXV_UNICODE_PATH
            if (p_->wpMode_ == Impl::wpUnicode) {
                throw WError(18, wpath(), strError().c_str());
            }
            else
#endif
            {
                throw Error(18, path(), strError());
            }
        }
    } // FileIo::transfer

    int FileIo::putb(byte data)
    {
        assert(p_->fp_ != 0);
        if (p_->switchMode(Impl::opWrite) != 0) return EOF;
        return putc(data, p_->fp_);
    }

    int FileIo::seek(long offset, Position pos)
    {
        assert(p_->fp_ != 0);

        int fileSeek = 0;
        switch (pos) {
        case BasicIo::cur: fileSeek = SEEK_CUR; break;
        case BasicIo::beg: fileSeek = SEEK_SET; break;
        case BasicIo::end: fileSeek = SEEK_END; break;
        }

        if (p_->switchMode(Impl::opSeek) != 0) return 1;
        return std::fseek(p_->fp_, offset, fileSeek);
    }

    long FileIo::tell() const
    {
        assert(p_->fp_ != 0);
        return std::ftell(p_->fp_);
    }

    long FileIo::size() const
    {
        // Flush and commit only if the file is open for writing
        if (p_->fp_ != 0 && (p_->openMode_[0] != 'r' || p_->openMode_[1] == '+')) {
            std::fflush(p_->fp_);
#if defined WIN32 && !defined __CYGWIN__
            // This is required on msvcrt before stat after writing to a file
            _commit(_fileno(p_->fp_));
#endif
        }

        Impl::StructStat buf;
        int ret = p_->stat(buf);

        if (ret != 0) return -1;
        return buf.st_size;
    }

    int FileIo::open()
    {
        // Default open is in read-only binary mode
        return open("rb");
    }

    int FileIo::open(const std::string& mode)
    {
        close();
        p_->openMode_ = mode;
        p_->opMode_ = Impl::opSeek;
#ifdef EXV_UNICODE_PATH
        if (p_->wpMode_ == Impl::wpUnicode) {
            p_->fp_ = ::_wfopen(wpath().c_str(), s2ws(mode).c_str());
        }
        else
#endif
        {
            p_->fp_ = ::fopen(path().c_str(), mode.c_str());
        }
        if (!p_->fp_) return 1;
        return 0;
    }

    bool FileIo::isopen() const
    {
        return p_->fp_ != 0;
    }

    int FileIo::close()
    {
        int rc = 0;
        if (munmap() != 0) rc = 2;
        if (p_->fp_ != 0) {
            if (std::fclose(p_->fp_) != 0) rc |= 1;
            p_->fp_= 0;
        }
        return rc;
    }

    DataBuf FileIo::read(long rcount)
    {
        assert(p_->fp_ != 0);
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long FileIo::read(byte* buf, long rcount)
    {
        assert(p_->fp_ != 0);
        if (p_->switchMode(Impl::opRead) != 0) return 0;
        return (long)std::fread(buf, 1, rcount, p_->fp_);
    }

    int FileIo::getb()
    {
        assert(p_->fp_ != 0);
        if (p_->switchMode(Impl::opRead) != 0) return EOF;
        return getc(p_->fp_);
    }

    int FileIo::error() const
    {
        return p_->fp_ != 0 ? ferror(p_->fp_) : 0;
    }

    bool FileIo::eof() const
    {
        assert(p_->fp_ != 0);
        return feof(p_->fp_) != 0;
    }

    std::string FileIo::path() const
    {
#ifdef EXV_UNICODE_PATH
        if (p_->wpMode_ == Impl::wpUnicode) {
            return ws2s(p_->wpath_);
        }
#endif
        return p_->path_;
    }

#ifdef EXV_UNICODE_PATH
    std::wstring FileIo::wpath() const
    {
        if (p_->wpMode_ == Impl::wpStandard) {
            return s2ws(p_->path_);
        }
        return p_->wpath_;
    }

#endif
    //! Internal Pimpl structure of class MemIo.
    class MemIo::Impl {
    public:
        Impl();                            //!< Default constructor
        Impl(const byte* data, long size); //!< Constructor 2

        // DATA
        byte* data_;                       //!< Pointer to the start of the memory area
        long idx_;                         //!< Index into the memory area
        long size_;                        //!< Size of the memory area
        long sizeAlloced_;                 //!< Size of the allocated buffer
        bool isMalloced_;                  //!< Was the buffer allocated?
        bool eof_;                         //!< EOF indicator

        // METHODS
        void reserve(long wcount);         //!< Reserve memory

    private:
        // NOT IMPLEMENTED
        Impl(const Impl& rhs);             //!< Copy constructor
        Impl& operator=(const Impl& rhs);  //!< Assignment

    }; // class MemIo::Impl

    MemIo::Impl::Impl()
        : data_(0),
          idx_(0),
          size_(0),
          sizeAlloced_(0),
          isMalloced_(false),
          eof_(false)
    {
    }

    MemIo::Impl::Impl(const byte* data, long size)
        : data_(const_cast<byte*>(data)),
          idx_(0),
          size_(size),
          sizeAlloced_(0),
          isMalloced_(false),
          eof_(false)
    {
    }

    void MemIo::Impl::reserve(long wcount)
    {
        long need = wcount + idx_;

        if (!isMalloced_) {
            // Minimum size for 1st block is 32kB
            long size  = EXV_MAX(32768 * (1 + need / 32768), size_);
            byte* data = (byte*)std::malloc(size);
            std::memcpy(data, data_, size_);
            data_ = data;
            sizeAlloced_ = size;
            isMalloced_ = true;
        }

        if (need > size_) {
            if (need > sizeAlloced_) {
                // Allocate in blocks of 32kB
                long want = 32768 * (1 + need / 32768);
                data_ = (byte*)std::realloc(data_, want);
                sizeAlloced_ = want;
                isMalloced_ = true;
            }
            size_ = need;
        }
    }

    MemIo::MemIo()
        : p_(new Impl())
    {
    }

    MemIo::MemIo(const byte* data, long size)
        : p_(new Impl(data, size))
    {
    }

    MemIo::~MemIo()
    {
        if (p_->isMalloced_) {
            std::free(p_->data_);
        }
        delete p_;
    }

    BasicIo::AutoPtr MemIo::temporary() const
    {
        return BasicIo::AutoPtr(new MemIo);
    }

    long MemIo::write(const byte* data, long wcount)
    {
        p_->reserve(wcount);
        assert(p_->isMalloced_);
        std::memcpy(&p_->data_[p_->idx_], data, wcount);
        p_->idx_ += wcount;
        return wcount;
    }

    void MemIo::transfer(BasicIo& src)
    {
        MemIo *memIo = dynamic_cast<MemIo*>(&src);
        if (memIo) {
            // Optimization if src is another instance of MemIo
            if (true == p_->isMalloced_) {
                std::free(p_->data_);
            }
            p_->idx_ = 0;
            p_->data_ = memIo->p_->data_;
            p_->size_ = memIo->p_->size_;
            p_->isMalloced_ = memIo->p_->isMalloced_;
            memIo->p_->idx_ = 0;
            memIo->p_->data_ = 0;
            memIo->p_->size_ = 0;
            memIo->p_->isMalloced_ = false;
        }
        else {
            // Generic reopen to reset position to start
            if (src.open() != 0) {
                throw Error(9, src.path(), strError());
            }
            p_->idx_ = 0;
            write(src);
            src.close();
        }
        if (error() || src.error()) throw Error(19, strError());
    }

    long MemIo::write(BasicIo& src)
    {
        if (static_cast<BasicIo*>(this) == &src) return 0;
        if (!src.isopen()) return 0;

        byte buf[4096];
        long readCount = 0;
        long writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            write(buf, readCount);
            writeTotal += readCount;
        }

        return writeTotal;
    }

    int MemIo::putb(byte data)
    {
        p_->reserve(1);
        assert(p_->isMalloced_);
        p_->data_[p_->idx_++] = data;
        return data;
    }

    int MemIo::seek(long offset, Position pos)
    {
        long newIdx = 0;

        switch (pos) {
        case BasicIo::cur: newIdx = p_->idx_ + offset; break;
        case BasicIo::beg: newIdx = offset; break;
        case BasicIo::end: newIdx = p_->size_ + offset; break;
        }

        if (newIdx < 0 || newIdx > p_->size_) return 1;
        p_->idx_ = newIdx;
        p_->eof_ = false;
        return 0;
    }

    byte* MemIo::mmap(bool /*isWriteable*/)
    {
        return p_->data_;
    }

    int MemIo::munmap()
    {
        return 0;
    }

    long MemIo::tell() const
    {
        return p_->idx_;
    }

    long MemIo::size() const
    {
        return p_->size_;
    }

    int MemIo::open()
    {
        p_->idx_ = 0;
        p_->eof_ = false;
        return 0;
    }

    bool MemIo::isopen() const
    {
        return true;
    }

    int MemIo::close()
    {
        return 0;
    }

    DataBuf MemIo::read(long rcount)
    {
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long MemIo::read(byte* buf, long rcount)
    {
        long avail = p_->size_ - p_->idx_;
        long allow = EXV_MIN(rcount, avail);
        std::memcpy(buf, &p_->data_[p_->idx_], allow);
        p_->idx_ += allow;
        if (rcount > avail) p_->eof_ = true;
        return allow;
    }

    int MemIo::getb()
    {
        if (p_->idx_ == p_->size_) {
            p_->eof_ = true;
            return EOF;
        }
        return p_->data_[p_->idx_++];
    }

    int MemIo::error() const
    {
        return 0;
    }

    bool MemIo::eof() const
    {
        return p_->eof_;
    }

    std::string MemIo::path() const
    {
        return "MemIo";
    }

#ifdef EXV_UNICODE_PATH
    std::wstring MemIo::wpath() const
    {
        return EXV_WIDEN("MemIo");
    }

#endif
    // *************************************************************************
    // free functions

    DataBuf readFile(const std::string& path)
    {
        FileIo file(path);
        if (file.open("rb") != 0) {
            throw Error(10, path, "rb", strError());
        }
        struct stat st;
        if (0 != ::stat(path.c_str(), &st)) {
            throw Error(2, path, strError(), "::stat");
        }
        DataBuf buf(st.st_size);
        long len = file.read(buf.pData_, buf.size_);
        if (len != buf.size_) {
            throw Error(2, path, strError(), "FileIo::read");
        }
        return buf;
    }

#ifdef EXV_UNICODE_PATH
    DataBuf readFile(const std::wstring& wpath)
    {
        FileIo file(wpath);
        if (file.open("rb") != 0) {
            throw WError(10, wpath, "rb", strError().c_str());
        }
        struct _stat st;
        if (0 != ::_wstat(wpath.c_str(), &st)) {
            throw WError(2, wpath, strError().c_str(), "::_wstat");
        }
        DataBuf buf(st.st_size);
        long len = file.read(buf.pData_, buf.size_);
        if (len != buf.size_) {
            throw WError(2, wpath, strError().c_str(), "FileIo::read");
        }
        return buf;
    }

#endif
    long writeFile(const DataBuf& buf, const std::string& path)
    {
        FileIo file(path);
        if (file.open("wb") != 0) {
            throw Error(10, path, "wb", strError());
        }
        return file.write(buf.pData_, buf.size_);
    }

#ifdef EXV_UNICODE_PATH
    long writeFile(const DataBuf& buf, const std::wstring& wpath)
    {
        FileIo file(wpath);
        if (file.open("wb") != 0) {
            throw WError(10, wpath, "wb", strError().c_str());
        }
        return file.write(buf.pData_, buf.size_);
    }

#endif
}                                       // namespace Exiv2
