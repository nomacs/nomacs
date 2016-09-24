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
  File:      rw2image.cpp
  Version:   $Rev: 3201 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-Jan-09, ahu: created

 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: rw2image.cpp 3201 2013-12-01 12:13:42Z ahuggel $")

#include "ssh.hpp"
#if EXV_USE_SSH == 1
// class member definitions
namespace Exiv2 {

    SSH::SSH(const std::string& host, const std::string& user, const std::string& pass, const std::string port):
        host_(host),user_(user),pass_(pass),sftp_(0) {

        std::string timeout = getEnv(envTIMEOUT);
        timeout_ = atol(timeout.c_str());
        if (timeout_ == 0) {
            throw Error(1, "Timeout Environmental Variable must be a positive integer.");
        }

        session_ = ssh_new();
        if (session_ == NULL) {
            throw Error(1, "Unable to create the the ssh session");
        }

        // try to connect
        ssh_options_set(session_, SSH_OPTIONS_HOST, host_.c_str());
        ssh_options_set(session_, SSH_OPTIONS_USER, user_.c_str());
        ssh_options_set(session_, SSH_OPTIONS_TIMEOUT, &timeout_);
        if (port != "") ssh_options_set(session_, SSH_OPTIONS_PORT_STR, port.c_str());

        if (ssh_connect(session_) != SSH_OK) {
            throw Error(1, ssh_get_error(session_));
        }
        // Authentication
        if (ssh_userauth_password(session_, NULL, pass_.c_str()) != SSH_AUTH_SUCCESS) {
            throw Error(1, ssh_get_error(session_));
        }
    }

    int SSH::runCommand(const std::string& cmd, std::string* output) {
        int rc;
        ssh_channel channel;
        channel = ssh_channel_new(session_);
        if (channel == NULL) {
           rc = SSH_ERROR;
        } else {
            rc = ssh_channel_open_session(channel);
            if (rc != SSH_OK) {
                ssh_channel_free(channel);
            } else {
                char buffer[256];
                rc = ssh_channel_request_exec(channel, cmd.c_str());
                if (rc == SSH_OK) {
                    while ((rc = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
                        output->append(buffer, rc);
                    }
                }
                ssh_channel_send_eof(channel);
                ssh_channel_close(channel);
                ssh_channel_free(channel);
            }
        }
        return rc;
    }

    int SSH::scp(const std::string& filePath, const byte* data, size_t size) {
        ssh_scp scp;
        int rc;

        size_t found = filePath.find_last_of("/\\");
        std::string filename = filePath.substr(found+1);
        std::string path = filePath.substr(0, found+1);

        scp = ssh_scp_new(session_, SSH_SCP_WRITE, path.c_str());
        if (scp == NULL) {
            rc = SSH_ERROR;
            throw Error(1, ssh_get_error(session_));
        } else {
            rc = ssh_scp_init(scp);
            if (rc != SSH_OK) {
                throw Error(1, ssh_get_error(session_));
            } else {
#ifdef  _MSC_VER
// S_IRUSR & S_IWUSR not in MSVC (0000400 & 0000200 in /usr/include/sys/stat.h on MacOS-X 10.8)
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#endif
                rc = ssh_scp_push_file (scp, filename.c_str(), size, S_IRUSR |  S_IWUSR);
                if (rc != SSH_OK) {
                    throw Error(1, ssh_get_error(session_));
                } else {
                    rc = ssh_scp_write(scp, data, size);
                    if (rc != SSH_OK) {
                        throw Error(1, ssh_get_error(session_));
                    }
                }
                ssh_scp_close(scp);
            }
            ssh_scp_free(scp);
        }

        return rc;
    }

    void SSH::openSftp() {
        if (sftp_) return;

        sftp_ = sftp_new(session_);
        if (sftp_ == NULL) {
            throw Error(1, "Unable to create the the sftp session");
        }
        if (sftp_init(sftp_) != SSH_OK) {
            sftp_free(sftp_);
            throw Error(1, "Error initializing SFTP session");
        }
    }

    void SSH::getFileSftp(const std::string& filePath, sftp_file& handle) {
        if (!sftp_) openSftp();
        handle = sftp_open(sftp_, ("/"+filePath).c_str(), 0x0000, 0); // read only
    }

    SSH::~SSH() {
        if (sftp_) sftp_free(sftp_);
        ssh_disconnect(session_);
        ssh_free(session_);
    }
}
#endif // EXV_USE_SSH == 1
