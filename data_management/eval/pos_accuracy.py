import sys,os

sys.path.append(os.getenv("SCARAB_ROOT")+"/data_management/")
root = "/home/srush/Projects/relax_decode/data_management/"
sys.path.append(root)

from itertools import *
from format.simple import *
from format.conll import *

punc = set(["?", ",", ".",  "-RRB-", "-LRB-", "``", "''", ":", "", "**", "*ROOT*", "ROOT"])
def eval_pos(test_simple, gold_simple, known_words):
  total = 0
  correct = 0
  unk_correct = 0
  unk_total = 0
  sents = 0
  correct_sents = 0
  for test_sent, gold_sent in izip(test_simple, gold_simple):
    is_correct = True
    for test_word, gold_word in izip(test_sent, gold_sent):
      #print test_word.word, gold_word.word
      assert test_word.word == gold_word.word, str(test_word.word) + " " + str(gold_word.word) + " " + str(sys.argv)
      #assert test_word.pos 
      #assert gold_word.pos
      if True:#test_word.pos not in punc:
        if test_word.pos == gold_word.pos:
          correct +=1
        else:
          is_correct = False
        total += 1

      if test_word.word not in known_words:
        if test_word.pos == gold_word.pos:
          unk_correct +=1 
        unk_total += 1
    if is_correct:
      sents +=1 
    correct_sents +=1 

        

  return correct, total, unk_correct, unk_total, sents, correct_sents


if __name__ == "__main__":
  test_file = sys.argv[1]
  gold_file = sys.argv[2]
  train_file = sys.argv[3]
  test_sents = parse_simple_columns(open(test_file))
  gold_sents = parse_simple_columns(open(gold_file))
  training_sents = parse_conll_file(open(train_file))

  known_words = set()
  for sent in training_sents:
    for word in sent:
      known_words.add(word.word)
  
  correct, total, unk_cor, unk_total, cor_sents, total_sents = eval_pos(test_sents, gold_sents, known_words)
  print "Correct of", float(correct)
  print "Out of", float(total)
  print "Accuracy is:", correct / float(total)

  print "unk correct", unk_cor
  print "unk total", unk_total
  print "Unk Accuracy is:", unk_cor/ float(unk_total)  

  print "Sent correct", cor_sents
  print "sent total", total_sents
  print "Sent Accuracy is:", cor_sents/ float(total_sents)  
