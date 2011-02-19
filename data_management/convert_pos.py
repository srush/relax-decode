import sys
root = "/home/srush/Projects/relax_decode/data_management/"
sys.path.append("../interfaces/hypergraph/gen-py/")
from hypergraph_pb2 import *
from features_pb2 import *
from tag_pb2 import *
from dep_pb2 import *
import random
sent = -1
last_node = -1
name = sys.argv[1]
q = open( name , "w")
print >>q, random.random()
q.close()

for i, l in enumerate(sys.stdin):

  t = l.strip().split()
  if t[1] == "START":

    h = Hypergraph()
    nodes = {}
    cur_edge_id = 0 
    cur_node_id = 0
    sent +=1

  elif t[1] == "END":
    h.root = last_node
    print "ROOT NODE", last_node
    f = open(name + str(sent), "wb")
    f.write(h.SerializeToString())
    f.close()

  elif t[1] == "EDGE":
    from_id = int(t[4])
    #print from_id
    edge = nodes[from_id].edge.add()
    edge.Extensions[edge_fv] = "value="+t[5]
    edge.tail_node_ids.append( int(t[3]))
    edge.label = t[2]

    
    edge.id = cur_edge_id
    #print "EDGE", from_id, t[4], edge.id, t[5]
    cur_edge_id += 1
    #edge.edge_fv

  elif t[1] == "NODE":
    node = h.node.add()
    node.id = int(t[2])
    node.label = t[3]
    nodes[node.id] = node
    last_node = node.id
    if cur_node_id <> node.id:
      print "Failed on ", i, cur_node_id, node.id
      assert False;
    cur_node_id +=1 
    #print "NODE", node.id

    if node.label <> "final" and node.label <> "START":
      ta = node.Extensions[tagging]
      ta.ind = int(node.label.split(':')[0])
      ta.tag_id = int(node.label.split(':')[-1])
      
      node.Extensions[has_tagging] = True
