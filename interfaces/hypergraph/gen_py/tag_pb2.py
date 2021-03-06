# Generated by the protocol buffer compiler.  DO NOT EDIT!

from google.protobuf import descriptor
from google.protobuf import message
from google.protobuf import reflection
from google.protobuf import service
from google.protobuf import service_reflection
from google.protobuf import descriptor_pb2

TAGGING_FIELD_NUMBER = 120
tagging = descriptor.FieldDescriptor(
  name='tagging', full_name='tagging', index=0,
  number=120, type=11, cpp_type=10, label=1,
  default_value=None,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)
HAS_TAGGING_FIELD_NUMBER = 121
has_tagging = descriptor.FieldDescriptor(
  name='has_tagging', full_name='has_tagging', index=1,
  number=121, type=8, cpp_type=7, label=1,
  default_value=False,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)


_TAGGING = descriptor.Descriptor(
  name='Tagging',
  full_name='Tagging',
  filename='tag.proto',
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='ind', full_name='Tagging.ind', index=0,
      number=1, type=5, cpp_type=1, label=2,
      default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='tag_id', full_name='Tagging.tag_id', index=1,
      number=2, type=5, cpp_type=1, label=2,
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

import hypergraph_pb2


class Tagging(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _TAGGING

tagging.message_type = _TAGGING
hypergraph_pb2.Hypergraph.Node.RegisterExtension(tagging)
hypergraph_pb2.Hypergraph.Node.RegisterExtension(has_tagging)
