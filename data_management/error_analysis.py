import sys 
from unknown_words import *
from itertools import *
import  pickle
from marginals import *
from map.unmap_pos import *
def doubles(handle):
  d = {}
  for l in handle:
    if not l.strip():  continue
    ls = l.split()[0].strip()
    d.setdefault(ls, 0)
    d[ls] += 1
  return d

double_map = doubles(open(sys.argv[1]))
print double_map
wc = pickle.load(open(sys.argv[2]))
marginals = Marginals.from_handle(open(sys.argv[3]))
unmapper = Unmapper()

files = map(open,sys.argv[4:])
right = []
good = []
bad = []
very_bad = []
weird = []

class Pairing:
  def __init__(self,words, sent_num, word_num):
    self.words = words
    
    self.sent_num = sent_num
    self.word_num = word_num
    self.pos = [w.split()[1] for w in self.words]
    self.pos_ind = [unmapper.string_to_ind(p) for p in self.pos ]
    self.marginals = [marginals.data.get((self.sent_num, self.word_num, p_ind), 0.0) for p_ind in self.pos_ind]


sent_num = 0
word_num = -1
for res in izip(*files):
  
  a,b,c = map(lambda a: a.replace("_", " ").strip(), res)
  c = str(c.strip())
  word_num +=1 
  if c and c.split()[0] in ["-LRB-", "-RRB-"]: continue
  
  #var = "|cc"
  if not c:
    sent_num +=1 
    word_num =-1
    continue 

  pairing = Pairing((a,b,c), sent_num, word_num)

  a = a.split()[1]
  b = b.split()[1]
  c = c.split()[1]
  
  if c == a and c == b:
    right.append(pairing)
  elif c == a and c <> b:
    bad.append(pairing)
  elif c <> a and c == b: 
    good.append(pairing)
  elif c <> a and c <> b and a == b: 
    #var = "|xx"
  
    very_bad.append(pairing)#(a,b,c))
  elif c <> a and c <> b and a <> b: 
    #var = "|xxd"
    weird.append(pairing) 
  
def mcat(w):
  return (
         double_map[w] > 1, not wc.is_unknown(w))

def bcat(pairing):
  return mcat(pairing.words[2].split()[0])

def analysis(sents):
  cats = {}
  words = [pairing.words[2].split()[0] for pairing in sents]
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

def display(ls, var):
  for pairing in sorted(ls, key =bcat):
    a,b,c = pairing.words
    am,bm,cm = pairing.marginals
    w = c.split()[0]
    cat = mcat(w)

    print "%2d %2d %0.3f %-25s %0.3f %-25s %-5s (%-1s,%-1s,%4s, %4s) %0.3f %-25s"%(pairing.sent_num,pairing.word_num, am,  a,bm,b,var, int(cat[0]), int(cat[1]),double_map[w], wc.word_counts.get(w,0), cm, c)
  print "################"



display(good, "|xc")
display(bad, "|cx")
display(very_bad, "|xx")
display(weird, "|xxd")
display(right, "|cc")


