class ConllWord:
  def __init__(self, num, word, pos, head, label):
    self.num = num
    self.word = word
    self.pos = pos
    self.head = head
    self.label = label

  @staticmethod
  def root():
    return ConllWord(0, '*ROOT*', '*ROOT*', 0, "_")
    
  def __repr__(self):
    return " ".join(map(str, (self.word, self.pos)))

  def remove_label(self):
    self.label = "_"

  def format(self):
    return "\t".join((str(self.num), self.word, self.word, self.pos, self.pos, "_", str(self.head), self.label))

class ConllSent:
  def __init__(self, words):
    self.words = (ConllWord.root(),) + tuple(words)
    
  def enum_words(self):
    return enumerate(self.words)

  def blank(self):
    for w in self.words:
      w.head = None

  def __iter__(self):
    return self.words.__iter__()

  def __repr__(self):
    
    return "\n".join([str(w)+ " " + str(self.words[w.head]) for w in self.words])
    #return map(str, (self.num, self.pos, self.head))
  
  def remove_labels(self):
    for w in self.words:
      w.remove_label()
    return self

  def format(self):
    return "\n".join(w.format() for w in self.words[1:]) 


def parse_conll_file(handle):
  def chunk():
    total = []
    for l in handle:
      if not l.strip():
        yield total
        total = []
      else:
        total.append(l.strip())
  for c in chunk():
    words = []
    for l in c:
      
      num, word, _, pos, _, _, head, label =  l.split("\t", 7)
      word = ConllWord(int(num), word, pos, int(head), label)
      words.append(word)
    yield ConllSent(words)
    words = []
