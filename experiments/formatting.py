import sys
root = "/home/srush/Projects/relax_decode/data_management/"
sys.path.append(root)
from format.simple import *

class Converter:
  def __call__(self, source, target, env):
    #print source, target
    self.convert(open(str(source[0])), open(str(target[0]), 'w'))
    return 0

  def convert(self, target_handle, source_handle):
    assert False

class SimpleSentConverter(Converter):
  def convert(self, sin, out):
    for sent in parse_simple_file(sin, '/'):

      print >>out, self.sent_convert(sent)  
    print >>out 
    

class SimpleToFlat(SimpleSentConverter):
  def sent_convert(self, sent):
    return "\n".join(word.word + " " + word.pos for word in sent)
      
