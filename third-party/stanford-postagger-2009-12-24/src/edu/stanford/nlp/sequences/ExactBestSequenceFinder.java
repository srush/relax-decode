package edu.stanford.nlp.sequences;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.HashSet;


/**
 * A class capable of computing the best sequence given a SequenceModel.
 * Uses the Viterbi algorithm.
 *
 * @author Dan Klein
 * @author Teg Grenager (grenager@stanford.edu)
 */
public class ExactBestSequenceFinder implements BestSequenceFinder {

  private static final boolean useOld = false;
  private static final boolean DEBUG = false;

    private static final double BEAM = 6.0;

  /**
   * A class for testing.
   */
  private static class TestSequenceModel implements SequenceModel {

    private int[] correctTags = {0, 0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1, 0, 0};
    private int[] allTags = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    private int[] midTags = {0, 1, 2, 3};
    private int[] nullTags = {0};

    public int length() {
      return correctTags.length - leftWindow() - rightWindow();
    }

    public int leftWindow() {
      return 2;
    }

    public int rightWindow() {
      return 2;
    }

    public int[] getPossibleValues(int pos) {
      if (pos < leftWindow() || pos >= leftWindow() + length()) {
        return nullTags;
      }
      if (correctTags[pos] < 4) {
        return midTags;
      }
      return allTags;
    }

    public double scoreOf(int[] tags, int pos) {
      //System.out.println("Was asked: "+arrayToString(tags)+" at "+pos);
      boolean match = true;
      for (int loc = pos - leftWindow(); loc <= pos + rightWindow(); loc++) {
        if (tags[loc] != correctTags[loc]) {
          match = false;
        }
      }
      if (match) {
        return pos;
      }
      return 0;
    }

    public double scoreOf(int[] sequence) {
      throw new UnsupportedOperationException();
    }

    public double[] scoresOf(int[] tags, int pos) {
      int[] tagsAtPos = getPossibleValues(pos);
      double[] scores = new double[tagsAtPos.length];
      for (int t = 0; t < tagsAtPos.length; t++) {
        tags[pos] = tagsAtPos[t];
        scores[t] = scoreOf(tags, pos);
      }
      return scores;
    }

  } // end class TestSequenceModel


  public static void main(String[] args) {
    BestSequenceFinder ti = new ExactBestSequenceFinder();
    SequenceModel ts = new TestSequenceModel();
    int[] bestTags = ti.bestSequence(ts);
    System.out.println("The best sequence is ... " + Arrays.toString(bestTags));
  }

  /**
   * Runs the Viterbi algorithm on the sequence model given by the TagScorer
   * in order to find the best sequence.
   * @param ts The SequenceModel to be used for scoring
   * @return An array containing the int tags of the best sequence
   */
  public int[] bestSequence(SequenceModel ts) {
    if (useOld) {
      return bestSequenceOld(ts);
    } else {
      return bestSequenceNew(ts);
    }
  }

  private int[] bestSequenceNew(SequenceModel ts) {


    // Set up tag options
    // Sequence length (sentence length)
    int length = ts.length();
    // How much we look to our left
    int leftWindow = ts.leftWindow();
    // how much we look to our right
    int rightWindow = ts.rightWindow();
    // padding <s> <s> </s> </s>
    int padLength = length + leftWindow + rightWindow;
    // tags[ind][tag] (all the possible tags
    int[][] tags = new int[padLength][];

    // tags that are actually seen in the beam
    ArrayList<HashSet<Integer>> beam_tags = new ArrayList<HashSet<Integer>>();
    //beam_tags.ensureCapacity(padLength);
    //int[][] beam_tags = new int[padLength][];

    // the number of available tags at each position
    int[] tagNum = new int[padLength];
    if (DEBUG) { System.err.println("Doing bestSequence length " + length + "; leftWin " + leftWindow + "; rightWin " + rightWindow + "; padLength " + padLength); }
    
    // Fill in the possible tags at each position
    for (int pos = 0; pos < padLength; pos++) {
      beam_tags.add( new HashSet<Integer>());
      tags[pos] = ts.getPossibleValues(pos);
      tagNum[pos] = tags[pos].length;
      if (DEBUG) { System.err.println("There are " + tagNum[pos] + " values at position " + pos + ": " + Arrays.toString(tags[pos])); }
    }

    // The termporary tag decided at each position
    int[] tempTags = new int[padLength];

    // poducts?
    // Set up product space sizes
    int[] productSizes = new int[padLength];

    // products = nodes

    // This is going to be the size of our chart space
    int curProduct = 1;
   
    // Give spaces for the tags in the left and right window (just 1 likely)
    for (int i = 0; i < leftWindow + rightWindow; i++) {
      curProduct *= tagNum[i];
    }

    for (int pos = leftWindow + rightWindow; pos < padLength; pos++) {
      if (pos > leftWindow + rightWindow) {
        curProduct /= tagNum[pos - leftWindow - rightWindow - 1]; // shift off
      }
      // add in room for all the tags at this position
      curProduct *= tagNum[pos]; // shift on
      // save the size of all states up until this point
      productSizes[pos - rightWindow] = curProduct;
    }

    // Score all of each window's options

    double[][] windowScore = new double[padLength][];
    for (int pos = leftWindow; pos < leftWindow + length; pos++) {
      if (DEBUG) { System.err.println("scoring word " + pos + " / " + (leftWindow + length) + ", productSizes =  " + productSizes[pos] + ", tagNum = " + tagNum[pos] + "..."); }

      // score for each product in the pos
      windowScore[pos] = new double[productSizes[pos]];
      // let each tag be the 0,0 tag
      // blank out temptags
      Arrays.fill(tempTags, tags[0][0]);

      if (DEBUG) { System.err.println("windowScore[" + pos + "] has size (productSizes[pos]) " + windowScore[pos].length); }

      // unmap a product to a window of tags
      for (int product = 0; product < productSizes[pos]; product++) {
        int p = product;
        int shift = 1;
        for (int curPos = pos + rightWindow; curPos >= pos - leftWindow; curPos--) {
          tempTags[curPos] = tags[curPos][p % tagNum[curPos]];
          p /= tagNum[curPos];
          if (curPos > pos) {
            shift *= tagNum[curPos];
          }
        }

        // Here now you get ts.scoresOf() for all classifications at a position at once, wwhereas the old code called ts.scoreOf() on each item.
        // CDM May 2007: The way this is done gives incorrect results if there are repeated values in the values of ts.getPossibleValues(pos) -- in particular if the first value of the array is repeated later.  I tried replacing it with the modulo version, but that only worked for left-to-right, not bidirectional inference, but I still think that if you sorted things out, you should be able to do it with modulos and the result would be conceptually simpler and robust to repeated values.  But in the meantime, I fixed the POS tagger to not give repeated values (which was a bug in the tagger).
        // if (product % tagNum[pos] == 0) {
        if (tempTags[pos] == tags[pos][0]) {
          // get all tags at once
          double[] scores = ts.scoresOf(tempTags, pos);
          if (DEBUG) { System.err.println("Matched at array index [product] " + product + "; tempTags[pos] == tags[pos][0] == " + tempTags[pos]); }
          if (DEBUG) { System.err.println("For pos " + pos + " scores.length is " + scores.length + "; tagNum[pos] = " + tagNum[pos] + "; windowScore[pos].length = " + windowScore[pos].length); }
          if (DEBUG) { System.err.println("scores: " + Arrays.toString(scores)); }
          // fill in the relevant windowScores
          
          // set the scores for each tag at pos
          for (int t = 0; t < tagNum[pos]; t++) {
            if (DEBUG) { System.err.println("Setting value of windowScore[" + pos + "][" + product + "+" + t + "*" + shift + "] = " + scores[t]); }
            windowScore[pos][product + t * shift] = scores[t];
          }
        }
      }
    }

    // Set up score and backtrace arrays
    double[][] score = new double[padLength][];
    int[][] trace = new int[padLength][];
    int[][] node_index = new int[padLength][];
    Boolean[][] in_beam = new Boolean[padLength][];
    for (int pos = 0; pos < padLength; pos++) {
      score[pos] = new double[productSizes[pos]];
      trace[pos] = new int[productSizes[pos]];
      node_index[pos] = new int[productSizes[pos]];
      in_beam[pos] = new Boolean[productSizes[pos]];
      
    }

    // Do forward Viterbi algorithm
    System.out.println("");
    System.out.println("LATTICE: START");
    int node_count = 0;


    // loop over the classification spot
    //System.err.println();
    System.out.println("LATTICE: NODE "+node_count+" START");
    node_count++;
    for (int pos = leftWindow; pos < length + leftWindow; pos++) {

        double max_at_pos = Double.NEGATIVE_INFINITY;;      
        // if (pos != leftWindow) {
        //     for (int newTagNum = 0; newTagNum < tagNum[pos - leftWindow - 1]; newTagNum++) {
        //         System.err.println("NODE " + pos + " " + newTagNum);        
        //     }
        // }

      for (int product = 0; product < productSizes[pos]; product++) {
          //in_beam[pos][product] = (max_at_pos - score[pos][product]) < BEAM; 
          in_beam[pos][product] = false;
          score[pos][product] = Double.NEGATIVE_INFINITY;
          

      }


      // all the states at the current point
      for (int product = 0; product < productSizes[pos]; product++) {
        int tag = tags[pos][product % tagNum[pos]];//tags[pos - leftWindow - 1][newTagNum];

        boolean has_seen = false;
          //+ " " + tags[pos - leftWindow - 1][newTagNum]
        // check for initial spot
        if (pos == leftWindow) {
          // no predecessor type
          score[pos][product] = windowScore[pos][product];
          trace[pos][product] = -1;
          
          if (!has_seen) {
              System.out.println("LATTICE: NODE "+node_count+" " + (pos-2) + ":" + product + ":" + tag); 
              node_index[pos][product] = node_count;
              node_count++;
              beam_tags.get(pos).add(tag);
          }
          has_seen = true;

          System.out.println("LATTICE: EDGE " + (pos-3) + ":0:" + (pos-2) + ":" + product +" 0 "+node_index[pos][product]+" " + windowScore[pos][product]);  
          
        } else {
          // loop over possible predecessor types
          score[pos][product] = Double.NEGATIVE_INFINITY;
          trace[pos][product] = -1;
          int sharedProduct = product / tagNum[pos + rightWindow];
          int factor = productSizes[pos] / tagNum[pos + rightWindow];
      
          // pos - leftWindow - 1
          for (int newTagNum = 0; newTagNum < tagNum[pos - leftWindow - 1]; newTagNum++) {
            
            // last node
            int predProduct = newTagNum * factor + sharedProduct;
            

            // edge score
            double edge_score = windowScore[pos][product];
            // new score
            double predScore = score[pos - 1][predProduct] + edge_score;

            // is prev node pruned or this node is pruned
            if (!in_beam[pos-1][predProduct]) {
                //System.out.println("DEBUG: was beamed " + (pos-1) + " " + predProduct);
                continue;
            }
            if ((max_at_pos - predScore) > BEAM) { 
                //System.out.println("DEBUG: will be beamed " + (max_at_pos) + " " + predScore);
                continue;
            }
            //if (!in_beam[pos-1][predProduct]) continue;
            
            if (!has_seen) {
               System.out.println("LATTICE: NODE "+node_count+" " + (pos-2) + ":" + product + ":" + tag);                node_index[pos][product] = node_count;
              node_count++;
              beam_tags.get(pos).add(tag);
            }
            has_seen = true;

            System.out.println("LATTICE: EDGE " + (pos-3) + ":" + predProduct + ":" + (pos-2) + ":" + product +" "+node_index[pos-1][predProduct]+ " "+ node_index[pos][product]+" " + edge_score);  
            if (predScore > score[pos][product]) {
              score[pos][product] = predScore;
              trace[pos][product] = predProduct;
              max_at_pos = Math.max(score[pos][product], max_at_pos);
            }
          }
        }
      }

              
      // delta beam
      for (int product = 0; product < productSizes[pos]; product++) {
          Boolean should_keep = (max_at_pos - score[pos][product]) < BEAM; 
          in_beam[pos][product] = should_keep;
          if (!should_keep) {
              //System.out.println("DEBUG: Is beamed " + max_at_pos + " " + score[pos][product] + " " + product);
          }
      }

    }



    // Project the actual tag sequence
    double bestFinalScore = Double.NEGATIVE_INFINITY;
    int bestCurrentProduct = -1;
    
    System.out.println("LATTICE: NODE "+ node_count +" final");
    node_count++;
    for (int product = 0; product < productSizes[leftWindow + length - 1]; product++) {
        int lastpos = leftWindow + length - 1;
        if (!in_beam[lastpos][product]) continue;
        System.out.println("LATTICE: EDGE last " + node_index[lastpos][product] + " " +  (node_count -1) + " 0.0");  
      if (score[leftWindow + length - 1][product] > bestFinalScore) {
        bestCurrentProduct = product;
        bestFinalScore = score[leftWindow + length - 1][product];
      }
    }
    // the final product
    int lastProduct = bestCurrentProduct;
    for (int last = padLength - 1; last >= length - 1 && last >= 0; last--) {
      tempTags[last] = tags[last][lastProduct % tagNum[last]];
      lastProduct /= tagNum[last];
    }
    for (int pos = leftWindow + length - 2; pos >= leftWindow; pos--) {
      int bestNextProduct = bestCurrentProduct;
      bestCurrentProduct = trace[pos + 1][bestNextProduct];
      tempTags[pos - leftWindow] = tags[pos - leftWindow][bestCurrentProduct / (productSizes[pos] / tagNum[pos - leftWindow])];
    }
    

    System.out.println("LATTICE: END");  
    System.out.println("BEAM: START" );
    for (int pos = leftWindow; pos < length + leftWindow -1; pos++) {
        System.out.print("BEAM: " + (pos-2) + " " );
        for (Integer tag: beam_tags.get(pos)) {
            System.out.print( tag + " " );
        }
        System.out.println();
    }
    System.out.println("BEAM: END" );
    System.out.println("best final score " + bestFinalScore);  
    return tempTags;
  }

  private int[] bestSequenceOld(SequenceModel ts) {

    // Set up tag options
    int length = ts.length();
    int leftWindow = ts.leftWindow();
    int rightWindow = ts.rightWindow();
    int padLength = length+leftWindow+rightWindow;
    int[][] tags = new int[padLength][];
    int[] tagNum = new int[padLength];
    for (int pos = 0; pos < padLength; pos++) {
      tags[pos] = ts.getPossibleValues(pos);
      tagNum[pos] = tags[pos].length;
    }

    int[] tempTags = new int[padLength];

    // Set up product space sizes
    int[] productSizes = new int[padLength];

    int curProduct = 1;
    for (int i=0; i<leftWindow+rightWindow; i++)
      curProduct *= tagNum[i];
    for (int pos = leftWindow+rightWindow; pos < padLength; pos++) {
      if (pos > leftWindow+rightWindow)
	curProduct /= tagNum[pos-leftWindow-rightWindow-1]; // shift off
      curProduct *= tagNum[pos]; // shift on
      productSizes[pos-rightWindow] = curProduct;
    }

    // Score all of each window's options
    double[][] windowScore = new double[padLength][];
    for (int pos=leftWindow; pos<leftWindow+length; pos++) {
      windowScore[pos] = new double[productSizes[pos]];
      Arrays.fill(tempTags,tags[0][0]);
      for (int product=0; product<productSizes[pos]; product++) {
	int p = product;
	for (int curPos = pos+rightWindow; curPos >= pos-leftWindow; curPos--) {
	  tempTags[curPos] = tags[curPos][p % tagNum[curPos]];
	  p /= tagNum[curPos];
	}
	windowScore[pos][product] = ts.scoreOf(tempTags, pos);
      }
    }


    // Set up score and backtrace arrays
    double[][] score = new double[padLength][];
    int[][] trace = new int[padLength][];
    for (int pos=0; pos<padLength; pos++) {
      score[pos] = new double[productSizes[pos]];
      trace[pos] = new int[productSizes[pos]];
    }

    // Do forward Viterbi algorithm

    // loop over the classification spot
    //System.err.println();
    for (int pos=leftWindow; pos<length+leftWindow; pos++) {
      //System.err.print(".");
      // loop over window product types
      for (int product=0; product<productSizes[pos]; product++) {
	// check for initial spot
	if (pos==leftWindow) {
	  // no predecessor type
	  score[pos][product] = windowScore[pos][product];
	  trace[pos][product] = -1;
	} else {
	  // loop over possible predecessor types
	  score[pos][product] = Double.NEGATIVE_INFINITY;
	  trace[pos][product] = -1;
	  int sharedProduct = product / tagNum[pos+rightWindow];
	  int factor = productSizes[pos] / tagNum[pos+rightWindow];
	  for (int newTagNum=0; newTagNum<tagNum[pos-leftWindow-1]; newTagNum++) {
	    int predProduct = newTagNum*factor+sharedProduct;
	    double predScore = score[pos-1][predProduct]+windowScore[pos][product];
	    if (predScore > score[pos][product]) {
	      score[pos][product] = predScore;
	      trace[pos][product] = predProduct;
	    }
	  }
	}
      }
    }

    // Project the actual tag sequence
    double bestFinalScore = Double.NEGATIVE_INFINITY;
    int bestCurrentProduct = -1;
    for (int product=0; product<productSizes[leftWindow+length-1]; product++) {
      if (score[leftWindow+length-1][product] > bestFinalScore) {
	bestCurrentProduct = product;
	bestFinalScore = score[leftWindow+length-1][product];
      }
    }
    int lastProduct = bestCurrentProduct;
    for (int last=padLength-1; last>=length-1; last--) {
      tempTags[last] = tags[last][lastProduct % tagNum[last]];
      lastProduct /= tagNum[last];
    }
    for (int pos=leftWindow+length-2; pos>=leftWindow; pos--) {
      int bestNextProduct = bestCurrentProduct;
      bestCurrentProduct = trace[pos+1][bestNextProduct];
      tempTags[pos-leftWindow] = tags[pos-leftWindow][bestCurrentProduct / (productSizes[pos]/tagNum[pos-leftWindow])];
    }
    return tempTags;
  }

}
