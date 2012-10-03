Notes about msvc 32 bit build of exiv2
--------------------------------------

+------------------------------------------------------+
| This version of the build environment builds 32 bit  |
| builds of exiv2 for DevStudio 2003 (VS7.1) and later |
|                                                      |
| Please use the project files in msvc64 for 32 and 64 |
| bit builds in VS2005 and later.                      |
|                                                      |
| Please see msvc64\ReadMe.txt for more information    |
+------------------------------------------------------+

Tools
-----

Created With:   VC9    (VS/2008 Std)
Tested  With:   VC7.1, VC8 Pro +SP1, VC9Std running Windows/XP 32bit)
                VC10 running on Windows/7 64bit, and Windows/XP 32bit
               (VC7.1 = VS/2003 .Net; VC8 = VS/2005; VC9 = VS/2008; VC10 = VS/2010)
               
I recommend you use the msvc64 environment for VS2010.
VC10/VS 2010 requires a project change which the conversion wizard doesn't handle.  See below.
              
All builds in msvc are 32bit.  Use msvc64 for 64bit and 32bit builds.

1)  Build environments (solution files)
    exiv2.sln           - this builds the exiv2 libraries   (static and dynamic)
                          and the utility and test programs (exiv2.exe, exifprint.exe)
                          
                          DEPENDS on zlib and expat-2.0.1
                          Builds with VC9,VC8 and VC7.1, VC10
            
    exiv2+organize.sln  - builds the same targets as exiv2
                          PLUS the organize.exe command-line program
                          
                          DEPENDS on zlib and expat-2.0.1 and boost_1_37_0
                          Builds with VC9 and VC8
                          DOES NOT BUILT WITH VC7.1
                          
                          set Environment string BOOST_ROOT=c:\boost_1_37_0

Notes for DevStudio 2010 Users
------------------------------

A new setting "TargetName"=$(ProjectName) has been added to DevStudio 2010.  There is only one project impacted by this:

Select Project "exiv2lib" in Project Explorer.  Right-Click/Properties

+ Configuration Properties\General

exiv2lib.vcproj
Target Name | exiv2sd		Debug
			| exiv2d		DebugDLL
			| exiv2s		Release
			| exiv2       	ReleaseDLL

If you don't add these setting, you will get 1,000s of errors.  The damming error concerns:
"Name of output is not equal to target name"  (or something obscure such as this).

This matter has been reported to Microsoft.
http://msdn.microsoft.com/EN-US/library/E0846D4D-2CE1-48E3-B219-674FF070BF4E#11

How to build and test exiv2 (with/without organize.exe)
-------------------------------------------------------

1)  Dependancies for exiv2
    You need expat-2.0.1 AND zlib-1.2.3 which you can download from
    http://expat.sourceforge.net/ and http://zlib.net/, respectively.
    expat-2.0.1, zlib and exiv2 should be in the same directory

    I usually use c:\gnu, however the name 'c:\gnu' does not matter
    nor does it have to be a topmost directory.

    c:\gnu\expat-2.0.1    <---- vanilla expat
    c:\gnu\zlib-1.2.3     <---- vanilla zlib
    c:\gnu\exiv2          <---- this directory

2)  Build expat. See expat documents for details.
    This is what I do:
    a) Open expat-2.0.1\expat.dsw with DevStudio
       This converts the expat-2.0.1 VC6 work space to DevStudio project files
       Say "Yes" to everything.

    b) Build/Batch Build/Select
       { expat | expat-static } { debug | release } (4 targets)
       / Build

    Rattle roll.... less than a minute ...... rattle grrrrrrrr rump.
    ========== Build: 4 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
    
3)  Build zlib.  See zlib documents for details.
    This is what I do:
    a) Open zlib\zlib-1.2.3\projects\visualc6\zlib.dsw with DevStudio
       This converts the zlib-1.2.3 VC6 workspace to DevStudio project files
       Say "Yes" to everything.

    b) Build/Batch Build/Select
       { zlib } {DLL Debug | DLL Release | LIB Debug | LIB Release } (4 targets)
       Build
      
    Rattle roll.... less than a minute ...... snap, crackle, pop (lots of warnings)
    ========== Build: 4 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

4)  Open the sln file exiv2\msvc\exiv2.sln
    On VS2005 and up, the Wizard will offer to upgrade the project files.
    Once more, say yes to everything.

5)  Build/Batch Build/Select All/Build

    This will build all the necessary libraries and all the test programs.
    At the end of building, you should see the beautiful output:

    ========== Build: 104 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
    
    The build takes 6 minutes on my iMac with a 2.8GHz Duo Core Intel Processor
    and 4GBytes of Ram. (VS 2005 and VS 2008)

    And about 16 minutes on my elderly 2003 laptop (VS 2003 .Net)
    (DeLL Inspiron 8500 2.2GHz Pentium and 1.25GBytes of Ram)

    Both machines are running XP/Pro SP3.

    +--------------------------------------------+
    |         DISK SPACE WARNING                 |
    +--------------------------------------------+

    The total build consumes 1.0GBytes of disk space.
    There are 112 targets (4 expats, 4 zlibs and 108 exiv2 targets)
    The pdb (debugging) files consume 300MB.
    The debug exiv2 static library alone (exiv2.lib) is 35MB

    A more modest build is to build exiv2
    { Debug | Release | DebugDLL | ReleaseDLL }
    This consumes: 400MB
    
    A minimum build is to build exiv2/Release
    This consumes: 100MB

6)  Test from the Command Line (cmd.exe):
    
    cd exiv2\msvc
    C:\gnu\exiv2\msvc>runner.bat > new.txt
    exiv2.exe: An action must be specified
    exiv2.exe: At least one file is required
    metacopy.exe: Read and write files must be specified
    bla bla white sheep......

    C:\gnu\exiv2\msvc>diff.exe   runner.txt  new.txt (Windows/XP)
    C:\gnu\exiv2\msvc>diff.exe   runner7.txt new.txt (Windows/7)
    --- no differences should be reported --
    
    The test takes less than 1 minute and runs 200+ different test programs.
    
How to build exiv2+organize
---------------------------

1)  boost

    organize has been built with boost_1_37_0
    It may build successfully with other versions
    Please send an email to record your experience to help other succeed (or avoid frustration)

    +---------------------------------------------------------------------------+
    | MSVC 7.1 Users:  You can't build organize with VC7.1 and BOOST 1_37_0     |
    | You'll get the following compiler error:                                  |
    | c:\boost_1_37_0\boost\format\feed_args.hpp(203) :                         |
    |             fatal error C1001: INTERNAL COMPILER ERROR                    |
    | (compiler file 'msc1.cpp', line 2701)                                     |
    | Please choose the Technical Support command on the Visual C++             |
    | Help menu, or open the Technical Support help file for more information   |
    |                                                                           |
    | If anyone is able to successfully build with VC7.1 + service pack + boost |
    | please share your successful experience with us - email address below     |
    | and let me know the version of BOOST and the Service Pack Level of VC7.1  |
    | I am using:                                                               |
    | Microsoft (R) 32-bit C/C++ Standard Compiler Version 13.00.9466 for 80x86 |
    | DevStudio Version 7.1.3088 (Service pack level unknown)                   |
    |                                                                           |
    | VisualC++ 2008 Express Edition (VC9) is a free download from Microsoft    |
    | and can successfully build exiv2+organize.sln                             |
    +---------------------------------------------------------------------------+

2)  Download or build boost
EITHER (recommended)
    download and install boost_1_37_0 using the BoostPro Computing Wizard
    http://www.boostpro.com/products/free

    You'll need:
    boost headers
    regex
    system
    filesystem
    program options
    
    +-------------------------------------------------------+
    | DISK SPACE WARNING                                    |
    | A full installation of all boost for vc7.1, vc8 & vc9 |
    | occupies an amazing 4.5GBytes                         |
    | boost offers 3 compilers (vc7.1, 8 and 9)             |
    | 8 flavors {MTdll, MT, MTs, STs} x {Debug|Release}     |
    | 22 libraries (regex, filesystem etc)                  |
    +-------------------------------------------------------+
    
    And here are the libraries which are linked with the different versions (for vc9 = VS/2008)
    Configuration: ReleaseDLL Win32
    libboost_regex-vc90-mt-1_37.lib
    libboost_filesystem-vc90-mt-1_37.lib
    libboost_system-vc90-mt-1_37.lib
    libboost_program_options-vc90-mt-1_37.lib

    Configuration: DebugDLL Win32
    libboost_regex-vc90-mt-gd-1_37.lib
    libboost_filesystem-vc90-mt-gd-1_37.lib
    libboost_system-vc90-mt-gd-1_37.lib
    libboost_program_options-vc90-mt-gd-1_37.lib

    Configuration: Release Win32
    libboost_regex-vc90-mt-s-1_37.lib
    libboost_filesystem-vc90-mt-s-1_37.lib
    libboost_system-vc90-mt-s-1_37.lib
    libboost_program_options-vc90-mt-s-1_37.lib

    Configuration: Debug Win32
    libboost_regex-vc90-mt-sgd-1_37.lib    
    libboost_filesystem-vc90-mt-sgd-1_37.lib
    libboost_system-vc90-mt-sgd-1_37.lib
    libboost_program_options-vc90-mt-sgd-1_37.lib
    
    I've added an appendix to explain the boost library name conventions.

OR (if you prefer to build from source)
    build boost_1_37_0 from source
    http://www.boost.org/
    unzip into c:\boost_1_37_0
    c:\boost_1_37_0\>bjam release
    
    I believe you can build everything with the command:
    bjam --build-type=complete release && bjam --build-type=complete debug
    
    You can download bjam from here:
    http://sourceforge.net/project/showfiles.php?group_id=7586&package_id=72941
    
    +------------------------------------------------------+
    | I used the wonderful BoostPro Computing Wizard to    |
    | install prebuilt libraries - so I didn't build from  |
    | source.  If you'd like to share your know-how about  |
    | this, please email me and I will updated these notes |
    | and of course acknowledge your contribution          |
    +------------------------------------------------------+
    
    The default source build does not build all the necessary targets.
    The build machinery for boost is documented here:
    http://www.boost.org/doc/libs/1_37_0/more/getting_started/windows.html
    
    I've added an appendix to explain the boost library name conventions.

3)  build dependances expat-2.0.1 and zlib-1.2.3 (see above)


4)  Open exiv2+organize.sln in DevStudio

    +--------------------------------------------------------+
    | Set the environment string BOOST_ROOT=c:\boost_1_37_0  |
    | (or where you put boost)                               |
    +--------------------------------------------------------+
    
    You'll have to set the environment string BEFORE you start DevStudio.
    Environment strings are set in the process when it is started.
    Or you could update the DevStudio search paths in Tools/Options.

5)  Batch Build/Batch Build... Select all/Build

    This time when you should see:
    
    ========== Build: 108 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

    Congratulations.

Where are the libraries and executables?
----------------------------------------

exiv2\msvc\bin\{Debug | DebugDLL | Release | ReleaseDLL }

What's been built?
------------------

Static and dynamic version of exiv2, expat, zlib and the exiv2 sample programs
Debug and Release versions of all targets.

The static libraries link with MT (MTd for Debug).
The objects are built to use the Static C runtime Libraries.

DLL targets link dynamically
- exiv2.exe requires libexpat.dll, exiv2.dll and zlib1.dll (zlib1d.dll for debug)
- the objects use the dynamic c runtime dlls (MSVCR70.dll and friends)

Linking Applications with the exiv2 libraries
---------------------------------------------

The following 'flavors' have been build:

a) DLLs + MD build and work

DebugDLL| ReleaseDLL = MD{d}  : link exiv2{d}.lib, xmpsdk.lib, libexpat.lib and zlib1{d}.lib
          Runtime DLLS        :      exiv2{d}.dll, libexpat.dll, zlib1{d}.dll and MSVC{R|P}{_0D.dll)
          _ = 7 for VS2003, 8 for VS2005 and 9 for VS2008
          The MSVC*.dll's are on most machines and a free download from Microsoft
                        
b) Static + MT build and work

Debug | Release      = MT{d} :  link exiv2s{d}.lib, libexpatMT.lib, zlib{d}.lib xmpsdk.lib 
          Runtime DLLs       : none

c) Static + MD will work (but not built)

During the development of the build environment, this was working.
However it was dropped from the release because expat-2.0.1 does not support an MD build.

When you choose to link your application with the exiv2 library, can choose any of the 3 configurations.  Please use Google to find additional advice about which flavor is best for your needs.

d) DLLs + MT WILL NOT FUNCTION even if you build it

Do not build DLLs + MT.  Exiv2.dll exports STL template objects which will crash when linked with the MT run-time.  This is because Exiv2.dll is linked with it's own (static) c-runtime library.  Exiv2.exe is linked with a different c-runtime library.  The two cannot co-exist.

This is not a bug.  There is no workaround.


Bugs
----

Please consult the issue database:
http://dev.exiv2.org/projects/exiv2/issues

Support and Questions
---------------------

Information/Documentation  : http://www.exiv2.org/index.html
Discussion Forum           : http://dev.exiv2.org/wiki/exiv2
The MSVC build environment : Please contact me directly by email.

Robin Mills
robin@clanmills.com
http://clanmills.com

APPENDIX

The following topics are documented for additional information about the status of the build.

A) Build warning messages
B) Partial Build errors
C) Debugging exiv2 library code
D) Boost library name conventions

A) Build warning messages
-------------------------

The following warning and message are part of the build:

1) exiv2.lib(xmlparse.obj) : warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/INCREMENTAL:NO' specification
   To eliminate this message, you will have to modify project settings in expat-2.0.1 and rebuild expat.

2) zlib.lib(zutil.obj) : warning LNK4217: locally defined symbol _malloc imported in function _zcalloc
   I believe this is coming from zlib which uses the local (MD) c-runtime library.
   It's harmess (although frightening)
   
B) Partial Build errors
-----------------------

Andreas assures me that if you cannot build some targets, eg exifprint/ReleaseDLL on a clean VC71 machine.  I havne't been able to reproduced this.  Please drop me an email if you find any of those.  I'm cross-eyed building different flavors of this on different version of DevStudio.

C) Debugging exiv2 library code
-------------------------------

You may wish to debug the exiv2 library code using one of the test utilities.  Examples: exiv2.exe and exifprint.exe.  You'll find this a little frustrating because the exiv2.sln file contains all the test programs.  When you change the library code, DevStudio will relink all 20 test programs.  To avoid this frustration, unload the project files which you are no using.  For example, if you are debugging exiv2.exe, you only require the following projects to be loaded:

xmpsdk
exiv2lib
exiv2

(when exiting DevStudio, say "No" when it asks you if you wish to save changes to exiv2.sln)

D) Boost library name conventions

    The library naming convention is:    
    lib
    Prefix: only ordinary static libraries use the lib prefix; import libraries and DLLs do not.

    boost_regex
    Library name: all boost library filenames begin with boost_.
    -vc71
    Toolset tag: identifies the toolset and version used to build the binary.
    -mt
    Threading tag: indicates that the library was built with multithreading support enabled.
    Libraries built without multithreading support can be identified by the absence of -mt.
    -d
    ABI tag: encodes details that affect the library's interoperability with other compiled code. For each such feature, a single letter is added to the tag:

    Key Use this library when:
    s   linking statically to the C++ standard library and compiler runtime support libraries.
    g   using debug versions of the standard and runtime support libraries.
    y   using a special debug build of Python.
    d   building a debug version of your code.
    p   using the STLPort standard library rather than the default one supplied with your compiler.
    n   using STLPort's deprecated ìnative iostreamsî feature.8
    For example, if you build a debug version of your code for use with debug versions of the static runtime library and the STLPort standard library in ìnative iostreamsî mode, the tag would be: -sgdpn. If none of the above apply, the ABI tag is ommitted. 

    -1_34
    Version tag: the full Boost release number, with periods replaced by underscores. For example, version 1.31.1 would be tagged as "-1_31_1".
    .lib
    Extension: determined according to the operating system's usual convention. On most unix-style platforms the extensions are .a and .so for static libraries (archives) and shared libraries, respectively. On Windows, .dll indicates a shared library and (except for static libraries built by the gcc toolset, whose names always end in .a) .lib indicates a static or import library. Where supported by toolsets on unix variants, a full version extension is added (e.g. ".so.1.34") and a symbolic link to the library file, named without the trailing version number, will also be created.

-- end --
