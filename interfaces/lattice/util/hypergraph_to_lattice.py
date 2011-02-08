# -*- coding: utf-8 -*-
import os,sys
sys.path.append("../gen-py/")
sys.path.append("../../hypergraph/gen_py/")
from lattice_pb2 import *
from hypergraph_pb2 import *
from lexical_pb2 import *

import  lattice_pb2 as lattice
DOWN = "D"
UP = "U"
class Graph(object):
  def __init__(self):
    self.nodes = {}
    self.id = 0
    self.edge_map = {}

    self.check = set()
    self.lattice = Lattice()
    self.total_edges = 0

  def set_start(self, node):
    self.lattice.start = node.id

  def set_final(self, node):
    self.lattice.final.append(node.id)

  def register_node(self, node):
    assert node not in self.nodes
    
    self.nodes[self.id] = node
    node.id = self.id
    n = self.lattice.node.add()
    n.id = self.id
    #print node
    s = str(node)
    n.label = unicode(s, errors="ignore")
    node.proto = n
    self.id += 1 
    return self.id - 1

  def register_edge_map(self, node, edge_id):
    self.edge_map.setdefault(edge_id, set())
    self.edge_map[edge_id].add(node)

  def size(self):
    return self.id
  
  def __iter__(self):
    for i in self.nodes:
      yield self.nodes[i]

  def filter(self, fn):
    todel = []
    for node in self:
      if fn(node):
        
        for bn in node.back_edges:
          bn.edges.remove(node)
        for n2 in node.edges:
          n2.back_edges.remove(node)
          
        for n2 in node.edges:
          for bn in node.back_edges:
            bn.add_edge(n2)
        todel.append(node.id)
    for i in todel:
      del self.nodes[i]
            
class LatNode(object):
  def __init__(self, graph):
    self.edges = set()
    self.back_edges = set()
    graph.register_node(self)
    self.lex = None
    self.graph = graph
    
  def add_edge(self, to_node, label):
    self.edges.add(to_node)
    to_node.back_edges.add(self)
    edge = self.proto.edge.add()
    edge.to_id = to_node.id
    return edge 

  def label(self):
    return str(self)

class NonTermNode(LatNode):
  def __init__(self, graph, forest_node, dir):    
    self.forest_node = forest_node
    self.dir = dir
    LatNode.__init__(self, graph)
    #self.proto.Extensions[original_node] = -1
    #self.proto.Extensions[ignore_node] = True

  def __str__(self):
    return "%s %s %s %s"%(str(self.forest_node.label), self.dir, self.forest_node.id, self.id)

  def color(self):
    return "red"

class LexNode(LatNode):
  def __init__(self, graph, lex, edge_id):
    self.lex = lex
    LatNode.__init__(self, graph)
    
    #print self.proto._known_extensions
    #self.proto.Extensions[lattice.is_word] = True
    #self.proto.Extensions[lattice.word] = lex.decode("utf-8")
    #print lex, self.id
    self.graph.register_edge_map(self, edge_id)
    #self.proto.Extensions[original_node] = edge_id

  def __str__(self):
    return "%s %s"%(self.lex, self.id)

  def color(self):
    return "blue"
    

class InternalNode(LatNode):
  def __init__(self, graph, rule, pos, name, dir, edge_id):
    
    self.name = name
    self.rule = rule

    self.dir = dir
    if self.dir == UP:
      self.pos = pos +1
    else :
      self.pos = pos
    LatNode.__init__(self, graph)
    self.graph.register_edge_map(self, edge_id)
    #self.proto.Extensions[original_node] = edge_id

  def __str__(self):
    #rhs = self.rule.rhs[:]
    
    
    #rhs.insert(self.pos, ".")
    #rhsstr = " ".join(rhs)
    
    return ("%s %s %s %s"%(self.rule, None , self.dir, self.id)) # self.rule.edge.id

  def label(self):
    lhs = unicode(str(self.rule.lhs), errors='ignore')
    return "%s %s"%(lhs, str(self))

  def color(self):
    return "green"



  
class NodeExtractor(object):
  "Class for creating the FSA that represents a translation forest (forest lattice) "


  def __init__(self):
    self.internal_nodes = 0
    self.word_nodes = 0
    self.edge_id = 0
    self.original_id = 0
    self.total_edges = 0

    self.inner_node_labels = {}

  def get_label(self, from_node, to_node, has_phrases):
    if  has_phrases:
      tmp = self.original_id
      self.original_id += 1
      return tmp
    
    key = (from_node, to_node)
    if not self.inner_node_labels.has_key(key):
      self.inner_node_labels[key] = self.original_id
      self.original_id += 1
    return self.inner_node_labels[key]
    #t = self.original_id
    #self.original_id += 1
    #return t
  
  def extract(self, forest):
    self.memo = {}
    self.graph = Graph()
    self.forest = forest
    first_state = LexNode(self.graph, "<s>", -1)
    first_state.proto.Extensions[has_phrases] = True
    proto_plet = first_state.proto.Extensions[phraselets].phraselet.add()
    proto_plet.phraselet_hypergraph_edge = -1
    for w in ['<s>', '<s>']:  
      subword = proto_plet.word.add()
      subword.word = w
      subword.subword_original_id = self.original_id
      # beginning not in graph
      subword.subword_hypergraph_node_id = -1
      #print self.original_id, subword.word
      self.original_id += 1

    (first, last) = self.extract_fsa(self.forest.node[forest.root])
    
    first_state.add_edge(first, "")

    last_state = LexNode(self.graph, "</s>", -1)
    last_state.proto.Extensions[has_phrases] = True
    last.add_edge(last_state, "")
    
    
    proto_plet = last_state.proto.Extensions[phraselets].phraselet.add()
    proto_plet.phraselet_hypergraph_edge = -1
    for w in ['</s>', '</s>']:  
      subword = proto_plet.word.add()
      subword.word = w
      subword.subword_original_id = self.original_id
      # end not in hypergraph yet
      subword.subword_hypergraph_node_id = -1
      #print self.original_id, subword.word
      self.original_id += 1

    self.graph.set_start(first_state)
    self.graph.set_final(last_state)
    print "Internal Nodes", self.internal_nodes
    print "Word Nodes", self.word_nodes
    print "Total Nodes", self.graph.size()
    
    print "Total edges", self.total_edges
    self.graph.lattice.Extensions[num_hypergraph_edges] = self.total_edges
    self.graph.lattice.Extensions[num_original_ids] = self.original_id

    return self.graph

  def extract_fsa(self, node):
    "Constructs the segment of the fsa associated with a node in the forest"
    

    # memoization if we have done this node already
    if self.memo.has_key(node.id):
      return self.memo[node.id]

    # Create the FSA state for this general node (non marked)
    # (These will go away during minimization)
    down_state = NonTermNode(self.graph, node, DOWN)
    up_state = NonTermNode(self.graph, node, UP) 
    self.memo[node.id] = (down_state, up_state)
  
    link = {} 
    for edge in node.edge:
      self.total_edges +=1 
      rhs = edge.tail_node_ids
      
      phraselet = []
      last = -2

      edge_dot = {}
      for i,node_id in enumerate(rhs):
        to_node = self.forest.node[node_id]
        if to_node.Extensions[is_word]:
          # it's a word, phraselet it and stick it in the list 
          phraselet.append(to_node)
        else:
          # we've hit a new node, remember to make a link
          key = (last, to_node.id, bool(phraselet))
          link.setdefault(key, [])
          link[key].append((edge, phraselet))
          phraselet = []
          last = to_node.id
      key = (last, -1, bool(phraselet))
      link.setdefault(key, [])
      link[key].append((edge, phraselet))
      phraselet = []
      #print link
    for ((from_node, to_node, my_has_phrases), my_phraselets) in link.iteritems():      
      #(_, phraselet) = my_phraselets[0]

      #words = " ".join([n.Extensions[word]for n in phraselet])
      new_state = LexNode(self.graph, "", self.edge_id)
      print "Phraselet len", len(my_phraselets)
      new_state.proto.Extensions[has_phrases] = my_has_phrases 
      for edge_dot, plet in my_phraselets:
        proto_plet = new_state.proto.Extensions[phraselets].phraselet.add()
        proto_plet.phraselet_hypergraph_edge = edge_dot.id
        
        for w in plet:  
          subword = proto_plet.word.add()
          subword.word = w.Extensions[word]
          subword.subword_original_id = self.original_id
          subword.subword_hypergraph_node_id = w.id
          #print self.original_id, subword.word
          self.original_id += 1


      if from_node == -2:
        previous_node = down_state
        protoedge = previous_node.add_edge(new_state, "")                

        assert(to_node != -1 or my_has_phrases);

        protoedge.label = "%s I %s"%((node.id, DOWN), (to_node, DOWN))
        original = protoedge.Extensions[origin]
        original.has_origin = True
        if my_has_phrases:
          protoedge.label = "%s I w"%((node.id, DOWN),)

        original.original_id = self.get_label((node.id, DOWN), (to_node, DOWN), True or my_has_phrases)


        for edge_dot, _ in my_phraselets:
          original.hypergraph_edge.append(edge_dot.id)

      else:
        _, previous_node = self.extract_fsa(self.forest.node[from_node])
        protoedge = previous_node.add_edge(new_state, ( " UP").decode('UTF-8'))
        #for edge_dot, phraselet in phraselets:

        original = protoedge.Extensions[origin]
        original.has_origin = True

        
        end_node= (to_node, DOWN)
        
        if to_node == -1:
          end_node = (node.id, UP)
          unique = True
        else:
          unique = True or my_has_phrases
          
        protoedge.label = "%s M %s"%((from_node, UP), end_node)

        if my_has_phrases:
          protoedge.label = "%s M w"%((from_node, UP),)
        
        original.original_id = self.get_label((from_node, UP), end_node , unique)
        
        for edge_dot, _ in my_phraselets:
          original.hypergraph_edge.append(edge_dot.id)
      

      if to_node == -1:
        next_node = up_state
        protoedge = new_state.add_edge(next_node, "")#(edge.label.encode('UTF-8') + " UP").decode('UTF-8') )

        if my_has_phrases:
          protoedge.label = "w E %s"%((node.id, UP),)
          original = protoedge.Extensions[origin]
          original.has_origin = True
          original.original_id = self.get_label((from_node, UP), (node.id, UP), my_has_phrases)
        
          for edge_dot, _ in my_phraselets:
            original.hypergraph_edge.append(edge_dot.id)

      else:
        next_node, _ = self.extract_fsa(self.forest.node[to_node])
        protoedge = new_state.add_edge(next_node, ( " DOWN").decode('UTF-8') )
        
        #for edge_dot, phraselet in phraselets:
        #CHANGED
        # WRONG!!!
        if my_has_phrases:
          protoedge.label = "w M %s"%((to_node, DOWN),)
          original = protoedge.Extensions[origin]
          original.has_origin = True
          original.original_id = self.get_label((from_node, UP), (to_node, DOWN), my_has_phrases)
        
          for edge_dot, _ in my_phraselets:
            original.hypergraph_edge.append(edge_dot.id)


      self.edge_id +=1 
    return self.memo[node.id]




if __name__ == "__main__":
  for i in range(1,101):
    hgraph = Hypergraph()  
    f = open(sys.argv[1] +str(i), "rb")
    hgraph.ParseFromString(f.read())
    f.close()

    graph = NodeExtractor().extract(hgraph)

    f = open(sys.argv[2]+str(i), "wb")
    f.write(graph.lattice.SerializeToString())
    f.close()



    # for edge in node.edge:
    #   previous_state = down_state
    
    #   rhs = edge.tail_node_ids
      
    #   # always start with the parent down state ( . P )       
    #   nts_num = 0


    #   for i,node_id in enumerate(rhs):
    #     to_node = self.forest.node[node_id]
        
    #     # next is a word ( . lex ) 
    #     if to_node.Extensions[is_word]:
    #       new_state = LexNode(self.graph, to_node.Extensions[word].encode("UTF-8"), edge.id)
    #       self.word_nodes += 1
    #       previous_state.add_edge(new_state)

    #       # Move the dot ( lex . )
    #       previous_state = new_state          

    #     else:
    #       # it's a symbol


    #       # local symbol name (lagrangians!)
    #       #pos = get_sym_pos(sym)
    #       #to_node = edge.subs[nts_num]
    #       #nts_num += 1
    #       # We are at (. N_id ) need to get to ( N_id .) 

    #       # First, Create a unique named version of this state (. N_id) and ( N_id . )
    #       # We need these so that we can assign lagrangians
    #       local_down_state = InternalNode(self.graph, edge.label.encode("UTF-8"), i, to_node.id, DOWN, edge.id)
    #       local_up_state = InternalNode(self.graph, edge.label.encode("UTF-8"), i , to_node.id, UP, edge.id)
    #       self.internal_nodes+=2

    #       down_sym, up_sym = self.extract_fsa(to_node)
          
    #       previous_state.add_edge(local_down_state)
    #       local_down_state.add_edge(down_sym)
    #       up_sym.add_edge(local_up_state)

    #       # move the dot
    #       previous_state = local_up_state
          
    #   previous_state.add_edge(up_state)
