from simple import *
import sys

for sent in parse_simple_file(sys.stdin, "_"):
  sent.strip_pos()
  print sent

  
  
