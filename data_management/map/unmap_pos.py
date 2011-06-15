import sys, os
root = os.getenv("SCARAB_ROOT") + "/data_management/"
sys.path.append(root)
from format.simple import *
from itertools import *

import gflags as flags
FLAGS = flags.FLAGS
flags.DEFINE_string('language', 'ENGLISH', 'language to use')


class Unmapper:
  def __init__(self):
    self.pos_map = {}
    self.other_map = {}
    for l in open(root + 'map/' + FLAGS.language + '.map'):
      num, word=  l.strip().split()
      self.pos_map[int(num)] = word  
      self.other_map[word] = int(num)  
      
  def string_to_ind(self,word):
    return self.other_map[word]

if __name__ == "__main__":

  try:
    argv = FLAGS(sys.argv)  
  except flags.FlagsError, e:
    print '%s\\nUsage: %s ARGS\\n%s' % (e, sys.argv[0], FLAGS)
    sys.exit(1)

  simple_sent = open(argv[1])
  pos_map = {} 
  for l in open(root + 'map/' + FLAGS.language + '.map'):
    num, word =  l.strip().split()
    pos_map[int(num)] = word  
  
  for sent_num, (sent, in_line) in  enumerate(izip(parse_simple_file(simple_sent),sys.stdin )):
    assert sent, sent_num
    #print >>sys.stderr, in_line
    orig_len = len(sent.words) 
    graph_len = len(in_line.strip().split()[1:-1])
    assert orig_len == graph_len,  "%s %s %s %s %s" %(sent_num, sent.words, orig_len, in_line, graph_len)
    for i, word_pair in enumerate(in_line.strip().split()[1:-1]):
      word_num, pos_num = map(int, word_pair.split("/"))
      
      assert i == word_num, str(i) + " " + str(word_num) + " " +str(sent_num) 
      
      assert pos_num in pos_map
      #print >>sys.stderr, i
      print sent.words[i].word + "/" +pos_map[pos_num],
    print 
