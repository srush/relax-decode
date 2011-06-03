class SimpleWord:
  def __init__(self, word, pos=None):
    self.pos = pos
    self.word = word

  def strip_pos(self):
    self.pos = None
    return self

  def __repr__(self):
    if self.pos:
      return "/".join((self.word,  self.pos))
    return self.word

class SimpleSent:
  def __init__(self, words):
    self.words = tuple(words)
    
  def enum_words(self):
    return enumerate(self.words)

  def strip_pos(self):
    self.words = [w.strip_pos() for w in self.words]

  def __iter__(self):
    return self.words.__iter__()  

  def __repr__(self):
    return " ".join(str(w) for w in self.words) 

def parse_simple_file(handle, divider=None):
  for c in handle:
    words = []

    for l in c.decode("utf8").strip().split():
      if divider <> None:        
        t = l.split(divider)

        word, pos =  divider.join(t[0:-1]), t[-1]
        pos = pos.encode("utf8")
      else: 
        word = l
        pos = None
      
      word = SimpleWord(word.encode("utf8"), pos)
      words.append(word)
    yield SimpleSent(words)
    words = []

def parse_simple_columns(handle, divider=None):
  words = []
  for l in handle:
    if not l.strip():
      yield SimpleSent(words)
      words = []
    else:
      word, pos = l.strip().split()
      word = SimpleWord(word, pos)
      words.append(word)
    

