#!/usr/bin/env python
# -*- coding: Latin-1 -*-

##
def syntax():
    print "syntax: python runner.py Win32|x64|all"
##

r"""runner.py - run some tests on the exiv2 build"""

##
import sys
import os.path

##
def Q(path):
    return '"' + path + '"'
##    

##
def exe(path,option):
    """exe - handle a .exe file"""
    
#   print "testing ",path

    testimages=os.path.realpath('testimages')
    tif=os.path.join(testimages,'test.tiff')
    png=os.path.join(testimages,'test.png')
    jpg=os.path.join(testimages,'test.jpg')

    os.system(path + " -V")
    os.system(path + " -pt "+Q(tif) + '2>NUL | grep Original')
    os.system(path + " -pt "+Q(png) + '2>NUL | grep Original')
    os.system(path + " -pt "+Q(jpg) + '2>NUL | grep Original')
    os.system(path + " -pt "+Q(jpg) )
##

##
def dll(path,option):
    """dll - handle a .dll file"""
    
#   print "testing ",path

    bits=32 if path.find('Win32')>=0 else 64

    depends='tools/bin/depends%d.exe' % (bits)
    depends=os.path.realpath( depends )
    os.system(depends + ' -q ' + path + ' | sort')
##

##
def visit(myData, directoryName, filesInDirectory):          # called for each dir 
    """visit - called by os.path.walk"""
    # print "in visitor",directoryName, "myData = ",myData
    # print "filesInDirectory => ",filesInDirectory
    for filename in filesInDirectory:                        # do non-dir files here
        pathname = os.path.join(directoryName, filename)
        if not os.path.isdir(pathname):
            global paths
            paths.append(pathname)
##

##
def handle(paths,handlers):
    for path in sorted(paths):
        ext=os.path.splitext(path)[1].lower()
        if handlers.has_key(ext):
            handlers[ext](path,option)
##

##
def runner(option):
    """runner -option == None, means both x64 and Win32"""
    if option in set(['x64','Win32',None]):
        directory = os.path.abspath(os.path.dirname(sys.argv[0]))
        directory = os.path.join(directory,"bin")
        if option:
            directory = os.path.join(directory,option)

        global paths

        paths=[]
        os.path.walk(directory, visit, None)
        handle(paths,{ '.exe' : exe } )
        handle(paths,{ '.dll' : dll } )
        handle(paths,{ '.exe' : dll } )
    else:
        syntax()
##

##
if __name__ == '__main__':
    
    argc     = len(sys.argv)
    syntaxError = argc < 2
    
    if not syntaxError:
        option='all'
        if argc>1:
            option=sys.argv[1].lower()
        options  =  { 'x64'     : 'x64'
                    , 'x86'     : 'Win32'
                    , 'win32'   : 'Win32'
                    , 'all'     :  None
                    , 'both'    :  None
                    }
        syntaxError = not options.has_key(option)
        if not syntaxError:
            runner(options[option])

    if syntaxError:
        syntax()

# That's all Folks!
##
