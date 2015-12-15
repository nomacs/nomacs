#!/usr/bin/python3

import os
import sys
import uuid
import optparse
import datetime
import platform
import xml.dom.minidom

def empty():
	return set([])

global uid			   # dict: UID of every projects
global project		   # dict: dependency sets for every project
global strings		   # dict: Visual Studio Strings
global ignore		   # set:  projects/directories to ignore
global filters		   # dict: patterns of filenames to be removed from the build
global filter		   # set:  patterns of filenames to be removed from the build
global externlib	   # set:  project to never filter (zlib etc)
global format		   # str:  output format
global build           # set:  projects to be built

format   = '%-20s\t%s'
externlib= set(['libcurl'      , 'libexpat'  , 'zlib'       , 'libcurl'
               ,'libeay32'     , 'ssleay32'  , 'libssh'     , 'openssl'
               ])
ignore	 = set(['expat'		   , 'expat201'	 , 'expat210'	, 'tests'   , 'testv'
			   ,'zlib123'	   , 'zlib125'	 , 'zlib127'	, 'tools'   , 'exiv2lib'
			   ])
apps	 = set(['exifdata'	   , 'exifvalue' , 'geotag'		 , 'xmpsample'	   ])
tests	 = set(['exifdata-test', 'conntest'	 , 'convert-test', 'easyaccess-test', 'exifcomment'	  , 'httptest'
			  , 'iotest'	   , 'iptceasy'	 , 'iptcprint'	 , 'iptctest'		, 'key-test'	  , 'largeiptc-test'
			  , 'mmap-test'	   , 'path-test' , 'prevtest'	 , 'remotetest'		, 'stringto-test' , 'taglist'
			  , 'tiff-test'	   , 'utiltest'	 , 'werror-test' , 'write-test'		, 'write2-test'	  , 'xmpparse'
			  , 'xmpparser-test'
			  ])
# always = set(['exiv2'		   , 'exiv2json' , 'addmoddel'	 , 'exifprint'		, 'metacopy' ])
build=empty()

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

##
# define project dependances
# This table is generated with the command 'tools/python/depends.py exiv2-webready.sln format'
project = {}
for p in uid:
	project[p]=empty()

project['addmoddel'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['conntest'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['convert-test'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['easyaccess-test'	] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['exifcomment'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['exifdata'			] = set(['libexiv2'])
project['exifdata-test'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['exifprint'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['exifvalue'			] = set(['libexiv2'])
project['exiv2'				] = set(['libeay32', 'ssleay32', 'libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['exiv2json'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['geotag'			] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['httptest'			] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['iotest'			] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['iptceasy'			] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['iptcprint'			] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['iptctest'			] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['key-test'			] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['largeiptc-test'	] = set(['zlib', 'libexpat', 'libexiv2', 'libcurl', 'xmpsdk'])
project['libcurl'			] = set(['libssh', 'libeay32', 'ssleay32'])
project['libeay32'			] = set(['openssl'])
project['libexiv2'			] = set(['libeay32', 'ssleay32', 'libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libssh'])
project['libexpat'			] = set([ ])
project['libssh'			] = set(['zlib', 'libeay32', 'ssleay32'])
project['metacopy'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['mmap-test'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['openssl'			] = set([ ])
project['path-test'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['prevtest'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['remotetest'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['ssleay32'			] = set(['openssl'])
project['stringto-test'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['taglist'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['tiff-test'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['utiltest'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['werror-test'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['write-test'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['write2-test'		] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['xmpparse'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['xmpparser-test'	] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['xmpsample'			] = set(['libcurl', 'zlib', 'libexpat', 'xmpsdk', 'libexiv2', 'libssh'])
project['xmpsdk'			] = set([ ])
project['zlib'				] = set([ ])
##
# filter
filters={}
filters['zlib'				] = set(['png'	])
filters['video'				] = set(['video'])
filters['xmp'				] = set(['xmp'	])
filters['webready'			] = set([		])
filter=empty()

##
# MSVC strings
strings = {}
strings['UID'	] = str(uuid.uuid1())

strings['Begin' ]='''Microsoft Visual Studio Solution File, Format Version 9.00
# Visual Studio 2005
''' + '# Created by:%s at:%s using:%s on:%s in:%s\n' % (sys.argv[0], datetime.datetime.now().time(), platform.node(), platform.platform(), os.path.abspath('.'))
strings['End'				] = ''

strings['globalBegin'		] = 'Global'
strings['globalEnd'			] = 'EndGlobal'

strings['platforms'			] = '''
\tGlobalSection(SolutionConfigurationPlatforms) = preSolution
\t\tDebug|Win32 = Debug|Win32
\t\tDebug|x64 = Debug|x64
\t\tDebugDLL|Win32 = DebugDLL|Win32
\t\tDebugDLL|x64 = DebugDLL|x64
\t\tRelease|Win32 = Release|Win32
\t\tRelease|x64 = Release|x64
\t\tReleaseDLL|Win32 = ReleaseDLL|Win32
\t\tReleaseDLL|x64 = ReleaseDLL|x64
\t\tEndGlobalSection
'''
strings['postSolutionBegin' ] = '\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n'
strings['postSolutionEnd'	] = '\tEndGlobalSection\n'

strings['postProjectBegin'	] = '\tProjectSection(ProjectDependencies) = postProject\n'
strings['postProjectEnd'	] = '\tProjectSectionEnd\n'

strings['preSolution'		] = '''\tGlobalSection(SolutionProperties) = preSolution
\t\tHideSolutionNode = FALSE
\tEndGlobalSection
'''
##
# {831EF580-92C8-4CA8-B0CE-3D906280A54D}.Debug|Win32.ActiveCfg = Debug|Win32
def compilationForProject(uid):
	result = ''
	for t in [ 'Debug' , 'DebugDLL' , 'Release' , 'ReleaseDLL' ]:
		for p in [ 'Win32','x64']:
			for z in ['ActiveCfg','Build.0']:
				result = result +  '\t\t{%s}.%s|%s.%s = %s|%s\n' % (uid,t,p,z,t,p)
	return result;

def compilationTable():
	result = strings['postSolutionBegin']
	for p in project:
		if type(project[p]) == type(empty()):
			result = result + compilationForProject(uid[p])
	return result + strings['postSolutionEnd']

##
# Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "libexiv2", "libexiv2\libexiv2_configure.vcproj", "{831EF580-92C8-4CA8-B0CE-3D906280A54D}"
#	ProjectSection(ProjectDependencies) = postProject
#		{09877CF4-83B6-44FE-A2E2-629AA5C8093E} = {09877CF4-83B6-44FE-A2E2-629AA5C8093E}
#		 ...
#	EndProjectSection
# EndProject
##
def projectRecord(project,projects):
	print( format % ((project), '{ }' if len(projects) == 0 else projects))

	UID	   = strings['UID']
	vcnew  = "%s\%s_configure.vcproj" % (project,project)  # write in DOS notation for Visual Studio
	result = 'Project("{%s}") = "%s", "%s", "{%s}"\n' % (UID,project,vcnew,uid[project])

	count  = 0
	out	   = strings['postProjectBegin']
	for p in projects:
		if not p in ignore:
			count=count+1
			out = out + '\t\t{%s} = {%s}\n' % (uid[p],uid[p])
	out = out + strings['postProjectEnd']
	if count > 0:
		result = result + out

	result = result + 'EndProject\n'
	return result

##
#   filter ....exv_msvc.h... -> ....exv_msvc_configure.h....
def modifyHeaderName(line):
	headerold='exv_msvc.h'
	headernew='exv_msvc_configure.h'
	if line.find(headerold):
		line=line.replace(headerold,headernew)
	return line

##
# Filter proj\proj.vcproj -> proj\proj_configure.vcproj
def writeVCproj(project,projects):
	vcold	 = os.path.join(project,("%s.vcproj"			% project) )  # path to old file
	vcnew	 = os.path.join(project,("%s_configure.vcproj"	% project) )  # path to new file
	xmllines = xml.dom.minidom.parse(vcold).toprettyxml().split('\n')
	out		 = ""
	for line in xmllines:
		# 1) Update the project GUID
		projectGUID='ProjectGUID="{'
		if line.find( projectGUID) > 0:
			start  = line.find(projectGUID) + len(projectGUID)
			olduid=line[start:line.find('}',start)-1]
			line=line.replace(olduid,uid[project]);

		# 2) Filter off unwanted files and modify the msvc header file
		if (not project in externlib) & (line.find( 'File RelativePath=' ) >= 0):
			for pattern in filter:
				if ( line.find(pattern) > 0 ):
					line =''
			line=modifyHeaderName(line)

		# 3) Add a preprocessor symbol to ask config.h to read exv_msvc_configure.h
		ppold=		'PreprocessorDefinitions="'
		ppnew=ppold+'EXV_MSVC_CONFIGURE;'
		if line.find( ppold ) > 0:
			line=line.replace(ppold,ppnew)

		if len(line)>0:
			out = out + line + '\n'

	open(vcnew,'w').write(out)

##
# not assert!
def cantHappen(condition,message):
	if condition:
		print(message)
		exit(1)

def headerFilter(path,options):
	result= ''
	lines = open(path).readlines();
	truth = {}
	truth['USE_CURL'		] = options.curl
	truth['USE_SSH'			] = options.ssh
	truth['ENABLE_WEBREADY' ] = options.webready
	truth['ENABLE_VIDEO'	] = options.video
	truth['HAVE_XMP_TOOLKIT'] = options.xmp
	truth['HAVE_ZLIB'		] = options.zlib
	truth['HAVE_EXPAT'		] = options.expat
	for line in lines:
		start=line.find('EXV_')
		if (start > 0) & ((line.find('define')>0) | (line.find('undef')>0)):
			key = line[start+4:line.find(' ',start+4)]
			if key in truth:
				line = ('#define EXV_%s 1\n' % key) if truth[key] else ('#undef EXV_%s\n' % key)
		line=modifyHeaderName(line)
		result += line

	return result

def enableWebready(option, opt_str, value, parser):
	b=True
	parser.values.webready = b
	parser.values.openssl  = b
	parser.values.ssh	   = b
	parser.values.curl	   = b

def disableWebready(option, opt_str, value, parser):
	b=False
	parser.values.webready = b
	parser.values.openssl  = b
	parser.values.ssh	   = b
	parser.values.curl	   = b

def main():
	global uid
	global project
	global strings
	global ignore
	global filter
	global filters
	global externlib
	global format
	global build

	##
	# set up argument parser
	usage = "usage: %prog [options]+"
	parser = optparse.OptionParser(usage)
	parser = optparse.OptionParser()

	parser.add_option('-A', '--with-app'		, action='store_true' , dest='app'	   ,help='build sample apps (false)' ,default=False)
	parser.add_option('-a', '--without-app'		, action='store_false', dest='app'	   ,help='do not build apps'	     )
	parser.add_option('-C', '--with-curl'		, action='store_true' , dest='curl'	   ,help='enable curl'			     ,default=False)
	parser.add_option('-c', '--without-curl'	, action='store_false', dest='curl'	   ,help='disable curl'			     )
	parser.add_option('-E', '--with-expat'		, action='store_true' , dest='expat'   ,help='enable expat (true)'		 ,default=True)
	parser.add_option('-e', '--without-expat'	, action='store_false', dest='expat'   ,help='disable expat'		     )
	parser.add_option('-O', '--with-openssl'	, action='store_true' , dest='openssl' ,help='enable openssl'		     ,default=False)
	parser.add_option('-o', '--without-openssl' , action='store_false', dest='openssl' ,help='disable openssl'		     )
	parser.add_option('-S', '--with-ssh'		, action='store_true' , dest='ssh'	   ,help='enable ssh'			     ,default=False)
	parser.add_option('-s', '--without-ssh'		, action='store_false', dest='ssh'	   ,help='disable ssh'			     )
	parser.add_option('-T', '--with-test'		, action='store_true' , dest='test'	   ,help='build test programs (true)',default=True)
	parser.add_option('-t', '--without-test'	, action='store_false', dest='test'	   ,help='do not build test progs'   )
	parser.add_option('-W', '--enable-webready' , action='callback'	  , dest='webready',help='enable webready (false)'   ,callback=enableWebready,default=False)
	parser.add_option('-w', '--disable-webready', action='callback'	  , dest='webready',help='enable webready'		     ,callback=disableWebready)
	parser.add_option('-V', '--enable-video'	, action='store_true' , dest='video'   ,help='enable video (false)'	     ,default=False)
	parser.add_option('-v', '--disable-video'	, action='store_false', dest='video'   ,help='disable video'		     )
	parser.add_option('-X', '--enable-xmp'		, action='store_true' , dest='xmp'	   ,help='enable xmp (true)'	     ,default=True)
	parser.add_option('-x', '--disable-xmp'		, action='store_false', dest='xmp'	   ,help='disable xmp'			     )
	parser.add_option('-Z', '--with-zlib'		, action='store_true' , dest='zlib'	   ,help='enable zlib/png (true)'    ,default=True)
	parser.add_option('-z', '--without-zlib'	, action='store_false', dest='zlib'	   ,help='disable zlib/png'		     )
	parser.add_option('-d' , '--default'		, action='store_true' , dest='default' ,help='default'				     ,default=False)

	##
	# no arguments, report and quit
	if len(sys.argv) == 1:
		parser.print_help()
		return

	##
	# parse and test for errors
	(options, args) = parser.parse_args()
	cantHappen(options.curl	   & (not options.webready),'cannot use curl without webready'	 );
	cantHappen(options.openssl & (not options.webready),'cannot use openssl without webready');
	cantHappen(options.ssh	   & (not options.webready),'cannot use libssh	without webready');
	cantHappen(options.xmp	   & (not options.expat	  ),'cannot use xmp without expat'		 );

	if not options.app:
		ignore = ignore | apps;
	if not options.test:
		ignore = ignore | tests;

	##
	# print options
	print(format % ('Option' , 'Value'))
	print(format % ('------' , '-----'))
	for o, v in sorted(options.__dict__.items()):
		if o != 'default':
			print(format % (o, v))
	print()
	print(format % ('Project','Dependancy'))
	print(format % ('-------','----------'))

	##
	# learn the filters
	if options.video:
		filters['video']=empty()
	if options.webready:
		filters['webready']=empty()
	if options.xmp:
		filters['xmp']=empty()
	if options.zlib:
		filters['zlib']=empty()
	filter = filters['video'] | filters['xmp'] | filters['webready'] | filters['zlib']

	##
	# learn build candidates
	for p in project:
		skip = p in ignore
		skip = skip | ((p == 'libssh'	) & ( not options.ssh	 ))
		skip = skip | ((p == 'libcurl'	) & ( not options.curl	 ))
		skip = skip | ((p == 'openssl'	) & ( not options.openssl))
		skip = skip | ((p == 'libeay32' ) & ( not options.openssl))
		skip = skip | ((p == 'ssleay32' ) & ( not options.openssl))
		skip = skip | ((p == 'xmpsdk'	) & ( not options.xmp	 ))
		skip = skip | ((p == 'libexpat' ) & ( not options.expat	 ))
		skip = skip | ((p == 'zlib'		) & ( not options.zlib	 ))
		if not skip:
			build.add(p)
	# build projects that intersect with the build!
	for p in build:
		project[p] = project[p] & build

	##
	# write solution file
	sln='exiv2_configure.sln'
	s = open(sln,'w')
	s.write(strings['Begin'])

	for p in sorted(build):
		s.write(projectRecord(p,project[p]))

	# write compilation table
	s.write(strings['globalBegin'])
	s.write(strings['platforms'	 ])
	s.write(compilationTable())
	s.write(strings['preSolution'])
	s.write(strings['globalEnd'	 ])

	# finish
	s.write(strings['End'])
	s.close()

	##
	# write project files
	for p in sorted(build):
		writeVCproj(p,project[p])

	##
	# write exv_msvc_configure.h by filtering exv_msvc.h
	include = os.path.join('..','include','exiv2')
	oldh	= os.path.join(include,'exv_msvc.h');
	newh	= os.path.join(include,'exv_msvc_configure.h');
	open(newh,'w').write(headerFilter(oldh,options))

	print()
	print('MSVC 2005 Solution file created: ' + sln)
	print('header = %s\n' % os.path.abspath(newh))

if __name__ == "__main__":
	main()

# That's all Folks
##