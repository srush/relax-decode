import sys 
from unknown_words import *
from itertools import *
import  pickle

def doubles(handle):
  d = {}
  for l in handle:
    if not l.strip():  continue
    ls = l.split('_')[0].strip()
    d.setdefault(ls, 0)
    d[ls] += 1
  return d

double_map = doubles(open(sys.argv[1]))
wc = pickle.load(open(sys.argv[2]))

files = map(open,sys.argv[3:])
right = []
good = []
bad = []
very_bad = []
weird = []
for res in izip(*files):
  a,b,c = map(lambda a: a.replace("_", " ").strip(), res)

  if c and c.split()[0] in ["-LRB-", "-RRB-"]: continue
  
  #var = "|cc"
  if not c:
    continue 

  if c == a and c == b:
    right.append((a,b,c))
  elif c == a and c <> b:
    bad.append((a,b,c))
  elif c <> a and c == b: 
    good.append( (a,b,c))
  elif c <> a and c <> b and a == b: 
    #var = "|xx"
    very_bad.append((a,b,c))
  elif c <> a and c <> b and a <> b: 
    #var = "|xxd"
    weird.append((a,b,c)) 
  
def mcat(w):
  return (
         double_map[w] > 1, not wc.is_unknown(w))

def bcat((_,q,c)):
  return mcat(c.split()[0])

def analysis(sents):
  cats = {}
  words = [c.split()[0] for _,_,c in sents]
  for w in words:
    cat = mcat(w)
    cats.setdefault(cat, 0)
    cats[cat] +=1 
  for a in (False, True):
    for b in (False, True):
      cat = (a,b)
      print "Cat %-1s %-1s | %-5s"%( int(a), int(b), cats.get((a,b), 0))



print "GOOD %s", len(good)
analysis(good)

print "BAD %s", len(bad)
analysis(bad)
print "VERYBAD %s", len(very_bad)
analysis(very_bad)

print "WEIRD %s", len(weird)
analysis(weird)

print "RIGHT %s", len(right)
analysis(right)

for a,b,c in sorted(good, key =bcat): 
  var = "|xc"
  w = c.split()[0]
  cat = mcat(w)

  print "%-30s %-25s %-5s (%-1s,%-1s,%4s, %4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], wc.word_counts.get(w,0), c)
#  print "%-30s %-25s %-5s (-1%s,-1%s,-4%s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], c)
 # print "%-30s %-25s %-5s %-30s"%(a,b,var, c)

print "################"

for a,b,c in sorted(bad, key=bcat): 
  var = "|cx"
  w = c.split()[0]
  cat = mcat(w)
  print "%-30s %-25s %-5s (%-1s,%-1s,%4s, %4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], wc.word_counts.get(w,0), c)
#  print "%-30s %-25s %-5s (%-1s,%-1s,%4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], c)
#  print "%-30s %-25s %-5s (-1%s,-1%s,-4%s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], c)
#  print "%-30s %-25s %-5s %-30s"%(a,b,var, c)

print "################"

for a,b,c in sorted(very_bad, key=bcat): 
  var = "|xx"
  w = c.split()[0]
  cat = mcat(w)
  print "%-30s %-25s %-5s (%-1s,%-1s,%4s, %4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], wc.word_counts.get(w,0), c)
 # print "%-30s %-25s %-5s (%-1s,%-1s,%4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], c)
#  print "%-30s %-25s %-5s (-1%s,-1%s,-4%s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], c)
#  print "%-30s %-25s %-5s %-30s"%(a,b,var, c)

print "################"

for a,b,c in sorted(weird, key=bcat) : 
  var = "|xxd"
  w = c.split()[0]
  cat = mcat(w)
  #print "%-30s %-25s %-5s (%-1s,%-1s,%4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], c)
  print "%-30s %-25s %-5s (%-1s,%-1s,%4s, %4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], wc.word_counts.get(w,0), c)

print "################"

print right
for a,b,c in sorted(right, key=bcat) : 
  var = "|cc"
  w = c.split()[0]
  cat = mcat(w)
  #print "%-30s %-25s %-5s (%-1s,%-1s,%4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], c)
  print "%-30s %-25s %-5s (%-1s,%-1s,%4s, %4s) %-30s"%(a,b,var, int(cat[0]), int(cat[1]),double_map[w], wc.word_counts.get(w,0), c)

print "################"
