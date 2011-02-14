import sys
root = "/home/srush/Projects/relax_decode/data_management/"
sys.path.append("../../hypergraph/gen_py/")
from hypergraph_pb2 import *
from features_pb2 import *
from tag_pb2 import *
from dep_pb2 import *
sent = -1
last_node = -1
name = sys.argv[1]
open( name , "w").close()
for l in sys.stdin:

  t = l.strip().split()
  if t[1] == "START":
    h = Hypergraph()
    nodes = {}
    cur_edge_id = 0 
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
    if edge.label <> "last":
      ta = edge.Extensions[tagging]
      ta.ind = int(edge.label.split(':')[0])
      ta.tag_id = int(edge.label.split(':')[-2])

      edge.Extensions[has_tagging] = True

    
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
    #print "NODE", node.id
