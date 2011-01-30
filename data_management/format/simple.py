class SimpleWord:
  def __init__(self, word, pos=None):
    self.pos = pos
    self.word = word

  def __repr__(self):
    if self.pos:
      return "/".join((self.word,  self.pos))
    return self.word

class SimpleSent:
  def __init__(self, words):
    self.words = tuple(words)
    
  def enum_words(self):
    return enumerate(self.words)

  def __iter__(self):
    return self.words.__iter__()  

  def __repr__(self):
    return " ".join(str(w) for w in self.words) 

def parse_simple_file(handle):
  for c in handle:
    words = []
    for l in c.strip().split():
      if "/" in l:
        word, pos =  l.split("/")
      else: 
        word = l
        pos = None
      word = SimpleWord(word, pos)
      words.append(word)
    yield SimpleSent(words)
    words = []
