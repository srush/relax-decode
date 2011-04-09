from format.conll import *
import sys
from itertools import * 

for sent in islice(parse_conll_file(sys.stdin), int(sys.argv[1])):
  print sent.format()
  print
  
