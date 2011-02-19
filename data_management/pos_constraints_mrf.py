import random
import sys
sys.path.append('.')
sys.path.append('../interfaces/graph/gen-py/')
from graph_pb2 import *
from mrf_pb2 import * 
from itertools import *
#from format.conll import *
from format.simple import *
from StringIO import StringIO
from unknown_words import *
from pickle import *
from pos_constraints import * 
import math
from marginals import *
states = range(45)

class PosNode:
  def __init__(self, id, label):
    self.id = id
    self.label = label
    self.edges = []
    self.potentials = []

  def add_blank_potentials(self):
    self.potentials = [(state, 0.0) for state in states]
    
  def convert_to_protobuf(self, proto_node):
    proto_node.id = self.id
    proto_node.label = self.label
    proto_mrf_node = proto_node.Extensions[mrf_node]
    #print self.potentials
    for state_id,p in self.potentials:
      proto_node_potentials = proto_mrf_node.node_potentials.add()
      proto_node_potentials.node_potential = p

      proto_node_potentials.state.id = state_id
      proto_node_potentials.state.label = ""

    for edge in self.edges:
      edge.convert_to_protobuf(proto_node.edge.add())


class PosEdge:
  def __init__(self, to_node):
    self.to_node = to_node
    #self.label = label
    self.potentials = []

  def add_same_potentials(self, bonus):
    self.potentials =[]
    for s1 in states:
      for s2 in states:
        self.potentials.append((s1,s2, bonus if s1 == s2 else 0.0))

  def convert_to_protobuf(self, proto_edge):
    proto_edge.to_node = self.to_node
    proto_mrf_edge = proto_edge.Extensions[mrf_edge]
    for s1, s2, bonus in self.potentials:
      if bonus == 0.0 : continue
      proto_edge_potentials = proto_mrf_edge.edge_potentials.add()
      proto_edge_potentials.edge_potential = bonus
      proto_edge_potentials.from_state_id = s1
      proto_edge_potentials.to_state_id = s2


class PosMrf:
  def __init__(self, label, data):
    self.label = label
    self.nodes = [PosNode(i, "%s:%s"%(sent_num, ind)) for i, (sent_num, ind, _) in enumerate(data)]
    #self.edges = {}

  def add_potts_edges(self, bonus):
    for i, node in enumerate(self.nodes):
      node.add_blank_potentials()
      for j in range(i+1, len(self.nodes)):
        self.nodes[i].edges.append(PosEdge(j))
        self.nodes[i].edges[-1].add_same_potentials(bonus)

  def add_naive_bayes_edges(self, bonus):
    self.nodes.append(PosNode(len(self.nodes), "mu"))
    for node in self.nodes:
      node.add_blank_potentials()

    for node in self.nodes[:-1]:
      node.edges.append(PosEdge(self.nodes[-1].id))
      node.edges[-1].add_same_potentials(bonus)
    

  def convert_to_protobuf(self):
    proto_graph = Graph()
    proto_graph.label  = self.label
    for node in self.nodes:
      node.convert_to_protobuf(proto_graph.node.add())
    return proto_graph
      
if __name__=="__main__":

  wc = load(open(sys.argv[1], 'rb'))
  #print len(wc.word_counts)
  manager = PosConstraint(wc)
  for s in enumerate(parse_simple_file(open(sys.argv[2]))):
    manager.inc_sent(s)
    
  t = sys.argv[4]
  
  marginals = Marginals.from_handle(open(sys.argv[5]))
  
  s, total = manager.stats()
  groups = manager.groups()
  for i in range(len(groups)):
    
    group1 = groups[i]
    posmrf = PosMrf(group1[0], group1[1])
    

    training_seen = group1[1][0][2]
    test_seen = len(group1[1])
    #print training_seen, test_seen 
    
    bonus = 5 * -min((20*test_seen  / float(20* test_seen + training_seen + 1)), 1.0);
    print >>sys.stderr, bonus
    #posmrf.add_node_potentials(marginals)
    if t == "nbayes":
      posmrf.add_naive_bayes_edges(bonus)
    elif t == "potts":

      # at best potts is the same as naive bayes
      posmrf.add_potts_edges(bonus/float(test_seen))
    else:
      assert False, t

    for j in range(len(group1[1])):
      print i, j, group1[1][j][0], group1[1][j][1]

      sent_num = group1[1][j][0]
      word_ind = group1[1][j][1]
      posmrf.nodes[j].potentials = [(s, 0.0 ) for s in states] #if marginals.data[sent_num, word_ind, s] > 0.05 else 100.0
      #print >>sys.stderr, sent_num, word_ind, posmrf.nodes[j].potentials


    proto_graph = posmrf.convert_to_protobuf()

    f = open(sys.argv[3] , "wb")
    print >>f, random.random()
    f = open(sys.argv[3] + str(i), "wb")      
    f.write(proto_graph.SerializeToString())
    f.close()

  # zerozero = manager.zerozero()
  # data = [ b[0] for a,b in zerozero]
  # print >>sys.stderr, data
  # posmrf = PosMrf("unknown", data)
  # posmrf.add_naive_bayes_edges(-1.0)
  # proto_graph = posmrf.convert_to_protobuf()
  # for j in range(len(data)):
  #   print i+1, j, data[j][0], data[j][1]
  # f = open(sys.argv[3] + str(i+1), "wb")      
  # f.write(proto_graph.SerializeToString())
  # f.close()

  #print manager
  #print s / float(total)
