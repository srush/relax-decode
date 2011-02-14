from itertools import *
from connl_format import *
from StringIO import StringIO

class Context:
  def __init__(self, left, word, right):
    self.word = word
    self.left = left
    self.right = right
  
  @classmethod
  def pos(cls, x): return x.pos if x else None

  def to_tuple(self):
    return (self.left, self.word, self.right)

  def __repr__(self):
    return " ".join(map(str, self.to_tuple()))

  def __eq__(self, other):
    
    return map(self.pos, self.to_tuple()) == map(self.pos, other.to_tuple())

  def match(self, pos_context):
    return tuple(map(self.pos, self.to_tuple())) == pos_context


  @staticmethod
  def make_context(sent, i):
    if i == 0:
      return Context(None, sent.words[i], sent.words[i+1])
    elif i == len(sent.words)-1:
      return Context(sent.words[i-1],sent.words[i], None)
    else:
      return Context(sent.words[i-1],sent.words[i], sent.words[i+1])


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
