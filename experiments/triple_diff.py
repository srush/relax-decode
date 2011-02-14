import sys 
import textwrap
from itertools import *
files = map(open,sys.argv[1:])

for res in izip(*files):
  a,b,c = map(lambda a: a.replace("_", " ").strip(), res)
  var = "|cc"
  if c == a and c <> b:
    var = "|cx"
  elif c <> a and c == b: 
    var = "|xc"
  elif c <> a and c <> b and a == b: 
    var = "|xx"

  elif c <> a and c <> b and a <> b: 
    var = "|xxd"
  elif not c:
    var =""
  print "%-30s %-25s %-5s %-30s"%(a,b,var, c)
