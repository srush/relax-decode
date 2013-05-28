# -*- coding: utf-8 -*-
import os,sys
sys.path.append("../gen-py/")
from lattice_pb2 import *


def reverse_lat(lat):
  back_edges ={} 
  by_id = {}
  for n in lat.node:
    by_id[n.id] = n 
    for e in n.edge:
      back_edges.setdefault(e.to_id, [])
      back_edges[e.to_id].append((n.id, e))
  
  ret = Lattice()
  ret.start = lat.final[0]
  ret.final.append(lat.start)
  for n_id in range(len(lat.node)):
    node = ret.node.add()
    node.id = n_id
    node.label = by_id[n_id].label
    
    
    node.Extensions[has_phrases] = by_id[n_id].Extensions[has_phrases]
    

    for old_plet in by_id[n_id].Extensions[phraselets].phraselet:
      plet = node.Extensions[phraselets].phraselet.add()
      plet.phraselet_hypergraph_edge = old_plet.phraselet_hypergraph_edge
      plet.hypergraph_edge_position = old_plet.hypergraph_edge_position
      old = list(old_plet.word)
      old.reverse()
      for sword in old: 
        new_word = plet.word.add()
        new_word.subword_original_id = sword.subword_original_id
        new_word.subword_hypergraph_node_id = sword.subword_hypergraph_node_id
        new_word.word = sword.word
    #node.Extensions[is_word] = by_id[n_id].Extensions[is_word]
    #node.Extensions[word] = by_id[n_id].Extensions[word]
    #print by_id[n_id].Extensions[word], n_id
    
    
    #node.Extensions[original_node] = by_id[n_id].Extensions[original_node]
    #node.Extensions[ignore_node] = by_id[n_id].Extensions[ignore_node]
    
    if  n_id in back_edges:
      for to_node,e in back_edges[n_id]:
        edge = node.edge.add()
        edge.to_id = to_node
        
        edge.id =e.id
        edge.label =e.label
        
        edge.Extensions[origin].original_id = e.Extensions[origin].original_id
        for i in e.Extensions[origin].hypergraph_edge:
          edge.Extensions[origin].hypergraph_edge.append(i)
        for i in e.Extensions[origin].hypergraph_edge_position:
          edge.Extensions[origin].hypergraph_edge_position.append(i)

        edge.Extensions[origin].has_origin = e.Extensions[origin].has_origin
  ret.Extensions[num_hypergraph_edges] = lat.Extensions[num_hypergraph_edges]
  print lat.Extensions[num_hypergraph_edges]
  ret.Extensions[num_original_ids] = lat.Extensions[num_original_ids]
  return ret

if __name__ == "__main__":
  for i in range(1,101):
    lat = Lattice()  
    f = open(sys.argv[1]+str(i), "rb")
    lat.ParseFromString(f.read())
    f.close()

    rlat = reverse_lat(lat)

    f = open(sys.argv[2] + str(i), "wb")
    print "done"
    f.write(rlat.SerializeToString())
    f.close()
