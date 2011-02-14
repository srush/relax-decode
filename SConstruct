from build_config import *
from protoc import *
import os
debug = ARGUMENTS.get('debug', 1)
profile = ARGUMENTS.get('profile', 0)

env = Environment(CC = 'g++', ENV=os.environ, tools=['default', 'protoc'], toolpath = '.')

if int(debug):
   env.Append(CCFLAGS = '-g -Wall')
elif int(profile):
   env.Append(CCFLAGS = '-O2 -pg',
                     LINKFLAGS = '-O2 -pg')
else:
   env.Append(CCFLAGS = '-O3  -DNDEBUG',
              LINKFLAGS = '-O3  -DNDEBUG')

env.Append(ROOT=build_config['scarab_root'])

sub_dirs = ['#/graph', '#/hypergraph', '#/lattice', '#/transforest', 
            '#/parse', '#/tagger', '#/optimization', '#/phrasebased', '#/trans_decode']

libs = ('hypergraph', 'lattice', "protobuf", "pthread", "boost_program_options")

lib_path = build_config['lib_extra']

include_path = build_config['include_extra']

if build_config['has_gurobi']:
   libs += ("gurobi_g++4.1", "gurobi40", "m", "stdc++", "lp")
   lib_path += (build_config['gurobi_lib'], '#/lp')
   include_path += (build_config['gurobi_path'],)
   sub_dirs += ['lp']

if build_config['has_sri']:
   libs+= ("oolm", "misc", "dstruct")
   lib_path += (build_config['sri_lib'],) 
   include_path += (build_config['sri_path'],)

env.Append(LIBPATH =('.',) + tuple(sub_dirs) + lib_path)

env.Append( CPPPATH=  ('.', '#/third-party/svector/',
                       '#/interfaces/hypergraph/gen_cpp',
                       '#/interfaces/lattice/gen-cpp',
                       '#/interfaces/graph/gen-cpp') + 
            include_path + tuple(sub_dirs) )

env.Append(LIBS=  libs)

env.Append(HYP_PROTO="#interfaces/hypergraph/gen-cpp/")
env.Append(LAT_PROTO="#interfaces/lattice/gen-cpp/")
env.Append(GRAPH_PROTO="#interfaces/graph/gen-cpp/")

local_libs = SConscript(dirs=sub_dirs,
                        exports=['env', 'build_config']) 

SConscript(dirs=["interfaces"], exports=['env'])

env.Program('trans', ("Run.cpp",) + local_libs, LIBS = libs)

env.Program('cube', ("CubeLM.cpp", )+ local_libs, LIBS = libs)

env.Program('parser', ("Parse.cpp", )+ local_libs, LIBS = libs)

env.Program('run_tagger', ("Tag.cpp", )+ local_libs, LIBS = libs)

env.Program('run_full_tagger', ("FullTagger.cpp", )+ local_libs, LIBS = libs)

env.Program('run_dual_tagger', ("DualDecompTagger.cpp", )+ local_libs, LIBS = libs)

env.Program('run_phrase_based_lp', ("PhraseLP.cpp", )+ local_libs, LIBS = libs)

env.Program('run_phrase_based_viterbi', ("PhraseViterbi.cpp", )+ local_libs, LIBS = libs)

env.Program('run_potts_tagger', ("PottsTagger.cpp", )+ local_libs, LIBS = libs)

env.Program('solve_mrf', ("MRFSolver.cpp", )+ local_libs, LIBS = libs)


if build_config['has_gurobi']:
   env.Program('fullparser', ("FullParser.cpp", )+ local_libs)
   env.Program('scarab', ("Main.cpp", ) + local_libs)



#third_parties = SConscript(dirs=['#/third-party/'], exports=['env']) 



