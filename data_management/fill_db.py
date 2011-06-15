import MySQLdb
import sys
from testcorpus.corpus import *
import gzip

conn = MySQLdb.connect (host = "mysql.csail.mit.edu",
                        user = "srush",
                        passwd = "baseball",
                        db = "emnlp2011")

direct =  "/data/nlp4/srush/mikedata/roi/nips/spanish/"
direct2 = "/data/nlp4/srush/mikedata/roi/nips/spanish/"
files = ["spanish_abstracts_tokenized%s.gz"%i for i in range(2,10)]
LANGUAGE = "spanish"


# files = ["nyt_stripped_sentences_13th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#          ["nyt_stripped_sentences_5th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#          ["nyt_stripped_sentences_4th_1000000_tokenized_part"+str(i) for i in range(1,5)] + \
#          ["nyt_stripped_sentences_6th_1000000_tokenized_part"+str(i) for i in range(1,5)]


#["nyt_stripped_sentences_9th_1000000_tokenized_part"+str(i) for i in range(1,5)]
#["nyt_stripped_sentences_1st_1000000_tokenized_part"+str(i) for i in range(2,3)]


if __name__ == "__main__":
  corpus = []
  wordmap = []
  for f in files:
    #context_map = ContextMap.from_handle(open(direct + "out2_context_"+ f))
    wordmap = WordMap.from_handle(gzip.open(direct + "out1_"+f), None)
    corpus = Corpus.from_handle(gzip.open(direct2 + f))

    print >>sys.stderr, "Done reading files"
    # add file to file table
    cursor = conn.cursor ()
    cursor.execute ("INSERT INTO file(original_name, language) VALUES ('%s', '%s')"%(f, LANGUAGE))
    cursor.execute ("SELECT id FROM file WHERE original_name='%s'"%f)
    row = cursor.fetchone ()
    file_id = row[0]
    cursor.close ()


    print >>sys.stderr, "File added"
    # add sentences to sentence table    
    cursor = conn.cursor ()
    q= "INSERT INTO sentence(file_id, id , sentence) VALUES (%s, %s,%s)"
    inserts = ((file_id, sent_num, sentence) for sent_num, sentence in enumerate(corpus.sentences))
    #inserts = list(inserts)[:100]
    #print q
    cursor.executemany(q, inserts)      
      
      #print file_id, sent_num, sentence
    cursor.close()

    print >>sys.stderr, "Done adding sentences"
#     # add words to word table
    cursor = conn.cursor ()
    q = "INSERT INTO word_token(type,sent_id, file_id, left_context, right_context) VALUES (%s, %s, %s, %s, %s)"
    #print q
    inserts = (( w.word, ind, file_id, w.context[0], w.context[1])
               for w in wordmap.words
               for ind in w.indices)
    #inserts =  list(inserts)[:100]
    cursor.executemany(q, inserts)
    cursor.close()
    print >>sys.stderr, "Done adding words"
conn.close ()
