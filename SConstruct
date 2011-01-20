from build_config import *
debug = ARGUMENTS.get('debug', 1)
profile = ARGUMENTS.get('profile', 0)


if int(debug):
   env = Environment(CC = 'g++',
                     CCFLAGS = '-g -Werror -Wall')
elif int(profile):
   env = Environment(CC = 'g++',
                     CCFLAGS = '-O2 -pg',
                     LINKFLAGS = '-O2 -pg')
else:
   env = Environment(CC = 'g++',
                     CCFLAGS = '-O3  -DNDEBUG',
                     LINKFLAGS = '-O3  -DNDEBUG')

sources = ("dual_subproblem.cpp", "util.cpp", "Subgradient.cpp", "Decode.cpp", "NGramCache.cpp")

libs = ("cpptest", "hypergraph", "lattice","protobuf", "oolm", "misc", 
        "dstruct", "pthread", "boost_program_options")

if build_config['has_gurobi']:
   libs += ("gurobi_g++4.1", "gurobi40", "m", "stdc++")

env.Append(LIBPATH =('.',build_config['sri_lib'], 'hypergraph', 'lattice', 'transforest',
                     build_config['boost_lib'], build_config['gurobi_lib']]) 



cpppath = ('.', build_config['svector_path'], 'hypergraph', 'lattice', 'transforest', 
           build_config['sri_path'])

local_libs = SConscript(dirs=['hypergraph', 'lattice', 'transforest'], exports=['env', 'build_config']) 


decode = env.Library('decode', sources + local_libs,
                      LIBS = libs, 
                      LIBPATH= libpath,
                      CPPPATH = cpppath)



env.Program('trans', ("Run.cpp", decode) + local_libs, LIBS = libs, LIBPATH= libpath,
                     CPPPATH = cpppath)

env.Program('cube', ("CubeLM.cpp", decode)+ local_libs, LIBS = libs, LIBPATH= libpath, CPPPATH= cpppath)

if build_config['has_gurobi']:
   env.Program('scarab', ("Main.cpp", decode) + local_libs, LIBS = libs, LIBPATH= libpath, CPPPATH= cpppath)





