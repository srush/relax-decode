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

class Extension:
  def __init__(self):
    pass

  def apply(self, word):
    return word

  def check(self, word):
    return True

class SExtension(Extension):
  def __init__(self):
    pass
  def apply(self, word):
    return word + "s"

class INGExtension(Extension):
  def apply(self,word):
    return  word[:-3]
  
  def check(self, word):
    return len(word)> 3 and  word[-3:] == "ing"

class MinusSExtension(Extension):
  def __init__(self):
    pass
  def apply(self, word):
    last_letter = word[-1]
    return word[:-1]
  
  def check(self, word):
    return word[-1] == 's'

class DExtension(Extension):
  def apply(self,word):
    penult_letter = word[-2]
    last_letter = word[-1]
    
    if penult_letter == "e" and last_letter == "s":
      return word[:-1] + "d"
    elif last_letter =="e":
      return word + "d"
    elif last_letter == "s":
      return word[:-1] + "ed"
    else:
      return word + "ed"

  def check(self, word):
    return len(word) > 1
  
class RExtension(Extension):
  def apply(self,word):
    last_letter = word[-1]
    if last_letter =="e":
      return word + "r"
    else:
      return word + "er"

class MinusRExtension(Extension):
  def apply(self, word):
    return word[:-2]
  
  def check(self, word):
    return len(word) >2 and word[-2:] == 'er'


class BasicExtension(Extension):
  def apply(self,word):
    return word


extensions = [SExtension()] #[MinusRExtension(), DExtension(), SExtension(), INGExtension(),  RExtension(), MinusSExtension()]


class AdditionalSent:
  def __init__(self, sent, left_con, right_con, original_word, word_ind, original_sent_num, morpho, original_context):
    self.sent = sent
    self.left_con = left_con
    self.right_con = right_con
    self.original_word = original_word
    self.original_sent_num = original_sent_num
    self.original_word_ind = word_ind
    self.morpho = morpho
    self.add_sent_index = [i for i,w1 in enumerate(sent.split()) if w1.lower() == w.lower()][0]
    self.ctxt = original_context
    
  def __hash__(self):
    return hash(self.sent)

  def __eq__(self, other):
    return self.sent == other.sent

  def good_sent(self):
    sent = self.sent
    w = self.original_word
    index = [i for i,w1 in enumerate(sent.split()) if w1.lower() == w.lower()][0]


    l1 = len(sent.split()) > 10
    l2 = len(sent.split()) < 25
    if not (l1 and l2):
      return False
    bad_pairs = [',', ':', '.', ';', 'and']
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

    # sort by the closeness of context
    context_close = 0
    def match(al, bl):
      m = 0
      for a, b in zip(al,bl):
        if a ==b: m+=1
        else: break
      return m
    
    if morpho <> "EXT":
      context_close = \
                    (match(self.ctxt[0], self.left_con) if self.ctxt[0] != None else 0) + \
                    (match(self.ctxt[1], self.right_con) if self.ctxt[1] != None else 0)
      #print >>sys.stderr, self.original_word, self.sent,  self.ctxt, self.left_con, self.right_con , context_close, num_context_known
      #context_close,
    return ( sum([ 1 if wc.count(w)>=1 else 0 for w in s]) / float(len(s)), num_context_known)

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
    total_reg_sents = 0
    for w_i, w in enumerate(words):
      original_word = w
      for morpho in ("REG", "EXT"):
        if morpho == "EXT" or wc.count(w) < 1:
          w_left = words[w_i-1].lower() if w_i <> 0 else "START" 
          w_right = words[w_i+1].lower() if w_i <> len(words)-1 else "END"
          print >>sys.stderr, words, w_i, w_left, w_right
          ctxt = Context.from_words(clusters, w_left, w_right)
          print >>sys.stderr, ctxt


          use_extensions = [BasicExtension()]
          if morpho == "EXT":
            use_extensions = extensions
          for ext in use_extensions:
            if not ext.check(original_word): continue 
            sents =[]
            w = ext.apply(original_word)
            cursor = conn.cursor ()
            query = """select s.sentence, w.left_context, w.right_context from word_token as w,
              sentence as s where  w.type = "%s"
              and s.file_id = w.file_id and  s.id = w.sent_id limit 2000;"""
            print >>sys.stderr, query%w
            cursor.execute (query%w)

            for row in cursor:
              sents.append(AdditionalSent(row[0], row[1], row[2], w, w_i, sent_num, morpho, ctxt))
            #print >>sys.stderr, [w == s[0].split()[0] or w == s[0].split()[1] for s in sents[:10]] 
            is_first_cap = w[0].isupper() and (sum([1 if w_i < 2 else 0 for s in sents[:10]]) > 6  or len(sents) < 10)
            cursor.close()
            if morpho == "EXT" and ( (len(sents) > 50)):
              break
          if (sents < 15 and not is_first_cap)  or (morpho == "EXT" and ((len(sents) < 50))):
            continue
          if morpho == "REG":
            total_reg_sents = len(sents)
          
          if is_first_cap:
            cursor = conn.cursor ()
            
            cursor.execute (query%w.lower())
            
            sents = []
            for row in cursor:
              sents.append(AdditionalSent(row[0], row[1], row[2], w, w_i, sent_num, morpho, ctxt))
            cursor.close()

          sents = list(set(sents))
          print >>sys.stderr, "words "+ w+  " sents " + str(len(sents))

          take_sents = [ sent for sent in sents if sent.good_sent() ]
          if len(sents) < 50:
            take_sents = sents
          take_sents.sort(key = lambda sent: sent.num_known())
          take_sents.reverse()
          print >> sys.stderr, w, len(take_sents)
          #if morpho=="EXT":
          to_write.extend(take_sents[:6])
          #else: 
          #  to_write.extend(take_sents[:4])
        
  print >>sys.stderr, "done", len(to_write)
  
  seen = set()
  out_num = num_take_sent
  for sent in to_write:
    h = hash(tuple(sent.sent.strip().split()[:10]) + (sent.original_sent_num,))
    #if h in seen: continue
    print sent.sent.strip()
    seen.add(h)
    print >>added_desc, out_num, sent.add_sent_index, sent.original_sent_num, sent.original_word_ind, sent.morpho 
    out_num+=1
conn.close ()
