// ***************************************************************** -*- C++ -*-
// exiv2json.cpp, $Id: exiv2json.cpp 518 2013-05-10 23:53:00Z robinwmills@gmail.com $
// Sample program to print metadata in JSON format

#include <exiv2/exiv2.hpp>
#include <exiv2/value.hpp>
#include "Jzon.h"

#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>

#if defined(__MINGW32__) || defined(__MINGW64__)
# ifndef  __MINGW__
#  define __MINGW__
# endif
#endif

#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__MINGW__)
#include <windows.h>
#ifndef  PATH_MAX
# define PATH_MAX 512
#endif
const char* realpath(const char* file,char* path)
{
	GetFullPathName(file,PATH_MAX,path,NULL);
	return path;
}
#else
#include <unistd.h>
#endif

struct Token {
	std::string n; // the name eg "History"
	bool        a; // name is an array eg History[]
	int         i; // index (indexed from 1) eg History[1]/stEvt:action
};
typedef std::vector<Token> Tokens ;

// "XMP.xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:Rectangle"
bool getToken(std::string& in,Token& token)
{
	bool result = false;

	token.n = ""    ;
	token.a = false ;
	token.i = 0     ;

	while ( !result && in.length() ) {
		std::string c = in.substr(0,1);
		char        C = c[0];
		in            = in.substr(1,std::string::npos);
		if ( in.length() == 0 && C != ']' )	token.n += c;
		if ( C == '/' || C == '[' || C == ':' || C == '.' || C == ']' || in.length() == 0 ) {
			token.a   = C == '[';
			if ( C == ']' ) token.i = std::atoi(token.n.c_str()); // encoded string first index == 1
			result    = token.n.length() > 0 ;
		}  else {
			token.n   += c;
		}
	}
	return result;
}

Jzon::Node& addToTree(Jzon::Node& r1,Token token)
{
	Jzon::Object object ;
	Jzon::Array  array  ;

	std::string  key    = token.n  ;
	size_t       index  = token.i-1; // array Eg: "History[1]" indexed from 1.  Jzon expects 0 based index.
	Jzon::Node&  empty  = token.a ? (Jzon::Node&) array : (Jzon::Node&) object ;

	if (  r1.IsObject() ) {
		Jzon::Object& o1 = r1.AsObject();
		if (   !o1.Has(key) ) o1.Add(key,empty);
		return  o1.Get(key);
	} else if ( r1.IsArray() ) {
		Jzon::Array& a1 = r1.AsArray();
		while ( a1.GetCount() <= index ) a1.Add(empty);
		return  a1.Get(index);
	}
	return r1;
}

Jzon::Node& recursivelyBuildTree(Jzon::Node& root,Tokens& tokens,size_t k)
{
	return addToTree( k==0 ? root : recursivelyBuildTree(root,tokens,k-1), tokens[k] );
}

// build the json tree for this key.  return location and discover the name
Jzon::Node& objectForKey(const std::string Key,Jzon::Object& root,std::string& name)
{
    // Parse the key
    Tokens      tokens ;
    Token       token  ;
    std::string input  = Key ; // Example: "XMP.xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:Rectangle"
    while ( getToken(input,token) ) tokens.push_back(token);
	size_t      l      = tokens.size()-1; // leave leaf name to push()
	name               = tokens[l].n ;
	return recursivelyBuildTree(root,tokens,l-1);

#if 0
	// recursivelyBuildTree:
	// Go to the root.  Climb out adding objects or arrays to create the tree
	// The leaf is pushed on the top by the caller of objectForKey()
	// The recursion could be expressed by these if statements:
	if ( l == 1 ) return                               addToTree(root,tokens[0]);
	if ( l == 2 ) return                     addToTree(addToTree(root,tokens[0]),tokens[1]);
	if ( l == 3 ) return           addToTree(addToTree(addToTree(root,tokens[0]),tokens[1]),tokens[2]);
	if ( l == 4 ) return addToTree(addToTree(addToTree(addToTree(root,tokens[0]),tokens[1]),tokens[2]),tokens[3]);
	...
#endif
}

bool isObject(std::string& value)
{
	return !value.compare(std::string("type=\"Struct\""));
}

bool isArray(std::string& value)
{
	return !value.compare(std::string("type=\"Seq\""))
	||     !value.compare(std::string("type=\"Bag\""))
	||     !value.compare(std::string("type=\"Alt\""))
	;
}

#define STORE(node,key,value) \
 	if  (node.IsObject()) node.AsObject().Add(key,value);\
 	else                  node.AsArray() .Add(    value)

template <class T>
void push(Jzon::Node& node,const std::string& key,T i)
{
    std::string value = i->value().toString();

    switch ( i->typeId() ) {
        case Exiv2::xmpText:
			 if (        ::isObject(value) ) {
				 Jzon::Object   v;
				 STORE(node,key,v);
			 } else if ( ::isArray(value) ) {
				 Jzon::Array    v;
				 STORE(node,key,v);
			 } else {
				 STORE(node,key,value);
			 }
    	break;

        case Exiv2::unsignedByte:
        case Exiv2::unsignedShort:
        case Exiv2::unsignedLong:
        case Exiv2::signedByte:
        case Exiv2::signedShort:
        case Exiv2::signedLong:
			 STORE(node,key,std::atoi(value.c_str()) );
	    break;

        case Exiv2::tiffFloat:
        case Exiv2::tiffDouble:
			 STORE(node,key,std::atof(value.c_str()) );
        break;

        case Exiv2::unsignedRational:
        case Exiv2::signedRational: {
             Jzon::Array     arr;
             Exiv2::Rational rat = i->value().toRational();
             arr.Add(rat.first );
             arr.Add(rat.second);
			 STORE(node,key,arr);
        } break;

        case Exiv2::langAlt: {
        	 Jzon::Object l ;
             const Exiv2::LangAltValue& langs = dynamic_cast<const Exiv2::LangAltValue&>(i->value());
        	 for ( Exiv2::LangAltValue::ValueType::const_iterator lang = langs.value_.begin()
        	     ; lang != langs.value_.end()
        	     ; lang++
        	 ) {
        	 	l.Add(lang->first,lang->second);
        	 }
        	 Jzon::Object o ;
        	 o.Add("lang",l);
        	 STORE(node,key,o);
        }
        break;

        default:
        case Exiv2::date:
        case Exiv2::time:
        case Exiv2::asciiString :
        case Exiv2::string:
        case Exiv2::comment:
        case Exiv2::undefined:
        case Exiv2::tiffIfd:
        case Exiv2::directory:
        case Exiv2::xmpAlt:
        case Exiv2::xmpBag:
        case Exiv2::xmpSeq:
             // http://dev.exiv2.org/boards/3/topics/1367#message-1373
             if ( key == "UserComment" ) {
                size_t pos  = value.find('\0') ;
             	if (   pos != std::string::npos )
             	    value = value.substr(0,pos);
             }
             if ( key == "MakerNote") return;
       	     STORE(node,key,value);
        break;
    }
}

void fileSystemPush(const char* path,Jzon::Node& nfs)
{
    Jzon::Object& fs = (Jzon::Object&) nfs;
    fs.Add("path",path);
    char resolved_path[2000]; // PATH_MAX];
    fs.Add("realpath",realpath(path,resolved_path));

	struct stat buf;
    memset(&buf,0,sizeof(buf));
    stat(path,&buf);

    fs.Add("st_dev"    ,(int) buf.st_dev    ); /* ID of device containing file    */
    fs.Add("st_ino"    ,(int) buf.st_ino    ); /* inode number                    */
    fs.Add("st_mode"   ,(int) buf.st_mode   ); /* protection                      */
    fs.Add("st_nlink"  ,(int) buf.st_nlink  ); /* number of hard links            */
    fs.Add("st_uid"    ,(int) buf.st_uid    ); /* user ID of owner                */
    fs.Add("st_gid"    ,(int) buf.st_gid    ); /* group ID of owner               */
    fs.Add("st_rdev"   ,(int) buf.st_rdev   ); /* device ID (if special file)     */
    fs.Add("st_size"   ,(int) buf.st_size   ); /* total size, in bytes            */
    fs.Add("st_atime"  ,(int) buf.st_atime  ); /* time of last access             */
    fs.Add("st_mtime"  ,(int) buf.st_mtime  ); /* time of last modification       */
    fs.Add("st_ctime"  ,(int) buf.st_ctime  ); /* time of last status change      */

#if defined(_MSC_VER) || defined(__MINGW__)
	size_t blksize     = 1024;
	size_t blocks      = (buf.st_size+blksize-1)/blksize;
#else
    size_t blksize     = buf.st_blksize;
    size_t blocks      = buf.st_blocks ;
#endif
    fs.Add("st_blksize",(int) blksize       ); /* blocksize for file system I/O   */
    fs.Add("st_blocks" ,(int) blocks        ); /* number of 512B blocks allocated */
}

int main(int argc, char* const argv[])
try {
    if (argc < 2 || argc > 3) {
        std::cout << "Usage: " << argv[0] << " [-option] file"       << std::endl;
        std::cout << "Option: all | exif | iptc | xmp | filesystem"  << std::endl;
        return 1;
    }
    const char* path   = argv[argc-1];
    const char* opt    = argc == 3 ? argv[1] : "-all" ;
    while      (opt[0] == '-') opt++ ; // skip past leading -'s
    char        option = opt[0];

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
    assert(image.get() != 0);
    image->readMetadata();

    Jzon::Object   root;

	if ( option == 'f' ) { // only report filesystem when requested
		const char*    FS="FS";
		Jzon::Object      fs  ;
		root.Add(FS,fs) ;
		fileSystemPush(path,root.Get(FS));
	}

	if ( option == 'a' || option == 'e' ) {
		Exiv2::ExifData &exifData = image->exifData();
		for ( Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end() ; ++i ) {
        	std::string name   ;
			Jzon::Node& object = objectForKey(i->key(),root,name);
        	push(object,name,i);
    	}
	}

	if ( option == 'a' || option == 'i' ) {
		Exiv2::IptcData &iptcData = image->iptcData();
		for (Exiv2::IptcData::const_iterator i = iptcData.begin(); i != iptcData.end(); ++i) {
			std::string name   ;
			Jzon::Node& object = objectForKey(i->key(),root,name);
			push(object,name,i);
		}
	}

	if ( option == 'a' || option == 'x' ) {
		Exiv2::XmpData  &xmpData  = image->xmpData();
    	for (Exiv2::XmpData::const_iterator i = xmpData.begin(); i != xmpData.end(); ++i) {
        	std::string name   ;
			Jzon::Node& object = objectForKey(i->key(),root,name);
        	push(object,name,i);
    	}
	}

    Jzon::Writer writer(root,Jzon::StandardFormat);
    writer.Write();
    std::cout << writer.GetResult() << std::endl;
    return 0;
}

//catch (std::exception& e) {
//catch (Exiv2::AnyError& e) {
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
