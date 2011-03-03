import sys, os
sys.path.append(os.getenv("SCARAB_ROOT")+"/data_management/")
from itertools import *

from format.conll import *

def eval_dep(test_simple, gold_simple):
  total = 0
  correct = 0
  
  for test_sent, gold_sent in izip(test_simple, gold_simple):
    for test_word, gold_word in izip(test_sent, islice(gold_sent, 1, None)):
      #print test_word, gold_word.num
      t = test_word.split("_")
      assert int(t[0]) == gold_word.num
      if int(t[1]) == gold_word.head:
        correct +=1
      total += 1
  return correct, total


if __name__ == "__main__":
  test_file = sys.argv[1]
  gold_file = sys.argv[2]

  def chunk(handle):
    total = []
    for l in handle:
      if not l.strip():
        yield total
        total = []
      else:
        total.append(l.strip())

  print test_file
  print gold_file
  test_sents = list(chunk(open(test_file)))
  gold_sents = list(parse_conll_file(open(gold_file)))
  
  correct, total = eval_dep(test_sents, gold_sents)
  print "Correct of", float(correct)
  print "Out of", float(total)
  print "Accuracy is:", correct / float(total)
  
