import os, sys
from itertools import *

def r(a):
  print a
  os.system(a)

flags = """--forest_prefix=../example/buildfull/output
--weight_file=../example/config.ini
--lm_file=../example/big.lm
--forest_range=1 9
--lm_weight=-0.141221"""

f = open("/tmp/regress_param", 'w')
print >>f, flags
f.close()
r("./trans --flagfile=/tmp/regress_param --lattice_prefix=../example/buildfull/rev_lat_out | grep END | tee /tmp/regress")
r("./cube --flagfile=/tmp/regress_param | grep END | tee /tmp/cube_regress")

new = open("/tmp/regress") 
old = open("regression/simple")

def important_lines(handle):
  for l in handle:
    t = l.strip().split()
    yield t[1]


assert(list(important_lines(new)) == list(important_lines(old)))
print "Success"


new = open("/tmp/cube_regress") 
old = open("regression/simple_cube")

def important_lines(handle):
  for l in handle:
    t = l.strip().split()
    yield t[2:3]


assert(list(important_lines(new)) == list(important_lines(old)))
print "Success"
