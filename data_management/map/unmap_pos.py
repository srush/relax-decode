import sys
root = "/home/srush/Projects/relax_decode/data_management/"
sys.path.append(root)
from format.simple import *
from itertools import *


if __name__ == "__main__":

  
  simple_sent = open(sys.argv[1])
  pos_map = {} 
  for l in open(root + 'map/POS.map'):
    word, num=  l.strip().split()
    pos_map[int(num)] = word  
  
  for sent_num, (sent, in_line) in  enumerate(izip(parse_simple_file(simple_sent),sys.stdin )):
    for i, word_pair in enumerate(in_line.strip().split()[1:]):
      word_num, pos_num = map(int, word_pair.split("/"))
      
      assert i == word_num, str(i) + " " + str(word_num) + " " +str(sent_num) 
      
      assert pos_num in pos_map

      print sent.words[i].word + "/" +pos_map[pos_num],
    print 
