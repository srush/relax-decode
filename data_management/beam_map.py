class BeamMap:
  def __init__(self, d):
    self.d = d

  def lookup(self, sent_num, word_num):
    return self.d[sent_num, word_num]
  
  @staticmethod
  def from_handle(handle):
    d = {}
    sent_num = 0
    for l in handle:
      word = l.strip().split()
      if word[1] == "END":
        sent_num +=1
      elif word[1] == "START":
        pass
      else:
        word_num = int(word[1])
        pos_tags = map(int, word[2:])
        d[sent_num, word_num] = pos_tags
    return BeamMap(d)
