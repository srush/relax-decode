# -*- coding: utf-8 -*-
#!/usr/bin/env python

# 199 2345
# 1   DT [0-1]    0 
# ...
# 6   NP [0-2]    1 
#       1 4 ||| 0=-5.342
# ...
    
import sys, os
sys.path.append("../gen_py/")
import json
from protobuf_json import pb2json

from hypergraph_pb2 import *
from features_pb2 import *
from translation_pb2 import *
from lexical_pb2 import *


new_style = True
order = 3
def load(handle):            
   line = None
   num_sents = 0        

   while True:
       forest = Hypergraph()
       line = handle.readline() 
       if len(line) == 0:
           break
       try:                        
           tag, sent = line.split("\t")   # foreign sentence
           
       except:
           ## no more forests
           yield None
           continue

       num_sents += 1
       node_id = 0
       edge_id = 0
       iden_map = {}
       
       ## read in references
       refnum = int(handle.readline().strip())
       for i in xrange(refnum):
           ref = handle.readline().strip()
           forest.Extensions[reference_sentences].append(ref)
           
       forest.Extensions[foreign_sentence] = sent.decode('UTF-8')
       
       ## sizes: number of nodes, number of edges (optional)
       num, nedges = map(int, handle.readline().split("\t"))   


       for i in xrange(1, num+1):
           node = forest.node.add()
           ## '2\tDT* [0-1]\t1 ||| 1232=2 ...\n'
           ## node-based features here: wordedges, greedyheavy, word(1), [word(2)], ...
           line = handle.readline()
           try:
               keys, fields = line.split(" ||| ")
           except:
               keys = line
               fields = ""

           orig_iden, labelspan, size = keys.split("\t") 
           size = int(size)

           

           node.id = node_id
           node_id+=1
           node.label = labelspan
           
           iden_map[orig_iden] = node

           fpair = node.Extensions[node_fv] =fields.strip().decode("UTF-8")
       
           for j in xrange(size):
               edge = node.edge.add()
               
               ## '\t1 ||| 0=8.86276 1=2 3\n'
               ## N.B.: can't just strip! "\t... ||| ... ||| \n" => 2 fields instead of 3
               tails, rule, fields = handle.readline().strip("\t\n").split(" ||| ")


               tails = tails.split() 
               tailnodes = []
               
               for x in tails:
                   if x[0]=='"': 
                       # convert words to nodes
                       local_lex_node = forest.node.add()
                       local_lex_node.id = node_id
                       node_id+=1
                       local_lex_node.label = x.decode('UTF-8')
                       local_lex_node.Extensions[is_word] = True
                       local_lex_node.Extensions[word] = x[1:-1].decode('UTF-8')
                       tailnodes.append(local_lex_node.id)
                   else: 
                       tailnodes.append(iden_map[x].id)
                       
               for t in tailnodes:
                   edge.tail_node_ids.append(t)
               edge.label = rule.decode('UTF-8')
               edge.id = edge_id
               edge_id+=1
               edge.Extensions[edge_fv] = fields.strip()
       
       if new_style:
           old_root = node.id
           new_root = forest.node.add()
           new_root.id = node_id
           node_id+=1
           new_root.label = "NEW ROOT".decode('UTF-8')

           new_edge = new_root.edge.add() 
           
           new_edge.id = edge_id
           edge_id+=1
           new_edge.Extensions[edge_fv] = ""
           
           for i in range(order-1):
              node = forest.node.add()
              node.id = node_id
              node_id+=1
              node.label = "Front".decode('UTF-8')
              node.Extensions[is_word] = True
              node.Extensions[word] = "<s>".decode('UTF-8')
              new_edge.tail_node_ids.append(node.id)
              
           new_edge.tail_node_ids.append(old_root)

           for i in range(order-1):
              node = forest.node.add()
              node.id = node_id
              node_id+=1
              node.label = "Back".decode('UTF-8')
              node.Extensions[is_word] = True
              node.Extensions[word] = "</s>".decode('UTF-8')
              new_edge.tail_node_ids.append(node.id)
           

           forest.root = new_root.id
       else:
           forest.root = node.id
       
       line = handle.readline()
       yield forest


if __name__ == "__main__":

    for i, forest in enumerate(load(sys.stdin),1):
        f = open(sys.argv[1] + str(i), "wb")
        #if i <> 2: continue
        f.write(forest.SerializeToString())
        f.close()

        
