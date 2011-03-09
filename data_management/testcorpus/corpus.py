import sys
from itertools import *

class BrownClusters:
  def __init__(self, cluster_map):
    self.word_to_cluster = cluster_map

  def lookup(self, word):
    if word in ["START", "END"]:
      return word
    return self.word_to_cluster.get(word, None)

  @staticmethod
  def from_handle( handle):
    "0000	Disintegrated	1"
    d = {}
    for l in handle:
      t = l.strip().split()
      d[t[1]] = t[0]
    return BrownClusters(d)

  @staticmethod
  def trim(bitstring, precision):
    if bitstring and bitstring <> "START" and bitstring <> "END":
      return bitstring[:precision]
    return bitstring
      
class Context:
  def __init__(self, bit_left, bit_right, lprecision, rprecision ):
    self.bit_left =  BrownClusters.trim(bit_left,  lprecision)
    self.bit_right = BrownClusters.trim(bit_right, rprecision)

  def use(self):
    return (self.bit_left, self.bit_right)

  def __hash__(self):
    return hash(self.use())#(self.bit_left, self.bit_right))

  def __cmp__(self, other):
    return cmp(self.use(), other.use())


  @staticmethod
  def from_words(clusters, left_word, right_word):
    return (clusters.lookup(left_word),
            clusters.lookup(right_word))

  def __repr__(self):
    return str((self.bit_left, self.bit_right))
    
  @staticmethod
  def from_string(str):
    """
    1 01010111100111 001111101
    """
    t = str.strip().split()
    return (t[1], t[2])
    
class ContextMap:
  @staticmethod
  def from_handle(handle):
    d= {}
    for l in handle:
      t = l.strip().split()
      local_id = int(t[0])
      context = Context.from_string(l)
      d[local_id] = context
    return ContextMap(d)

  def __init__(self, cmap):
    self.cmap = cmap

  def lookup(self, context_index):
    return self.cmap[context_index]
  
class Corpus:
  @staticmethod
  def from_handle(handle):
    return Corpus((" ".join(w for w in l.strip().split() if w <> "0") for l in handle))
  
  def __init__(self, sentences):
    self.sentences = sentences

  def get_sentence(self, i):
    return (i, self.sentences[i])

class Word:
  def __init__(self, word, indices, context):
    self.word = word
    self.indices = indices
    self.context = context
    

class WordMap:
  @staticmethod
  def from_handle(handle, context_map):
    words = []
    for l in handle:
      fields = l.strip().split()
      word = fields[0]
      context = context_map.lookup(int(fields[1]))

      _ = fields[2]
      indices = map(int, fields[3:])
      words.append(Word(word, indices, context))
      #print word, context, indices, hash((word, context))
    print "building"
    return WordMap(words)

  def __init__(self, words):
    self.d_both = {}
    self.d_left = {}
    self.d_none = {}
    self.words = words
    for w in words:
      lclust, rclust = w.context
      
      key1 = (w.word, Context(lclust, rclust, 4, 4))
      self.d_both.setdefault(key1, [])
      self.d_both[key1].append(w)

      key2 = (w.word, Context(lclust, rclust, 4, 0))
      self.d_left.setdefault(key2, [])
      self.d_left[key2].append(w)

      key3 = (w.word, Context(lclust, rclust, 0, 0))
      self.d_none.setdefault(key3, [])
      self.d_none[key3].append(w)

  
  def get_sentences(self, word, (lword,rword), corpus):
    key1 = (word, Context(lword, rword, 4, 4))
    w = self.d_both.get(key1, None)
    if w:
      return [corpus.get_sentence(i) for rs in w
              for i in rs.indices]
    else:
      key2 = (word, Context(lword, rword, 4, 0))
      w = self.d_left.get(key2, None)
      if w:
        return [corpus.get_sentence(i) for rs in w
                for i in rs.indices]
      else:
        key3 = (word, Context(lword, rword, 0, 0))
        w = self.d_none.get(key3, None)
        if w:
          return [corpus.get_sentence(i) for rs in w
                  for i in rs.indices]
        else:
          return []


if __name__=="__main__":
  for sent in Corpus.from_handle(sys.stdin):
    print " ".join(sent)


