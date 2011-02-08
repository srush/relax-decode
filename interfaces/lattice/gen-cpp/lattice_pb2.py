# Generated by the protocol buffer compiler.  DO NOT EDIT!

from google.protobuf import descriptor
from google.protobuf import message
from google.protobuf import reflection
from google.protobuf import service
from google.protobuf import service_reflection
from google.protobuf import descriptor_pb2

IS_WORD_FIELD_NUMBER = 110
is_word = descriptor.FieldDescriptor(
  name='is_word', full_name='lattice.is_word', index=0,
  number=110, type=8, cpp_type=7, label=1,
  default_value=False,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)
WORD_FIELD_NUMBER = 111
word = descriptor.FieldDescriptor(
  name='word', full_name='lattice.word', index=1,
  number=111, type=9, cpp_type=9, label=1,
  default_value=unicode("", "utf-8"),
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)
ORIGINAL_NODE_FIELD_NUMBER = 112
original_node = descriptor.FieldDescriptor(
  name='original_node', full_name='lattice.original_node', index=2,
  number=112, type=5, cpp_type=1, label=1,
  default_value=0,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)
IGNORE_NODE_FIELD_NUMBER = 113
ignore_node = descriptor.FieldDescriptor(
  name='ignore_node', full_name='lattice.ignore_node', index=3,
  number=113, type=8, cpp_type=7, label=1,
  default_value=False,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)


_LATTICE_NODE = descriptor.Descriptor(
  name='Node',
  full_name='lattice.Lattice.Node',
  filename='lattice.proto',
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='id', full_name='lattice.Lattice.Node.id', index=0,
      number=1, type=5, cpp_type=1, label=2,
      default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='label', full_name='lattice.Lattice.Node.label', index=1,
      number=2, type=9, cpp_type=9, label=1,
      default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='edge', full_name='lattice.Lattice.Node.edge', index=2,
      number=3, type=11, cpp_type=10, label=3,
      default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],  # TODO(robinson): Implement.
  enum_types=[
  ],
  options=None)

_LATTICE_EDGE = descriptor.Descriptor(
  name='Edge',
  full_name='lattice.Lattice.Edge',
  filename='lattice.proto',
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='id', full_name='lattice.Lattice.Edge.id', index=0,
      number=1, type=5, cpp_type=1, label=1,
      default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='label', full_name='lattice.Lattice.Edge.label', index=1,
      number=2, type=9, cpp_type=9, label=1,
      default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='to_id', full_name='lattice.Lattice.Edge.to_id', index=2,
      number=3, type=5, cpp_type=1, label=2,
      default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],  # TODO(robinson): Implement.
  enum_types=[
  ],
  options=None)

_LATTICE = descriptor.Descriptor(
  name='Lattice',
  full_name='lattice.Lattice',
  filename='lattice.proto',
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='start', full_name='lattice.Lattice.start', index=0,
      number=5, type=5, cpp_type=1, label=2,
      default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='final', full_name='lattice.Lattice.final', index=1,
      number=6, type=5, cpp_type=1, label=3,
      default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='node', full_name='lattice.Lattice.node', index=2,
      number=7, type=11, cpp_type=10, label=3,
      default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],  # TODO(robinson): Implement.
  enum_types=[
  ],
  options=None)


_LATTICE_NODE.fields_by_name['edge'].message_type = _LATTICE_EDGE
_LATTICE.fields_by_name['node'].message_type = _LATTICE_NODE

class Lattice(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  
  class Node(message.Message):
    __metaclass__ = reflection.GeneratedProtocolMessageType
    DESCRIPTOR = _LATTICE_NODE
  
  class Edge(message.Message):
    __metaclass__ = reflection.GeneratedProtocolMessageType
    DESCRIPTOR = _LATTICE_EDGE
  DESCRIPTOR = _LATTICE

Lattice.Node.RegisterExtension(is_word)
Lattice.Node.RegisterExtension(word)
Lattice.Node.RegisterExtension(original_node)
Lattice.Node.RegisterExtension(ignore_node)