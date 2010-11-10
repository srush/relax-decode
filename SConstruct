debug = ARGUMENTS.get('debug', 1)
profile = ARGUMENTS.get('profile', 0)
if int(debug):
   env = Environment(CC = 'g++',
                     CCFLAGS = '-g -Wall')
elif int(profile):
   env = Environment(CC = 'g++',
                     CCFLAGS = '-O2 -pg',
                     LINKFLAGS = '-O2 -pg')
else:
   env = Environment(CC = 'g++',
                     CCFLAGS = '-O3  -DNDEBUG',
                     LINKFLAGS = '-O3  -DNDEBUG')

sources = ["dual_subproblem.cpp", "util.cpp", "Subgradient.cpp", "Decode.cpp", "Run.cpp"]
libs = ["cpptest", "hypergraph", "lattice","protobuf", "oolm", "misc", "dstruct", "pthread"]


SConscript(dirs=['hypergraph', 'lattice'], exports=['env']) 

env.Program('trans', sources , LIBS = libs, LIBPATH= ['.','hypergraph', 'lattice'], CPPPATH = ['.', '/home/srush/Projects/transforest/svector/', '/home/srush/libs/sri/lib/i686/', 'hypergraph', 'lattice', '/home/srush/libs/sri/include/'])





