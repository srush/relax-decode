import sys
sys.path.append('.')

from itertools import *
#from format.conll import *
from format.simple import *
from StringIO import StringIO
from unknown_words import *
from pickle import *
    
class PosConstraint:
  def __init__(self, wc):
    self.constraints = {}
    self.wc = wc

  def inc_sent(self, (sent_num, sent)):
    for pos, w in sent.enum_words():
      unk = self.wc.is_unknown(w.word)
      count = self.wc.count(w.word)
      
        

      is_num = True
      try:
        float(w.word.replace(',',''))
      except:
        is_num = False
      if unk and not is_num:
        self.constraints.setdefault(w.word, [])
        self.constraints[w.word].append((sent_num, pos, count))
        

  def stats(self):
    single = 0
    total = 0
    for word,const in self.constraints.iteritems():
      total +=1
      if len(self.constraints[word]) == 1:
        single += 1
    return (single, total)

  def groups(self): 
    return [ (unk_word, constraints) for unk_word, constraints in self.constraints.iteritems()
      if len(constraints) > 1]

  def zerozero(self): 
    return [ (unk_word, constraints) for unk_word, constraints in self.constraints.iteritems()
             if len(constraints) == 1 and constraints[0][2] <= 5]
    
  def __repr__(self):
    groups = self.groups()
    zerozero = self.zerozero()
    s = str(len(groups)) + "\n"
    s +=  "\n".join([ "\t".join(map(str,[i, unk_word, sent_num, pos, train_count, len(constraints)]))
                      for i, (unk_word, constraints) in enumerate(groups + zerozero)
                      for sent_num, pos, train_count in constraints                       
                      ])
    return s
      

if __name__=="__main__":

  wc = load(open(sys.argv[1], 'rb'))
  #print len(wc.word_counts)
  manager = PosConstraint(wc)
  for s in enumerate(parse_simple_file(open(sys.argv[2]))):
    manager.inc_sent(s)
  s, total = manager.stats()

  print manager
  #print s / float(total)
