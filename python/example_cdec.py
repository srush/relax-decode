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

import argparse

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
    def __init__(self, config):
        self.decoder = cdec.Decoder(config)
        # self.decoder.read_weights(weights_file)
        # with gzip.open(grammar_file) as f:
        #     self.grammar = f.read()



    def test_weights(self):
        pass

    def translate(self, input_sentence):
        forest = self.decoder.translate(input_sentence)
        forest.prune(density = 5)
        hypergraph = hypergraphpb.Hypergraph()

        names = []
        edge_id = 0

        # for node in forest.nodes:
        #   names.append((node, counter))
        #   counter += 1
        def get_id(node):
          for n, id in names:
            if node == n and node.span == n.span and node.cat == n.cat:
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
          hnode.Extensions[node_fv] = ""
          for edge in node.in_edges:
            hedge = hnode.edge.add()
            hedge.label = str(edge.trule).decode('UTF-8')
            hedge.id = edge_id
            edge_id += 1

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
                tailnodes.append(get_id(t_ns[x.ref - 1]))
            hedge.Extensions[edge_fv] = ' '.join('%s=%s' % feat for feat in edge.trule.scores) + " WordPenalty=" + str(word_count)
            # print hedge.Extensions[edge_fv]
            for id in tailnodes:
                hedge.tail_node_ids.append(id)
        add_root(hypergraph, get_id(forest.goal),
                 get_id.counter, edge_id)
        return hypergraph


if __name__ == "__main__":


    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--weights')
    parser.add_argument('-g', '--grammar')
    parser.add_argument('-s', '--sentences')
    parser.add_argument('-c', '--config')
    args = parser.parse_args()
    t = TestDecoder(open(args.config).read())
    for i, l in enumerate(open(args.sentences)):
        hyper = t.translate(l.strip())
        f = open("/tmp/hyper" + str(i), "wb")
        f.write(hyper.SerializeToString())
        f.close()

