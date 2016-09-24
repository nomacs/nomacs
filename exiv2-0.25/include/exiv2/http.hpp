#ifndef HTTP_HPP_
#define HTTP_HPP_

#include <string>
#include <map>
#include <algorithm>

namespace Exiv2 {
    typedef std::map<std::string,std::string> dict_t;
    typedef dict_t::iterator                  dict_i;

    EXIV2API int http(dict_t& request,dict_t& response,std::string& errors);

}

#ifndef EXV_USE_CURL
#define EXV_USE_CURL 0
#endif
#if EXV_USE_CURL == 1
#include <curl/curl.h>
#endif

#endif
