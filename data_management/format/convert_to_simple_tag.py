from conll import *
import sys

sep = "_"
if len(sys.argv) > 1:
  sep  = sys.argv[1]

for sent in parse_conll_file(sys.stdin):
  print " ".join([ w.word + sep + w.pos for w in list(sent)[1:]])
  
  
