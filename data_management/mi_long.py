import sys 
from context import *
THRESHOLD = 0.0
mi = {}
pc = {}
times = {}
punc = set(["CD", "$",  "#", ":", ""])
bigpunc = set(["CD", ",", ".", "$", "-RRB-", "-LRB-", "#", "``", "''", ":", ""])

for l in sys.stdin:
  t = l.strip().split()
  context = tuple(t[:5])
  if len(set(context) & punc) != 0:
    continue
  times[context] = int(t[6])
  mis = t[7:]
  mi[context]={}
  for elem in mis:
    elem = elem.split(":")
    a = int(elem[0])
    b = int(elem[1])
    p = float(elem[2])
    
    mi[context][a,b] = p

contexts = []
for context, ctxt_mi in mi.iteritems():
  if times[context] > 0: #context == ("JJ", "NN", "CC"):
    #print context, ctxt_mi
    count =0 
    is_predicted = set()
    
    for i in range(len(context)):
      if context[i] in bigpunc: continue
      valid = []
      min_j = i
      max_j = i
      for j in range(i+1, len(context)):
        if i == j:continue
        
        v = ctxt_mi[tuple(sorted([j-1,j]))]
        if v > THRESHOLD:
          if j== i+1:
            max_j = max(max_j, j)
          
          else:
            better = False
            for j_prime in range(i, j):
              v = ctxt_mi[tuple(sorted([j_prime,j]))]
              better = better or v > THRESHOLD
            if better:
              max_j = max(max_j, j)
            else:
              break
          #valid.append((j,context[i],context[j], v))
        else:
          break

      for j in range(i-1, -1, -1):
        if i == j:continue
        
        v = ctxt_mi[tuple(sorted([j+1,j]))]
        if v > THRESHOLD:
          if j== i-1:
            min_j = min(min_j, j)
          
          else:
            better = False
            for j_prime in range(j+1, i+1):
              v = ctxt_mi[tuple(sorted([j_prime,j]))]
              better = better or v > THRESHOLD
            if better:
              min_j = min(min_j, j)
            else:
              break
          #valid.append((j,context[i],context[j], v))
        else:
          break
      


      #if len(valid) == 4:
        #if len(is_predicted) == 5:
      print >>sys.stderr, valid
      
      if max_j - min_j >= 2:
        contexts.append((context[min_j:max_j+1], i - min_j))
      if max_j - i > 1:
        contexts.append((context[min_j:max_j], i - min_j))

      if i - min_j > 1:
        contexts.append((context[min_j+1:max_j+1], i - (min_j+1)))

      
#     for i in range(3):
#       for j in range(i,3):
#         if i <> j:
#           print i,j, pc[context][i,j], pc[context][j,i]

contexts = list(set(contexts))

for c,i in contexts:
  #print c, i 
  c = list(c)
  lc = len(c)
  def simple_word(pos):
    return SimpleWord(None, pos)
  #for i in range(lc):
  
    #print i
  start_boundaries = (["*"]*(4-i)) + c[:i]
  end_boundaries =  (c[i+1:] if i+1 <> lc else []) + (["*"]*(4-(lc -1 -i)))
  boundaries = map(simple_word, start_boundaries + end_boundaries)
  #print lc, i, len(boundaries), len(start_boundaries), len(end_boundaries)
  context = GeneralContext(simple_word(c[i]), boundaries)
  print context 
  #if ctxt_mi[0,1] > 1.0 and ctxt_mi[0,2] > 1.0:
    
   # print context, ctxt_mi


