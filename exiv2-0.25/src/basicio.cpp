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
  File:      basicio.cpp
  Version:   $Rev: 3868 $
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   04-Dec-04, brad: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: basicio.cpp 3868 2015-06-21 14:21:21Z ahuggel $")

// included header files
#include "config.h"

#include "basicio.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "error.hpp"
#include "http.hpp"

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
#if EXV_USE_CURL == 1
#include <curl/curl.h>
#endif
#if EXV_USE_SSH == 1
#include "ssh.hpp"
#else
#define mode_t unsigned short
#endif

// Platform specific headers for handling extended attributes (xattr)
#if defined(__APPLE__)
# include <sys/xattr.h>
#endif

#if defined WIN32 && !defined __CYGWIN__
// Windows doesn't provide nlink_t
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
        byte*  pMappedArea_;            //!< Pointer to the memory-mapped area
        size_t mappedLength_;           //!< Size of the memory-mapped area
        bool   isMalloced_;             //!< Is the mapped area allocated?
        bool   isWriteable_;            //!< Can the mapped area be written to?
        // TYPES
        //! Simple struct stat wrapper for internal use
        struct StructStat {
            StructStat() : st_mode(0), st_size(0), st_nlink(0) {}
            mode_t  st_mode;            //!< Permissions
            off_t   st_size;            //!< Size
            nlink_t st_nlink;           //!< Number of hard links (broken on Windows, see winNumberOfLinks())
        };
// #endif
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
        //! copy extended attributes (xattr) from another file
        void copyXattrFrom(const FileIo& src);
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
                buf.st_nlink = st.st_nlink;
                buf.st_mode = st.st_mode;
            }
        }
        return ret;
    } // FileIo::Impl::stat

#if defined(__APPLE__)
    void FileIo::Impl::copyXattrFrom(const FileIo& src)
#else
    void FileIo::Impl::copyXattrFrom(const FileIo&)
#endif
    {
#if defined(__APPLE__)
# if defined(EXV_UNICODE_PATH)
#  error No xattr API for MacOS X with unicode support
# endif
        const ssize_t namebufSize = ::listxattr(src.p_->path_.c_str(), 0, 0, 0);
        if (namebufSize < 0) {
            throw Error(2, src.p_->path_, strError(), "listxattr");
        }
        if (namebufSize == 0) {
            // No extended attributes in source file
            return;
        }
        char namebuf[namebufSize];
        if (::listxattr(src.p_->path_.c_str(), namebuf, sizeof(namebuf), 0) != namebufSize) {
            throw Error(2, src.p_->path_, strError(), "listxattr");
        }
        for (ssize_t namebufPos = 0; namebufPos < namebufSize;) {
            const char *name = namebuf + namebufPos;
            namebufPos += strlen(name) + 1;
            const ssize_t valueSize = ::getxattr(src.p_->path_.c_str(), name, 0, 0, 0, 0);
            if (valueSize < 0) {
                throw Error(2, src.p_->path_, strError(), "getxattr");
            }
            char value[valueSize];
            if (::getxattr(src.p_->path_.c_str(), name, value, sizeof(value), 0, 0) != valueSize) {
                throw Error(2, src.p_->path_, strError(), "getxattr");
            }
// #906.  Mountain Lion 'sandbox' terminates the app when we call setxattr
#ifndef __APPLE__
#ifdef  DEBUG
            EXV_DEBUG << "Copying xattr \"" << name << "\" with value size " << valueSize << "\n";
#endif
            if (::setxattr(path_.c_str(), name, value, valueSize, 0, 0) != 0) {
                throw Error(2, path_, strError(), "setxattr");
            }
#endif
        }
#else
        // No xattr support for this platform.
#endif
    } // FileIo::Impl::copyXattrFrom

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

    void FileIo::setPath(const std::string& path) {
        close();
#ifdef EXV_UNICODE_PATH
        if (p_->wpMode_ == Impl::wpUnicode) {
            std::wstring wpath;
            wpath.assign(path.begin(), path.end());
            p_->wpath_ = wpath;
        }
        p_->path_ = path;
#else
        p_->path_ = path;
#endif
    }

#ifdef EXV_UNICODE_PATH
    void FileIo::setPath(const std::wstring& wpath) {
        close();
        if (p_->wpMode_ == Impl::wpStandard) {
            std::string path;
            path.assign(wpath.begin(), wpath.end());
            p_->path_ = path;
        } else {
            p_->wpath_ = wpath;
        }
    }
#endif

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
            fileIo->p_->copyXattrFrom(*this);
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
#if defined(WIN32) && defined(REPLACEFILE_IGNORE_MERGE_ERRORS)
                // Windows implementation that deals with the fact that ::rename fails
                // if the target filename still exists, which regularly happens when
                // that file has been opened with FILE_SHARE_DELETE by another process,
                // like a virus scanner or disk indexer
                // (see also http://stackoverflow.com/a/11023068)
                typedef BOOL (WINAPI * ReplaceFileW_t)(LPCWSTR, LPCWSTR, LPCWSTR, DWORD, LPVOID, LPVOID);
                HMODULE hKernel = LoadLibraryA("kernel32.dll");
                if (hKernel) {
                    ReplaceFileW_t pfcn_ReplaceFileW = (ReplaceFileW_t)GetProcAddress(hKernel, "ReplaceFileW");
                    if (pfcn_ReplaceFileW) {
                        BOOL ret = pfcn_ReplaceFileW(wpf, fileIo->wpath().c_str(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL);
                        FreeLibrary(hKernel);
                        if (ret == 0) {
                            if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                                if (::_wrename(fileIo->wpath().c_str(), wpf) == -1) {
                                    throw WError(17, fileIo->wpath(), wpf, strError().c_str());
                                }
                                ::_wremove(fileIo->wpath().c_str());
                            }
                            else {
                                throw WError(17, fileIo->wpath(), wpf, strError().c_str());
                            }
                        }
                    }
                    else {
                        FreeLibrary(hKernel);
                        if (fileExists(wpf) && ::_wremove(wpf) != 0) {
                            throw WError(2, wpf, strError().c_str(), "::_wremove");
                        }
                        if (::_wrename(fileIo->wpath().c_str(), wpf) == -1) {
                            throw WError(17, fileIo->wpath(), wpf, strError().c_str());
                        }
                        ::_wremove(fileIo->wpath().c_str());
                    }
                }
#else
                if (fileExists(wpf) && ::_wremove(wpf) != 0) {
                    throw WError(2, wpf, strError().c_str(), "::_wremove");
                }
                if (::_wrename(fileIo->wpath().c_str(), wpf) == -1) {
                    throw WError(17, fileIo->wpath(), wpf, strError().c_str());
                }
                ::_wremove(fileIo->wpath().c_str());
#endif
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
#if defined(WIN32) && defined(REPLACEFILE_IGNORE_MERGE_ERRORS)
                // Windows implementation that deals with the fact that ::rename fails
                // if the target filename still exists, which regularly happens when
                // that file has been opened with FILE_SHARE_DELETE by another process,
                // like a virus scanner or disk indexer
                // (see also http://stackoverflow.com/a/11023068)
                typedef BOOL (WINAPI * ReplaceFileA_t)(LPCSTR, LPCSTR, LPCSTR, DWORD, LPVOID, LPVOID);
                HMODULE hKernel = LoadLibraryA("kernel32.dll");
                if (hKernel) {
                    ReplaceFileA_t pfcn_ReplaceFileA = (ReplaceFileA_t)GetProcAddress(hKernel, "ReplaceFileA");
                    if (pfcn_ReplaceFileA) {
                        BOOL ret = pfcn_ReplaceFileA(pf, fileIo->path().c_str(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL);
                        FreeLibrary(hKernel);
                        if (ret == 0) {
                            if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                                if (::rename(fileIo->path().c_str(), pf) == -1) {
                                    throw Error(17, fileIo->path(), pf, strError());
                                }
                                ::remove(fileIo->path().c_str());
                            }
                            else {
                                throw Error(17, fileIo->path(), pf, strError());
                            }
                        }
                    }
                    else {
                        FreeLibrary(hKernel);
                        if (fileExists(pf) && ::remove(pf) != 0) {
                            throw Error(2, pf, strError(), "::remove");
                        }
                        if (::rename(fileIo->path().c_str(), pf) == -1) {
                            throw Error(17, fileIo->path(), pf, strError());
                        }
                        ::remove(fileIo->path().c_str());
                    }
                }
#else
                if (fileExists(pf) && ::remove(pf) != 0) {
                    throw Error(2, pf, strError(), "::remove");
                }
                if (::rename(fileIo->path().c_str(), pf) == -1) {
                    throw Error(17, fileIo->path(), pf, strError());
                }
                ::remove(fileIo->path().c_str());
#endif
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

#if defined(_MSC_VER)
    int FileIo::seek( int64_t offset, Position pos )
    {
        assert(p_->fp_ != 0);

        int fileSeek = 0;
        switch (pos) {
        case BasicIo::cur: fileSeek = SEEK_CUR; break;
        case BasicIo::beg: fileSeek = SEEK_SET; break;
        case BasicIo::end: fileSeek = SEEK_END; break;
        }

        if (p_->switchMode(Impl::opSeek) != 0) return 1;
#ifdef _WIN64
        return _fseeki64(p_->fp_, offset, fileSeek);
#else
        return std::fseek(p_->fp_,static_cast<long>(offset), fileSeek);
#endif
    }
#else
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
#endif

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

    void FileIo::populateFakeData() {

    }

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
            msync();
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
            if (p_->isMalloced_) {
				msync();
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

#if defined(_MSC_VER)
    int MemIo::seek( int64_t offset, Position pos )
    {
        uint64_t newIdx = 0;

        switch (pos) {
        case BasicIo::cur: newIdx = p_->idx_ + offset; break;
        case BasicIo::beg: newIdx = offset; break;
        case BasicIo::end: newIdx = p_->size_ + offset; break;
        }

        p_->idx_ = static_cast<long>(newIdx);   //not very sure about this. need more test!!    - note by Shawn  fly2xj@gmail.com //TODO
        p_->eof_ = false;
        return 0;
    }
#else
    int MemIo::seek(long offset, Position pos)
    {
        long newIdx = 0;

        switch (pos) {
        case BasicIo::cur: newIdx = p_->idx_ + offset; break;
        case BasicIo::beg: newIdx = offset; break;
        case BasicIo::end: newIdx = p_->size_ + offset; break;
        }

        if (newIdx < 0) return 1;
        p_->idx_ = newIdx;
        p_->eof_ = false;
        return 0;
    }
#endif

    byte* MemIo::mmap(bool /*isWriteable*/)
    {
        return p_->data_;
    }

    int MemIo::munmap()
    {
        return msync();
    }

    int MemIo::msync()
    {
#ifdef MS_SYNC
        return ::msync(p_, p_->size_, MS_SYNC);
#else
        return 0;
#endif
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
        long avail = EXV_MAX(p_->size_ - p_->idx_, 0);
        long allow = EXV_MIN(rcount, avail);
        std::memcpy(buf, &p_->data_[p_->idx_], allow);
        p_->idx_ += allow;
        if (rcount > avail) p_->eof_ = true;
        return allow;
    }

    int MemIo::getb()
    {
        if (p_->idx_ >= p_->size_) {
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
    void MemIo::populateFakeData() {

    }

#if EXV_XPATH_MEMIO
    XPathIo::XPathIo(const std::string& path) {
        Protocol prot = fileProtocol(path);

        if (prot == pStdin)         ReadStdin();
        else if (prot == pDataUri)  ReadDataUri(path);
    }
#ifdef EXV_UNICODE_PATH
    XPathIo::XPathIo(const std::wstring& wpath) {
        std::string path;
        path.assign(wpath.begin(), wpath.end());
        Protocol prot = fileProtocol(path);
        if (prot == pStdin)         ReadStdin();
        else if (prot == pDataUri)  ReadDataUri(path);
    }
#endif

    void XPathIo::ReadStdin() {
        if (isatty(fileno(stdin)))
            throw Error(53);

#ifdef _O_BINARY
        // convert stdin to binary
        if (_setmode(_fileno(stdin), _O_BINARY) == -1)
            throw Error(54);
#endif

        char readBuf[100*1024];
        std::streamsize readBufSize = 0;
        do {
            std::cin.read(readBuf, sizeof(readBuf));
            readBufSize = std::cin.gcount();
            if (readBufSize > 0) {
                write((byte*)readBuf, (long)readBufSize);
            }
        } while(readBufSize);
    }

    void XPathIo::ReadDataUri(const std::string& path) {
        size_t base64Pos = path.find("base64,");
        if (base64Pos == std::string::npos)
            throw Error(1, "No base64 data");

        std::string data = path.substr(base64Pos+7);
        char* decodeData = new char[data.length()];
        long size = base64decode(data.c_str(), decodeData, data.length());
        if (size > 0)
            write((byte*)decodeData, size);
        else
            throw Error(1, "Unable to decode base 64.");
        delete[] decodeData;
    }

#else
    const std::string XPathIo::TEMP_FILE_EXT = ".exiv2_temp";
    const std::string XPathIo::GEN_FILE_EXT  = ".exiv2";

    XPathIo::XPathIo(const std::string& orgPath) : FileIo(XPathIo::writeDataToFile(orgPath)) {
        isTemp_ = true;
        tempFilePath_ = path();
    }

#ifdef EXV_UNICODE_PATH
    XPathIo::XPathIo(const std::wstring& wOrgPathpath) : FileIo(XPathIo::writeDataToFile(wOrgPathpath)) {
        isTemp_ = true;
        tempFilePath_ = path();
    }
#endif

    XPathIo::~XPathIo() {
        if (isTemp_ && remove(tempFilePath_.c_str()) != 0) {
            // error when removing file
            // printf ("Warning: Unable to remove the temp file %s.\n", tempFilePath_.c_str());
        }
    }

    void XPathIo::transfer(BasicIo& src) {
        if (isTemp_) {
            // replace temp path to gent path.
            std::string currentPath = path();
            setPath(ReplaceStringInPlace(currentPath, XPathIo::TEMP_FILE_EXT, XPathIo::GEN_FILE_EXT));
            // rename the file
            tempFilePath_ = path();
            if (rename(currentPath.c_str(), tempFilePath_.c_str()) != 0) {
                // printf("Warning: Failed to rename the temp file. \n");
            }
            isTemp_ = false;
            // call super class method
            FileIo::transfer(src);
        }
    }

    std::string XPathIo::writeDataToFile(const std::string& orgPath) {
        Protocol prot = fileProtocol(orgPath);

        // generating the name for temp file.
        std::time_t timestamp = std::time(NULL);
        std::stringstream ss;
        ss << timestamp << XPathIo::TEMP_FILE_EXT;
        std::string path = ss.str();
        std::ofstream fs(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

        if (prot == pStdin) {
            if (isatty(fileno(stdin)))
                throw Error(53);
#ifdef _MSC_VER
            // convert stdin to binary
            if (_setmode(_fileno(stdin), _O_BINARY) == -1)
                throw Error(54);
#endif
            // read stdin and write to the temp file.
            char readBuf[100*1024];
            std::streamsize readBufSize = 0;
            do {
                std::cin.read(readBuf, sizeof(readBuf));
                readBufSize = std::cin.gcount();
                if (readBufSize > 0) {
                    fs.write (readBuf, readBufSize);
                }
            } while(readBufSize);
        } else if (prot == pDataUri) {
            // read data uri and write to the temp file.
            size_t base64Pos = orgPath.find("base64,");
            if (base64Pos == std::string::npos)
                throw Error(1, "No base64 data");

            std::string data = orgPath.substr(base64Pos+7);
            char* decodeData = new char[data.length()];
            long size = base64decode(data.c_str(), decodeData, data.length());
            if (size > 0)
                fs.write(decodeData, size);
            else
                throw Error(1, "Unable to decode base 64.");
            delete[] decodeData;
        }

        fs.close();
        return path;
    }

#ifdef EXV_UNICODE_PATH
    std::string XPathIo::writeDataToFile(const std::wstring& wOrgPath) {
        std::string orgPath;
        orgPath.assign(wOrgPath.begin(), wOrgPath.end());
        return XPathIo::writeDataToFile(orgPath);
    }
#endif

#endif

    //! Internal Pimpl abstract structure of class RemoteIo.
    class RemoteIo::Impl {
    public:
        //! Constructor
        Impl(const std::string& path, size_t blockSize);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        Impl(const std::wstring& wpath, size_t blockSize);
#endif
        //! Destructor. Releases all managed memory.
        virtual ~Impl();

        // DATA
        std::string     path_;          //!< (Standard) path
#ifdef EXV_UNICODE_PATH
        std::wstring    wpath_;         //!< Unicode path
#endif
        size_t          blockSize_;     //!< Size of the block memory.
        BlockMap*       blocksMap_;     //!< An array contains all blocksMap
        size_t          size_;          //!< The file size
        long            idx_;           //!< Index into the memory area
        bool            isMalloced_;    //!< Was the blocksMap_ allocated?
        bool            eof_;           //!< EOF indicator
        Protocol        protocol_;      //!< the protocol of url

        // METHODS
        /*!
          @brief Get the length (in bytes) of the remote file.
          @return Return -1 if the size is unknown. Otherwise it returns the length of remote file (in bytes).
          @throw Error if the server returns the error code.
         */
        virtual long getFileLength() = 0;
        /*!
          @brief Get the data by range.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @param response The data from the server.
          @throw Error if the server returns the error code.
          @note Set lowBlock = -1 and highBlock = -1 to get the whole file content.
         */
        virtual void getDataByRange(long lowBlock, long highBlock, std::string& response) = 0;
        /*!
          @brief Submit the data to the remote machine. The data replace a part of the remote file.
                The replaced part of remote file is indicated by from and to parameters.
          @param data The data are submitted to the remote machine.
          @param size The size of data.
          @param from The start position in the remote file where the data replace.
          @param to The end position in the remote file where the data replace.
          @note The write access is available on some protocols. HTTP and HTTPS require the script file
                on the remote machine to handle the data. SSH requires the permission to edit the file.
          @throw Error if it fails.
         */
        virtual void writeRemote(const byte* data, size_t size, long from, long to) = 0;
        /*!
          @brief Get the data from the remote machine and write them to the memory blocks.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @return Number of bytes written to the memory block successfully
          @throw Error if it fails.
         */
        virtual size_t populateBlocks(size_t lowBlock, size_t highBlock);

    }; // class RemoteIo::Impl

    RemoteIo::Impl::Impl(const std::string& url, size_t blockSize)
        : path_(url), blockSize_(blockSize), blocksMap_(0), size_(0),
          idx_(0), isMalloced_(false), eof_(false), protocol_(fileProtocol(url))
    {
    }
#ifdef EXV_UNICODE_PATH
    RemoteIo::Impl::Impl(const std::wstring& wurl, size_t blockSize)
        : wpath_(wurl), blockSize_(blockSize), blocksMap_(0), size_(0),
          idx_(0), isMalloced_(false), eof_(false), protocol_(fileProtocol(wurl))
    {
    }
#endif

    size_t RemoteIo::Impl::populateBlocks(size_t lowBlock, size_t highBlock)
    {
        assert(isMalloced_);

        // optimize: ignore all true blocks on left & right sides.
        while(!blocksMap_[lowBlock].isNone()  && lowBlock  < highBlock) lowBlock++;
        while(!blocksMap_[highBlock].isNone() && highBlock > lowBlock)  highBlock--;

        size_t rcount = 0;
        if (blocksMap_[highBlock].isNone())
        {
            std::string data;
            getDataByRange( (long) lowBlock, (long) highBlock, data);
            rcount = (size_t)data.length();
            if (rcount == 0) {
                throw Error(1, "Data By Range is empty. Please check the permission.");
            }
            byte* source = (byte*)data.c_str();
            size_t remain = rcount, totalRead = 0;
            size_t iBlock = (rcount == size_) ? 0 : lowBlock;

            while (remain) {
                size_t allow = EXV_MIN(remain, blockSize_);
                blocksMap_[iBlock].populate(&source[totalRead], allow);
                remain -= allow;
                totalRead += allow;
                iBlock++;
            }
        }

        return rcount;
    }

    RemoteIo::Impl::~Impl() {
        if (blocksMap_) delete[] blocksMap_;
    }

    RemoteIo::~RemoteIo()
    {
        if (p_) {
            close();
            delete p_;
        }
    }

    int RemoteIo::open()
    {
        close(); // reset the IO position
        if (p_->isMalloced_ == false) {
            long length = p_->getFileLength();
            if (length < 0) { // unable to get the length of remote file, get the whole file content.
                std::string data;
                p_->getDataByRange(-1, -1, data);
                p_->size_ = (size_t) data.length();
                size_t nBlocks = (p_->size_ + p_->blockSize_ - 1) / p_->blockSize_;
                p_->blocksMap_  = new BlockMap[nBlocks];
                p_->isMalloced_ = true;
                byte* source = (byte*)data.c_str();
                size_t remain = p_->size_, iBlock = 0, totalRead = 0;
                while (remain) {
                    size_t allow = EXV_MIN(remain, p_->blockSize_);
                    p_->blocksMap_[iBlock].populate(&source[totalRead], allow);
                    remain -= allow;
                    totalRead += allow;
                    iBlock++;
                }
            } else if (length == 0) { // file is empty
                throw Error(1, "the file length is 0");
            } else {
                p_->size_ = (size_t) length;
                size_t nBlocks = (p_->size_ + p_->blockSize_ - 1) / p_->blockSize_;
                p_->blocksMap_  = new BlockMap[nBlocks];
                p_->isMalloced_ = true;
            }
        }
        return 0; // means OK
    }

    int RemoteIo::close()
    {
        if (p_->isMalloced_) {
            p_->eof_ = false;
            p_->idx_ = 0;
        }
        return 0;
    }

    long RemoteIo::write(const byte* /* unused data*/, long /* unused wcount*/)
    {
        return 0; // means failure
    }

    long RemoteIo::write(BasicIo& src)
    {
        assert(p_->isMalloced_);
        if (!src.isopen()) return 0;

        /*
         * The idea is to compare the file content, find the different bytes and submit them to the remote machine.
         * To simplify it, it:
         *      + goes from the left, find the first different position -> $left
         *      + goes from the right, find the first different position -> $right
         * The different bytes are [$left-$right] part.
         */
        long  left       = 0;
        long  right      = 0;
        long  blockIndex = 0;
        long  i          = 0;
        long  readCount  = 0;
        long  blockSize  = 0;
        byte* buf        = (byte*) std::malloc(p_->blockSize_);
        long  nBlocks    = (long)((p_->size_ + p_->blockSize_ - 1) / p_->blockSize_);

        // find $left
        src.seek(0, BasicIo::beg);
        bool findDiff = false;
        while (blockIndex < nBlocks && !src.eof() && !findDiff) {
            blockSize = (long)p_->blocksMap_[blockIndex].getSize();
            bool isFakeData = p_->blocksMap_[blockIndex].isKnown(); // fake data
            readCount = src.read(buf, blockSize);
            byte* blockData = p_->blocksMap_[blockIndex].getData();
            for (i = 0; (i < readCount) && (i < blockSize) && !findDiff; i++) {
                if ((!isFakeData && buf[i] != blockData[i]) || (isFakeData && buf[i] != 0)) {
                    findDiff = true;
                } else {
                    left++;
                }
            }
            blockIndex++;
        }


        // find $right
        findDiff    = false;
        blockIndex  = nBlocks - 1;
        blockSize   = (long)p_->blocksMap_[blockIndex].getSize();
        while ((blockIndex + 1 > 0) && right < src.size() && !findDiff) {
            if(src.seek(-1 * (blockSize + right), BasicIo::end)) {
                findDiff = true;
            } else {
                bool isFakeData = p_->blocksMap_[blockIndex].isKnown(); // fake data
                readCount = src.read(buf, blockSize);
                byte* blockData = p_->blocksMap_[blockIndex].getData();
                for (i = 0; (i < readCount) && (i < blockSize) && !findDiff; i++) {
                    if ((!isFakeData && buf[readCount - i - 1] != blockData[blockSize - i - 1]) || (isFakeData && buf[readCount - i - 1] != 0)) {
                        findDiff = true;
                    } else {
                        right++;
                    }
                }
            }
            blockIndex--;
            blockSize = (long)p_->blocksMap_[blockIndex].getSize();
        }

        // free buf
        if (buf) std::free(buf);

        // submit to the remote machine.
        long dataSize = src.size() - left - right;
        if (dataSize > 0) {
            byte* data = (byte*) std::malloc(dataSize);
            src.seek(left, BasicIo::beg);
            src.read(data, dataSize);
            p_->writeRemote(data, (size_t)dataSize, left, (long) p_->size_ - right);
            if (data) std::free(data);
        }
        return src.size();
    }

    int RemoteIo::putb(byte /*unused data*/)
    {
        return 0;
    }

    DataBuf RemoteIo::read(long rcount)
    {
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long RemoteIo::read(byte* buf, long rcount)
    {
        assert(p_->isMalloced_);
        if (p_->eof_) return 0;

        size_t allow     = EXV_MIN(rcount, (long)( p_->size_ - p_->idx_));
        size_t lowBlock  =  p_->idx_         /p_->blockSize_;
        size_t highBlock = (p_->idx_ + allow)/p_->blockSize_;

        // connect to the remote machine & populate the blocks just in time.
        p_->populateBlocks(lowBlock, highBlock);
        byte* fakeData = (byte*) std::calloc(p_->blockSize_, sizeof(byte));
        if (!fakeData) {
            throw Error(1, "Unable to allocate data");
        }

        size_t iBlock = lowBlock;
        size_t startPos = p_->idx_ - lowBlock*p_->blockSize_;
        size_t totalRead = 0;
        do {
            byte* data = p_->blocksMap_[iBlock++].getData();
            if (data == NULL) data = fakeData;
            size_t blockR = EXV_MIN(allow, p_->blockSize_ - startPos);
            std::memcpy(&buf[totalRead], &data[startPos], blockR);
            totalRead += blockR;
            startPos = 0;
            allow -= blockR;
        } while(allow);

        if (fakeData) std::free(fakeData);

        p_->idx_ += (long) totalRead;
        p_->eof_ = (p_->idx_ == (long) p_->size_);

        return (long) totalRead;
    }

    int RemoteIo::getb()
    {
        assert(p_->isMalloced_);
        if (p_->idx_ == (long)p_->size_) {
            p_->eof_ = true;
            return EOF;
        }

        size_t expectedBlock = (p_->idx_ + 1)/p_->blockSize_;
        // connect to the remote machine & populate the blocks just in time.
        p_->populateBlocks(expectedBlock, expectedBlock);

        byte* data = p_->blocksMap_[expectedBlock].getData();
        return data[p_->idx_++ - expectedBlock*p_->blockSize_];
    }

    void RemoteIo::transfer(BasicIo& src)
    {
        if (src.open() != 0) {
            throw Error(1, "unable to open src when transferring");
        }
        write(src);
        src.close();
    }

#if defined(_MSC_VER)
    int RemoteIo::seek( int64_t offset, Position pos )
    {
        assert(p_->isMalloced_);
        uint64_t newIdx = 0;

        switch (pos) {
            case BasicIo::cur: newIdx = p_->idx_ + offset; break;
            case BasicIo::beg: newIdx = offset; break;
            case BasicIo::end: newIdx = p_->size_ + offset; break;
        }

        if ( /*newIdx < 0 || */ newIdx > static_cast<uint64_t>(p_->size_) ) return 1;
        p_->idx_ = static_cast<long>(newIdx);   //not very sure about this. need more test!!    - note by Shawn  fly2xj@gmail.com //TODO
        p_->eof_ = false;
        return 0;
    }
#else
    int RemoteIo::seek(long offset, Position pos)
    {
        assert(p_->isMalloced_);
        long newIdx = 0;

        switch (pos) {
            case BasicIo::cur: newIdx = p_->idx_ + offset; break;
            case BasicIo::beg: newIdx = offset; break;
            case BasicIo::end: newIdx = p_->size_ + offset; break;
        }

        if (newIdx < 0 || newIdx > (long) p_->size_) return 1;
        p_->idx_ = newIdx;
        p_->eof_ = false;
        return 0;
    }
#endif

    byte* RemoteIo::mmap(bool /*isWriteable*/)
    {
        return NULL;
    }

    int RemoteIo::munmap()
    {
        return 0;
    }

    long RemoteIo::tell() const
    {
        return p_->idx_;
    }

    long RemoteIo::size() const
    {
        return (long) p_->size_;
    }

    bool RemoteIo::isopen() const
    {
        return p_->isMalloced_;
    }

    int RemoteIo::error() const
    {
        return 0;
    }

    bool RemoteIo::eof() const
    {
        return p_->eof_;
    }

    std::string RemoteIo::path() const
    {
        return p_->path_;
    }

#ifdef EXV_UNICODE_PATH
    std::wstring RemoteIo::wpath() const
    {
        return p_->wpath_;
    }
#endif

    BasicIo::AutoPtr RemoteIo::temporary() const
    {
        return BasicIo::AutoPtr(new MemIo);
    }

    void RemoteIo::populateFakeData()
    {
        assert(p_->isMalloced_);
        size_t nBlocks = (p_->size_ + p_->blockSize_ - 1) / p_->blockSize_;
        for (size_t i = 0; i < nBlocks; i++) {
            if (p_->blocksMap_[i].isNone())
                p_->blocksMap_[i].markKnown(p_->blockSize_);
        }
    }


    //! Internal Pimpl structure of class HttpIo.
    class HttpIo::HttpImpl : public Impl  {
    public:
        //! Constructor
        HttpImpl(const std::string&  path,  size_t blockSize);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        HttpImpl(const std::wstring& wpath, size_t blockSize);
#endif
        Exiv2::Uri hostInfo_; //!< the host information extracted from the path

        // METHODS
        /*!
          @brief Get the length (in bytes) of the remote file.
          @return Return -1 if the size is unknown. Otherwise it returns the length of remote file (in bytes).
          @throw Error if the server returns the error code.
         */
        long getFileLength();
        /*!
          @brief Get the data by range.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @param response The data from the server.
          @throw Error if the server returns the error code.
          @note Set lowBlock = -1 and highBlock = -1 to get the whole file content.
         */
        void getDataByRange(long lowBlock, long highBlock, std::string& response);
        /*!
          @brief Submit the data to the remote machine. The data replace a part of the remote file.
                The replaced part of remote file is indicated by from and to parameters.
          @param data The data are submitted to the remote machine.
          @param size The size of data.
          @param from The start position in the remote file where the data replace.
          @param to The end position in the remote file where the data replace.
          @note The data are submitted to the remote machine via POST. This requires the script file
                on the remote machine to receive the data and edit the remote file. The server-side
                script may be specified with the environment string EXIV2_HTTP_POST. The default value is
                "/exiv2.php". More info is available at http://dev.exiv2.org/wiki/exiv2
          @throw Error if it fails.
         */
        void writeRemote(const byte* data, size_t size, long from, long to);
    protected:
        // NOT IMPLEMENTED
        HttpImpl(const HttpImpl& rhs); //!< Copy constructor
        HttpImpl& operator=(const HttpImpl& rhs); //!< Assignment
    }; // class HttpIo::HttpImpl

    HttpIo::HttpImpl::HttpImpl(const std::string& url, size_t blockSize):Impl(url, blockSize)
    {
        hostInfo_ = Exiv2::Uri::Parse(url);
        Exiv2::Uri::Decode(hostInfo_);
    }
#ifdef EXV_UNICODE_PATH
    HttpIo::HttpImpl::HttpImpl(const std::wstring& wurl, size_t blockSize):Impl(wurl, blockSize)
    {
        std::string url;
        url.assign(wurl.begin(), wurl.end());
        path_ = url;

        hostInfo_ = Exiv2::Uri::Parse(url);
        Exiv2::Uri::Decode(hostInfo_);
    }
#endif

    long HttpIo::HttpImpl::getFileLength()
    {
        dict_t response;
        dict_t request;
        std::string errors;
        request["server"] = hostInfo_.Host;
        request["page"  ] = hostInfo_.Path;
        if (hostInfo_.Port != "") request["port"] = hostInfo_.Port;
        request["verb"]   = "HEAD";
        long serverCode = (long)http(request, response, errors);
        if (serverCode < 0 || serverCode >= 400 || errors.compare("") != 0) {
            throw Error(55, "Server", serverCode);
        }

        dict_i lengthIter = response.find("Content-Length");
        return (lengthIter == response.end()) ? -1 : atol((lengthIter->second).c_str());
    }

    void HttpIo::HttpImpl::getDataByRange(long lowBlock, long highBlock, std::string& response)
    {
        dict_t responseDic;
        dict_t request;
        request["server"] = hostInfo_.Host;
        request["page"  ] = hostInfo_.Path;
        if (hostInfo_.Port != "") request["port"] = hostInfo_.Port;
        request["verb"]   = "GET";
        std::string errors;
        if (lowBlock > -1 && highBlock > -1) {
            std::stringstream ss;
            ss << "Range: bytes=" << lowBlock * blockSize_  << "-" << ((highBlock + 1) * blockSize_ - 1) << "\r\n";
            request["header"] = ss.str();
        }

        long serverCode = (long)http(request, responseDic, errors);
        if (serverCode < 0 || serverCode >= 400 || errors.compare("") != 0) {
            throw Error(55, "Server", serverCode);
        }
        response = responseDic["body"];
    }

    void HttpIo::HttpImpl::writeRemote(const byte* data, size_t size, long from, long to)
    {
        std::string scriptPath(getEnv(envHTTPPOST));
        if (scriptPath == "") {
            throw Error(1, "Please set the path of the server script to handle http post data to EXIV2_HTTP_POST environmental variable.");
        }

        // standadize the path without "/" at the beginning.
        std::size_t protocolIndex = scriptPath.find("://");
        if (protocolIndex == std::string::npos && scriptPath[0] != '/') {
            scriptPath = "/" + scriptPath;
        }

        dict_t response;
        dict_t request;
        std::string errors;

        Uri scriptUri = Exiv2::Uri::Parse(scriptPath);
        request["server"] = scriptUri.Host == "" ? hostInfo_.Host : scriptUri.Host;
        if (scriptUri.Port != "") request["port"] = scriptUri.Port;
        request["page"] = scriptUri.Path;
        request["verb"] = "POST";

        // encode base64
        size_t encodeLength = ((size + 2) / 3) * 4 + 1;
        char* encodeData = new char[encodeLength];
        base64encode(data, size, encodeData, encodeLength);
        // url encode
        char* urlencodeData = urlencode(encodeData);
        delete[] encodeData;

        std::stringstream ss;
        ss << "path="   << hostInfo_.Path << "&"
           << "from="   << from           << "&"
           << "to="     << to             << "&"
           << "data="   << urlencodeData;
        std::string postData = ss.str();
        delete[] urlencodeData;

        // create the header
        ss.str("");
        ss << "Content-Length: " << postData.length()  << "\n"
           << "Content-Type: application/x-www-form-urlencoded\n"
           << "\n" << postData << "\r\n";
        request["header"] = ss.str();

        int serverCode = http(request, response, errors);
        if (serverCode < 0 || serverCode >= 400 || errors.compare("") != 0) {
            throw Error(55, "Server", serverCode);
        }
    }
    HttpIo::HttpIo(const std::string& url, size_t blockSize)
    {
        p_ = new HttpImpl(url, blockSize);
    }
#ifdef EXV_UNICODE_PATH
    HttpIo::HttpIo(const std::wstring& wurl, size_t blockSize)
    {
         p_ = new HttpImpl(wurl, blockSize);
    }
#endif

#if EXV_USE_CURL == 1
    //! Internal Pimpl structure of class RemoteIo.
    class CurlIo::CurlImpl : public Impl  {
    public:
        //! Constructor
        CurlImpl(const std::string&  path, size_t blockSize);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        CurlImpl(const std::wstring& wpath, size_t blockSize);
#endif
        //! Destructor. Cleans up the curl pointer and releases all managed memory.
        ~CurlImpl();

        CURL*        curl_;             //!< libcurl pointer

        // METHODS
        /*!
          @brief Get the length (in bytes) of the remote file.
          @return Return -1 if the size is unknown. Otherwise it returns the length of remote file (in bytes).
          @throw Error if the server returns the error code.
         */
        long getFileLength();
        /*!
          @brief Get the data by range.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @param response The data from the server.
          @throw Error if the server returns the error code.
          @note Set lowBlock = -1 and highBlock = -1 to get the whole file content.
         */
        void getDataByRange(long lowBlock, long highBlock, std::string& response);
        /*!
          @brief Submit the data to the remote machine. The data replace a part of the remote file.
                The replaced part of remote file is indicated by from and to parameters.
          @param data The data are submitted to the remote machine.
          @param size The size of data.
          @param from The start position in the remote file where the data replace.
          @param to The end position in the remote file where the data replace.
          @throw Error if it fails.
          @note The write access is only available on HTTP & HTTPS protocols. The data are submitted to server
                via POST method. It requires the script file on the remote machine to receive the data
                and edit the remote file. The server-side script may be specified with the environment
                string EXIV2_HTTP_POST. The default value is "/exiv2.php". More info is available at
                http://dev.exiv2.org/wiki/exiv2
         */
        void writeRemote(const byte* data, size_t size, long from, long to);
    protected:
        // NOT IMPLEMENTED
        CurlImpl(const CurlImpl& rhs); //!< Copy constructor
        CurlImpl& operator=(const CurlImpl& rhs); //!< Assignment
    private:
        long timeout_; //!< The number of seconds to wait while trying to connect.
    }; // class RemoteIo::Impl

    CurlIo::CurlImpl::CurlImpl(const std::string& url, size_t blockSize):Impl(url, blockSize)
    {
        // init curl pointer
        curl_ = curl_easy_init();
        if(!curl_) {
            throw Error(1, "Uable to init libcurl.");
        }

        // The default block size for FTP is much larger than other protocols
        // the reason is that getDataByRange() in FTP always creates the new connection,
        // so we need the large block size to reduce the overhead of creating the connection.
        if (blockSize_ == 0) {
            blockSize_ = protocol_ == pFtp ? 102400 : 1024;
        }

        std::string timeout = getEnv(envTIMEOUT);
        timeout_ = atol(timeout.c_str());
        if (timeout_ == 0) {
            throw Error(1, "Timeout Environmental Variable must be a positive integer.");
        }
    }
#ifdef EXV_UNICODE_PATH
    CurlIo::CurlImpl::CurlImpl(const std::wstring& wurl, size_t blockSize):Impl(wurl, blockSize)
    {
        std::string url;
        url.assign(wurl.begin(), wurl.end());
        path_ = url;

        // init curl pointer
        curl_ = curl_easy_init();
        if(!curl_) {
            throw Error(1, "Uable to init libcurl.");
        }

        // The default block size for FTP is much larger than other protocols
        // the reason is that getDataByRange() in FTP always creates the new connection,
        // so we need the large block size to reduce the overhead of creating the connection.
        if (blockSize_ == 0) {
            blockSize_ = protocol_ == pFtp ? 102400 : 1024;
        }
    }
#endif

    long CurlIo::CurlImpl::getFileLength()
    {
        curl_easy_reset(curl_); // reset all options
        std::string response;
        curl_easy_setopt(curl_, CURLOPT_URL, path_.c_str());
        curl_easy_setopt(curl_, CURLOPT_NOBODY, 1); // HEAD
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curlWriter);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeout_);
        //curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1); // debugging mode

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl_);
        if(res != CURLE_OK) { // error happends
            throw Error(1, curl_easy_strerror(res));
        }
        // get return code
        long returnCode;
        curl_easy_getinfo (curl_, CURLINFO_RESPONSE_CODE, &returnCode); // get code
        if (returnCode >= 400 || returnCode < 0) {
            throw Error(55, "Server", returnCode);
        }
        // get length
        double temp;
        curl_easy_getinfo(curl_, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &temp); // return -1 if unknown
        return (long) temp;
    }

    void CurlIo::CurlImpl::getDataByRange(long lowBlock, long highBlock, std::string& response)
    {
        curl_easy_reset(curl_); // reset all options
        curl_easy_setopt(curl_, CURLOPT_URL, path_.c_str());
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L); // no progress meter please
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curlWriter);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeout_);
        //curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1); // debugging mode

        if (lowBlock > -1 && highBlock> -1) {
            std::stringstream ss;
            ss << lowBlock * blockSize_  << "-" << ((highBlock + 1) * blockSize_ - 1);
            std::string range = ss.str();
            curl_easy_setopt(curl_, CURLOPT_RANGE, range.c_str());
        }

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl_);

        if(res != CURLE_OK) {
            throw Error(1, curl_easy_strerror(res));
        } else {
            long serverCode;
            curl_easy_getinfo (curl_, CURLINFO_RESPONSE_CODE, &serverCode); // get code
            if (serverCode >= 400 || serverCode < 0) {
                throw Error(55, "Server", serverCode);
            }
        }
    }

    void CurlIo::CurlImpl::writeRemote(const byte* data, size_t size, long from, long to)
    {
        std::string scriptPath(getEnv(envHTTPPOST));
        if (scriptPath == "") {
            throw Error(1, "Please set the path of the server script to handle http post data to EXIV2_HTTP_POST environmental variable.");
        }

        Exiv2::Uri hostInfo = Exiv2::Uri::Parse(path_);

        // add the protocol and host to the path
        std::size_t protocolIndex = scriptPath.find("://");
        if (protocolIndex == std::string::npos) {
            if (scriptPath[0] != '/') scriptPath = "/" + scriptPath;
            scriptPath = hostInfo.Protocol + "://" + hostInfo.Host + scriptPath;
        }

        curl_easy_reset(curl_); // reset all options
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L); // no progress meter please
        //curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1); // debugging mode
        curl_easy_setopt(curl_, CURLOPT_URL, scriptPath.c_str());
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);


        // encode base64
        size_t encodeLength = ((size + 2) / 3) * 4 + 1;
        char* encodeData = new char[encodeLength];
        base64encode(data, size, encodeData, encodeLength);
        // url encode
        char* urlencodeData = urlencode(encodeData);
        delete[] encodeData;
        std::stringstream ss;
        ss << "path="       << hostInfo.Path << "&"
           << "from="       << from          << "&"
           << "to="         << to            << "&"
           << "data="       << urlencodeData;
        std::string postData = ss.str();
        delete[] urlencodeData;

        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, postData.c_str());
        // Perform the request, res will get the return code.
        CURLcode res = curl_easy_perform(curl_);

        if(res != CURLE_OK) {
            throw Error(1, curl_easy_strerror(res));
        } else {
            long serverCode;
            curl_easy_getinfo (curl_, CURLINFO_RESPONSE_CODE, &serverCode);
            if (serverCode >= 400 || serverCode < 0) {
                throw Error(55, "Server", serverCode);
            }
        }
    }

    CurlIo::CurlImpl::~CurlImpl() {
        curl_easy_cleanup(curl_);
    }

    long CurlIo::write(const byte* data, long wcount)
    {
        if (p_->protocol_ == pHttp || p_->protocol_ == pHttps) {
            return RemoteIo::write(data, wcount);
        } else {
            throw Error(1, "doesnt support write for this protocol.");
        }
    }

    long CurlIo::write(BasicIo& src)
    {
        if (p_->protocol_ == pHttp || p_->protocol_ == pHttps) {
            return RemoteIo::write(src);
        } else {
            throw Error(1, "doesnt support write for this protocol.");
        }
    }

    CurlIo::CurlIo(const std::string& url, size_t blockSize)
    {
        p_ = new CurlImpl(url, blockSize);
    }
#ifdef EXV_UNICODE_PATH
    CurlIo::CurlIo(const std::wstring& wurl, size_t blockSize)
    {
        p_ = new CurlImpl(wurl, blockSize);
    }
#endif

#endif

#if EXV_USE_SSH == 1
    //! Internal Pimpl structure of class RemoteIo.
    class SshIo::SshImpl : public Impl  {
    public:
        //! Constructor
        SshImpl(const std::string&  path,  size_t blockSize);
#ifdef EXV_UNICODE_PATH
        //! Constructor accepting a unicode path in an std::wstring
        SshImpl(const std::wstring& wpath, size_t blockSize);
#endif
        //! Destructor. Closes ssh session and releases all managed memory.
        ~SshImpl();

        Exiv2::Uri      hostInfo_;          //!< host information extracted from path
        SSH*            ssh_;               //!< SSH pointer
        sftp_file       fileHandler_;       //!< sftp file handler

        // METHODS
        /*!
          @brief Get the length (in bytes) of the remote file.
          @return Return -1 if the size is unknown. Otherwise it returns the length of remote file (in bytes).
          @throw Error if the server returns the error code.
         */
        long getFileLength();
        /*!
          @brief Get the data by range.
          @param lowBlock The start block index.
          @param highBlock The end block index.
          @param response The data from the server.
          @throw Error if the server returns the error code.
          @note Set lowBlock = -1 and highBlock = -1 to get the whole file content.
         */
        void getDataByRange(long lowBlock, long highBlock, std::string& response);
        /*!
          @brief Submit the data to the remote machine. The data replace a part of the remote file.
                The replaced part of remote file is indicated by from and to parameters.
          @param data The data are submitted to the remote machine.
          @param size The size of data.
          @param from The start position in the remote file where the data replace.
          @param to The end position in the remote file where the data replace.
          @note The write access is only available on the SSH protocol. It requires the write permission
                to edit the remote file.
          @throw Error if it fails.
         */
        void writeRemote(const byte* data, size_t size, long from, long to);

    protected:
        // NOT IMPLEMENTED
        SshImpl(const SshImpl& rhs); //!< Copy constructor
        SshImpl& operator=(const SshImpl& rhs); //!< Assignment
    }; // class RemoteIo::Impl

    SshIo::SshImpl::SshImpl(const std::string& url, size_t blockSize):Impl(url, blockSize)
    {
        hostInfo_ = Exiv2::Uri::Parse(url);
        Exiv2::Uri::Decode(hostInfo_);

        // remove / at the beginning of the path
        if (hostInfo_.Path[0] == '/') {
            hostInfo_.Path = hostInfo_.Path.substr(1);
        }
        ssh_ = new SSH(hostInfo_.Host, hostInfo_.Username, hostInfo_.Password, hostInfo_.Port);

        if (protocol_ == pSftp) {
            ssh_->getFileSftp(hostInfo_.Path, fileHandler_);
            if (fileHandler_ == NULL) throw Error(1, "Unable to open the file");
        } else {
            fileHandler_ = NULL;
        }
    }
#ifdef EXV_UNICODE_PATH
    SshIo::SshImpl::SshImpl(const std::wstring& wurl, size_t blockSize):Impl(wurl, blockSize)
    {
        std::string url;
        url.assign(wurl.begin(), wurl.end());
        path_ = url;

        hostInfo_ = Exiv2::Uri::Parse(url);
        Exiv2::Uri::Decode(hostInfo_);

        // remove / at the beginning of the path
        if (hostInfo_.Path[0] == '/') {
            hostInfo_.Path = hostInfo_.Path.substr(1);
        }
        ssh_ = new SSH(hostInfo_.Host, hostInfo_.Username, hostInfo_.Password, hostInfo_.Port);

        if (protocol_ == pSftp) {
            ssh_->getFileSftp(hostInfo_.Path, fileHandler_);
            if (fileHandler_ == NULL) throw Error(1, "Unable to open the file");
        } else {
            fileHandler_ = NULL;
        }
    }
#endif

    long SshIo::SshImpl::getFileLength()
    {
        long length = 0;
        if (protocol_ == pSftp) { // sftp
            sftp_attributes attributes = sftp_fstat(fileHandler_);
            length = (long)attributes->size;
        } else { // ssh
            std::string response;
            //std::string cmd = "stat -c %s " + hostInfo_.Path;
            std::string cmd = "declare -a x=($(ls -alt " + hostInfo_.Path + ")); echo ${x[4]}";
            if (ssh_->runCommand(cmd, &response) != 0) {
                throw Error(1, "Unable to get file length.");
            } else {
                length = atol(response.c_str());
                if (length == 0) {
                    throw Error(1, "File is empty or not found.");
                }
            }
        }
        return length;
    }

    void SshIo::SshImpl::getDataByRange(long lowBlock, long highBlock, std::string& response)
    {
        if (protocol_ == pSftp) {
            if (sftp_seek(fileHandler_, (uint32_t) (lowBlock * blockSize_)) < 0) throw Error(1, "SFTP: unable to sftp_seek");
            size_t buffSize = (highBlock - lowBlock + 1) * blockSize_;
            char* buffer = new char[buffSize];
            long nBytes = (long) sftp_read(fileHandler_, buffer, buffSize);
            if (nBytes < 0) throw Error(1, "SFTP: unable to sftp_read");
            response.assign(buffer, buffSize);
            delete[] buffer;
        } else {
            std::stringstream ss;
            if (lowBlock > -1 && highBlock > -1) {
                ss  << "dd if=" << hostInfo_.Path
                    << " ibs=" << blockSize_
                    << " skip=" << lowBlock
                    << " count=" << (highBlock - lowBlock) + 1<< " 2>/dev/null";
            } else {
                ss  << "dd if=" << hostInfo_.Path
                    << " ibs=" << blockSize_
                    << " 2>/dev/null";
            }
            std::string cmd = ss.str();
            if (ssh_->runCommand(cmd, &response) != 0) {
                throw Error(1, "Unable to get data by range.");
            }
        }
    }

    void SshIo::SshImpl::writeRemote(const byte* data, size_t size, long from, long to)
    {
        if (protocol_ == pSftp) throw Error(1, "not support SFTP write access.");

        //printf("ssh update size=%ld from=%ld to=%ld\n", (long)size, from, to);
        assert(isMalloced_);

        std::string tempFile = hostInfo_.Path + ".exiv2tmp";
        std::string response;
        std::stringstream ss;
        // copy the head (byte 0 to byte fromByte) of original file to filepath.exiv2tmp
        ss  << "head -c " << from
            << " "   << hostInfo_.Path
            << " > " << tempFile;
        std::string cmd = ss.str();
        if (ssh_->runCommand(cmd, &response) != 0) {
            throw Error(1, "SSH: Unable to cope the head of file to temp");
        }

        // upload the data (the byte ranges which are different between the original
        // file and the new file) to filepath.exiv2datatemp
        if (ssh_->scp(hostInfo_.Path + ".exiv2datatemp", data, size) != 0) {
            throw Error(1, "SSH: Unable to copy file");
        }

        // concatenate the filepath.exiv2datatemp to filepath.exiv2tmp
        cmd = "cat " + hostInfo_.Path + ".exiv2datatemp >> " + tempFile;
        if (ssh_->runCommand(cmd, &response) != 0) {
            throw Error(1, "SSH: Unable to copy the rest");
        }

        // copy the tail (from byte toByte to the end of file) of original file to filepath.exiv2tmp
        ss.str("");
        ss  << "tail -c+" << (to + 1)
            << " "   << hostInfo_.Path
            << " >> "   << tempFile;
        cmd = ss.str();
        if (ssh_->runCommand(cmd, &response) != 0) {
            throw Error(1, "SSH: Unable to copy the rest");
        }

        // replace the original file with filepath.exiv2tmp
        cmd = "mv " + tempFile + " " + hostInfo_.Path;
        if (ssh_->runCommand(cmd, &response) != 0) {
            throw Error(1, "SSH: Unable to copy the rest");
        }

        // remove filepath.exiv2datatemp
        cmd = "rm " + hostInfo_.Path + ".exiv2datatemp";
        if (ssh_->runCommand(cmd, &response) != 0) {
            throw Error(1, "SSH: Unable to copy the rest");
        }
    }

    SshIo::SshImpl::~SshImpl() {
        if (fileHandler_) sftp_close(fileHandler_);
        if (ssh_) delete ssh_;
    }

    SshIo::SshIo(const std::string& url, size_t blockSize)
    {
        p_ = new SshImpl(url, blockSize);
    }
#ifdef EXV_UNICODE_PATH
    SshIo::SshIo(const std::wstring& wurl, size_t blockSize)
    {
        p_ = new SshImpl(wurl, blockSize);
    }
#endif

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
    std::string ReplaceStringInPlace(std::string subject, const std::string& search,
                          const std::string& replace) {
        size_t pos = 0;
        while((pos = subject.find(search, pos)) != std::string::npos) {
             subject.replace(pos, search.length(), replace);
             pos += replace.length();
        }
        return subject;
    }
#ifdef EXV_UNICODE_PATH
    std::wstring ReplaceStringInPlace(std::wstring subject, const std::wstring& search,
                          const std::wstring& replace) {
        std::wstring::size_type pos = 0;
        while((pos = subject.find(search, pos)) != std::wstring::npos) {
             subject.replace(pos, search.length(), replace);
             pos += replace.length();
        }
        return subject;
    }
#endif
#if EXV_USE_CURL == 1
    size_t curlWriter(char* data, size_t size, size_t nmemb,
                      std::string* writerData)
    {
        if (writerData == NULL) return 0;
        writerData->append(data, size*nmemb);
        return size * nmemb;
    }
#endif
}                                       // namespace Exiv2
