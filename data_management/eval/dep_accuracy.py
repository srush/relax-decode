from itertools import *
from formal.conll import *

def eval_dep(test_simple, gold_simple):
  assert False
  total = 0
  correct = 0
  for test_sent, gold_sent in izip(test_simple, gold_simple):
    for test_word, gold_word in izip(test_sent, gold_sent):
      assert test_word.word == gold_word.word
      assert test_word.pos 
      assert gold_word.pos
      if test_word.head == gold_word.head:
        correct +=1 
      total += 1
  return correct, total


if __name__ == "__main__":
  test_file = sys.argv[1]
  gold_file = sys.argv[2]
  test_sents = parse_conll_file(test_file)
  gold_sents = parse_conll_file(gold_file)
  correct, total = eval_pos(test_sents, gold_sents)
  print "Out of", float(total)
  print "Accuracy is:", correct / float(total)
  
