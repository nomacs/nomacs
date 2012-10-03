#!/usr/bin/env python
# -*- coding: Latin-1 -*-

"""   setbuild - set the build environment you require  """

##
def syntax():
    "syntax - print syntax of setbuild.py "
    print "syntax: python setbuild.py Win32|x64|all|reset"
##

##
# import modules
import os.path
import sys
import xml.dom.minidom
from   xml.sax.saxutils import escape

##
# from PP3E/System/Filetools/cpall.py
maxfileload =   5 * 1024 * 1024
blksize     = 100 * 1024

##
def cp(pathFrom, pathTo, maxfileload=maxfileload):
    """
    copy file pathFrom to pathTo, byte for byte
    """
    if os.path.getsize(pathFrom) <= maxfileload:
        bytesFrom = open(pathFrom, 'rb').read()   # read small file all at once
        open(pathTo, 'wb').write(bytesFrom)       # need b mode on Windows
    else:
        fileFrom = open(pathFrom, 'rb')           # read big files in chunks
        fileTo   = open(pathTo,   'wb')           # need b mode here too 
        while 1:
            bytesFrom = fileFrom.read(blksize)    # get one block, less at end
            if not bytesFrom: break               # empty after last chunk
            fileTo.write(bytesFrom)
##

##
def save(path):
    """save - make a backup (or restore the backup)"""
    orig = path+'.orig'
    if os.path.exists(orig):
        cp(orig,path)
    else:
        cp(path,orig)
##

##
def sln(path,remove):
    """sln - remove the x64 stuff from a sln"""

    save(path)

    if remove:
    	f = open(path, 'r')
    	lines = []
    	for line in f:
    	    if line.find(remove)<0:
    	        lines.append(line)
    	f.close()

    	f = open(path,'w')
    	for line in lines:
    	    f.write(line)
    	f.close()
##

##
def vcproj(path,remove):
    """vcproj - remove the x64 stuff from a vcproj"""
    
    save(path)
    
    if remove:
		f = open(path,'r')
		dom = xml.dom.minidom.parseString(f.read())
		f.close()

		for tag in [ 'Platform','Configuration' ]:
			tags = dom.getElementsByTagName(tag)
			kills = []
			for t in tags:
				if t.getAttribute("Name").find(remove)>=0:
					kills.append(t)

			for kill in kills:
				kill.parentNode.removeChild(kill)

		# repair the command lines!
		for tool in dom.getElementsByTagName('Tool'):
			cl=tool.getAttribute('CommandLine')
			if cl:
				cl=escape(cl)
				cl=cl.replace('\r','__CR__')
				cl=cl.replace('\n','__LF__')
				tool.setAttribute('CommandLine',cl)
				# print '-----------',cl,'----------'

		code=dom.toxml()
		code=code.replace('__CR__','&#x0d;')
		code=code.replace('__LF__','&#x0a;')
		f = open(path,'w')
		f.write(code)
		f.close()
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
def setbuild(remove):
    """setbuild - remove == None, means both x64 and Win32"""
    if remove in set(['x64','Win32',None]):
        directory = os.path.abspath(os.path.dirname(sys.argv[0]))
        print "directory = ",directory
        global paths

        paths=[]
        os.path.walk(directory, visit, None)
        for path in paths:
            # print path
            handlers =  { '.sln'    : sln
                        , '.vcproj' : vcproj
                        } ;
            ext=os.path.splitext(path)[1]
            if handlers.has_key(ext):
                handlers[ext](path,remove)
    else:
        syntax()
##

##
if __name__ == '__main__':
    
    argc     = len(sys.argv)
    syntaxError = argc < 2
    
    if not syntaxError:
        option=sys.argv[1].lower()
        removes  = 	{ 'x64' 	: 'Win32' 
        			, 'win32' 	: 'x64'
        			, 'all' 	: None
        			, 'reset' 	: None
        			}
        syntaxError = not removes.has_key(option)
        if not syntaxError:
            setbuild(removes[option])
            
        if option=='reset':
        	os.system('del/s *.orig')

    if syntaxError:
        syntax()

# That's all Folks!
##
