// ***************************************************************** -*- C++ -*-
// geotag.cpp, $Rev: 2286 $
// Sample program to read gpx files and update images with GPS tags

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <expat.h>

#include <vector>
#include <string>

#if defined(__MINGW32__) || defined(__MINGW64__)
# ifndef  __MINGW__
#  define __MINGW__
# endif
#endif

using namespace std;

#ifndef  lengthof
#define  lengthof(x) (sizeof(*x)/sizeof(x))
#endif
#ifndef nil
#define nil NULL
#endif

#if defined(_MSC_VER) || defined(__MINGW__)
#include <windows.h>
char*    realpath(const char* file,char* path);
#define  lstat _stat
#define  stat  _stat
#if      _MSC_VER < 1400
#define strcpy_s(d,l,s) strcpy(d,s)
#define strcat_s(d,l,s) strcat(d,s)
#endif
#endif

#if ! defined(_MSC_VER)
#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#define  stricmp strcasecmp
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

#define UNUSED(x) (void)(x)

// prototypes
class Options;
int getFileType(const char* path ,Options& options);
int getFileType(std::string& path,Options& options);

string getExifTime(const time_t t);
time_t parseTime(const char* ,bool bAdjust=false);
int    timeZoneAdjust();

// platform specific code
#if defined(_MSC_VER) || defined(__MINGW__)
char* realpath(const char* file,char* path)
{
    char* result = (char*) malloc(_MAX_PATH);
    if   (result) GetFullPathName(file,_MAX_PATH,result,NULL);
    return result ;
    UNUSED(path);
}
#endif

// Command-line parser
class Options  {
public:
    bool        verbose;
    bool        help;
    bool        version;
    bool        dst;
    bool        dryrun;

    Options()
    {
        verbose     = false;
        help        = false;
        version     = false;
        dst         = false;
        dryrun      = false;
    }

    virtual ~Options() {} ;
} ;

enum
{   resultOK=0
,   resultSyntaxError
,   resultSelectFailed
};

enum                        // keyword indices
{   kwHELP = 0
,   kwVERSION
,   kwDST
,   kwDRYRUN
,   kwVERBOSE
,   kwADJUST
,   kwTZ
,   kwDELTA
,   kwMAX                   // manages keyword array
,   kwNEEDVALUE             // bogus keywords for error reporting
,   kwSYNTAX                // -- ditto --
,   kwNOVALUE = -kwVERBOSE  // keywords <= kwNOVALUE are flags (no value needed)
};

// file types supported
enum
{   typeUnknown   = 0
,   typeDirectory = 1
,   typeImage     = 2
,   typeXML       = 3
,   typeFile      = 4
,   typeDoc       = 5
,   typeCode      = 6
,   typeMax       = 7
};

// Position (from gpx file)
class Position
{
public:
             Position(time_t time,double lat,double lon,double ele) : time_(time),lon_(lon),lat_(lat),ele_(ele) {};
             Position() { time_=0 ; lon_=0.0 ; lat_=0.0 ; ele_=0.0 ; };
    virtual ~Position() {} ;
//  copy constructor
    Position(const Position& o) : time_(o.time_),lon_(o.lon_),lat_(o.lat_),ele_(o.ele_) {};

//  instance methods
    bool good()                 { return time_ || lon_ || lat_ || ele_ ; }
    std::string getTimeString() { if ( times_.empty() ) times_ = getExifTime(time_) ;  return times_; }
    time_t      getTime()       { return time_ ; }
    std::string toString();

//  getters/setters
    double lat()            {return lat_   ;}
    double lon()            {return lon_   ;}
    double ele()            {return ele_   ;}
    int    delta()          {return delta_ ;}
    void   delta(int delta) {delta_=delta  ;}

//  data
private:
    time_t      time_;
    double      lon_ ;
    double      lat_ ;
    double      ele_ ;
    std::string times_;
    int         delta_;

// public static data
public:
    static int    adjust_  ;
    static int    tz_      ;
    static int    dst_     ;
    static time_t deltaMax_;

// public static member functions
public:
    static int    Adjust() {return Position::adjust_ + Position::tz_ + Position::dst_ ;}
    static int    tz()     {return tz_    ;}
    static int    dst()    {return dst_   ;}
    static int    adjust() {return adjust_;}

    static std::string toExifString(double d,bool bRational,bool bLat);
    static std::string toExifString(double d);
    static std::string toExifTimeStamp(std::string& t);
};

std::string Position::toExifTimeStamp(std::string& t)
{
    char        result[200];
    const char* arg = t.c_str();
    int HH = 0 ;
    int mm = 0 ;
    int SS = 0 ;
    if ( strstr(arg,":") || strstr(arg,"-") ) {
        int  YY,MM,DD    ;
        char a,b,c,d,e   ;
        sscanf(arg,"%d%c%d%c%d%c%d%c%d%c%d",&YY,&a,&MM,&b,&DD,&c,&HH,&d,&mm,&e,&SS);
    }
    sprintf(result,"%d/1 %d/1 %d/1",HH,mm,SS);
    return std::string(result);
}

std::string Position::toExifString(double d)
{
    char result[200];
    d *= 100;
    sprintf(result,"%d/100",abs((int)d));
    return std::string(result);
}

std::string Position::toExifString(double d,bool bRational,bool bLat)
{
    const char* NS   = d>=0.0?"N":"S";
    const char* EW   = d>=0.0?"E":"W";
	const char* NSEW = bLat  ? NS: EW;
    if ( d < 0 ) d = -d;
    int deg = (int) d;
        d  -= deg;
        d  *= 60;
    int min = (int) d ;
        d  -= min;
        d  *= 60;
    int sec = (int)d;
    char result[200];
    sprintf(result,bRational ? "%d/1 %d/1 %d/1%s" : "%03d.%02d'%02d\"%s" ,deg,min,sec,bRational?"":NSEW);
    return std::string(result);
}

std::string Position::toString()
{
    char result[200];
    std::string sLat = Position::toExifString(lat_,false,true );
    std::string sLon = Position::toExifString(lon_,false,false);
    sprintf(result,"%s %s %-8.3f",sLon.c_str(),sLat.c_str(),ele_);
    return std::string(result);
}

int    Position::adjust_   = 0;
int    Position::tz_       = timeZoneAdjust();
int    Position::dst_      = 0;
time_t Position::deltaMax_ = 60 ;

// globals
typedef std::map<time_t,Position>           TimeDict_t;
typedef std::map<time_t,Position>::iterator TimeDict_i;
typedef std::vector<std::string>            strings_t;
TimeDict_t   gTimeDict ;
strings_t    gFiles;

///////////////////////////////////////////////////////////
// UserData - used by XML Parser
class UserData
{
public:
    UserData(Options& options) : indent(0),count(0),nTrkpt(0),bTime(false),bEle(false),options_(options) {};
    virtual ~UserData() {} ;

//  public data members
    int         indent;
    size_t      count ;
    Position    now ;
    Position    prev;
    int         nTrkpt;
    bool        bTime ;
    bool        bEle  ;
    double      ele;
    double      lat;
    double      lon;
    std::string xmlt;
    std::string exift;
    time_t      time;
    Options&    options_;
// static public data memembers
};

// XML Parser Callbacks
static void startElement(void* userData, const char* name, const char** atts )
{
    UserData* me = (UserData*) userData;
    //for ( int i = 0 ; i < me->indent ; i++ ) printf(" ");
    //printf("begin %s\n",name);
    me->bTime = strcmp(name,"time")==0;
    me->bEle  = strcmp(name,"ele")==0;

    if ( strcmp(name,"trkpt")==0 ) {
        me->nTrkpt++;
        while ( *atts ) {
            const char* a=atts[0];
            const char* v=atts[1];
            if ( !strcmp(a,"lat") ) me->lat = atof(v);
            if ( !strcmp(a,"lon") ) me->lon = atof(v);
            atts += 2 ;
        }
    }
    me->count++  ;
    me->indent++ ;
}

static void endElement(void* userData, const char* name)
{
    UserData* me = (UserData*) userData;
    me->indent-- ;
    if ( strcmp(name,"trkpt")==0 ) {

        me->nTrkpt--;
        me->now = Position(me->time,me->lat,me->lon,me->ele) ;

        if ( !me->prev.good() && me->options_.verbose ) {
            printf("trkseg %s begin ",me->now.getTimeString().c_str());
        }

        // printf("lat,lon = %f,%f ele = %f xml = %s exif = %s\n",me->lat,me->lon,me->ele,me->xmlt.c_str(),me->exift.c_str());

        // if we have a good previous position
        // add missed entries to timedict
        //if ( me->prev.good() && (me->now.getTime() - me->prev.getTime()) < Position::timeDiffMax ) {
        //  time_t missed = me->prev.getTime() ;
        //  while ( ++missed < me->now.getTime() )
        //      gTimeDict[missed] = me->prev ; // Position(missed,me->lat,me->lon,me->ele) ;
        //}

        // remember our location and put it in gTimeDict
        gTimeDict[me->time] = me->now ;
        me->prev = me->now ;
    }
    if ( strcmp(name,"trkseg")==0 && me->options_.verbose ) {
        printf("%s end\n",me->now.getTimeString().c_str());
    }
}

void charHandler(void* userData,const char* s,int len)
{
    UserData* me = (UserData*) userData;

    if ( me->nTrkpt == 1 ) {
        char buffer[100];
        int  l_max = 98 ; // lengthof(buffer) -2 ;

        if ( me->bTime && len > 5 ) {
            if ( len < l_max ) {
                memcpy(buffer,s,len);
                buffer[len]=0;
                char* b = buffer ;
                while ( *b == ' ' && b < buffer+len ) b++ ;
                me->xmlt  = b ;
                me->time  = parseTime(me->xmlt.c_str());
                me->exift = getExifTime(me->time);
            }
            me->bTime=false;
        }
        if ( me->bEle && len > 5 ) {
            if ( len < l_max ) {
                memcpy(buffer,s,len);
                buffer[len]=0;
                char* b = buffer ;
                while ( *b == ' ' && b < buffer+len ) b++ ;
                me->ele = atof(b);
            }
            me->bEle=false;
        }
    }
}

///////////////////////////////////////////////////////////
// Time Functions
time_t parseTime(const char* arg,bool bAdjust)
{
    time_t result = 0 ;
    try {
        //559 rmills@rmills-imac:~/bin $ exiv2 -pa ~/R.jpg | grep -i date
        //Exif.Image.DateTime                          Ascii      20  2009:08:03 08:58:57
        //Exif.Photo.DateTimeOriginal                  Ascii      20  2009:08:03 08:58:57
        //Exif.Photo.DateTimeDigitized                 Ascii      20  2009:08:03 08:58:57
        //Exif.GPSInfo.GPSDateStamp                    Ascii      21  2009-08-03T15:58:57Z

        // <time>2012-07-14T17:33:16Z</time>

        if ( strstr(arg,":") || strstr(arg,"-") ) {
            int  YY,MM,DD,HH,mm,SS ;
            char a,b,c,d,e   ;
            sscanf(arg,"%d%c%d%c%d%c%d%c%d%c%d",&YY,&a,&MM,&b,&DD,&c,&HH,&d,&mm,&e,&SS);

            struct tm T;
    #if 0
            int tm_sec;     /* seconds (0 - 60) */
            int tm_min;     /* minutes (0 - 59) */
            int tm_hour;    /* hours (0 - 23) */
            int tm_mday;    /* day of month (1 - 31) */
            int tm_mon;     /* month of year (0 - 11) */
            int tm_year;    /* year - 1900 */
            int tm_wday;    /* day of week (Sunday = 0) */
            int tm_yday;    /* day of year (0 - 365) */
            int tm_isdst;   /* is summer time in effect? */
            char *tm_zone;  /* abbreviation of timezone name */
            long tm_gmtoff; /* offset from UTC in seconds */
    #endif
            memset(&T,0,sizeof(T));
            T.tm_min  = mm  ;
            T.tm_hour = HH  ;
            T.tm_sec  = SS  ;
            if ( bAdjust ) T.tm_sec -= Position::Adjust();
            T.tm_year = YY -1900 ;
            T.tm_mon  = MM -1    ;
            T.tm_mday = DD  ;
            result = mktime(&T);
        }
    } catch ( ... ) {};
    return result ;
}

// West of GMT is negative (PDT = Pacific Daylight = -07:00 == -25200 seconds
int timeZoneAdjust()
{
    time_t    now   = time(NULL);
    int       offset;

#if   defined(_MSC_VER) || defined(__MINGW__)
    TIME_ZONE_INFORMATION TimeZoneInfo;
    GetTimeZoneInformation( &TimeZoneInfo );
    offset = - (((int)TimeZoneInfo.Bias + (int)TimeZoneInfo.DaylightBias) * 60);
    UNUSED(now);
#elif defined(__CYGWIN__)
    struct tm lcopy = *localtime(&now);
    time_t    gmt   =  timegm(&lcopy) ; // timegm modifies lcopy
    offset          = (int) ( ((long signed int) gmt) - ((long signed int) now) ) ;
#elif defined(OS_SOLARIS)
    struct tm local = *localtime(&now) ;
    time_t local_tt = (int) mktime(&local);
    time_t time_gmt = (int) mktime(gmtime(&now));
    offset          = time_gmt - local_tt;
#else
    struct tm local = *localtime(&now) ;
    offset          = local.tm_gmtoff ;
#endif

#if 0
    // debugging code
    struct tm utc = *gmtime(&now);
    printf("utc  :  offset = %6d dst = %d time = %s", 0     ,utc  .tm_isdst, asctime(&utc  ));
    printf("local:  offset = %6d dst = %d time = %s", offset,local.tm_isdst, asctime(&local));
    printf("timeZoneAdjust = %6d\n",offset);
#endif
    return offset ;
}

string getExifTime(const time_t t)
{
    static char result[100];
    strftime(result,sizeof(result),"%Y-%m-%d %H:%M:%S",localtime(&t));
    return result ;
}

std::string makePath(std::string dir,std::string file)
{
    return dir + std::string(EXV_SEPARATOR_STR) + file ;
}

const char* makePath(const char* dir,const char* file)
{
    static char result[_MAX_PATH] ;
    std::string r = makePath(std::string(dir),std::string(file));
    strcpy(result,r.c_str());
    return result;
}

// file utilities
bool readDir(const char* path,Options& options)
{
    bool bResult = false;

#ifdef _MSC_VER
    DWORD attrs    =  GetFileAttributes(path);
    bool  bOKAttrs =  attrs != INVALID_FILE_ATTRIBUTES;
    bool  bIsDir   = (attrs  & FILE_ATTRIBUTE_DIRECTORY) ? true : false ;

    if( bOKAttrs && bIsDir ) {
        bResult = true ;

        char     search[_MAX_PATH+10];
        strcpy_s(search,_MAX_PATH,path);
        strcat_s(search,_MAX_PATH,"\\*");

        WIN32_FIND_DATA ffd;
        HANDLE  hFind = FindFirstFile(search, &ffd);
        BOOL    bGo = hFind != INVALID_HANDLE_VALUE;

        if ( bGo ) {
            while ( bGo ) {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // _tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
                }
                else
                {
                    std::string pathName = makePath(path,std::string(ffd.cFileName));
                    if ( getFileType(pathName,options) == typeImage ) {
                        gFiles.push_back( pathName );
                    }
                }
                bGo = FindNextFile(hFind, &ffd) != 0;
            }
            // CloseHandle(hFind);
        }
    }
#else
    DIR*    dir = opendir (path);
    if (dir != NULL)
    {
        bResult = true;
        struct dirent*  ent;

        // print all the files and directories within directory
        while ((ent = readdir (dir)) != NULL)
        {
            std::string pathName = makePath(path,ent->d_name);
            struct stat  buf     ;
            lstat(path, &buf );
            if ( ent->d_name[0] != '.' ) {

                // printf("reading %s => %s\n",ent->d_name,pathName.c_str());
                if ( getFileType(pathName,options) == typeImage ) {
                    gFiles.push_back( pathName );
                }
            }
        }
        closedir (dir);
    }
#endif
    return bResult ;
}

inline size_t sip(FILE* f,char* buffer,size_t max_len,size_t len)
{
    while ( !feof(f) && len < max_len && buffer[len-1] != '>')
        buffer[len++] = fgetc(f);
    return len;
}

bool readXML(const char* path,Options& options)
{
    FILE*       f       = fopen(path,"r");
    XML_Parser  parser  = XML_ParserCreate(NULL);
    bool bResult        = f && parser ;
    if ( bResult ) {
        char   buffer[8*1024];
        UserData me(options) ;

        XML_SetUserData            (parser, &me);
        XML_SetElementHandler      (parser, startElement, endElement);
        XML_SetCharacterDataHandler(parser,charHandler);

        // a little sip at the data
        size_t len = fread(buffer,1,sizeof(buffer)-100,f);
        const char* lead   = "<?xml" ;
        bResult = strncmp(lead,buffer,strlen(lead))==0;

        // swallow it
        if ( bResult ) {
            len = sip(f,buffer,sizeof buffer,len);
            bResult = XML_Parse(parser, buffer,(int)len, len == 0 ) == XML_STATUS_OK;
        }

        // drink the rest of the file
        while ( bResult && len != 0 ) {
            len = fread(buffer,1,sizeof(buffer)-100,f);
            len = sip(f,buffer,sizeof buffer,len);
            bResult = XML_Parse(parser, buffer,(int)len, len == 0 ) == XML_STATUS_OK;
        };
    }

    if ( f      ) fclose(f);
    if ( parser ) XML_ParserFree(parser);

    return bResult ;
}

bool readImage(const char* path,Options& /* options */)
{
    using namespace Exiv2;
    bool bResult = false ;

    try {
        Image::AutoPtr image = ImageFactory::open(path);
        if ( image.get() ) {
            image->readMetadata();
            ExifData &exifData = image->exifData();
            bResult = !exifData.empty();
        }
    } catch ( ... ) {};
    return bResult ;
}

time_t readImageTime(std::string path,std::string* pS=NULL)
{
    using namespace Exiv2;

    time_t       result       = 0 ;
	static std::map<std::string,time_t> cache;
	if ( cache.count(path) == 1 ) return cache[path];

    const char* dateStrings[] =
    { "Exif.Photo.DateTimeOriginal"
    , "Exif.Photo.DateTimeDigitized"
    , "Exif.Image.DateTime"
    , NULL
    };
    const char* ds            = dateStrings[0] ;

    while ( !result && ds++  ) {
        try {
            Image::AutoPtr image = ImageFactory::open(path);
            if ( image.get() ) {
                image->readMetadata();
                ExifData &exifData = image->exifData();
            //  printf("%s => %s\n",(ds-1), exifData[ds-1].toString().c_str());
                result = parseTime(exifData[ds-1].toString().c_str(),true);
                if ( result && pS ) *pS = exifData[ds-1].toString();
            }
        } catch ( ... ) {};
    }
	if ( result ) cache[path] = result;
    return result ;
}

bool sina(const char* s,const char** a)
{
    bool bResult = false ;
    int i = 0 ;
    while ( *s == '-' ) s++;
    while ( !bResult && a[i]) {
        const char* A = a[i] ;
        while ( *A == '-' ) A++ ;
        bResult = stricmp(s,A)==0;
        i++;
    }
    return bResult;
}

int readFile(const char* path,Options /* options */)
{
    FILE* f     = fopen(path,"r");
    int nResult = f ? typeFile : typeUnknown;
    if (  f ) {
        const char* docs[] = { ".doc",".txt", nil };
        const char* code[] = { ".cpp",".h"  ,".pl" ,".py" ,".pyc", nil };
        const char*  ext   = strstr(path,".");
        if  ( ext ) {
            if ( sina(ext,docs) ) nResult = typeDoc;
            if ( sina(ext,code) ) nResult = typeCode;
        }
    }
    if ( f ) fclose(f) ;

    return nResult ;
}

Position* searchTimeDict(TimeDict_t& td, const time_t& time,long long delta)
{
    Position* result = NULL;
    for ( int t = 0 ; !result && t < delta ; t++ ) {
        for ( int x = 0 ; !result && x < 2 ; x++ ) {
            int T = t * ((x==0)?-1:1);
            if ( td.count(time+T) ) {
                result = &td[time+T];
                result->delta(T);
            }
        }
    }
    return result;
}

int getFileType(std::string& path,Options& options) { return getFileType(path.c_str(),options); }
int getFileType(const char* path,Options& options)
{
    return readXML  (path,options) ? typeXML
        :  readDir  (path,options) ? typeDirectory
        :  readImage(path,options) ? typeImage
        :  readFile (path,options)
        ;
}

int version(const char* program)
{
    printf("%s: %s %s\n",program,__DATE__,__TIME__);
    return 0;
}

int help(const char* program,char const* words[],int nWords,bool /*bVerbose*/)
{
    printf("usage: %s ",program);
    for ( int i = 0 ; i < nWords ; i++ ) {
        if ( words[i] )
            printf("%c-%s%s",i?'|':'{',words[i],i>(-kwNOVALUE)?" value":"");
    }
    printf("} path+\n");
    return 0;
}

int compare(const char* a,const char* b)
{
    int result=*a && *b;
    while ( result && *a && *b) {
        char A=*a++;
        char B=*b++;
        result=tolower(A)==tolower(B);
    }
    return result;
}

int find(const char* arg,char const* words[],int nWords)
{
    if ( arg[0] != '-' ) return kwSYNTAX;

    int result=0;
    int count =0;

    for ( int i = 0 ; i < nWords ; i++) {
        int j = 0 ;
        while ( arg[j] == '-' ) j++;
        if ( ::compare(arg+j,words[i]) ) {
            result = i ;
            count++;
        }
    }

    return count==1?result:kwSYNTAX;
}

int parseTZ(const char* adjust)
{
    int   h=0;
    int   m=0;
    char  c  ;
    try {
        sscanf(adjust,"%d%c%d",&h,&c,&m);
    } catch ( ... ) {} ;

    return (3600*h)+(60*m);
}

bool mySort(std::string a,std::string b)
{
    time_t A = readImageTime(a);
    time_t B = readImageTime(b);
    return (A<B);
}

int main(int argc,const char* argv[])
{
    int result=0;
    const char* program = argv[0];

    const char* types[typeMax];
    types[typeUnknown  ] = "unknown";
    types[typeDirectory] = "directory";
    types[typeImage    ] = "image";
    types[typeXML      ] = "xml";
    types[typeDoc      ] = "doc";
    types[typeCode     ] = "code";
    types[typeFile     ] = "file";

    char const* keywords[kwMAX];
    memset(keywords,0,sizeof(keywords));
    keywords[kwHELP    ] = "help";
    keywords[kwVERSION ] = "version";
    keywords[kwVERBOSE ] = "verbose";
    keywords[kwDRYRUN  ] = "dryrun";
    keywords[kwDST     ] = "dst";
    keywords[kwADJUST  ] = "adjust";
    keywords[kwTZ      ] = "tz";
    keywords[kwDELTA   ] = "delta";

    map<std::string,string> shorts;
    shorts["-?"] = "-help";
    shorts["-h"] = "-help";
    shorts["-v"] = "-verbose";
    shorts["-V"] = "-version";
    shorts["-d"] = "-dst";
    shorts["-a"] = "-adjust";
    shorts["-t"] = "-tz";
    shorts["-D"] = "-delta";
    shorts["-s"] = "-delta";
    shorts["-X"] = "-dryrun";

    Options options ;
    options.help    = sina(keywords[kwHELP   ],argv) || argc < 2;
    options.verbose = sina(keywords[kwVERBOSE],argv);
    options.dryrun  = sina(keywords[kwDRYRUN ],argv);
    options.version = sina(keywords[kwVERSION],argv);
    options.dst     = sina(keywords[kwDST    ],argv);
    options.dryrun  = sina(keywords[kwDRYRUN ],argv);

    for ( int i = 1 ; !result && i < argc ; i++ ) {
        const char* arg   = argv[i++];
        if ( shorts.count(arg) ) arg = shorts[arg].c_str();

        const char* value = argv[i  ];
        int        ivalue = ::atoi(value?value:"0");
        int         key   = ::find(arg,keywords,kwMAX);
        int         needv = key < kwMAX && key > (-kwNOVALUE);

        if (!needv ) i--;
        if ( needv && !value) key = kwNEEDVALUE;

        switch ( key ) {
            case kwDST      : options.dst     = true ; break;
            case kwHELP     : options.help    = true ; break;
            case kwVERSION  : options.version = true ; break;
            case kwDRYRUN   : options.dryrun  = true ; break;
            case kwVERBOSE  : options.verbose = true ; break;
            case kwTZ       : Position::tz_      = parseTZ(value);break;
            case kwADJUST   : Position::adjust_  = ivalue;break;
            case kwDELTA    : Position::deltaMax_= ivalue;break;
            case kwNEEDVALUE: fprintf(stderr,"error: %s requires a value\n",arg); result = resultSyntaxError ; break ;
            case kwSYNTAX   : default:
            {
                int  type   = getFileType(arg,options) ;
                if ( options.verbose ) printf("%s %s ",arg,types[type]) ;
                if ( type == typeImage ) {
                    time_t t    = readImageTime(std::string(arg)) ;
                    char*  path = realpath(arg,NULL);
                    if  ( t && path ) {
                        if ( options.verbose) printf("%s %ld %s",path,(long int)t,asctime(localtime(&t)));
                        gFiles.push_back(path);
                    }
                    if ( path ) :: free((void*) path);
                }
                if ( type == typeUnknown ) {
                    fprintf(stderr,"error: illegal syntax %s\n",arg);
                    result = resultSyntaxError ;
                }
                if ( options.verbose ) printf("\n") ;
            }break;
        }
    }

    if ( options.help    ) ::help(program,keywords,kwMAX,options.verbose);
    if ( options.version ) ::version(program);

    if ( !result ) {
        sort(gFiles.begin(),gFiles.end(),mySort);
        if ( options.dst ) Position::dst_ = 3600;
        if ( options.verbose ) {
            int t = Position::tz();
            int d = Position::dst();
            int a = Position::adjust();
            int A = Position::Adjust();
            int s = A     ;
            int h = s/3600;
                s-= h*3600;
                s = abs(s);
            int m = s/60  ;
                s-= m*60  ;
            printf("tz,dsl,adjust = %d,%d,%d total = %dsecs (= %d:%d:%d)\n",t,d,a,A,h,m,s);
        }
        for ( size_t p = 0 ; !options.dryrun && p < gFiles.size() ; p++ ) {
            std::string arg = gFiles[p] ;
            std::string stamp ;
            try {
                time_t t       = readImageTime(arg,&stamp) ;
                Position* pPos = searchTimeDict(gTimeDict,t,Position::deltaMax_);
                Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(gFiles[p]);
                if ( image.get() ) {
                    image->readMetadata();
                    Exiv2::ExifData& exifData = image->exifData();
#if 0
                    /*
					char* keys[]={ "Exif.Image.GPSTag"
						         , "Exif.GPSInfo.GPSProcessingMethod"
					             , "Exif.GPSInfo.GPSAltitudeRef"
							     , "Exif.GPSInfo.GPSVersionID"
                                 , "Exif.GPSInfo.GPSProcessingMethod"
                                 , "Exif.GPSInfo.GPSVersionID"
                                 , "Exif.GPSInfo.GPSMapDatum"
                                 , "Exif.GPSInfo.GPSLatitude"
                                 , "Exif.GPSInfo.GPSLongitude"
                                 , "Exif.GPSInfo.GPSAltitude"
                                 , "Exif.GPSInfo.GPSAltitudeRef"
                                 , "Exif.GPSInfo.GPSLatitudeRef"
                                 , "Exif.GPSInfo.GPSLongitudeRef"
                                 , "Exif.GPSInfo.GPSDateStamp"
                                 , "Exif.GPSInfo.GPSTimeStamp"
					};
					static int bPrint = true ;
					for ( int k = 0 ; k < 15 ;   k++ ) {
						try {
							if ( bPrint ) printf("erasing %s\n",keys[k]);
							Exiv2::ExifKey  key = Exiv2::ExifKey(keys[k]);
							Exiv2::ExifData::iterator kk = exifData.findKey(key);
							if ( kk != exifData.end() ) exifData.erase(kk);
						} catch (...) {};
					}
					bPrint = false;
                    */
#endif
#if 0
					Exiv2::ExifData::const_iterator end = exifData.end();
					for (Exiv2::ExifData::iterator i = exifData.begin(); i != end; ++i) {
						char name[100];
						strcpy(name,i->key().c_str());
						// std::cout << "sniff " << i->key() << std::endl;
						if ( strstr(name,"GPS") )  {
							Exiv2::ExifData::iterator pos;
							Exiv2::ExifKey exifKey = Exiv2::ExifKey(name);
							pos = exifData.findKey(exifKey);
							while( pos != exifData.end()) {
								exifData.erase(pos);
							}
						}
					}
#endif
					if ( pPos ) {
						/*
						   struct _stat buf;
   int result;
   char timebuf[26];
   char* filename = "crt_stat.c";
   errno_t err;

   // Get data associated with "crt_stat.c":
   result = _stat( filename, &buf );

   int _utime(
   const char *filename,
   struct _utimbuf *times
);
   */

                        exifData["Exif.GPSInfo.GPSProcessingMethod" ] = "65 83 67 73 73 0 0 0 72 89 66 82 73 68 45 70 73 88"; // ASCII HYBRID-FIX
                        exifData["Exif.GPSInfo.GPSVersionID"        ] = "2 2 0 0";
                        exifData["Exif.GPSInfo.GPSMapDatum"         ] = "WGS-84";

                        exifData["Exif.GPSInfo.GPSLatitude"         ] = Position::toExifString(pPos->lat(),true,true);
                        exifData["Exif.GPSInfo.GPSLongitude"        ] = Position::toExifString(pPos->lon(),true,false);
                        exifData["Exif.GPSInfo.GPSAltitude"         ] = Position::toExifString(pPos->ele());

                        exifData["Exif.GPSInfo.GPSAltitudeRef"      ] = pPos->ele()<0.0?"1":"0";
						exifData["Exif.GPSInfo.GPSLatitudeRef"      ] = pPos->lat()>0?"N":"S";
						exifData["Exif.GPSInfo.GPSLongitudeRef"     ] = pPos->lon()>0?"E":"W";

                        exifData["Exif.GPSInfo.GPSDateStamp"        ] = stamp;
                        exifData["Exif.GPSInfo.GPSTimeStamp"        ] = Position::toExifTimeStamp(stamp);
						exifData["Exif.Image.GPSTag"                ] = 4908;

						printf("%s %s % 2d\n",arg.c_str(),pPos->toString().c_str(),pPos->delta());
                    } else {
                        printf("%s *** not in time dict ***\n",arg.c_str());
                    }
                    image->writeMetadata();
				}
            } catch ( ... ) {};
        }
    }

    return result ;
}

// That's all Folks!
////
