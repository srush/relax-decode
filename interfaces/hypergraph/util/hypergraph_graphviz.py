import sys,os
from pygraphviz import *
sys.path.append("../gen_py/")

from hypergraph_pb2 import *
from features_pb2 import *
from translation_pb2 import *
from lexical_pb2 import *


def make_graph(hgraph):
  G=AGraph(strict=False,directed=True)
  G.graph_attr['rankdir']='LR'
  total = 0
  for n in hgraph.node:
    G.add_node("n" + str(n.id))
    node = G.get_node("n" + str(n.id))
    node.attr["color"] = "Red"
    node.attr["label"] = str(n.id) + n.label.encode("UTF-8") 

    if n.Extensions[is_word]:
      node.attr["color"] = "Green"

    for e in n.edge:
      G.add_edge("n" + str(n.id), "e" + str(e.id))
      edge = G.get_node("e" + str(e.id))
      edge.attr["color"] = "Blue"
      edge.attr["label"] = str(e.id) + str(unicode(e.label.encode("UTF-8"), errors="ignore"))

      for t in e.tail_node_ids:
        G.add_edge("e" + str(e.id), "n" + str(t))
      total +=1 
  print total
  G.draw("/tmp/graph.ps", prog="dot")



if __name__ == "__main__":
  hgraph = Hypergraph()  
  f = open(sys.argv[1], "rb")
  hgraph.ParseFromString(f.read())
  f.close()
  make_graph(hgraph)
