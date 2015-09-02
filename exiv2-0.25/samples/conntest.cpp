// ***************************************************************** -*- C++ -*-
// con-test.cpp
// Tester application for testing the http/https/ftp/ssh/sftp connection

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <stdlib.h>
using namespace std;

void httpcon(const std::string& url, bool useHttp1_0 = false) {
    Exiv2::dict_t response;
    Exiv2::dict_t request;
    string        errors;

    Exiv2::Uri uri = Exiv2::Uri::Parse(url);
    Exiv2::Uri::Decode(uri);

    request["server"] = uri.Host;
    request["page"]   = uri.Path;
    request["port"]   = uri.Port;
    if (!useHttp1_0) request["version"] = "1.1";

    int serverCode = Exiv2::http(request,response,errors);
    if (serverCode < 0 || serverCode >= 400 || errors.compare("") != 0) {
        throw Exiv2::Error(55, "Server", serverCode);
    }
}

#if EXV_USE_CURL == 1
void curlcon(const std::string& url, bool useHttp1_0 = false) {
    CURL* curl = curl_easy_init();
    if(!curl) {
        throw Exiv2::Error(1, "Uable to init libcurl.");
    }

    // get the timeout value
    std::string timeoutStr = Exiv2::getEnv(Exiv2::envTIMEOUT);
    long timeout = atol(timeoutStr.c_str());
    if (timeout == 0) {
        throw Exiv2::Error(1, "Timeout Environmental Variable must be a positive integer.");
    }

    string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Exiv2::curlWriter);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); // debug
    if (useHttp1_0) curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    else            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) { // error happends
        throw Exiv2::Error(1, curl_easy_strerror(res));
    }

    // get return code
    long returnCode;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &returnCode); // get code
    curl_easy_cleanup(curl);

    if (returnCode >= 400 || returnCode < 0) {
        throw Exiv2::Error(55, "Server", returnCode);
    }
}
#endif

#if EXV_USE_SSH == 1
void sshcon(const std::string& url) {
    Exiv2::Uri uri = Exiv2::Uri::Parse(url);
    Exiv2::Uri::Decode(uri);

    string page = uri.Path;
    // remove / at the beginning of the path
    if (page[0] == '/') {
        page = page.substr(1);
    }
    Exiv2::SSH ssh(uri.Host, uri.Username, uri.Password, uri.Port);
    string response = "";
    string cmd = "declare -a x=($(ls -alt " + page + ")); echo ${x[4]}";
    if (ssh.runCommand(cmd, &response) != 0) {
        throw Exiv2::Error(1, "Unable to get file length.");
    } else {
        long length = atol(response.c_str());
        if (length == 0) {
            throw Exiv2::Error(1, "File is empty or not found.");
        }
    }
}

void sftpcon(const std::string& url) {
    Exiv2::Uri uri = Exiv2::Uri::Parse(url);
    Exiv2::Uri::Decode(uri);

    string page = uri.Path;
    // remove / at the beginning of the path
    if (page[0] == '/') {
        page = page.substr(1);
    }
    Exiv2::SSH ssh(uri.Host, uri.Username, uri.Password, uri.Port);
    sftp_file handle;
    ssh.getFileSftp(page, handle);
    if (handle == NULL) throw Exiv2::Error(1, "Unable to open the file");
    else sftp_close(handle);
}
#endif

int main(int argc,const char** argv)
{
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " url {-http1_0}\n";
        return 1;
    }
    std::string url(argv[1]);
    Exiv2::Protocol prot = Exiv2::fileProtocol(url);

    bool useHttp1_0 = false;
    for ( int a = 1 ; a < argc ; a++ ) {
        std::string arg(argv[a]);
        if (arg == "-http1_0") useHttp1_0 = true;
    }

    bool isOk = false;
    try {
        #if EXV_USE_SSH == 1
            if (prot == Exiv2::pSsh) {
                sshcon(url);
                isOk = true;
            } else if (prot == Exiv2::pSftp){
                sftpcon(url);
                isOk = true;
            }
        #endif
        #if EXV_USE_CURL == 1
            if (prot == Exiv2::pHttp || prot == Exiv2::pHttps || prot == Exiv2::pFtp) {
                curlcon(url, useHttp1_0);
                isOk = true;
            }
        #endif
        if (!isOk && prot == Exiv2::pHttp) {
            httpcon(url, useHttp1_0);
            isOk = true;
        }
    } catch (Exiv2::AnyError& e) {
        std::cout << "Error: '" << e << "'\n";
        return -1;
    }

    if (!isOk)  cout << "The protocol is unsupported.\n";
    else        cout << "OK.\n";
    return 0;
}

// That's all Folks!
