from conll import *
import sys

for sent in parse_conll_file(sys.stdin):
  print " ".join([ w.word + "_" + w.pos for w in list(sent)[1:]])
  
  
