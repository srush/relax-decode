import sys
if __name__ == "__main__":
  for l in sys.stdin:
    print "\n".join(l.strip().split()[1:])
    print
