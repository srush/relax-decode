from itertools import *
from format.conll import *
from format.simple import *
from StringIO import StringIO
import sys
punc = set(["CD", "$", "#", ":", ""])
bigpunc = set(["CD", ",", ".", "$", "-RRB-", "-LRB-", "#", "``", "''", ":", "", "**", "*ROOT*", "ROOT"])

class Context:
  def __init__(self, left, word, right):
    self.word = word
    self.left = left
    self.right = right
  
  @classmethod
  def pos(cls, x): return x.pos if x else None

  def to_tuple(self):
    return (self.left.pos, self.word.pos, self.right.pos)



  def __hash__(self):
    return hash(self.to_tuple())

  def __repr__(self):
    return " ".join(map(str, self.to_tuple()))

  def __eq__(self, other):
    return self.to_tuple() == other.to_tuple()

  def match(self, pos_context):
    return tuple(map(self.pos, self.to_tuple())) == pos_context

  @staticmethod
  def from_string(s):
    left,word,right = s.strip().split()
    return Context(SimpleWord(None,left),SimpleWord(None,word),SimpleWord(None,right))
  
  @staticmethod
  def make_context(sent, i):
    if i == 0:
      return Context(SimpleWord(None, "START"), sent.words[i], sent.words[i+1])
    elif i == len(sent.words)-1:
      return Context(sent.words[i-1],sent.words[i], SimpleWord(None, "END"))
    else:
      return Context(sent.words[i-1],sent.words[i], sent.words[i+1])

  @staticmethod
  def contexts_from_sent(sent):
    for i, words in sent.enum_words():
      local_context = Context.make_context(sent, i)
      yield local_context



# class Shape:
#   def __init__(self, offsets):
#     self.offsets = offsets

#   def __eq__(self, other):
#     return self.offsets = other.offsets

#   def __hash__(self, other):
#     return hash(self.offsets)

# shapes=  [shape([-1,1]),
#           shape([-2,-1,1,2]),
#           shape([-4,-3,-2,-1]),
#           shape([1,2,3,4]),
#           shape([-1,1,2,3]),
#           shape([-3,-2,-1,1])]

# brown_religious/                   constraints_00001100
# constraints_00001111               constraints_00011000
# constraints_00011100               constraints_00011110
# constraints_00110000               constraints_00111000
# constraints_00111100               constraints_01111000
# constraints_11110000               data_tokenized_context/       
def coarsen(pos):
  if pos[:2] == "NN":
    return pos[:2]
  return pos

class GeneralContext:
  SIZE = 4
  MASKS = [
#     [0,0,1,1,1,1,0,0],
#     [0,1,1,1,1,0,0,0], 
#     [0,0,0,1,1,1,1,0],
#     

#     [0,0,0,1,1,1,1,0],
#     [0,0,0,0,1,1,1,1],

#     [0,0,0,0,1,1,1,1],
    #[0,0,0,0,1,1,1,1],
    #[1,1,1,1,0,0,0,0],
    [0,0,0,0,1,1,1,0],
    [0,1,1,1,0,0,0,0],
    [0,0,1,1,1,0,0,0],
    [0,0,0,1,1,1,0,0],
    [0,0,0,1,1,0,0,0],
    [0,0,0,0,1,1,0,0],
    [0,0,1,1,0,0,0,0],
    [0,0,0,1,0,0,0,0],
    #[0,0,0,0,1,0,0,0],
    [0,0,0,0,0,0,0,0],

    # weird 
#     [0,1,0,1,0,0,0,0],
#     [0,0,1,0,1,0,0,0],
#     [0,0,0,1,0,1,0,0],
#     [0,0,0,0,1,0,1,0],
 
    ]

  
  def __init__(self, word, boundaries, mask= None):
    self.word = word
    assert len(boundaries) == GeneralContext.SIZE *2
    self.boundaries = boundaries
    self._mask = mask

  def punc_check(self):
    if self.word.pos in bigpunc:
      return False
    if any([w.pos in punc for w in self.boundaries]):
      return False
    return True
    
    
  def to_tuple(self):
    def get_pos(w):
      #print >>sys.stderr, w.pos, w.word
      #if w.pos == "IN" or w.pos == "TO":
      #  return w.pos + "(" + w.word.lower() +")"
      return coarsen(w.pos)
    
    return tuple( [get_pos(w) for w in self.boundaries] + [get_pos(self.word)]) #(self.left.pos, self.word.pos, self.right.pos)

  def __hash__(self):
    return hash(self.to_tuple())

  def __repr__(self):
    return " ".join(map(str, self.to_tuple()))

  def __eq__(self, other):
    return self.to_tuple() == other.to_tuple()

  def mask(self, mask):
    new_boundaries = []
    for i, word in enumerate(self.boundaries):
      if mask[i]:
        new_boundaries.append(word)
      else:
        new_boundaries.append(SimpleWord(None, "*"))
    return GeneralContext(self.word, new_boundaries, mask)

  @staticmethod
  def from_string(s):
    tmp = s.strip().split()
    boundaries = [ SimpleWord(None, w) for w in tmp[:GeneralContext.SIZE*2]]
    word = SimpleWord(None, tmp[GeneralContext.SIZE*2])
    return GeneralContext(word, boundaries)
  
  @staticmethod
  def make_context(sent, i):
    word = sent.words[i]
    boundaries = []
    #print >>sys.stderr, "Make Contexts"
    for offset in range(-GeneralContext.SIZE, GeneralContext.SIZE+1):
      if offset == 0: continue
      pos = i + offset
      if pos <= 0:
        boundaries.append(SimpleWord(None, "START"))
      elif pos >= len(sent.words)-1:  
        boundaries.append(SimpleWord(None, "END"))
      else:
        #print >>sys.stderr, sent.words[pos].pos, sent.words[pos].word 
        boundaries.append(sent.words[pos])
        
    return GeneralContext(word,boundaries)

  @staticmethod
  def contexts_from_sent(sent):
    #print >>sys.stderr, "Contexts"
    for i, words in sent.enum_words():
      local_context = GeneralContext.make_context(sent, i)
      yield local_context
      
def constraints_from_context(sent, context):
  head_map = sent_to_head_map(sent)
  for i, words in sent.enum_words():
    local_context = Context.make_context(sent, i)
    #print local_context, context
    if local_context.match(context):
      #print "match", local_context
      for tag, inds in head_map.iteritems():
        yield i, tag, inds 
    


def sent_to_head_map(sent):
  """
  Takes a sent and returns a dict from pos tag to all the positions 
  with that tag
  """
  heads = {}
  for i, word in sent.enum_words(): 
    heads.setdefault(word.pos, [])
    heads[word.pos].append(i)
  return heads

    

    
if __name__=="__main__":
  import sys
  context = ("NNP", "NNP", "NNP")
  all_t = []
  
  appears_in = set()
  for i, sent in enumerate(parse_conll_file(open(sys.argv[1]))):
    for orig_ind, pos, head_ind in constraints_from_context(sent, context):
      appears_in.add(i)
      all_t.append(( i, pos, orig_ind, head_ind))
  
  res = sorted(all_t, key=lambda x: x[1])
  #groups = len(list(groupby(res, lambda x: x[1])))
  #print groups
  i = 0
  buf = StringIO()
  for  (k, a) in groupby(res, lambda x: x[1]):
    group_appears = set()
    a1, a2= tee(a, 2)
    for v in a1:
      sent_num = v[0]
      group_appears.add(sent_num)
    #print group_appears, appears_in
    if group_appears == appears_in:
      for v in a2:
        print >> buf, k, i, v[0], v[2], len(v[3]), " ".join(map(str,v[3]))
      print >> buf
      i+=1
  print i
  print buf.getvalue()
