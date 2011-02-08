from build_config import *
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

env.Append(ROOT=build_config['scarab_root'])

sources = ("dual_subproblem.cpp", "Decode.cpp", "NGramCache.cpp")

sub_dirs = ['graph', 'hypergraph', 'lattice', 'transforest', 'parse', 'tagger', 'optimization', 'phrasebased']

libs = ('graph', "hypergraph", "lattice", "parse", "protobuf", "oolm", "misc", 
        "dstruct", "pthread", "boost_program_options")


lib_path = build_config['lib_extra']
include_path = build_config['include_extra']
if build_config['has_gurobi']:
   libs += ("gurobi_g++4.1", "gurobi40", "m", "stdc++", "lp")
   lib_path += (build_config['gurobi_lib'], '#/lp')
   include_path += (build_config['gurobi_path'],)
   sub_dirs += ['lp']

env.Append(LIBPATH =('.',build_config['sri_lib'], '#/graph', '#/hypergraph', '#/lattice', '#/transforest', '#/parse', '#/tagger', '#/optimization', '#/tagger', '#/phrasebased') +
           lib_path 
           )


env.Append( CPPPATH=  ('.', build_config['svector_path'], '#/graph', '#/lp','#hypergraph', '#lattice', '#transforest', '#/parse', '#/tagger', '#/optimization',
                       '#/phrasebased',
                       build_config['sri_path'], 
                       build_config['lattice_proto_path'], 
                       build_config['forest_proto_path']) + include_path )

env.Append( LIBS=  libs)

local_libs = SConscript(dirs=sub_dirs,
                        exports=['env', 'build_config']) 

decode = env.Library('decode', sources + local_libs,
                      LIBS = libs)

env.Program('trans', ("Run.cpp", decode) + local_libs, LIBS = libs)

env.Program('cube', ("CubeLM.cpp", decode)+ local_libs, LIBS = libs)

env.Program('parser', ("Parse.cpp", decode)+ local_libs, LIBS = libs)

env.Program('run_tagger', ("Tag.cpp", decode)+ local_libs, LIBS = libs)

env.Program('run_full_tagger', ("FullTagger.cpp", decode)+ local_libs, LIBS = libs)

env.Program('run_dual_tagger', ("DualDecompTagger.cpp", decode)+ local_libs, LIBS = libs)
env.Program('run_phrase_based', ("PhraseLP.cpp", decode)+ local_libs, LIBS = libs)





if build_config['has_gurobi']:
   env.Program('fullparser', ("FullParser.cpp", decode)+ local_libs)
   env.Program('scarab', ("Main.cpp", decode) + local_libs)



#third_parties = SConscript(dirs=['#/third-party/'], exports=['env']) 



