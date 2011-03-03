from format.conll import *
import sys

for sent in parse_conll_file(sys.stdin):
  print sent.remove_labels().format()
  print
  
