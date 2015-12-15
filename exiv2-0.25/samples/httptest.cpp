// ***************************************************************** -*- C++ -*-
/*
 httptest.cpp
 This application is to test http.cpp. It provides the function to GET|HEAD|PUT the file via http protocol.
 */

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <stdlib.h>
using namespace std;

static int testSyntax(const char* arg)
{
	if ( !arg ) {
		cout << "insufficient input" << endl;
		exit(0);
	}
	return 0;
}

int main(int argc,const char** argv)
{
    if ( argc < 2 ) {
        cout << "usage  : " << argv[0] << " [key value]+" << endl;
        cout << "example: " << argv[0] << " [[-url] url | -server clanmills.com -page /LargsPanorama.jpg] -header \"Range: bytes=0-200\"" << endl;
		cout << "or     : " << argv[0] << " http://clanmills.com/LargsPanorama.jpg"                                     << endl;
        cout << "useful  keys: -verb {GET|HEAD|PUT}  -page str -server str -port number -version [-header something]+ " << endl;
        cout << "default keys: -verb GET -server clanmills.com -page robin.shtml -port 80 -version 1.0"                 << endl;
        cout << "export http_proxy=url eg export http_proxy=http://64.62.247.244:80"                                    << endl;
        return 0;
    }

    Exiv2::dict_t response;
    Exiv2::dict_t request;
    string        errors;

    // convert the command-line arguments into the request dictionary
    for ( int i = 1 ; i < argc ; i +=2 ) {
        const char* arg = argv[i];
        // skip past the -'s on the key
        while ( arg[0] == '-' ) arg++;

        if ( string(arg) == "header" ) {
			testSyntax(argv[i+1]);
            string header = argv[i+1];
            if ( ! strchr(argv[i+1],'\n') ) {
                header += "\r\n";
            }
            request[arg] += header;
        } else if ( string(arg) == "uri" || string(arg) == "url" ) {
			testSyntax(argv[i+1]);
        	Exiv2::Uri uri = Exiv2::Uri::Parse(argv[i+1]);
        	if ( uri.Protocol == "http" ) {
        	    request["server"] = uri.Host;
        	    request["page"]   = uri.Path;
        	    request["port"]   = uri.Port;
        	}
		} else if ( string(arg).substr(0,7) == "http://" ) {
        	Exiv2::Uri uri = Exiv2::Uri::Parse(argv[i--]);
        	if ( uri.Protocol == "http" ) {
        	    request["server"] = uri.Host;
        	    request["page"]   = uri.Path;
        	    request["port"]   = uri.Port;
        	}
        } else {
			testSyntax(argv[i+1]);
            request[arg]=argv[i+1];
        }
    }
    if ( !request.count("page"  ) ) request["page"  ]   = "robin.shtml";
    if ( !request.count("server") ) request["server"]   = "clanmills.com";

    int result = Exiv2::http(request,response,errors);
    cout << "result = " << result << endl;
    cout << "errors = " << errors << endl;
    cout << endl;

    for ( Exiv2::dict_i it = response.begin() ; it != response.end() ; it++ ) {
        // don't show request header
        if (it->first ==  "requestheaders") continue;

        cout << it->first << " -> ";

        if ( it->first ==  "body") {
        	string& value = it->second;
        	cout << "# " << value.length();
        	if ( value.length() < 1000 ) cout << " = " << value ;
        } else {
        	cout << it->second;
        }

        cout << endl;
    }

    return 0;
}

// That's all Folks!
////
