import os
for i in range(10, 100):
  # os.system("build/opt/trans --flagfile=/home/srush/data/t2s/dev/params --forest_range=\"%d %d\" --ilp_mode=cube"%(i * 10, (i + 1) * 10 - 1))


  os.system("build/opt/trans --flagfile=/home/srush/data/t2s/dev/params --forest_range=\"%d %d\" --ilp_mode=proj"%(i * 5, (i + 1) * 5))
