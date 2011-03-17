import MySQLdb
import sys
from testcorpus.corpus import *
import gzip

conn = MySQLdb.connect (host = "mysql.csail.mit.edu",
                        user = "srush",
                        passwd = "baseball",
                        db = "emnlp2011")

direct = "/data/nlp4/srush/mikedata/roi/feb_27_2011_context/"
direct2 = "/data/nlp4/srush/mikedata/roi/feb_27_2011_context/"
files = ["nyt_9_sentences_%s_%s"%(i,b) for b in (1,2) for i in range(1,11)]



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
    wordmap = WordMap.from_handle(open(direct + "out1_"+f), None)
    corpus = Corpus.from_handle(open(direct2 + f))


    # add file to file table
    cursor = conn.cursor ()
    cursor.execute ("INSERT INTO file(original_name) VALUES ('%s')"%f)
    cursor.execute ("SELECT id FROM file WHERE original_name='%s'"%f)
    row = cursor.fetchone ()
    file_id = row[0]
    cursor.close ()


    

    # add sentences to sentence table    
    cursor = conn.cursor ()
    q= "INSERT INTO sentence(file_id, id , sentence) VALUES (%s, %s,%s)"
    inserts = ((file_id, sent_num, sentence) for sent_num, sentence in enumerate(corpus.sentences))
      
    #print q
    cursor.executemany(q, inserts)
      
      #print file_id, sent_num, sentence
    cursor.close()

#     # add words to word table
    cursor = conn.cursor ()
    q = "INSERT INTO word_token(type,sent_id, file_id, left_context, right_context) VALUES (%s, %s, %s, %s, %s)"
    #print q
    inserts = (( w.word, ind, file_id, w.context[0], w.context[1])
               for w in wordmap.words
               for ind in w.indices)
    cursor.executemany(q, inserts)
    cursor.close()
  
conn.close ()
