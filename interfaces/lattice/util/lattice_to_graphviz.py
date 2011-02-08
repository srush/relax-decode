import sys,os
from pygraphviz import *
sys.path.append("../gen-py/")

from lattice_pb2 import *


def make_graph(lattice):
  G=AGraph(strict=False,directed=True)
  G.graph_attr['rankdir']='LR'
  total = 0
  edge_count = 0
  for n in lattice.node:
    G.add_node("n" + str(n.id))
    node = G.get_node("n" + str(n.id))

    node.attr["color"] = "Red"
    if n.Extensions[has_phrases]:
      node.attr["color"] = "Green"
      print n.id
      for p in n.Extensions[phraselets].phraselet:
        for w in p.word:
          print w.word,
        print 

    node.attr["label"] = n.label.encode("UTF-8")
    
    for e in n.edge:
      G.add_edge("n" + str(n.id), "n" + str(e.to_id))
      edge = G.get_edge("n" + str(n.id), "n" + str(e.to_id))
      edge.attr["label"] = str(unicode(e.label.encode('UTF-8'), errors="ignore"))
      edge_count +=1 
  print "Num Nodes", len(lattice.node)
  print "Num Edges", edge_count

  print total
  G.draw("/tmp/graph.ps", prog="dot")



if __name__ == "__main__":
  lat = Lattice()  
  f = open(sys.argv[1], "rb")
  lat.ParseFromString(f.read())
  f.close()
  make_graph(lat)
