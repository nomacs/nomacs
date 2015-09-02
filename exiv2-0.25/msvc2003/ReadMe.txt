Notes about msvc2003 32 bit build of exiv2
------------------------------------------

+-----------------------------------------------------------+
| msvc2003 builds 32bit binaries                            |
|          with Visual Studio 2003 .Net                     |
|                                                           |
| msvc2005 builds 32bit and 64bit binaries                  |
|          with Visual Studio 2005/8/10/12/13/14            |
+-----------------------------------------------------------+

Updated: 2015-01-12

Tools
-----

Created and Tested  With:   VC7.1 (Visual Studio .Net)

All builds in msvc2003 are 32bit.

1)  Build environments (solution files)
    exiv2.sln           - this builds the exiv2 libraries   (static and dynamic)
                          and the utility and test programs (exiv2.exe, exifprint.exe)
                          
                          DEPENDS on zlib-1.2.3 and expat-2.0.1
            

How to build and test exiv2
----------------------------

1)  Dependancies for exiv2
    You need expat-2.0.1 AND zlib-1.2.3 which you can download from
    http://expat.sourceforge.net/ and http://zlib.net/, respectively.
    expat-2.0.1, zlib-1.2.3 and exiv2 should be in the same directory

    I usually use c:\gnu, however the name 'c:\gnu' does not matter
    nor does it have to be a topmost directory.

    c:\gnu\expat-2.0.1    <---- vanilla expat
    c:\gnu\zlib-1.2.3     <---- vanilla zlib
    c:\gnu\exiv2          <---- this directory
    
    You can download the libraries from http://clanmills.com/files/exiv2libs.zip (20mb)
    This archive includes libraries for use by msvc2003 and msvc2005.
    
    If you unzip exiv2libs.zip into c:\exiv2libs, the batch file msvc2003/copylibs.zip
    will copy the files from c:\exiv2libs to the correct location.

2)  Build expat. See expat documents for details.

    If you have copied expat-2.0.1 from the archive, you can skip this step.

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

    If you have copied zlib-1.2.3 from the archive, you can skip this step.

    This is what I do:
    a) Open zlib\zlib-1.2.3\projects\visualc6\zlib.dsw with DevStudio
       This converts the zlib-1.2.3 VC6 workspace to DevStudio project files
       Say "Yes" to everything.

    b) Build/Batch Build/Select
       { zlib } {DLL Debug | DLL Release | LIB Debug | LIB Release } (4 targets)
       Build
      
    Rattle roll.... less than a minute ...... snap, crackle, pop (lots of warnings)
    ========== Build: 4 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

4)  Open the sln file exiv2\msvc2003\exiv2.sln

5)  Build/Batch Build/Select All/Build

    This will build all the necessary libraries and all the test programs.
    At the end of building, you should see the beautiful output:

    ========== Build: 104 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
    
    The build takes 6 minutes on my iMac with a 2.8GHz Duo Core Intel Processor
    and 4GBytes of Ram running XP/Pro SP3.

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

    Use the bash/Cygwin (or msys) script test/testMSVC.sh to run the test suite.
    Please see msvc2005/ReadMe.txt for more information.

Where are the libraries and executables?
----------------------------------------

exiv2\msvc2003\bin\{Debug | DebugDLL | Release | ReleaseDLL }

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

Information/Documentation      : http://www.exiv2.org/index.html
Discussion Forum               : http://dev.exiv2.org/wiki/exiv2
The MSVC2003 build environment : Please contact me directly by email.

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

# That's all Folks
##
