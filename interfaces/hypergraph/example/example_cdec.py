# -*- coding: utf-8 -*-
import os
import gzip
import cdec
import unittest
# from nose.tools import assert_almost_equals, assert_equal
import sys, os
sys.path.append("../gen_py/")
import json
from protobuf_json import pb2json

import hypergraph_pb2 as hypergraphpb
from features_pb2 import *
import translation_pb2 as transpb
from lexical_pb2 import *


weights = '/home/alexanderrush/libs/cdec/tests/system_tests/australia/weights'
# weights = {'WordPenalty': -2.844814, 'LanguageModel':  1.0, 'PhraseModel_0': -1.066893, 'PhraseModel_1': -0.752247, 'PhraseModel_2': -0.589793, 'PassThrough': -20.0, 'Glue': 0}

grammar_file = '/home/alexanderrush/libs/cdec/tests/system_tests/australia/australia.scfg.gz'

input_sentence = u'澳洲 是 与 北韩 有 邦交 的 少数 国家 之一 。'
# ref_output_sentence = u'australia is have diplomatic relations with north korea one of the few countries .'
# ref_f_tree = u'(S (S (S (S (X 澳洲 是)) (X (X 与 北韩) 有 邦交)) (X 的 少数 国家 之一)) (X 。))'
# ref_e_tree = u'(S (S (S (S (X australia is)) (X have diplomatic relations (X with north korea))) (X one of the few countries)) (X .))'
# ref_fvector = {'PhraseModel_2': 7.082652, 'Glue': 3.0, 'PhraseModel_0': 2.014353, 'PhraseModel_1': 8.591477}

def assert_fvector_equal(vec, ref):
    vecd = dict(vec)

class NodeWrapper:
  def __init__(self, t):
    self.t = t
  def __hash__(self):
    return hash(repr(self.t))
order = 3

def add_root(forest, root_node_id, node_id, edge_id):
   old_root = root_node_id
   new_root = forest.node.add()
   new_root.id = node_id
   node_id += 1
   new_root.label = "NEW ROOT".decode('UTF-8')

   new_edge = new_root.edge.add()

   new_edge.id = edge_id
   edge_id+=1
   new_edge.Extensions[edge_fv] = ""

   for i in range(order-1):
      node = forest.node.add()
      node.id = node_id
      node_id += 1
      node.label = "Front".decode('UTF-8')
      node.Extensions[is_word] = True
      node.Extensions[word] = "<s>".decode('UTF-8')
      new_edge.tail_node_ids.append(node.id)

   new_edge.tail_node_ids.append(old_root)

   for i in range(order-1):
      node = forest.node.add()
      node.id = node_id
      node_id+=1
      node.label = "Back".decode('UTF-8')
      node.Extensions[is_word] = True
      node.Extensions[word] = "</s>".decode('UTF-8')
      new_edge.tail_node_ids.append(node.id)

   forest.root = new_root.id


class TestDecoder:
    def setUp(self):
        self.decoder = cdec.Decoder(formalism='scfg')
        self.decoder.read_weights(weights)
        with gzip.open(grammar_file) as f:
            self.grammar = f.read()

    def test_weights(self):
        pass

    def test_translate(self):
        forest = self.decoder.translate(input_sentence, grammar=self.grammar)
        forest.prune(density = 15)
        hypergraph = hypergraphpb.Hypergraph()

        names = []
        edge_id = 0

        # for node in forest.nodes:
        #   names.append((node, counter))
        #   counter += 1
        def get_id(node):
          # print "start"
          for n, id in names:
            # print n
            if node == n and node.span == n.span and node.cat == n.cat:
              #print n, node, repr(node), node.span, n.span, node.cat, n.cat
              return id
          id = get_id.counter
          names.append((node, get_id.counter))
          get_id.counter += 1
          return get_id.counter - 1

        get_id.counter = 0
        for node in forest.nodes:
          hnode = hypergraph.node.add()
          hnode.id = get_id(node)
          hnode.label = str(node.cat) + ":" + str(node.span)

          for edge in node.in_edges:
            hedge = hnode.edge.add()
            hedge.label = str(edge.trule).decode('UTF-8')
            hedge.id = edge_id
            edge_id += 1
            # print str(edge.trule).decode('UTF-8')
            # print edge.trule.lhs

            tailnodes = []

            t_ns = list(edge.tail_nodes)
            word_count = 0
            for x in edge.trule.e:
              if isinstance(x, basestring):
                # It is a lexical item.
                local_lex_node = hypergraph.node.add()
                local_lex_node.id = get_id.counter
                get_id.counter += 1
                local_lex_node.label = x
                local_lex_node.Extensions[is_word] = True
                local_lex_node.Extensions[word] = x
                tailnodes.append(local_lex_node.id)
                word_count += 1
              else:

                # assert(not t_ns[x.ref - 1] == node)
                #print node.span, node.cat
                #print t_ns[x.ref - 1].span,  t_ns[x.ref - 1].cat
                tailnodes.append(get_id(t_ns[x.ref - 1]))

                #tailnodes.append(iden_map[x].id)

            # print edge.trule.scores
            # print edge.trule.e
            # print edge.trule.f

            # [X] ||| [X,1] 是 ||| [1] conference was ||| PhraseModel_0=5.00688505173 PhraseModel_1=0.659752309322 PhraseModel_2=4.57912826538^C
            hedge.Extensions[edge_fv] = ' '.join('%s=%s' % feat for feat in edge.trule.scores) + " WordPenalty=" + str(word_count)
            print hedge.Extensions[edge_fv]
            # print hnode.id, tailnodes, len(t_ns)
            # print edge.trule
            for id in tailnodes:
                hedge.tail_node_ids.append(id)
        add_root(hypergraph, get_id(forest.goal), get_id.counter, edge_id)
        return hypergraph

        # hypergraph.root = node.id
if __name__ == "__main__":
    t = TestDecoder()
    t.setUp()
    hyper = t.test_translate()

    f = open("/tmp/hyper", "wb")
    f.write(hyper.SerializeToString())
    f.close()
