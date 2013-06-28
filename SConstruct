from build_config import *
from protoc import *
import os

debug = ARGUMENTS.get('debug', 1)
profile = ARGUMENTS.get('profile', 0)

env = Environment(CC = 'g++', ENV=os.environ, tools=['default', 'protoc', 'doxygen'], toolpath = ['.'])

env.Append(ROOT=build_config['scarab_root'])

build_mode = ""
if int(debug):
   env.Prepend(CCFLAGS =('-g',))
   build_mode = "debug/"
elif int(profile):
   env.Append(CCFLAGS = ('-O2', '-p', "-ggdb", "-fprofile-arcs", "-ftest-coverage"),
              LINKFLAGS = ('-O2', '-p', "-ggdb" ,  "-fprofile-arcs", "-ftest-coverage"))
   build_mode = "profile/"
else:
   env.Append(CCFLAGS = ('-O2', '-DNDEBUG', '-Werror', '-Wno-deprecated'),
              LINKFLAGS = ('-O2', '-DNDEBUG'))
   build_mode = "opt/"

variant = 'build/' + build_mode
env.VariantDir(variant, '.')


sub_dirs = ['#/' + variant + 'graph',
            '#/' + variant + 'hypergraph',
            '#/' + variant + 'lattice',
            '#/' + variant + 'transforest',
            '#/' + variant + 'parse',
            '#/' + variant + 'tagger',
            '#/' + variant + 'optimization',
            '#/' + variant + 'mrf',
            '#/' + variant + 'phrasebased']



libs = ('hypergraph', 'lattice', "protobuf", "pthread", "gflags")

lib_path = build_config['lib_extra']

include_path = build_config['include_extra']

if build_config['has_gurobi']:
   libs += ("gurobi45", "m", "stdc++", "lp")
   lib_path += (build_config['gurobi_lib'], '#/lp')
   include_path += (build_config['gurobi_path'],)
   sub_dirs += ['lp']

if build_config['has_sri']:
   libs+= ("oolm", "misc", "dstruct")
   lib_path += (build_config['sri_lib'],)
   include_path += (build_config['sri_path'],)
   sub_dirs += ['#/' + variant + 'trans_decode']

env.Append(LIBPATH =('.',) + tuple(sub_dirs) + lib_path)

cpppath  = ('.', '#/third-party/svector/',
            '#/' + variant + 'interfaces/hypergraph/gen-cpp',
            '#/' + variant + 'interfaces/lattice/gen-cpp',
            '#/' + variant + 'interfaces/graph/gen-cpp') + \
            include_path + tuple(sub_dirs)
print cpppath
env.Append(CPPPATH=  [cpppath] )

env.Append(LIBS=  libs)

env.Append(HYP_PROTO="#/" + variant + "interfaces/hypergraph/gen-cpp/")
env.Append(LAT_PROTO="#/" + variant + "interfaces/lattice/gen-cpp/")
env.Append(GRAPH_PROTO="#/" + variant + "interfaces/graph/gen-cpp/")
env.Append(PROTOCPROTOPATH = [variant + "interfaces/graph/",
                              variant + "interfaces/hypergraph/",
                              variant + "interfaces/lattice/"])
print env
interfaces = SConscript(dirs=[variant + "interfaces"], exports=['env'])




local_libs = SConscript(dirs=sub_dirs,
                        exports=['env', 'build_config'])


if build_config['has_sri']:
   trans = env.Program(variant + 'trans', ( variant + "Run.cpp",) + local_libs , LIBS = libs)

   cube = env.Program(variant + 'cube', (variant + "CubeLM.cpp",) +local_libs, LIBS = libs)

   env.Program('exact', ("Exact.cpp",) +local_libs, LIBS = libs)

   env.Program('translp', ("TransLP.cpp",) +local_libs, LIBS = libs)

   regress = env.Command('regress', [trans, cube], 'python scripts/acl_regression.py')

   env.Alias('regression', regress)

   env.Command('cscope.out', [trans, cube], 'cscope-indexer -r')

env.Program('viterbi', ("SimpleViterbi.cpp",) +local_libs, LIBS = libs)

env.Program('marginals', ("Marginals.cpp",) +local_libs, LIBS = libs)

env.Program('run_parser', ("Parse.cpp", )+ local_libs, LIBS = libs)

env.Program('run_decomp_parser', ("DecompParser.cpp", )+ local_libs, LIBS = libs)

env.Program('run_tagger', ("Tag.cpp", )+ local_libs, LIBS = libs)

#env.Program('run_full_tagger', ("FullTagger.cpp", )+ local_libs, LIBS = libs)

#env.Program('run_dual_tagger', ("DualDecompTagger.cpp", )+ local_libs, LIBS = libs)

#env.Program('run_phrase_based_lp', ("PhraseLP.cpp", )+ local_libs, LIBS = libs)

#env.Program('run_phrase_based_viterbi', ("PhraseViterbi.cpp", )+ local_libs, LIBS = libs)

env.Program('run_potts_tagger', ("PottsTagger.cpp", )+ local_libs, LIBS = libs)

env.Program('run_decomp_tagger', ("DecompTagger.cpp", )+ local_libs, LIBS = libs)

#env.Program('solve_mrf', ("MRFSolver.cpp", )+ local_libs, LIBS = libs)

env.Program('example1', ("examples/Example1.cpp", )+ local_libs, LIBS = libs)

env.Program('example2', ("examples/Example2.cpp", )+ local_libs, LIBS = libs)

if build_config['has_gurobi']:
   env.Program('run_full_parser', ("FullParser.cpp", )+ local_libs)

   #env.Program('scarab', ("Main.cpp", ) + local_libs)


docs = env.Doxygen('Doxyfile')
env.Alias('document', docs)

#third_parties = SConscript(dirs=['#/third-party/'], exports=['env'])



