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
#ifndef SSH_HPP_
#define SSH_HPP_

// included header files
#include "config.h"

#if EXV_USE_SSH == 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <sys/stat.h>
#include <string>
#include "error.hpp"
#include "types.hpp"
#include "futils.hpp"

namespace Exiv2 {
    /*!
      @brief The class provides the high-level functions related to libssh.
            It makes the libssh transparent. The functions in this class can
            be used without the requirement of understanding libssh.
     */
    class EXIV2API SSH {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to set up the connection to ssh server.
          @param host The host name of ssh server.
          @param user The username used to connect to ssh server.
          @param pass The password used to connect to ssh server.
          @param port The port to connect to ssh server. Set empty string to use the default port.
          @throw Error if it fails to connect the server.
         */
        SSH (const std::string& host, const std::string& user, const std::string& pass, const std::string port = "");
        //! Destructor
        ~SSH();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Run the command on the remote machine.
          @param cmd The command
          @param output The container for the command's output
          @return 0 (SSH_OK) if there is no error.
         */
        int runCommand(const std::string& cmd, std::string* output);
        /*!
          @brief SCP data to the remote machine.
          @param filePath The path of the new file on the remote machine where the data is saved.
          @param data The data copied to the remote machine.
          @param size The size of the data.
          @return 0 (SSH_OK) if there is no error.
          @throw Error if it is unable to copy the data.
         */
        int scp(const std::string& filePath, const byte* data, size_t size);
        /*!
          @brief Return the sftp file handle of the file on the remote machine to read the data.
          @param filePath The path of the file on the remote machine.
          @param handle The container for the file handle.
          @throw Error if it is unable to get the sftp file handle.

          @note Be sure to close() the file handle after use.
         */
        void getFileSftp(const std::string& filePath, sftp_file& handle);
        //@}
    private:
        /*!
          @brief Open the sftp session.
         */
        void openSftp();
        // DATA
        //! The number of seconds to wait while trying to connect.
        long timeout_;
        //! the ssh server host
        std::string host_;
        //! the username
        std::string user_;
        //! the password
        std::string pass_;
        //! the ssh session
        ssh_session session_;
        //! the sftp session
        sftp_session sftp_;
    }; // class SSH
} // namespace Exiv2
#endif

#endif // #ifdef EXIV2_HPP_
