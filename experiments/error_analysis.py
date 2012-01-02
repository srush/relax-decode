import sys 
from unknown_words import *
from itertools import *
from pickle import *

def doubles(handle):
  d = {}
  for l in handle:
    if not l.strip():  continue
    ls = l.split()[0]
    d.setdefault(ls, 0)
    d[ls] += 1
  return d

double_map = doubles(open(sys.argv[1]))
wc = pickle.load(open(sys.argv[2]))


files = map(open,sys.argv[3:])
good = []
bad = []
verybad = []
weird = []
for res in izip(*files):
  a,b,c = map(lambda a: a.replace("_", " ").strip(), res)

  
  #var = "|cc"
  if c == a and c <> b:
    good.append((a,b,c))
  elif c <> a and c == b: 
    bad.append( (a,b,c))
  elif c <> a and c <> b and a == b: 
    #var = "|xx"
    very_bad.append((a,b,c))
  elif c <> a and c <> b and a <> b: 
    #var = "|xxd"
    weird.append((a,b,c)) 
  elif not c:
    continue 

def analysis(sents):
  cats = {}
  words = [c.split()[0] for _,_,c in sent]
  for w in words:
    cat = (wc.is_unknown(w), 
           double_map[lstrip] > 1)
    cats.setdefault(cat, 0)
    cats[cat] +=1 
  for a in (False, True):
    for b in (False, True):
      cat = (a,b)
      print "Cat RTrain: %s RTest: %s |  %s", a, b, cats.get((a,b), 0)

print "GOOD %s", len(good)
analysis(good)

print "BAD %s", len(bad)
analysis(bad)
print "VERYBAD %s", len(very_bad)
analysis(very_bad)

print "WEIRD %s", len(weird)
analysis(weird)

for a,b,c in good: 
  var = "|xc"
  print "%-30s %-25s %-5s %-30s"%(a,b,var, c)

print "################"

for a,b,c in bad: 
  var = "|cx"
  print "%-30s %-25s %-5s %-30s"%(a,b,var, c)

print "################"

for a,b,c in verybad: 
  var = "|xx"
  print "%-30s %-25s %-5s %-30s"%(a,b,var, c)

print "################"

for a,b,c in weird: 
  var = "|xxd"
  print "%-30s %-25s %-5s %-30s"%(a,b,var, c)

print "################"
