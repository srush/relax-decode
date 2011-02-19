import sys


class Corpus:
  @staticmethod
  def from_handle(handle):
    return Corpus([" ".join([w for w in l.strip().split() if w <> "0"])for l in handle])
  
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
  def from_handle(handle):
    words = []
    for l in handle:
      fields = l.strip().split()
      word = fields[0]
      context = int(fields[1])
      _ = fields[2]
      indices = map(int, fields[3:])
      words.append(Word(word, indices, context) )
    return WordMap(words)

  def __init__(self, words):
    self.d = dict([((w.word, w.context), w)for w in words])
  
  def get_sentences(self, word, context, corpus):
    w = self.d.get((word, context), None)
    if w:
      return [corpus.get_sentence(i) for i in w.indices]
    else:
      return []



if __name__=="__main__":
  for sent in Corpus.from_handle(sys.stdin):
    print " ".join(sent)


