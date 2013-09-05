import sys, itertools

def sent_len(handle):
  m = {}
  for l in handle:
    n, l = l.split()
    m[int(n)] = int(l)
  return m

def sent_opt(handle):
  m = {}
  for l in handle:
    n, l = l.split()
    m[int(n)] = float(l)
  return m


def group(handle):
  t = []
  for l in handle:
    if "what" in l or "Killed" in l:
      t = []
    elif "END" in l:
      number = int(l.split()[0][5:])
      time = int(l.split()[2])
      score = float(l.split()[1])
      opt = int(l.split()[-1])
      yield (t, number, time, opt, score)
      t = []
    else:
      t.append(l)

iter_list = []
cube_list = []
iter_first_list = []

# > 30
iter_list_30 = []
cube_list_30 = []
iter_first_list_30 = []

time_list = []

length = sent_len(open(sys.argv[1]))
opts = sent_opt(open(sys.argv[2]))
by_lens = []
class ByLen:
  def __init__(self, len, time, opt, exact):
    self.len = len
    self.time = time
    self.opt = opt
    self.exact = exact


for g, number, time, opt, score in group(sys.stdin):
  iter = 0
  cube = 0
  iter_first = 0
  for l in g:
    t = int(l.strip().split()[-1])
    if "ITERATION" in l:
      if iter == 0: iter_first = t
      iter += t
    elif "CUBE" in l:
      cube += t
  iter_list.append(iter / 1000)
  cube_list.append(cube / 1000)
  iter_first_list.append(iter_first / 1000)

  l = length[number]
  if l >= 30:
    iter_list_30.append(iter / 1000)
    cube_list_30.append(cube / 1000)
    iter_first_list_30.append(iter_first / 1000)

  time_list.append(time)

  by_lens.append(ByLen(l, time, opt, score == opts.get(number, 0.0)))
  #print number, iter / 1000, cube / 1000, iter_first / 1000, time, l
  print number, score

def mean(ls):  return sum(ls) / float(len(ls))
def print_ls(ls):
  ls.sort()
  mean, med = sum(ls) / float(len(ls)), ls[len(ls) / 2]
  print mean, med
  return mean, med

by_lens.sort(key = lambda l: int(l.len / 10))
groups = itertools.groupby(by_lens, key = lambda l: int(min(4, l.len / 10)))
# for k, g in groups:
#   print k
#   g2 = list(g)
#   ls = [p.time for p in g2]
#   print_ls(ls)
#   ls = [p.opt for p in g2]
#   print_ls(ls)

for k, g in groups:
  g2 = list(g)

  #print k, len(g2), mean([p.time for p in g2]), mean([p.exact for p in g2])

  print " & ".join(map(lambda f: "%0.2f"%f, [0.001 * mean([p.time for p in g2]), 100 * mean([p.opt for p in g2]), 100 * mean([p.exact for p in g2]),])),
  print " & ",
  # ls = [p.time for p in g2]
  # print_ls(ls)
  # ls = [p.opt for p in g2]
  # print_ls(ls)

# print "all", len(by_lens),  mean([p.time for p in by_lens]), mean([p.opt for p in by_lens])
print " & ".join(map(lambda f: "%0.2f"%f, [0.001 * mean([p.time for p in by_lens]), 100 * mean([p.opt for p in by_lens]), 100 * mean([p.exact for p in by_lens]),])),
print
lr_mean, lr_med = print_ls(iter_list)
cube_mean, cube_med = print_ls(cube_list)
cons_mean, cons_med = print_ls(iter_first_list)
print_ls(time_list)

means = [cons_mean, lr_mean, cube_mean]
meds = [cons_med, lr_med, cube_med]

print "MEANS"
for m in means:
  print m / float(sum(means))

print "MEDS"
for m in meds:
  print m / float(sum(meds))

lr_mean, lr_med = print_ls(iter_list_30)
cube_mean, cube_med = print_ls(cube_list_30)
cons_mean, cons_med = print_ls(iter_first_list_30)

means = [cons_mean, lr_mean, cube_mean]
meds = [cons_med, lr_med, cube_med]

print "MEANS"
for m in means:
  print m / float(sum(means))

print "MEDS"
for m in meds:
  print m / float(sum(meds))
