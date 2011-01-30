class ConllTagWord:
  def __init__(self, word, pos):
    self.pos = pos
    self.word = word

  def __repr__(self):
    return "\t".join((self.word,  self.pos))
    

class ConllTagSent:
  def __init__(self, words):
    self.words = tuple(words)
    
  def enum_words(self):
    return enumerate(self.words)

  def __iter__(self):
    return self.words.__iter__()  

  def __repr__(self):
    return "\n".join(str(w) for w in self.words) 

  @staticmethod 
  def from_conll(conll):
    return ConllTagSent([ConllTagWord(w.word, w.pos) for w in conll.words[1:]])


