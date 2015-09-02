exiv2\msvc2005\ReadMe.txt
-------------------------

+-----------------------------------------------------------+
| msvc2005 builds 32bit and 64bit binaries                  |
|          with Visual Studio 2005/8/10/12/13/14            |
| msvc2003 builds 32bit binaries                            |
|          with Visual Studio 2003/5/8                      |
+-----------------------------------------------------------+

Updated: 2015-05-08

Robin Mills
http://clanmills.com
email: robin@clanmills.com

####
T A B L E  o f  C O N T E N T S

1    Build Instructions
1.1  Tools
1.2  Install zlib and expat sources.
1.3  Open exiv2\msvc2005\exiv2.sln
1.4  Configuring build options (such as video and webready)
1.5  Building with exiv2-webready.sln
1.6  What is build
1.7  Express editions of DevStudio (or 32 bit only builds, or 64 bit only builds)

2    Design
2.1  Architecture
2.2  Relationship with msvc build environment

3    Batch builds and tests
3.1  buildall.bat
3.2  Running the test suite

4    Building Applications to use Exiv2
4.1  Recommended work-flow to create applications
4.2  Compiler Include Paths and Options
4.3  Library Link Order
4.4  tools/bin/depends32.exe and depends64.exe
4.5  exiv2.exe --verbose --version

5    Acknowledgment of prior work
5.1  Differences between inherited project and the exiv2 projects

## End Table of Contents End ##
####

1    Build Instructions

1.1  Tools
     This has been tested with the "Pro" versions of VS 2005/08/10/12
     Some Express editions don't support 64 bit builds
     however it is  possible to build 32 bit libraries with "Express".
     See notes below about DevStudio Express and building only Win32 or x64 builds

     You need a DOS version of perl to build openssl.  Not the cygwin version.
     I use ActiveState Perl.

     configure.bat is used to configure "non-standard" version of exiv2
     configure.bat requires a python3 interpreter.  I use ActivePython.
     ActivePython 3.4.1.0 (ActiveState Software Inc.) based on
     Python 3.4.1 (default, Aug  7 2014, 13:09:27) [MSC v.1600 64 bit (AMD64)] on win32

     You need Cygwin to run the test suite because it is written in bash.

1.2  Install zlib and expat sources.
     I use the directory c:\gnu for this purpose, however the name isn't important.

     c:\gnu>dir
     Directory of c:\gnu
     2010-12-05  10:05     <DIR>    exiv2     <--- this tree
     2012-05-04  23:35     <DIR>    expat     <--- "vanilla" expat   2.1.0  source tree
     2012-05-04  23:35     <DIR>    zlib      <--- "vanilla" zlib    1.2.7  source tree
     2012-05-04  23:35     <DIR>    curl      <--- "vanilla" curl    7.39.0 source tree
     2012-05-04  23:35     <DIR>    openssl   <--- "vanilla" openssl 1.0.1j source tree
     2012-05-04  23:35     <DIR>    libssh    <--- "vanilla" libssh  0.5.5  source tree
     c:\gnu>

     You can obtain the libraries from http://clanmills.com/files/exiv2libs.zip (20mb)
     I copy those to the directory c:\exiv2libs
     The script msvc2005/copylibs.bat will copy them from c:\exiv2libs to the correct location

     11/05/2014  07:26 AM  <DIR>  curl-7.39.0
     12/07/2014  09:18 AM  <DIR>  expat-2.1.0
     12/17/2014  09:40 AM  <DIR>  libssh-0.5.5
     12/17/2014  09:38 AM  <DIR>  openssl-1.0.1j
     12/07/2014  09:18 AM  <DIR>  zlib-1.2.7

     The following directories are also in the archive for use by msvc2003
     01/07/2015  11:11 AM  <DIR>  expat-2.0.1
     01/07/2015  11:10 AM  <DIR>  zlib-1.2.3

     The URLs from which to obtain zlib and expat are documented in exiv2\msvc2003\ReadMe.txt
     expat-2.1.0 is available from http://voxel.dl.sourceforge.net/sourceforge/expat/expat-2.1.0.tar.gz
     zlib-1.2.7  is available from http://zlib.net/zlib-1.2.7.tar.gz
     curl        is available from http://curl.haxx.se/download.html
     openssh     is available from https://www.openssl.org/source/
     libssh      is available from https://www.libssh.org/get-it/

1.3  Open exiv2\msvc2005\exiv2.sln
     Projects are zlib, expat, xmpsdk, exiv2lib, exiv2, addmoddel etc...
     Build/Batch build...  Select All, Build
     - 36 projects      (zlib, expat, xmpsdk, exiv2lib, exiv2, addmoddel etc)
     x 2 Platforms      (x64|Win32)
     x 4 Configurations (Debug|Release|DebugDLL|ReleaseDLL)
     = 38x2x4 = 304 builds.

     When building with webready, you add 5 libraries for a total of 344 builds.

     If you haven't installed the x64 compiler, don't select the 64 bit configurations!
     You may have to hand-edit the vcproj and sln files to hide the 64 bit information.
     See the notes about DevStudio Express for more information about this.

     Build time is 20 minutes on a 2.2GHz Duo Core and consumes 3.0 gBytes of disk space.
     Build time with webready is of the order of one hour as we add 5 libraries.
     (libcurl, libeay32, ssleay32, libssh, openssl)

1.4  Configuring build options (such as video and webready)

     The batch file configure.bat is used to reconfigure for various options.
     You will need a python3 interpreter.  I personally use ActivePython 3.4.1

	 C:\cygwin64\home\rmills\gnu\exiv2\trunk\msvc2005>configure -h
	 Usage: configure.py [options]

	 Options:
	  -h, --help            show this help message and exit
	  -A, --with-app        build sample apps (false)
	  -a, --without-app     do not build apps
	  -C, --with-curl       enable curl
	  -c, --without-curl    disable curl
	  -E, --with-expat      enable expat (true)
	  -e, --without-expat   disable expat
	  -O, --with-openssl    enable openssl
	  -o, --without-openssl
							disable openssl
	  -S, --with-ssh        enable ssh
	  -s, --without-ssh     disable ssh
	  -T, --with-test       build test programs (true)
	  -t, --without-test    do not build test progs
	  -W, --enable-webready
							enable webready (false)
	  -w, --disable-webready
							enable webready
	  -V, --enable-video    enable video (false)
	  -v, --disable-video   disable video
	  -X, --enable-xmp      enable xmp (true)
	  -x, --disable-xmp     disable xmp
	  -Z, --with-zlib       enable zlib/png (true)
	  -z, --without-zlib    disable zlib/png
	  -d, --default         default

	 When you run configure.bat it creates a new solution exiv2_configure.sln
	 configure.bat does not modify the solution or project files used by exiv2.sln
	 Instead it creates a parallel set of projects files.  For example exivprint\exifprint_configure.vcproj

1.5  Building with exiv2-webready.sln
	 The solution exiv2-webready.sln was used during development before
	 we created the configure.bat script to generate solution/project files
	 We have decided to ship this build mechanism for v0.25 only.

     Building the complete library with webready support requires building
     5 additional libraries.  This is time consuming.  The build time
     increases from 5 to 20 minutes.

     By default, you will not build with webready.

     To build with webready:
     1 copy include\exiv2\exv_msvc-webready.h include\exiv2\exv_msvc.h
     2 open msvc2005\exiv2-webready.vcproj

1.6  What is built
     The DLL builds use the DLL version of the C runtime libraries
     The Debug|Release builds use static C runtime libraries
     This is discussed in exiv2\msvc2003\ReadMe.txt

1.7  Express editions of DevStudio (or 32 bit only builds, or 64 bit only builds)
     Some Express Editions do not provide a 64 bit compiler.
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
     ..\..\..\zlib                                            Source code

     exiv2\msvc2005\expat\expat.vcproj                        DevStudio files
     ..\..\..\expat                                           Source code

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
     bin\{win32|x64}\Win32\{Debug|Release|DebugDLL|ReleaseDLL}

2.2  Relationship with msvc2003 build environment
     msvc2005 is similar to msvc2003.
     However there are significant differences:
     1) msvc2005 supports 64 bit and 32 bit builds
     2) msvc2005 provides projects to build expat, zlib, curl, libssh and openssl
     3) msvc2005 is designed to accomodate new versions of expat and zlib when they become available.
     4) msvc2005 supports DevStudio 2005 and later (no support for 2003)
     5) msvc2005 does not require you to build 'vanilla' expat and zlib projects in advance
     6) msvc2005 does not support the organize application
     7) msvc2005 supports building with zlib1.2.7 or 1.2.8
     8) msvc2005 supports building with expat2.1.0 or expat2.0.1

     msvc2003 will continue to be supported for 32 bit builds using DevStudio 2003/05/08,
     however there is no plan to enhance or develop msvc2003 going forward.

3    Batch builds and tests

3.1  buildall.bat
     This was intended to be a "throw away" kind of script and it's grown to be quite useful.
     You will have to run vcvars32.bat for the compiler you intend to use to ensure devenv is
     on your path.

     It doesn't know anything about building only x64 or only Win32. Change the script if you
     want something special.

3.2  Running the test suite
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
             for d in $(find /c/gnu.2005/exiv2/msvc2005/bin \
                        -name exiv2.exe -exec dirname {} ";"); do
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
            I recommend running listdlls exiv2.exe occasionally during testing to be
            confident that the test suite is running the MSVC built
            versions of exiv2 and libraries.

            From cygwin:
            while sleep 1 do; listdlls exiv2.exe ; done
            or
            while sleep 10 do ; listdlls exiv2.exe | grep exiv2.exe ; done

     Note: Cygwin currently ships diff-utils 2.9.2 which treats binary files differently
     from 2.8.7 (on Mac) and 3.2 (on Ubuntu 12.04).  For this reason, the executable (and
     support dlls) for GNU diff.exe 2.8.7 is provided in msvc2003/diff.exe.
     The test suite has been "doctored" on cygwin to modify the path appropriately to
     use this preferred version of diff.exe.

4    Building Applications to use Exiv2

4.1  Recommended work-flow to create applications

     a) add your code code to a sample applications (such as exifprint.exe)
        get the code building and running from there.
        our program will be called exifprint.exe
        you will be very confident that your code works!

     b) copy <exiv2dir>/msvc2005/exifprint to <exiv2dir>/msvc2005/yourprojectname
        get yourprojectname.exe to build and run.
        By using <exiv2dir>/msvc2005/exifprint as a template,:
            i) you can build 32/64 dll/static debug/release "out of the box".
           ii) your include path, dependent libraries and link order are already set.
        Revert your changes to <exiv2dir>/msvc2005/exifprint

     c) move <exiv2dir>/msvc2005/yourprojectname in your buildtree.

     d) It's a good idea for your buildtree to reference <exiv2dir>/msvc2005
        When you reference <exiv2dir>, you can update <exiv2dir> occasionally
        and rebuild with little effort.

     Of course, there are always other ways to achieve your goals in software.
     You may prefer to copy the generated libs and dlls in bin/$platform/$configuration
     to your build tree.  When you do this, you will have to explicitly link
     the libraries you have copied.

4.2  Compiler Include Paths and Options

     The correct include paths and compiler options are revealed in Visual Studio,
     by examining exifprint/Properties/C++/Command Line:

     /Od
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\../include"
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\../include/exiv2"
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\../xmpsdk/include"
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\../../expat/lib"
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\../../zlib"
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\/../../curl/include"
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\/../../libssh/include"
     /I "C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\xssl/Win32/Release/include"
     /D "WIN32" /D "_NDEBUG" /D "_CONSOLE" /D "BUILD_GETOPT" /D "EXV_HAVE_DLL" /D "_MBCS"
     /Gm /EHsc /MD /Fo"build/x64/ReleaseDLL\\" /Fd"build/x64/ReleaseDLL\vc80.pdb"
     /FR"build/x64/ReleaseDLL\\" /W3 /nologo /c /Zi /TP /errorReport:prompt</pre>The correct library link order

4.3  Library Link Order

     The correct order is revealed in Visual Studio,
     by examining exifprint/Properties/Linker/Command-Line:

     /OUT:"C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin/x64/ReleaseDLL\exifprint.exe"
     /INCREMENTAL:NO /NOLOGO /MANIFEST /MANIFESTFILE:"build/x64/ReleaseDLL\exifprint.exe.intermediate.manifest"
     /SUBSYSTEM:CONSOLE /MACHINE:X64 /ERRORREPORT:PROMPT kernel32.lib user32.lib
     gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
     "..\bin\x64\releasedll\libexiv2.lib" "..\bin\x64\releasedll\xmpsdk.lib"
     "..\bin\x64\releasedll\libexpat.lib" "..\bin\x64\releasedll\zlib1.lib"

     I don't believe you have to explicitly link msvc{r|p}XXX as they are linked automatically by the version of Visual Studio.
     800=Visual Studio 2005, 900=2008, 100=2010, 110=2012, 120=2013.

4.4  tools/bin/depends32.exe and depends64.exe

     C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin\x64\releasedll>depends64 exiv2.exe
     libexpat.dll
     PSAPI.DLL
     NSI.dll
     API-MS-Win-Core-DelayLoad-L1-1-0.dll
     API-MS-Win-Core-Interlocked-L1-1-0.dll
     RPCRT4.dll
     API-MS-Win-Core-LocalRegistry-L1-1-0.dll
     msvcrt.dll
     WS2_32.dll
     zlib1.dll
     API-MS-Win-Security-Base-L1-1-0.dll
     API-MS-Win-Core-Profile-L1-1-0.dll
     API-MS-Win-Core-Util-L1-1-0.dll
     API-MS-Win-Core-Fibers-L1-1-0.dll
     API-MS-Win-Core-ErrorHandling-L1-1-0.dll
     API-MS-Win-Core-Debug-L1-1-0.dll
     API-MS-Win-Core-String-L1-1-0.dll
     API-MS-Win-Core-ProcessEnvironment-L1-1-0.dll
     API-MS-Win-Core-Localization-L1-1-0.dll
     API-MS-Win-Core-SysInfo-L1-1-0.dll
     API-MS-Win-Core-Misc-L1-1-0.dll
     API-MS-Win-Core-NamedPipe-L1-1-0.dll
     API-MS-Win-Core-LibraryLoader-L1-1-0.dll
     API-MS-Win-Core-ThreadPool-L1-1-0.dll
     API-MS-Win-Core-IO-L1-1-0.dll
     API-MS-Win-Core-File-L1-1-0.dll
     API-MS-Win-Core-Synch-L1-1-0.dll
     API-MS-Win-Core-Handle-L1-1-0.dll
     API-MS-Win-Core-Memory-L1-1-0.dll
     API-MS-Win-Core-Heap-L1-1-0.dll
     API-MS-Win-Core-ProcessThreads-L1-1-0.dll
     KERNELBASE.dll
     ntdll.dll
     API-MS-Win-Core-RtlSupport-L1-1-0.dll
     KERNEL32.dll
     libexiv2.dll
       Not found: MSVCR80.dll
       Not found: MSVCP80.dll
     exiv2.exe
       Not found: MSVCR80.dll
       Not found: MSVCP80.dll

     C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin\x64\releasedll>

4.5  exiv2.exe --verbose --version

     You can inspect build information using exiv2 -v -V (verbose version):

     C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin\x64\releasedll>exiv2 -v -V
     exiv2 0.25 001900 (64 bit build)
     Copyright (C) 2004-2015 Andreas Huggel.

     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License
     as published by the Free Software Foundation; either version 2
     of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public
     License along with this program; if not, write to the Free
     Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301 USA
     exiv2=0.25.0
     platform=windows
     compiler=MSVC
     bits=64
     dll=1
     debug=0
     version=8.00
     date=Feb  1 2015
     time=21:45:35
     svn=3592
     ssh=0
     curl==0
     id=$Id: version.cpp 3564 2015-01-11 21:38:40Z robinwmills $
     executable=C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin\x64\releasedll\exiv2.exe
     library=C:\Windows\SYSTEM32\ntdll.dll
     library=C:\Windows\system32\kernel32.dll
     library=C:\Windows\system32\KERNELBASE.dll
     library=C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin\x64\releasedll\libexiv2.dll
     library=C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin\x64\releasedll\zlib1.dll
     library=C:\Windows\WinSxS\amd64_microsoft.vc80.crt_1fc8b3b9a1e18e3b_8.0.50727.6195_none_88e41e092fab0294\MSVCR80.dl
     library=C:\Windows\system32\msvcrt.dll
     library=C:\Windows\system32\WS2_32.dll
     library=C:\Windows\system32\RPCRT4.dll
     library=C:\Windows\system32\NSI.dll
     library=C:\Windows\WinSxS\amd64_microsoft.vc80.crt_1fc8b3b9a1e18e3b_8.0.50727.6195_none_88e41e092fab0294\MSVCP80.dl
     library=C:\Windows\system32\PSAPI.DLL
     library=C:\cygwin64\home\rmills\gnu\exiv2\video-write\msvc2005\bin\x64\releasedll\libexpat.dll
     have_regex=0
     have_strerror_r=0
     have_gmtime_r=0
     have_inttypes=0
     have_libintl=0
     have_lensdata=1
     have_iconv=0
     have_memory=0
     have_memset=0
     have_lstat=0
     have_stdbool=0
     have_stdint=0
     have_stdlib=0
     have_strlib=0
     have_strchr=0
     have_strerror=0
     have_strerror_r=0
     have_strings_h=0
     have_strtol=0
     have_mmap=0
     have_munmap=0
     have_sys_stat=0
     have_timegm=0
     have_unistd_h=0
     have_sys_mman=0
     have_libz=1
     have_xmptoolkit=1
     have_bool=0
     have_strings=0
     have_sys_types=0
     have_unistd=0

     The keys library=path are based on the actual libraries loaded in memory by exiv2.exe
     The output of exiv2.exe -v -V is used by the test suite to verify that we are using
     the correct libraries and not some other bandits which happen to reside on the host machine.

5    Acknowledgement of prior work
     This work is based on work by the following people:
     zlib 64 bit build

        Copyright (C) 2000-2004 Simon-Pierre Cadieux.
        Copyright (C) 2004 Cosmin Truta.
        For conditions of distribution and use, see copyright notice in zlib.h.

        And was obtained from:  http://www.achacha.org/vc2008_zlib/

     expat 64 bit build
     http://www.start64.com/index.php?option=com_content&task=view&id=3461&Itemid=114

     I recognise and respect the work performed by those individuals.

5.1  Differences between inherited projects and the exiv2 projects
     There is no compatiblity.

# That's all Folks!
##
