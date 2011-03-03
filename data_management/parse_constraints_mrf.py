import random
import sys
sys.path.append('.')
sys.path.append('../interfaces/graph/gen-py/')
from graph_pb2 import *
from mrf_pb2 import * 
from itertools import *
from format.conll import *
from format.simple import *
from StringIO import StringIO
from unknown_words import *
from pickle import *
import math
from marginals import *
from beam_map import *
from context import *
from map.unmap_pos import *

pos_map = {}
num_map = {}
m = -1
for l in open(root + 'map/STANFORD.map'):
  num, word =  l.strip().split()
  pos_map[word] = int(num)
  num_map[int(num)] = word
  m = max(int(num), m)
pos_map["*R"] = m + 1
pos_map["*ROOT*"] = m + 1
num_map[m+1] = "**"

punc = set([",",".", "$", "-RRB-", "-LRB-", "#", "``", "''", ":", "", "START"])

def are_close(t1, t2):
  if num_map[t1][0:2] == num_map[t2][0:2]:
    return 0.9
  if num_map[t1][0:1] == num_map[t2][0:1]:
    return 0.7
  return 0.0

def get_tag_ind(tag):
  # if tag[:2] in pos_map:
#     return pos_map[tag[:2]]
#   else:
  return pos_map[tag]
class PosNode:
  def __init__(self, id, label, states):
    
    self.id = id
    self.label = label
    self.edges = []
    self._potentials = [(state, 0.0) for state in states]
    
    self.states = states
#   def add_blank_potentials(self):
#     self.potentials = [(state, 0.0) for state in states]
    
  def convert_to_protobuf(self, proto_node):
    proto_node.id = self.id
    proto_node.label = self.label
    proto_mrf_node = proto_node.Extensions[mrf_node]
    
    for ((state_id,state_label),p) in self._potentials:
      proto_node_potentials = proto_mrf_node.node_potentials.add()
      proto_node_potentials.node_potential = p

      proto_node_potentials.state.id = state_id
      proto_node_potentials.state.label = state_label

    for edge in self.edges:
      edge.convert_to_protobuf(proto_node.edge.add())

  def __repr__(self):
    return str(self.id) + " " +self.label + " " +" ".join([str(s) for s in self.states]) + "\n"+\
           "\n".join(str(e) for e in self.edges)


class PosEdge:
  def __init__(self, from_node, to_node):
    self.from_node = from_node
    self.to_node = to_node
    #self.label = label
    self.potentials = []

  def add_same_potentials(self, bonus):
    self.potentials =[]
    for s1 in self.from_node.states:
        
      for s2 in self.to_node.states:
        if s1 == s2:
          self.potentials.append((s1,s2, bonus if s1 == s2 else 0.0))
        else:
          closeness = are_close(s1[0], s2[0])
          if closeness <> 0.0:
            self.potentials.append((s1,s2, closeness * bonus))

  def convert_to_protobuf(self, proto_edge):
    proto_edge.to_node = self.to_node.id
    proto_mrf_edge = proto_edge.Extensions[mrf_edge]
    #print self.potentials
    for (s1, _), (s2,_ ), bonus in self.potentials:
      if bonus == 0.0 : continue
      proto_edge_potentials = proto_mrf_edge.edge_potentials.add()
      proto_edge_potentials.edge_potential = bonus
      proto_edge_potentials.from_state_id = s1
      proto_edge_potentials.to_state_id = s2

  def __repr__(self):
    return "EDGE " + str(self.from_node.id) + " " + str(self.to_node.id) + " " + " ".join([str(n) for n in self.potentials])


class ParseMrf:
  def __init__(self, label, data=[], beam_map=None):
    self.label = label
    self.nodes = [PosNode(i, "%s:%s"%(sent_num, ind), [ d[1] for d in beam_map[sent_num, ind]])
                  for i, (sent_num, ind) in enumerate(data)]
    #self.edges = {}
    self.beam_map = beam_map
    
#   def add_potts_edges(self, bonus):
#     for i, node in enumerate(self.nodes):
#       #node.add_blank_potentials()
#       for j in range(i+1, len(self.nodes)):
#         self.nodes[i].edges.append(PosEdge(node, self.nodes[j]))
#         self.nodes[i].edges[-1].add_same_potentials(bonus)

  def add_naive_bayes_edges(self, bonus):
    
    total_states = set()
    for node in self.nodes:
      total_states.update(node.states)

    
    self.nodes.append(PosNode(len(self.nodes), "mu", list(total_states)))
    #for node in self.nodes:
    #  node.add_blank_potentials(beam_map)

    for node in self.nodes[:-1]:
      node.edges.append(PosEdge(node, self.nodes[-1]))
      node.edges[-1].add_same_potentials(bonus)

  def __repr__(self):
    return "\n".join([str(n) for n in self.nodes])
    

  def convert_to_protobuf(self):
    proto_graph = Graph()
    proto_graph.label  = self.label
    for node in self.nodes:
      node.convert_to_protobuf(proto_graph.node.add())
    return proto_graph    

      
if __name__=="__main__":


  #wc = load(open(sys.argv[1], 'rb'))
  #print len(wc.word_counts)
  #manager = PosConstraint(wc)
  pen = float(sys.argv[2])
  is_hot = set([GeneralContext.from_string(l) for l in open(sys.argv[3], 'r')])
  training = list(parse_conll_file(open(sys.argv[4])))  #is_hot = set([GeneralContext.from_string(l) for l in open(sys.argv[3], 'r')])
  link_desc = open(sys.argv[5], 'w')


  states = {}
  ctxt_map = {}
  for sn, s in enumerate(training):
    head_map = sent_to_head_map(s)
    contexts = GeneralContext.contexts_from_sent(s)
    for i, ctxt in enumerate(contexts):
      for m in GeneralContext.MASKS:
        mask_context = ctxt.mask(m)
  
        if mask_context in is_hot:
          key = mask_context
          ctxt_map.setdefault(key, [])
          ctxt_map[key].append((-sn,i))
          states[(-sn,i)] = [(i, get_tag_ind(tag), [ind for ind in inds if ind <> i] ) for tag, inds in head_map.iteritems()]

          #only one mask per context
          break

  for sn, s in enumerate(parse_conll_file(open(sys.argv[1]))):
    head_map = sent_to_head_map(s)
    contexts = GeneralContext.contexts_from_sent(s)
    for i, ctxt in enumerate(contexts):
      for m in GeneralContext.MASKS:
        mask_context = ctxt.mask(m)
  
        if mask_context in is_hot:
          key = mask_context
          ctxt_map.setdefault(key, [])
          ctxt_map[key].append((sn,i))
          states[(sn,i)] = [(i, get_tag_ind(s.words[i].pos), [s.words[i].head] ) ]

          #only one mask per context
          break 
  #t = sys.argv[4]
  t = "nbayes"
  #marginals = Marginals.from_handle(open(sys.argv[5]))
  
  
  #beam_map = BeamMap.from_handle(open(sys.argv[8], 'r'))

  i = -1
  for _, (ctxt, positions) in enumerate(ctxt_map.iteritems()):
    
    posmrf = ParseMrf(str(ctxt))
    
    bonus = -pen #* -min((20*test_seen  / float(20* test_seen + training_seen + 1)), 1.0);
    #print >>sys.stderr, bonus    

    dep_nodes = [PosNode(pos, "Dep %s:%s"%(sent_num, ind), sorted(sum([ [(e,"dep "+ str(e))for e in d[2]]  for d in states[sent_num, ind]],[])))
                 for pos,(sent_num, ind) in enumerate(positions)]
    
    num_dep_nodes = len(dep_nodes)
    if num_dep_nodes == 1: continue
    else: i += 1
    pos_nodes = [PosNode(pos+num_dep_nodes, "Pos %s:%s"%(sent_num, ind), [ (d[1], "pos "+str(num_map[d[1]])) for d in states[sent_num, ind]])
                 for  pos, (sent_num, ind) in enumerate(positions)]

    for p_node in pos_nodes:
      p_node._potentials = [(state, 0.0 if num_map[state[0]] not in punc else 1000) for state in p_node.states] 
      
    
    all_edge = []
    for q, (d_node, p_node) in enumerate(zip(dep_nodes, pos_nodes)):
      edge = PosEdge(d_node, p_node)
      key = positions[q]
      
      for (_, pos, heads) in states[key]:
        edge.potentials.extend([((h,""), (pos,""), -10000) for h in heads])
      d_node.edges.append(edge)
      
    all_nodes = dep_nodes + pos_nodes
    total_states = set()
    for node in pos_nodes:
      total_states.update(node.states)
      
    # constraint off!
    #total_states.add(1000)
    mu_node = PosNode(len(all_nodes), "mu", list(total_states))
    mu_node._potentials.append( ((100,"off"), len(dep_nodes) * (3.0/4.0) *bonus))
    for p_node in pos_nodes:
      edge = PosEdge(p_node, mu_node)
      # hack for training nodes
      if len(p_node.states) == 1:
        edge.add_same_potentials(10.0 * bonus)
      else:
        edge.add_same_potentials(bonus)
      p_node.edges.append(edge)
    
    all_nodes.append(mu_node)

    posmrf.nodes = all_nodes

    #posmrf.add_naive_bayes_edges(bonus)

    
    for j in range(len(positions)):
      if positions[j][0] < 0: continue
      print i, j, positions[j][0],positions[j][1]

      sent_num = positions[j][0]
      word_ind = positions[j][1]
      #posmrf.nodes[j].potentials = [(s, 0.0 ) for s in states]
    

    proto_graph = posmrf.convert_to_protobuf()

    f = open(sys.argv[6] , "wb")
    print >>f, random.random()
    f = open(sys.argv[6] + str(i), "wb")      
    f.write(proto_graph.SerializeToString())
    f.close()

    print >>sys.stderr, str(posmrf)
  print >>link_desc, i 
