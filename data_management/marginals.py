
class Marginals:
  @staticmethod
  def from_handle(handle):
    data = []
    sent_num = -1
    for l in handle:
      if l.strip().split()[0] == "SENT": 
        sent_num = int(l.strip().split()[1])
      else:
        a, b, prob, c, d , tag= l.strip().split()
        ind,tag = tag.split('/')
        data.append(((sent_num, int(ind), int(tag)), float(prob)))
    return Marginals(data)

  def __init__(self, data):
    self.data = dict(data)
