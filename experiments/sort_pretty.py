import sys
out = []
us =0
them = 0
for l in sys.stdin:
  t = l.strip().split()
  train = int( t[-1])
  broke = int( t[-3])
  fixed = int( t[-5])
  out.append((train,broke, fixed, l))
  
out = list(set(out))
out.sort(key= lambda a: a[1] )
out.reverse()

train_count = {}

for train,broke, fixed,l in out:
  train_count.setdefault(train, 0)
  train_count[train] += fixed - broke
  print l.strip()

  us += fixed
  #if fixed !=0 and broke < 10:
  them += broke
print train_count
print us, them
