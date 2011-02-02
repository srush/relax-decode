import sys
root = "/home/srush/Projects/relax_decode/data_management/"
sys.path.append(root)

from itertools import *
from format.simple import *

def eval_pos(test_simple, gold_simple):
  total = 0
  correct = 0
  for test_sent, gold_sent in izip(test_simple, gold_simple):
    for test_word, gold_word in izip(test_sent, gold_sent):
      #print test_word
      assert test_word.word == gold_word.word
      assert test_word.pos 
      assert gold_word.pos
      if test_word.pos == gold_word.pos:
        correct +=1 
      total += 1
  return correct, total


if __name__ == "__main__":
  test_file = sys.argv[1]
  gold_file = sys.argv[2]
  test_sents = parse_simple_file(open(test_file), '/')
  gold_sents = parse_simple_file(open(gold_file), '_')
  correct, total = eval_pos(test_sents, gold_sents)
  print "Out of", float(total)
  print "Accuracy is:", correct / float(total)
  
