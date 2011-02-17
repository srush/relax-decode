import os, sys
from itertools import *

r = os.system

r("./trans ../example/buildfull/output ../example/buildfull/rev_lat_out ../example/config.ini ../example/big.lm  1 9 | grep END | tee /tmp/regress")
r("./cube ../example/buildfull/output ../example/config.ini ../example/big.lm  100 1 9 | grep END | tee /tmp/cube_regress")

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
