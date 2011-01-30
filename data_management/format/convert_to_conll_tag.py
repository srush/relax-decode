import sys
from conll_tag import *
from conll import *


if __name__=="__main__":
  for sent in parse_conll_file(sys.stdin):
    print ConllTagSent.from_conll(sent)
    print
