import sys,os
from relaxdecode.interfaces.lattice_pb2 import *
from relaxdecode.graph import *


if __name__ == "__main__":
  lat = Lattice()
  f = open(sys.argv[1], "rb")
  lat.ParseFromString(f.read())
  f.close()
  G = make_graph(lat)
  G.draw("/tmp/graph.png", prog="dot")

