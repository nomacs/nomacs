#!/usr/bin/python3

##
# tee.py
# This little script was an effort tsort the dependancies
# I don't actually need it because I've used depend.py to extract
# the dependency table from exiv2-webready.sln and pasted it into configure.py
##

import os
import sys
import uuid
import optparse
import datetime
import platform
import xml.dom.minidom

class GraphError(Exception):
    pass

def tsort(partials):
    """
      Copyright Nathan Hurst 2001
      Licenced under the LGNU GPL.  see COPYING for details.
    """
    preds = {}
    succs = {}
    output = []
    distinct_value = (0,) # Julian Satchell suggested I use a unique
                          # identifier, rather than None as my sentinel
    for i in partials:
        prev = distinct_value;
        for j in i:
            if(not j in preds):
                preds[j] = 0;

            if(not j in succs):
                succs[j] = {};
            if(not prev is distinct_value):
                if(not prev in succs):
                    succs[prev] = {};
                if(not j in succs[prev]):
                    succs[prev][j] = 1;
                    preds[j] = preds[j] + 1;
            prev = j

    starts = list(map(lambda a: a[0], filter(lambda a: (a[1] == 0), preds.items())))
    while(len(starts) > 0):
        start = starts[0]
        starts = starts[1:] # prune off start
        output = output + [start];
        for i in succs[start].keys():
            preds[i] = preds[i] - 1;
            if(preds[i] == 0):
                starts.append(i)
        del succs[start];
    result=[]
    for i in reversed(output):
    	result.append(i)
    return result

def empty():
	return set([])

ignore	 = set(['expat'		   , 'expat201'	 , 'expat210'	, 'tests'   , 'testv'
			   ,'zlib123'	   , 'zlib125'	 , 'zlib127'	, 'tools'   , 'exiv2lib'
			   ])

##
# build dict:uid - hunt the tree for .vcproj files
uid = {}
for d in os.listdir('.'):
	if os.path.isdir(d) & (not d in ignore):
		for root, dirs, files in os.walk(d):
			for file in files:
				ext = ".vcproj"
				if file.endswith(ext) & (file.find('configure') < 0):
					uid[d]=str(uuid.uuid1())


externlib= set(['libcurl'      , 'libexpat'  , 'zlib'       , 'libcurl'
               ,'libeay32'     , 'ssleay32'  , 'libssh'     , 'openssl'
               ])


##
# define project dependances
# TODO: read exiv-webready.sln to build project/dependency set
project = {}
# no dependancy
for p in externlib:
	project[p]=empty()

##
# dependancies
project['xmpparser-test'	] = set(['libexiv2','xmpsdk'			 ])
project['xmpparse'			] = set(['libexiv2','xmpsdk'			 ])
project['xmpsample'			] = set(['libexiv2','xmpsdk'			 ])
project['xmpsdk'			] = set([			'libexpat'			 ])
project['geotag'			] = set([			'libexpat','libexiv2'])
project['libexiv2'			] = set([			'libexpat','xmpsdk'	 ,'zlib','libcurl','libeay32','ssleay32','libssh','openssl'])
project['libcurl'			] = set(['libeay32','ssleay32','libssh'	 ])
project['libssh'			] = set(['libeay32','ssleay32','openssl' ])
project['libeay32'			] = set(['openssl' ])
project['ssleay32'			] = set(['openssl' ])
project['openssl'           ] = set(['libcurl' , 'libssh'      ])
project['libexiv2'			] = set(['openssl' ,'zlib','xmpsdk'])

##
# all others depend on libexiv2
for p in uid:
	if not p in project:
		project[p]=set(['libexiv2'])

##
# tsort the dependencies
edges=[]
for p in project:
	for d in project[p]:
		edges.append([p,d])

print(edges)
print('-------------')
print(tsort(edges))

# That's all folks
##
