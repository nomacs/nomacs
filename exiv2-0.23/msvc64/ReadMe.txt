exiv2\msvc64\ReadMe.txt
-----------------------

Updated: 2011-02-25 
This is work in progress.
However this is sufficiently robust to be in current use by a commercial licensee of exiv2.

1)   Working
     Build 32 bit and 64 bit builds of exiv2.exe (and libraries exiv2lib,expat,zlib,xmpsdk).
     Builds with VS2005, 2008 and 2010.
     Supports zlib-1.2.5 or zlib-1.2.3
     setbuild.py "doctors" the project files for DevStudio Express
     buildall.bat batch building script
     runner.py build verification script (and binaries/code for depends{32|64}.exe)

2)   TODO (in priority order)
     a) Build sample/test applications (exifprint etc)
     b) Use .vsprop files to hold "common" project settings

     Assistance appreciated if you wish to become involved.

Robin Mills
http://clanmills.com
email: robin@clanmills.com

####
T A B L E  o f  C O N T E N T S

1    Build Instructions
1.1  Tools
1.2  Install zlib and expat sources.
1.3  Open exiv2\msvc64\exiv2.sln
1.4  What is build
1.5  Building with zlib1.2.3
1.6  Express editions of DevStudio (or 32 bit only builds, or 64 bit only builds)

2    Design
2.1  Architecture
2.2  Changing the version of expat/zlib/xmpsdk
2.3  Relationship with msvc build environment

3    Acknowledgment of prior work
3.1  Differences between inherited project and the exiv2 projects

4    Batch builds and tests
4.1  buildall.bat
4.2  runner.py

## End Table of Contents End ##
####

1    Build Instructions

1.1  Tools
     This has been tested with the "Pro" versions of VS 2005/08/10
     Express editions don't support 64 bit builds, however it is
     possible to build 32 bit libraries with "Express".
     See notes below about DevStudio Express and building only Win32 or x64 builds

1.2  Install zlib and expat sources.
     I use the directory c:\gnu for this purpose, however the name isn't important.

     c:\gnu>dir
      Directory of c:\gnu
     2010-12-05  10:05    <DIR>             exiv2                <--- this tree
     2010-12-02  21:06    <DIR>             expat-2.0.1          <--- "vanilla" expat 2.0.1 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.5           <--- "vanilla" zlib 1.2.5 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.3 OPTIONAL  <--- "vanilla" zlib 1.2.3 source tree
     c:\gnu>

     The URLs from which to obtain zlib and expat are documented in exiv2\msvc\README-MSVC.txt
     expat-2.0.1 is available from http://voxel.dl.sourceforge.net/sourceforge/expat/expat-2.0.1.tar.gz
     zlib-1.2.5  is available from http://zlib.net/zlib-1.2.5.tar.gz

1.3  Open exiv2\msvc64\exiv2.sln
     Projects are zlib, expat, xmpsdk, exiv2lib, exiv2
     Build/Batch build...  Select All, Build
     - 5 projects       (zlib, expat, xmpsdk, exiv2lib, exiv2)
     x 2 Platforms      (x64|Win32)
     x 4 Configurations (Debug|Release|DebugDLL|ReleaseDLL)
     = 5x2x4 = 40 builds.

     If you haven't installed the x64 compiler, don't select the 64 bit configurations!
     You may have to hand-edit the vcproj and sln files to hide the 64 bit information.
     See the notes about DevStudio Express for more information about this.

     Build time is 15 minutes on a 2.2GHz Duo Core and consumes 1.0 gBytes of disk space.

1.4  What is built
     The DLL builds use the DLL version of the C runtime libraries
     The Debug|Release builds use static C runtime libraries
     This is discussed in exiv2\msvc\README-MSVC.txt 

1.5  Building with zlib1.2.3
     By default, msvc64 builds with zlib-1.2.5
     You can build with zlib1.2.3.  To do this:

     1) copy/y exiv2\msvc64\zlib123\zlib.vcproj exiv2\msvc64\zlib\zlib.vcproj

     2) You will need the sources in a directory parallel to exiv2
     c:\gnu>dir
      Directory of c:\gnu
     2010-12-05  10:05    <DIR>             exiv2                <--- this tree
     2010-12-02  21:06    <DIR>             expat-2.0.1          <--- "vanilla" expat 2.0.1 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.5           <--- "vanilla" zlib 1.2.5 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.3 OPTIONAL  <--- "vanilla" zlib 1.2.3 source tree
     c:\gnu>
     Please see exiv2\msvc\README-MSVC.txt for information about obtaining zlib-1.2.3

     3) Alter the "AdditionalIncludeDirectories" in the following projects:
        xmpsdk.vcproj
        exiv2lib.vcproj
        exiv2.vcproj
        There are 8 occurances of  "AdditionalIncludeDirectories" in each of the 3 projects.

        If you have both 1.2.3 and 1.2.5 in your build tree, you'll probably "get away with" not
        making this change - however this is due to good luck and you should really adjust the vcprojects.

1.6  Express editions of DevStudio (or 32 bit only builds, or 64 bit only builds)
     Express does not provide a 64 bit compiler.
     You can build 32 bit libraries with DevStudio Express (with a little effort)

     Before loading the project, use the python script setbuild.py to select Win32:

     	c:\gnu\exiv2\msvc64>setbuild.py Win32

     setbuild.py is none destructive.  If you have a 64 bit compiler, you can:
     1)	Restore the build environment with:   setbuild.py all
     2) Select x64 bit builds only with:      setbuild.py x64

     If you don't have python available (it's a free download from ActiveState.com), 
     you can "doctor" to project files manually to remove mentiosn of X64 using an editor:

     Cleanup your tree and edit the files.
     cd exiv2\msvc64
     call cleaner.bat
     for /r %f in (*.vcproj) do notepad %f
     for /r %f in (*.sln)    do notepad %f

     I personally don't recommend notepad for any purpose at all.
     I use TextPad http://www.textpad.com/  Notepad++ is also good.

     DevStudio Express 2010 does not have the "Batch Build" feature.
     Select "exiv2" right-click "Set as Startup Project" and
     Select Platform="Win32" Configuration="Debug|DebugDLL|Release|ReleaseDLL"  Build.
     Build the Configurations you need.  Build time is about 2 minutes/Configuration.

     To remove the "memory" of old configurations:
     setbuild.py reset

2    Design

     expat and zlib do not provide 64 bit builds for DevStudio,
     so it has been necessary to create the build environments for exiv2.
     However, we don't include the source code for zlib or expat - only the build environment.
     You are expected to install the "vanilla" expat and zlib libraries
     in a directory at the same level as exiv2.
     I personally always build in the directory c:\gnu,
     however the name/location/spaces of the build directory are all irrelevant,
     it's only the relative position of expat-2.0.1 and zlib-1.2.5 that matter.
     The names expat-2.0.1 and zlib-1.2.5 fixed (and used by the .vcproj files)

     zlib and expat
     exiv2\msvc64\zlib\zlib.{sln|vcproj}                    DevStudio files
     ..\..\..\zlib-1.2.5\                                   Source code

     exiv2\msvc64\expat\expat.sln expat.vcproj              DevStudio files
     ..\..\..\expat-2.0.1\                                  Source code

2.1  Architecture
     There are directories for every component:
     The libraries: zlib, expat, xmpsdk, exiv2lib
     Applications:  exiv2 (exifprint.exe and other sample apps will be added on request)

     For each component, there are three build directories:
     exiv2lib\build                                         intermediate results
     exiv2lib\Win32\{Debug|Release|DebugDLL|ReleaseDLL}     32 bit builds
     exiv2lib\x64\{Debug|Release|DebugDLL|ReleaseDLL}       64 bit builds

     Final builds and include directories (for export to "foreign" projects)
     bin\{win32|x84}\Win32\{Debug|Release|DebugDLL|ReleaseDLL} 
     include

2.2  Changing the version of expat/zlib/xmpsdk
     At the moment, zlib contains references to zlib-1.2.3 and expat to expat-2.0.1
     It is planned to support zlib 1.2.5 shortly and these notes will be updated at that time.

2.3  Relationship with msvc build environment
     msvc64 is similar to msvc.
     However there are significant differences:
     1) msvc64 supports 64 bit and 32 bit builds
     2) msvc64 provides projects to build expat and zlib
     3) msvc64 is designed to accomodate new versions of expat and zlib when they become available.
     4) msvc64 supports DevStudio 2005, 2008 and 2010 (no support for 2003)
     5) msvc64 does not require you to build 'vanilla' expat and zlib projects in advance
     6) msvc64 does not support the organize application
     7) msvc64 does not build the sample/test applications (such as exfprint.exe)
     8) msvc64 has no test/build verification scripts
     9) msvc64 supports building with zlib1.2.5 (default) or zlib1.2.3

     msvc will continue to be supported for 32 bit builds using DevStudio 2003/05/08,
     however there is no plan to enhance or develop msvc going forward.

3    Acknowledgement of prior work
     This work is based on work by the following people:
     zlib 64 bit build

	    Copyright (C) 2000-2004 Simon-Pierre Cadieux.
	    Copyright (C) 2004 Cosmin Truta.
	    For conditions of distribution and use, see copyright notice in zlib.h.

	    And was obtained from:  http://www.achacha.org/vc2008_zlib/

     expat 64 bit build
     http://www.start64.com/index.php?option=com_content&task=view&id=3461&Itemid=114

	 I recognise and respect the work performed by those individuals.

3.1  Differences between inherited projects and the exiv2 projects
     There is no compatiblity.

4    Batch builds and tests

4.1  buildall.bat
     This was intended to be a "throw away" kind of script and it's grown to be quite useful.
     You will have to run vcvars32.bat for the compiler you intend to use to ensure devenv is
     on your path.

     It doesn't know anything about building only x64 or only Win32. Change the script if you
     want something special.

     I'll probably combine buildall.bat and setbuild.py into a single script one day.

4.2  runner.py
	 runner.py [Win32|x64|all]

	 This script runs some basic "sanity" checks on the build.  You should compare the
         output of runner.py with the reference output runner.txt.  
         diff/windiff/winmergeu - or whatever your favorite diff tool.

	 python runner.py all > new.txt
	 winmergeu.exe new.txt runner.txt

	 If you have only build Win32 (or x64), you'll have to remove the output from
         runner.txt for the target that isn't of interest.

# That's all Folks!
##
