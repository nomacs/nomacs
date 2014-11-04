exiv2\msvc2005\ReadMe.txt
-------------------------

+-----------------------------------------------------------+
| msvc2012 builds 32bit and 64bit binaries                  |
|          with Visual Studio 2012                          |
| msvc2005 builds 32bit and 64bit binaries                  |
|          with Visual Studio 2005/8/10                     |
| msvc2003 builds 32bit binaries                            |
|          with Visual Studio 2003/5/8                      |
+-----------------------------------------------------------+

Updated: 2013-07-22 

Robin Mills
http://clanmills.com
email: robin@clanmills.com

####
T A B L E  o f  C O N T E N T S

1    Build Instructions
1.1  Tools
1.2  Install zlib and expat sources.
1.3  Open exiv2\msvc2005\exiv2.sln
1.4  What is build
1.5  Building with zlib1.2.3 (or 1.2.5) and expat 2.0.1
1.6  Express editions of DevStudio (or 32 bit only builds, or 64 bit only builds)
1.7  Support for DevStudio 11

2    Design
2.1  Architecture
2.2  Relationship with msvc build environment

3    Acknowledgment of prior work
3.1  Differences between inherited project and the exiv2 projects

4    Batch builds and tests
4.1  buildall.bat
4.2  runner.py
4.3  Running the test suite

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
     2012-05-04  23:35    <DIR>             expat-2.1.0          <--- "vanilla" expat 2.1.0 source tree
     2012-05-04  23:35    <DIR>             zlib-1.2.7           <--- "vanilla" zlib  1.2.7 source tree
     2010-12-02  21:06    <DIR>             expat-2.0.1 OPTIONAL <--- "vanilla" expat 2.0.1 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.5  OPTIONAL <--- "vanilla" zlib  1.2.5 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.3  OPTIONAL <--- "vanilla" zlib  1.2.3 source tree
     c:\gnu>

     The URLs from which to obtain zlib and expat are documented in exiv2\msvc2003\ReadMe.txt
     expat-2.1.0 is available from http://voxel.dl.sourceforge.net/sourceforge/expat/expat-2.1.0.tar.gz
     zlib-1.2.7  is available from http://zlib.net/zlib-1.2.7.tar.gz

1.3  Open exiv2\msvc2005\exiv2.sln
     Projects are zlib, expat, xmpsdk, exiv2lib, exiv2, addmoddel etc...
     Build/Batch build...  Select All, Build
     - 29 projects      (zlib, expat, xmpsdk, exiv2lib, exiv2, addmoddel etc)
     x 2 Platforms      (x64|Win32)
     x 4 Configurations (Debug|Release|DebugDLL|ReleaseDLL)
     = 29x2x4 = 232 builds.

     If you haven't installed the x64 compiler, don't select the 64 bit configurations!
     You may have to hand-edit the vcproj and sln files to hide the 64 bit information.
     See the notes about DevStudio Express for more information about this.

     Build time is 20 minutes on a 2.2GHz Duo Core and consumes 3.0 gBytes of disk space.

1.4  What is built
     The DLL builds use the DLL version of the C runtime libraries
     The Debug|Release builds use static C runtime libraries
     This is discussed in exiv2\msvc2003\ReadMe.txt 

1.5  Building with zlib1.2.5 (or 1.2.3) and/or expat 2.0.1
     By default, msvc2005 builds with zlib-1.2.7 and expat 2.1.0
     You can build with zlib1.2.3.  To do this:

     1) copy/y exiv2\msvc2005\zlib123\zlib.vcproj   exiv2\msvc2005\zlib\
        copy/y exiv2\msvc2005\expat201\expat.vcproj exiv2\msvc2005\expat

     2) You will need the sources in a directory parallel to exiv2
     c:\gnu>dir
      Directory of c:\gnu
     2010-12-05  10:05    <DIR>             exiv2                <--- this tree
     2012-05-04  23:35    <DIR>             expat-2.1.0          <--- "vanilla" expat 2.1.0 source tree
     2010-12-02  21:06    <DIR>             expat-2.0.1          <--- "vanilla" expat 2.0.1 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.7           <--- "vanilla" zlib 1.2.7 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.3 OPTIONAL  <--- "vanilla" zlib 1.2.3 source tree
     c:\gnu>
     Please see exiv2\msvc2003\ReadMe.txt for information about obtaining zlib-1.2.3

     3) Alter the "AdditionalIncludeDirectories" in the following projects:
        xmpsdk.vcproj
        exiv2lib.vcproj
        exiv2.vcproj
        There are 8 occurances of  "AdditionalIncludeDirectories" in each of the 3 projects.

        If you have both 1.2.3 and 1.2.5 in your build tree, you'll probably "get away with" not
        making this change - however this is due to good luck and you should really adjust the vcprojects.
        
     4) Alter "AdditionalIncludeDirectories" for expat 2.0.1

1.6  Express editions of DevStudio (or 32 bit only builds, or 64 bit only builds)
     Express does not provide a 64 bit compiler.
     You can build 32 bit libraries with DevStudio Express (with a little effort)

     Before loading the project, use the python script setbuild.py to select Win32:

        c:\gnu\exiv2\msvc2005>setbuild.py Win32

     setbuild.py is none destructive.  If you have a 64 bit compiler, you can:
     1) Restore the build environment with:   setbuild.py all
     2) Select x64 bit builds only with:      setbuild.py x64

     If you don't have python available (it's a free download from ActiveState.com), 
     you can "doctor" to project files manually to remove mentiosn of X64 using an editor:

     Cleanup your tree and edit the files.
     cd exiv2\msvc2005
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
     
1.7  Support for DevStudio 11
     I have successfully built and tested a sub-set of exiv2/msvc2005 with DevStudio 11 beta.
     
     I have no plan to support beta versions of DevStudio.
     
     I will need to purchase DevStudio 11 when it's available to support exiv2.
     This is a private purchase and will only be used to support exiv2.
     I would appreciate a donation of a legal copy of DevStudio 11.
     (Any Microsoft employees/contractors with staff software purchase privileges?)

2    Design

     expat and zlib1.2.5 (and earlier) do not provide 64 bit builds for DevStudio.
     
     The projects provided for zlib1.2.7 support 64 bit builds, however it didn't
     work cleanly for me.  They use different projects for VC9 and VC10.
     They don't provide support for VC8 or 11beta.
     
     I have created build environments for zlib and expat within exiv2/msvc2005.
     I don't include the source code for zlib or expat - only the build environment.
     
     You are expected to install the "vanilla" expat and zlib libraries
     in a directory at the same level as exiv2.
     I personally always build in the directory c:\gnu,
     however the name/location/spaces of the build directory are all irrelevant,
     it's only the relative position of expat-2.1.0 and zlib-1.2.7 that matter.
     The names expat-2.1.0 and zlib-1.2.7 are fixed (and used by the .vcproj files)

     zlib and expat
     exiv2\msvc2005\zlib\zlib.vcproj                          DevStudio files
     ..\..\..\zlib-1.2.7\                                   Source code

     exiv2\msvc2005\expat\expat.vcproj                        DevStudio files
     ..\..\..\expat-2.1.0\                                  Source code

2.1  Architecture
     There are directories for every component:
     The libraries: zlib, expat, xmpsdk, exiv2lib
     Applications:  exiv2.exe
     Sample Apps:   exifprint.exe, addmoddel.exe and many more (used by test suite)

     For each component, there are three build directories:
     exiv2lib\build                                         intermediate results
     exiv2lib\Win32\{Debug|Release|DebugDLL|ReleaseDLL}     32 bit builds
     exiv2lib\x64\{Debug|Release|DebugDLL|ReleaseDLL}       64 bit builds

     Final builds and include directories (for export to "foreign" projects)
     bin\{win32|x84}\Win32\{Debug|Release|DebugDLL|ReleaseDLL} 
     include

2.2  Relationship with msvc2003 build environment
     msvc2005 is similar to msvc2003.
     However there are significant differences:
     1) msvc2005 supports 64 bit and 32 bit builds
     2) msvc2005 provides projects to build expat and zlib
     3) msvc2005 is designed to accomodate new versions of expat and zlib when they become available.
     4) msvc2005 supports DevStudio 2005, 2008 and 2010 (no support for 2003)
     5) msvc2005 does not require you to build 'vanilla' expat and zlib projects in advance
     6) msvc2005 does not support the organize application
     7) msvc2005 supports building with zlib1.2.7 (default) or zlib1.2.3/5
     7) msvc2005 supports building with expat2.1.0 (default) or expa2.0.1

     msvc2003 will continue to be supported for 32 bit builds using DevStudio 2003/05/08,
     however there is no plan to enhance or develop msvc2003 going forward.

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

4.2  runner.py
     runner.py [Win32|x64|all]

     This script runs some basic "sanity" checks on the build.  You should compare the
         output of runner.py with the reference output runner.txt.  
         diff/windiff/winmergeu - or whatever your favorite diff tool.

     python runner.py all > new.txt
     winmergeu.exe new.txt runner.txt

     If you have only build Win32 (or x64), you'll have to remove the output from
         runner.txt for the target that isn't of interest.
         
4.3  Running the test suite
     You will need to install cygwin to run the test suite.
     
     This is a two stage process:
     Step1:  Build exiv2 for cygwin and run the test suite
             Typical Unix type build:
             make config
             ./configure --disable-visibility
             export PKG_CONFIG_PATH=$PWD/config
             make clean ; make ; make samples ; make install ; cd test ; make test
             
     Step2:  set the environment variable EXIV2_BINDIR appropriately and rerun make test
             export EXIV2_BINDIR=<path-to-directory-with-exiv2.exe>

             I find the following little bash loop very useful.  You should test
             against all the directories in the msvc2005/bin directory:
             for d in $(find /c/gnu.2005/exiv2/msvc2005/bin -name exiv2.exe -exec dirname {} ";"); do
                export EXIV2_BINDIR=$d
                echo ---------------------------------
                echo ---- $d ----
                echo ---------------------------------
                make test
            done
            
    Free gift: (you get what you pay for)
            The script testMSVC.sh to runs this loop for you.
            The script verifyMSVC.sh validates the output of testMSVC.sh
            I've added those for my convenience and I hope you'll find them useful.

    And to pass the time (the test suite takes about an hour to run)
            I recommend running listdlls exiv2.exe occasionally during testing to be confident
            that the test suite is running the MSVC built versions of exiv2 and libraries.
            
            From cygwin:
            while sleep 1 do; listdlls exiv2.exe ; done
            or
            while sleep 10 do ; listdlls exiv2.exe | grep exiv2.exe ; done
            
    Note: Cygwin currently ships diff-utils 2.9.2 which treats binary files differently
    from 2.8.7 (on Mac) and 3.2 (on Ubuntu 12.04).  For this reason, the executable (and
    support dlls) for GNU diff.exe 2.8.7 is provided in msvc2003/diff.exe.
    The test suite has been "doctored" on cygwin to modify the path appropriately to
    use this preferred version of diff.exe.
             
# That's all Folks!
##
