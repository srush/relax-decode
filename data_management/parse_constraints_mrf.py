import random
import sys
sys.path.append('.')
sys.path.append('../interfaces/graph/gen-py/')
from collections import *
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
TRY = False
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
num_pos_states = m+2
ordered = set(["$", "RP", "MD", "POS",  "CC",  "VBP",  "VB", "TO",  "VBD",  "IN", "DT", "JJ", "WDT", "JJR", "RBR", "PRP$", "WP", "EX",  "WRB"])
def is_order_dependent(pos):
  return pos in ordered
  

punc = set(["-RRB-", "-LRB-", "#", "``", "''", ":", ""])
bigpunc = set([".", "$", "-RRB-", "-LRB-", "#", "``", "''", ":", "", "START"])

def are_close(t1, t2):
  b1 = False
  b2 = False
  if t1 >= num_pos_states:
    t1 = t1 - num_pos_states
    b1 = True
  if t2 >= num_pos_states:
    t2 = t2 - num_pos_states
    b2 = True
  if b1==b2 and num_map[t1][0:2] == num_map[t2][0:2] and num_map[t2][0:2] == "NN":
    return 1.0
    
  if b1==b2 and num_map[t1][0:2] == num_map[t2][0:2]:
    return 0.7
  if b1==b2 and num_map[t1][0:1] == num_map[t2][0:1]:
    return 0.5
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
        # TODO!!! Back on
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

def diff_to_mask_pos(n):
  if n < 0: return n + 4
  elif n>0: return n+4-1


  
if __name__=="__main__":
  #wc = load(open(sys.argv[1], 'rb'))
  #print len(wc.word_counts)
  #manager = PosConstraint(wc)
  pen = float(sys.argv[2])
  #is_hot = set([GeneralContext.from_string(l) for l in open(sys.argv[3], 'r')])
  training = list(parse_conll_file(open(sys.argv[4])))  #is_hot = set([GeneralContext.from_string(l) for l in open(sys.argv[3], 'r')])
  link_desc = open(sys.argv[5], 'w')

  training_data = training
  states = {}
  incontexts = {}
  ctxt_map = {}
  for sn, s in enumerate(training, 1):
    head_map = sent_to_head_map(s)
    contexts = GeneralContext.contexts_from_sent(s)
    for i, ctxt in enumerate(contexts):
      for m in GeneralContext.MASKS:
        mask_context = ctxt.mask(m)
        
        #if mask_context in is_hot:
        key = mask_context
        ctxt_map.setdefault(key, [])
        ctxt_map[key].append((-sn,i))
        #print -sn, i, s.words[i].head, s.words[s.words[i].head].pos
        head_pos = s.words[i].head
        head_tag = s.words[s.words[i].head].pos
        
        diff  = head_pos -(i)
        mask_pos = diff_to_mask_pos(diff)
        in_context = coarsen(head_tag) in [coarsen(w.pos) for w in mask_context.boundaries] #mask_pos >=0 and mask_pos < 8 and m[mask_pos] == 1
        print >>sys.stderr, sn, head_pos, (i), mask_pos, m, mask_context, in_context
        incontexts[((-sn,i),mask_context)] = in_context
        states[(-sn,i)] = [(i, get_tag_ind(s.words[s.words[i].head].pos), [s.words[i].head] ) ]
        
          #only one mask per context
          #break
  test_data = list(parse_conll_file(open(sys.argv[1])))
  for sn, s in enumerate(parse_conll_file(open(sys.argv[1]))):
    head_map = sent_to_head_map(s)
    contexts = GeneralContext.contexts_from_sent(s)
    for i, ctxt in enumerate(contexts):
      best = (0,0)
      best_mask = None
      near_cc = False
      near_prep = False
      
      if coarsen(ctxt.boundaries[3].pos) == "CC" or coarsen(ctxt.boundaries[2].pos) == "CC":
        near_cc = True

      if coarsen(ctxt.boundaries[3].pos) == "IN" or coarsen(ctxt.boundaries[2].pos) == "IN":
        near_prep = True


      right_nn = False
      max_nn = 0
      if coarsen(ctxt.boundaries[4].pos) in ["NN","POS"]:
        right_nn = True
        max_nn = 4
        for j in range(4,8):
          if coarsen(ctxt.boundaries[j].pos) in ["NN", "POS"]:
            max_nn = j
          else:
            break


      for m in GeneralContext.MASKS:
        mask_context = ctxt.mask(m)
        num_on = sum(m)
        if not mask_context.punc_check():  continue
        #if mask_context in is_hot and mask_context in ctxt_map :
        if mask_context in ctxt_map:
          
          total_seen = len([1 for key in ctxt_map[mask_context] if len(states[key]) ==1])
          possible_best = [coarsen(num_map[states[key][0][1]])
                           for key in ctxt_map[mask_context]
                           if len(states[key]) ==1]
          if not possible_best: continue
          agreed = Counter(possible_best).most_common(1)[0]
          
          #if agreed[0][:2] == "VB" : continue
          con_pos = [coarsen(w.pos) for w in mask_context.boundaries]
          coarsened = "NN" in [coarsen(w.pos) for w in mask_context.boundaries]
          score = ((agreed[1] / (float(total_seen) )), 1 if total_seen < 5 or coarsened else 0, num_on, min(agreed[1],4))

          if all([incontexts[key,mask_context] for key in ctxt_map[mask_context] if len(states[key]) ==1]): score = (score[0]+1,score[1], score[2], score[3])
          #if total_seen > 5: score -=1 
          

          nn_constraint = (not right_nn) or m[max_nn] == 1  
          cc_constraint = (not near_cc or "CC" in con_pos )
          in_constraint = (not near_prep or "IN" in con_pos )

          if right_nn:
            fits_constraints = nn_constraint
          else:
            fits_constraints =  cc_constraint and in_constraint
          

          print >> sys.stderr, agreed[1],total_seen, score, mask_context, max_nn, right_nn, near_cc, near_prep, nn_constraint, cc_constraint, in_constraint, best_mask, mask_context.punc_check()
          if score > best and fits_constraints:
            best_mask = mask_context
            best = score
      
      if best[0] >= 1.0:
        print >>sys.stderr,  "Choose ", best_mask
        key = best_mask
        ctxt_map.setdefault(key, [])
        ctxt_map[key].append((sn,i))
        #states[(sn,i)] = [(i, get_tag_ind(s.words[i].pos), [s.words[i].head] ) ]
        states[(sn,i)] = [(i, get_tag_ind(tag), [ind for ind in inds if ind <> i] ) for tag, inds in head_map.iteritems()]
        #only one mask per context
        
  #t = sys.argv[4]
  t = "nbayes"
  #marginals = Marginals.from_handle(open(sys.argv[5]))
  
  
  #beam_map = BeamMap.from_handle(open(sys.argv[8], 'r'))

  i = -1
  for _, (ctxt, positions) in enumerate(ctxt_map.iteritems()):
    
    posmrf = ParseMrf(str(ctxt))
    
    bonus = -pen #* -min((20*test_seen  / float(20* test_seen + training_seen + 1)), 1.0);
    #print >>sys.stderr, bonus    

    dep_nodes = [PosNode(pos, "Dep %s:%s"%(sent_num, ind),
                         sorted(sum([ [(e,"dep "+ str(e))for e in d[2]]  for d in states[sent_num, ind]],[])))
                 for pos,(sent_num, ind) in enumerate(positions)]
    
    num_dep_nodes = len(dep_nodes)
    num_non_training = len([() for sent_num, ind in positions if sent_num >=0])
    num_training_nodes = num_dep_nodes - num_non_training
    if num_non_training == 0 or num_dep_nodes == 1: continue
    else: i += 1

    pos_nodes = []
    all_edge = []
    if TRY:
      mask = ctxt._mask
      ls = []
      offset = 4
      for j, ind in enumerate(mask):
        if mask[j]:
          if j < 4:
            ls.append(j)
          else:
            ls.append(j+1)
      #print ls
      pos_nodes = [PosNode(pos+num_dep_nodes, "Pos %s:%s"%(sent_num, ind),
                           [ (d, "pos "+str(d)) for d in ls] + [(10, "pos out")] )
                   for  pos, (sent_num, ind) in enumerate(positions)]
      for p_node in pos_nodes:
        p_node._potentials = [(state, 0.0) for state in p_node.states] 
        #if num_map[state[0]] not in punc else 1000

      for q, (d_node, p_node) in enumerate(zip(dep_nodes, pos_nodes)):
        edge = PosEdge(d_node, p_node)
        key = positions[q]
        sent_num, ind = key
        for (_, pos, heads) in states[key]:
          for h in heads:
            if h - ind + offset in ls:
              edge.potentials.append(((h,""), (h-ind + offset,""), -10000))
  #             edge.potentials.append(((h,""), (10,""), -10000 - 0.05 * bonus))
            else:
              edge.potentials.append(((h,""), (10,""), -10000))


#         for context_ind in (0,2):
#           edge.potentials.extend([((ind + (context_ind - 1),""), (context_ind,""), -10000) for h in heads])
        d_node.edges.append(edge)

    else: 
      
      pos_nodes = [PosNode(pos+num_dep_nodes, "Pos %s:%s"%(sent_num, ind),
                         [ (d[1], "pos "+str(num_map[d[1]])) for d in states[sent_num, ind]] +
                         [ (d[1] + num_pos_states, "pos_after "+str(num_map[d[1]])) for d in states[sent_num, ind]]
                           )
                 for  pos, (sent_num, ind) in enumerate(positions)]

  
#       pos_nodes.extend([PosNode(pos+num_dep_nodes, "Pos %s:%s"%(sent_num, ind),
#                                  [ (d[1] + num_pos_nodes, "pos "+str(num_map[d[1]])) for d in states[sent_num, ind]])
#                          for  pos, (sent_num, ind) in enumerate(positions)])

      for p_node in pos_nodes:
        p_node._potentials = [(state, 0.0 if num_map.get(state[0], num_map.get(state[0]-num_pos_states,"")) not in bigpunc else 1000) for state in p_node.states] 
      
    

      for q, (d_node, p_node) in enumerate(zip(dep_nodes, pos_nodes)):
        edge = PosEdge(d_node, p_node)
        key = positions[q]
        sent, ind = key
        if sent >= 0:
          mod_pos = test_data[sent].words[ind].pos
        else:
          mod_pos = training_data[(-sent) -1].words[ind].pos
      
        for (_, pos, heads) in states[key]:
          for h in heads:

            if not is_order_dependent(mod_pos) or h < ind:
              edge.potentials.append(((h,""), (pos,""), -10000) )
            else:
              edge.potentials.append(((h,""), (pos + num_pos_states,""), -10000))
        d_node.edges.append(edge)
      
    all_nodes = dep_nodes + pos_nodes
    total_states = set()
    for node in pos_nodes:
      if TRY:
        total_states.update([s for s in node.states if s[0]<> 10])
      else:
        total_states.update(node.states )
      
    # constraint off!
    #total_states.add(1000)

    mu_node = PosNode(len(all_nodes), "mu", list(total_states))

    #total_bonus = (float(num_non_training) * bonus) * (3.0 / 2.0)
    #training_node_bonus = (total_bonus * (1.0/3.0) ) / float(num_training_nodes)
    training_node_bonus = bonus
    total_bonus = len(pos_nodes) * bonus #num_non_training * bonus + num_training_nodes * training_node_bonus 
    #non_training_total_bonus = num_non_training * bonus
    mu_node._potentials.append( ((100,"off"), (3.0/4.0) *float(total_bonus)))
    for p_node in pos_nodes:
      edge = PosEdge(p_node, mu_node)
      # hack for training nodes
      if len(p_node.states) == 1:
        edge.add_same_potentials(bonus)
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

    #print >>sys.stderr, str(posmrf)
  print >>link_desc, i 
