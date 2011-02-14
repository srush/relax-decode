import sys
from pickle import *
from format.conll import *
from format.simple import *
from collections import defaultdict
import gc

class WordCounter:

  
  def __init__(self, thres):
    self.word_counts = {} #defaultdict(lambda : 0)
    self.unknown_thres = thres


  def inc_sent(self, sent):
    for w in sent:
      self.word_counts.setdefault(w.word, 0)
      self.word_counts[w.word] = self.word_counts[w.word] + 1

  def is_unknown(self, w):
    return self.word_counts.get(w,0) < self.unknown_thres

  def count(self, w):
    return self.word_counts.get(w,0)


  def trim(self):
    self.word_counts = dict([ (k,w)   for k,w in self.word_counts.iteritems() if w > 2])


if __name__ == "__main__":
  gc.disable()
  wc = WordCounter(int(sys.argv[1]))
  for i, sent in enumerate(parse_conll_file(sys.stdin)):
    
    if i % 1000  == 0 : print >>sys.stderr, i
    wc.inc_sent(sent)
  wc.trim()
  dump( wc, open(sys.argv[2], 'wb'))
