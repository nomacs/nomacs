This is a native version of the unix command rm
It can delete files and trees
rm.exe -rf [directory|file] +

I've added this for use by the script jenkins_build.bat
We have to remove the support libraries which are copied during the build
I gave up on rmdir and del
becase the file openssl/NUL isn't removed a causes havoc.

http://hg.mozilla.org/users/jford_mozilla.com/jhford-native-rm/file/84a240176b93/rm.cpp

Code is built
call vcvars32.bat
nmake rm.exec

I provide the 32-bit version for use on Win32 and x64 platforms in tools/bin/rm.exe

Robin Mills
robin@clanmills.com
2014-12-18

