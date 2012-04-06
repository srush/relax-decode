import sys
param = open("config.temp.ini", 'w')
lm = open("config.temp.lm", 'w')

for l in sys.stdin:
  t = l.split()
  
  if t[0] != "LM":
    print >>param, t[0] +"="+t[1],
  else:
    print >>lm, t[1]
    
