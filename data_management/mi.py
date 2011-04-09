import sys 
THRESHOLD = 0.5
mi = {}
pc = {}
times = {}
for l in sys.stdin:
  t = l.strip().split()
  context = tuple(t[:3])
  punc = set([".", "$", "-RRB-", "-LRB-", "#", "``", "''", ":", ""])
  if len(set(context) & punc) != 0:
    continue
  mi[context]={}
  mi[context][0,1] = float(t[4]) 
  mi[context][0,2] = float(t[6])
  mi[context][1,2] = float(t[8])

  pc[context]={}
  pc[context][0,1] = float(t[10]) 
  pc[context][1,0] = float(t[12])
  pc[context][1,2] = float(t[14])
  pc[context][2,1] = float(t[16]) 
  pc[context][0,2] = float(t[18])
  pc[context][2,0] = float(t[20])
  
  times[context] = int(t[22])

contexts = []
for context, ctxt_mi in mi.iteritems():
  if times[context] > 20: #context == ("JJ", "NN", "CC"):
    #print context, ctxt_mi
    count =0 
    for k, v in ctxt_mi.iteritems():
      if v > THRESHOLD:
        count +=1
    if count >= 3:
      contexts.append(context)
      
#     for i in range(3):
#       for j in range(i,3):
#         if i <> j:
#           print i,j, pc[context][i,j], pc[context][j,i]

for c in contexts:
  print "* * * "+c[0] + " " + c[2] + " * * * " + c[1] 
  #if ctxt_mi[0,1] > 1.0 and ctxt_mi[0,2] > 1.0:
    
   # print context, ctxt_mi


