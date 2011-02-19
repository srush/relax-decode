from pickle import *
import sys
from testcorpus.corpus import *
import gzip
from unknown_words import *
if __name__ == "__main__":
  wc = load(open(sys.argv[1], 'rb'))
  test_file = open(sys.argv[2])
  wordmap = WordMap.from_handle(gzip.open(sys.argv[3]))
  corpus = Corpus.from_handle(gzip.open(sys.argv[4]))
  
  seen = set()
  to_write = []
  for sent_num, l in enumerate(test_file):
    if sent_num > 20 : break
    seen.add(hash(l.strip()))
    print l.strip()
    for w in l.strip().split():
      if wc.count(w) < 5:
        sents = wordmap.get_sentences(w, 0, corpus)
        if len(sents) < 10:
          sents = [ (n,sent) for n, sent in sents ]
        else:
          sents = [ (n,sent) for n, sent in sents if len(sent.split()) <= 30]
        to_write.extend(sents[:10])
        #print "UNK WORD IS %s %s %s" %( w, wc.count(w), len(to_write)) 
        
  for id, sent in to_write:
    h = hash(sent.strip())
    if h in seen: continue
    print sent.strip()
    seen.add(h)

