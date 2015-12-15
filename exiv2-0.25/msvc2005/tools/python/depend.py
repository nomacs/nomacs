#!/usr/bin/python3

##
# read an MSVC solution file and generate the project dependency table
# syntax: tools/python/depend solution-file [format]
# - the argument 'format' can be anything!
##

import os
import sys

def main():
	##
	# no arguments, report and quit
	if len(sys.argv)==1:
		print('%s solutionfile' % sys.argv[0])
		return
	bFormat = len(sys.argv)>2

	sln=sys.argv[1]
	print(sln)

	lines=open(sln).readlines()

	project={}
	depends={}
	bDepends = False
	for line in lines:
		if line.find('Project(')>=0:
			start = line.find(' = "')
			end   = line.find('"',start+4)
			name  = line[start+4:end]
			start = line.find('{',end)
			end   = line.find('}',start)
			uid   = line[start+1:end]
			project[uid] = name
		if line.find('Global')>0:
			break

	# print(project)

	for line in lines:
		if line.find('Project(')>=0:
			start = line.find(' = "')
			end   = line.find('"',start+4)
			name  = line[start+4:end]
			depends[name]=set([])

		if bDepends  & (line.find('ProjectSectionEnd')>0):
			bDepends=False
		if (not bDepends) & (line.find('ProjectSection(ProjectDependencies)')>=0):
			bDepends=True
		if bDepends:
			start = line.find('{')
			end   = line.find('}',start+1)
			if (start>0) & (end>0):
				uid   = line[start+1:end]
				# print('found uid = ',uid)
				if not name in depends:
					depends[name]=set([])
				if uid in project:
					depends[name].add(project[uid])
		if line.find('Global')>0:
			break

	for p in sorted(depends.keys()):
		if bFormat:
			s=str(depends[p])
			s=s.replace('{','[').replace('}',']')
			s=s.replace('set()','[ ]')
			P="project['" + p + "']"
			print('%-30s = set(%s)' % (P,s)  )
		else:
			print('%-20s %s' % (p,depends[p])  )


# project['xmpparser-test'	] = set(['libexiv2','xmpsdk'			 ])


if __name__ == "__main__":
	main()

# That's all Folks
##