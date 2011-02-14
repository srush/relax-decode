from unknown_words import *

if __name__=="__main__":

  wc = load(open(sys.argv[1], 'rb'))
  for w in wc.word_counts:
    print w
