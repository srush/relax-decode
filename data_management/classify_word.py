import sys
import pickle
from unknown_words import *

def doubles(handle):
  d = {}
  for l in handle:
    if not l.strip():  continue
    ls = l.split()[0]
    d.setdefault(ls, 0)
    d[ls] += 1
  return d

wc = pickle.load(open(sys.argv[1]))
double_map = doubles(open(sys.argv[2]))

for l in sys.stdin:
  lstrip = l.strip()
  unk = wc.is_unknown(lstrip)
  repeat = double_map[lstrip] > 1
  
  n = wc.word_counts.get(lstrip, 0)
  print "%-30s %-7s %-7s %d"%(lstrip, 1 if unk else 0, 1 if repeat else 0, n)
