from pickle import *
import sys
from testcorpus.corpus import *
import gzip
from unknown_words import *
direct = "/data/nlp4/srush/mikedata/roi/output_context/"
direct2 = "/data/nlp4/srush/mikedata/roi/data_tokenized_context/"
files = ["nyt_stripped_sentences_1st_1000000_tokenized_part"+str(i) for i in range(1,2)]
# files = ["nyt_stripped_sentences_1st_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_9th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_13th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_5th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_4th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_6th_1000000_tokenized_part"+str(i) for i in range(1,5)]

if __name__ == "__main__":

  
  wc = load(open(sys.argv[1], 'rb'))
  test_file = open(sys.argv[2])
  clusters = BrownClusters.from_handle(open(sys.argv[3]))

  corpus = []
  wordmap = []
  for f in files:
    context_map = ContextMap.from_handle(open(direct + "out2_context_"+ f))
    wordmap.append(WordMap.from_handle(open(direct + "out1_context_"+f), context_map))
    corpus.append(Corpus.from_handle(open(direct2 + f)))

  #print "loaded"
  
  seen = set()
  to_write = []
  for sent_num, l in enumerate(test_file):
    if sent_num > 2 : break
    seen.add(hash(l.strip()))
    print l.strip()
    words = l.strip().split()
    for w_i, w in enumerate(words):
      if wc.count(w) < 5:
        w_left = words[w_i-1] if w_i <> 0 else "START" 
        w_right = words[w_i+1] if w_i <> len(words)-1 else "END"
        ctxt = Context.from_words(clusters, w_left, w_right)

        sents = [] 
        for i in range(len(wordmap)):
          sents.extend(wordmap[i].get_sentences(w,
                                             ctxt,
                                             corpus[i]))
        #print "Word %s Context %s %s"%(w, ctxt, len(sents))
        if len(sents) < 3:
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

