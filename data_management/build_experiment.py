from pickle import *
import sys
from testcorpus.corpus import *
import gzip
from unknown_words import *
import MySQLdb
# direct = "/data/nlp4/srush/mikedata/roi/output_context/"
# direct2 = "/data/nlp4/srush/mikedata/roi/data_tokenized_context/"
# files = ["nyt_stripped_sentences_1st_1000000_tokenized_part"+str(i) for i in range(1,2)]

direct = "/data/nlp4/srush/mikedata/roi/feb_27_2011_context/"
direct2 = "/data/nlp4/srush/mikedata/roi/feb_27_2011_context/"
files = ["nyt_9_sentences_%s_%s"%(i,b) for b in (1,) for i in range(1,2)]

conn = MySQLdb.connect (host = "mysql.csail.mit.edu",
                        user = "srush",
                        passwd = "baseball",
                        db = "emnlp2011")


# files = ["nyt_stripped_sentences_1st_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_9th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_13th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_5th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_4th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#         ["nyt_stripped_sentences_6th_1000000_tokenized_part"+str(i) for i in range(1,5)]


class AdditionalSent:
  def __init__(self, sent, left_con, right_con, original_word, word_ind, original_sent_num):
    self.sent = sent
    self.left_con = left_con
    self.right_con = right_con
    self.original_word = original_word
    self.original_sent_num = original_sent_num
    self.original_word_ind = word_ind 

  def good_sent(self):
    sent = self.sent
    w = self.original_word
    index = [i for i,w1 in enumerate(sent.split()) if w1.lower() == w.lower()][0]


    l1 = len(sent.split()) > 10
    l2 = len(sent.split()) < 25
    if not (l1 and l2):
      return False
    bad_pairs= [',', ':', '.', ';']
    if (index != 0 and sent.split()[index-1] in bad_pairs) or (index != len(sent.split()) -1 and  sent.split()[index+1] in bad_pairs):
      return False

    bad_word = all([s not in [':', '``', "''", "(", ")", "--", "..."] and not s.isupper() for s in sent.split() ])
    #bad_word = True
    is_good = bad_word and sent.split()[-1] in ['.', '?']
    return is_good
    
  def num_known(self):
    w= self.original_word
    s = self.sent.split()
    blacklist = ['.', ':', ',', '``', "''", "(", ")"]
    index = [i for i,w1 in enumerate(s) if w1.lower() == w.lower()]
    if len(index) > 1: return False
    index = index[0]
    num_context_known = 0
    if index != 0 and wc.count(s[index-1]) > 0 and s[index-1] not in blacklist:
      num_context_known +=1 
    if index > 1 and wc.count(s[index-2]) > 0 and s[index-2] not in blacklist:
      num_context_known +=1
    if index != len(s) -1 and wc.count(s[index+1]) > 0 and s[index+1] not in blacklist:
      num_context_known +=1 
    if index < len(s) -2 and wc.count(s[index+2]) > 0 and s[index+2] not in blacklist :
      num_context_known +=1 
    #print s, w, num_context_known
    return (num_context_known, sum([ 1 if wc.count(w)>=1 else 0 for w in s]) / float(len(s)))

#   def close_context(self, sent, l , r):
#     l_close = 0
#     r_close = 0
#     for a,b in zip(l,ctxt[0] if ctxt[0] else []):
#       if a == b: l_close+=1
#       else: break
#     for a,b in zip(r,ctxt[1] if ctxt[1] else []):
#       if a == b: r_close+=1
#       else: break
#     return min(3,l_close) + min(3,r_close)
  

if __name__ == "__main__":

  
  wc = load(open(sys.argv[1], 'rb'))
  test_file = open(sys.argv[2])
  clusters = BrownClusters.from_handle(open(sys.argv[3]))
  num_take_sent = int(sys.argv[4])
  added_desc = open(sys.argv[5],'w')

  corpus = []
  wordmap = []
#   for f in files:
#     #context_map = ContextMap.from_handle(open(direct + "out2_context_"+ f))
#     wordmap.append(WordMap.from_handle(open(direct + "out1_"+f), None))
#     corpus.append(Corpus.from_handle(open(direct2 + f)))

  #print "loaded"


  seen = set()
  to_write = []
  for sent_num, l in enumerate(test_file):
    if sent_num > num_take_sent +1 : break
    seen.add(hash(l.strip()))
    print l.strip()
    words = l.strip().split()
    for w_i, w in enumerate(words):
      for morpho in ("REG", "PL"): # "PAST"):
        if morpho == "PL":
          w = w+"s"
#         if morpho == "PAST":
#           if  w[-1] != 's':
#             continue
#           else:
#             w = w[:-1] + 'd'
        if morpho == ["PL", "PAST"] or wc.count(w) < 1:
          w_left = words[w_i-1] if w_i <> 0 else "START" 
          w_right = words[w_i+1] if w_i <> len(words)-1 else "END"
          ctxt = Context.from_words(clusters, w_left, w_right)
          print >>sys.stderr, ctxt

          sents =[] 
          cursor = conn.cursor ()
          query = """select s.sentence, w.left_context, w.right_context from word_token as w,
                   sentence as s where  w.type = "%s"
                   and s.file_id = w.file_id and  s.id = w.sent_id limit 1000;"""
          print >>sys.stderr, query%w
          cursor.execute (query%w)

          for row in cursor:
            sents.append(AdditionalSent(row[0], row[1], row[2], w, w_i, sent_num))
          if morpho == "PL" and sents < 10:
            continue
          #print >>sys.stderr, [w == s[0].split()[0] or w == s[0].split()[1] for s in sents[:10]] 
          is_first_cap = w[0].isupper() and sum([1 if w_i < 2 else 0 for s in sents[:10]]) > 6 
          cursor.close()

          if is_first_cap:
            cursor = conn.cursor ()
            cursor.execute (query%w.lower())
            sents = []
            for row in cursor:
              sents.append(AdditionalSent(row[0], row[1], row[2], w, w_i, sent_num))
            cursor.close()

          sents = list(set(sents))
          print >>sys.stderr, "words "+ w+  " sents " + str(len(sents))

          take_sents = [ sent for sent in sents if sent.good_sent() ]
          if len(sents) < 50:
            take_sents = sents
          take_sents.sort(key = lambda sent: sent.num_known())
          take_sents.reverse()
          print >> sys.stderr, w, len(take_sents)
          to_write.extend(take_sents[:6])
        
  print >>sys.stderr, "done", len(to_write)
  
  seen = set()
  out_num = num_take_sent
  for sent in to_write:
    h = hash(tuple(sent.sent.strip().split()[:10]) + (sent.original_sent_num,))
    if h in seen: continue
    print sent.sent.strip()
    seen.add(h)
    print >>added_desc, out_num, sent.original_sent_num, sent.original_word_ind 
    out_num+=1
conn.close ()
