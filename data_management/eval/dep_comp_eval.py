import sys, os
sys.path.append(os.getenv("SCARAB_ROOT")+"/data_management/")
from itertools import *
from map.unmap_pos import * 
from format.conll import *
try:
  argv = FLAGS(sys.argv)  
except flags.FlagsError, e:
  print >>sys.stderr, '%s\\nUsage: %s ARGS\\n%s' % (e, sys.argv[0], FLAGS)
  sys.exit(1)

unmap = Unmapper()


class ConstraintOut:
  def __init__(self, label, deps, pos, trains ):
    self.label = label
    self.deps = deps
    self.pos = pos
    self.trains = trains

  def show_results(self, lab_num, test_sents, example_sents, gold_sents, verbose):
    print 
    print "|Label %s %s "%(lab_num, self.label),
    us = 0
    them = 0
    for (sent, ind), pos in zip(self.deps, self.pos):
      
      v = eval_word(test_sents[sent], example_sents[sent], gold_sents[sent], ind-1)
      if v == "xc":
        us +=1
      elif v == "cx":
        them +=1
    print "| Fixed %s Broke %s Trains %s " %(us, them, len(self.trains))
    
    for (sent, ind), pos in zip(self.deps, self.pos):      
      v = eval_word(test_sents[sent], example_sents[sent], gold_sents[sent], ind-1)
      #vpos = test_sents[sent][ind]
      print "%s\t%s\t%s\t%s" %(sent, ind, pos, v)



    if verbose:
      for sent, ind in self.deps:
        print 
        print sent, ind
        print 
        display_sent(test_sents[sent], example_sents[sent], gold_sents[sent])
    return us, them


    
  @staticmethod
  def from_handle(handle):
    group = -1
    for l in handle:
      l = l.strip().split()
    
      if l[1] == "NEW":
        group +=1
        label = " ".join(l[2:])
        deps = []
        pos = []
        trains = []
      elif l[1] == "DONE":
        
        yield ConstraintOut(label, deps, pos, trains)
      else:
        if l[2] == "Dep":
          index = map(int, l[3].split(":"))
          if index[0] >=0:
            deps.append(index)
          else:
            trains.append(index)
        if l[2] == "Pos":
          index = map(int, l[3].split(":"))
          if index[0] >=0:
            pos.append(unmap.pos_map.get(int(l[4]),""))
            #pos.append(int(l[4]))
        if l[2] == "mu":
          label += ' ' +" ".join(l[3:])
          
def display_sent(test_sent, extra_sent, gold_sent):
   for test_word, extra_word, gold_word in izip(test_sent, extra_sent, islice(gold_sent, 1, None)):
      #print test_word, gold_word.num
      test_head = int(test_word.split("_")[-1])
      extra_head = int(extra_word.split("_")[-1])
      v = '  '
      if test_head == gold_word.head and  extra_head != gold_word.head:
        v = 'cx'
      if test_head != gold_word.head and  extra_head == gold_word.head:
        v = 'xc'
      if test_head != gold_word.head and  extra_head != gold_word.head:
        v = '  '
      print "%s\t%s\t%5s %s\t%s" %(test_word, extra_word, v,str(gold_word.num) + "_" +str(gold_word.head), gold_word.format() )
      #assert int(t[0]) == gold_word.num

def eval_word(test_sent, extra_sent, gold_sent, ind):
  print >>sys.stderr, ind
  print >>sys.stderr, test_sent, extra_sent, gold_sent.words
  test_word = test_sent[ind]
  extra_word = extra_sent[ind]
  gold_word = gold_sent.words[ind+1]

  test_head = int(test_word.split("_")[-1])
  extra_head = int(extra_word.split("_")[-1])
  v = 'cc'
  if test_head == gold_word.head and  extra_head != gold_word.head:
    v = 'cx'
  if test_head != gold_word.head and  extra_head == gold_word.head:
    v = 'xc'
  if test_head != gold_word.head and  extra_head != gold_word.head:
    v = 'xx'
  return v

# def eval_dep(test_simple, extra_simple, gold_simple):
  
#   for test_sent, extra_sent, gold_sent in izip(test_simple, extra_simple, gold_simple):
#     for test_word, extra_word, gold_word in izip(test_sent, extra_sent, islice(gold_sent, 1, None)):
#       #print test_word, gold_word.num
#       test_head = int(test_word.split("_")[-1])
#       extra_head = int(extra_word.split("_")[-1])
#       v = '  '
#       if test_head == gold_word.head and  extra_head != gold_word.head:
#         v = 'cx'
#       if test_head != gold_word.head and  extra_head == gold_word.head:
#         v = 'xc'
#       if test_head != gold_word.head and  extra_head != gold_word.head:
#         v = '  '
#       print "%s\t%s\t%5s %s\t%s" %(test_word, extra_word, v,str(gold_word.num) + "_" +str(gold_word.head), gold_word.format() )
#       #assert int(t[0]) == gold_word.num
  


if __name__ == "__main__":
  viterbi_file = open(argv[1])
  extra_file = open(argv[2])
  gold_file = open(argv[3])
  constraint_results = open(argv[4])
  print argv[4]
  constraints = list(ConstraintOut.from_handle(constraint_results))

  def chunk(handle):
    total = []
    for l in handle:
      if not l.strip():
        yield total
        total = []
      else:
        total.append(l.strip())

  viterbi_sents = list(chunk(viterbi_file))
  extra_sents = list(chunk(extra_file))
  gold_sents = list(parse_conll_file(gold_file))
  us =0 ;them = 0

  for i, c in enumerate(constraints):
    u,t = c.show_results(i, viterbi_sents, extra_sents, gold_sents, False)
    us += u
    them += t
    #eval_dep(viterbi_sents, extra_sents, gold_sents)
  for i, c in enumerate(constraints):
    c.show_results(i, viterbi_sents, extra_sents, gold_sents, True)

  print >>sys.stderr, us, them
  print us, them
