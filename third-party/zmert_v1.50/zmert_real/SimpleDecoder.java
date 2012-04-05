import java.math.*;
import java.util.*;
import java.io.*;
import java.text.DecimalFormat;

public class SimpleDecoder
{
  private static DecimalFormat f3 = new DecimalFormat("###0.000");
  public static void main(String[] args) throws Exception
  {
    String configFileName = args[0];
    String sourceFileName = args[1];
    String outputFileName = args[2];

    int numParams = 3;
    int numSentences = countLines(sourceFileName);

    double[] weights = new double[numParams];
    String candsFileName = "";
    int cps = 0;
    int N = 0;

    InputStream inStream = new FileInputStream(new File(configFileName));
    BufferedReader inFile = new BufferedReader(new InputStreamReader(inStream, "utf8"));

    String line = inFile.readLine();
    while (line != null) {
      if (line.startsWith("cands_file")) {
        candsFileName = (line.substring(line.indexOf("=")+1)).trim();
      } else if (line.startsWith("cands_per_sen")) {
        cps = Integer.parseInt((line.substring(line.indexOf("=")+1)).trim());
      } else if (line.startsWith("top_n")) {
        N = Integer.parseInt((line.substring(line.indexOf("=")+1)).trim());
      } else if (line.startsWith("LM")) {
        weights[0] = Double.parseDouble((line.substring(2+1)).trim());
      } else if (line.startsWith("first model")) {
        weights[1] = Double.parseDouble((line.substring(11+1)).trim());
      } else if (line.startsWith("second model")) {
        weights[2] = Double.parseDouble((line.substring(12+1)).trim());
      } else if (line.startsWith("#")) {
      } else if (line.length() > 0) {
        println("Wrong format in config file.");
        System.exit(1);
      }
      line = inFile.readLine();
    }

    inFile.close();

    String[][] candidates = new String[numSentences][cps];
    double[][][] features = new double[numSentences][cps][numParams];

    inStream = new FileInputStream(new File(candsFileName));
    inFile = new BufferedReader(new InputStreamReader(inStream, "utf8"));

    for (int i = 0; i < numSentences; ++i) {
      for (int n = 0; n < cps; ++n) {
        // read the nth candidate for the ith sentence
        line = inFile.readLine();

/*
line format:

i ||| words of candidate translation . ||| feat-1_val feat-2_val ... feat-numParams_val .*

*/

        line = (line.substring(line.indexOf("|||")+3)).trim(); // get rid of initial text

        String candidate_str = (line.substring(0,line.indexOf("|||"))).trim();
        String feats_str = (line.substring(line.indexOf("|||")+3)).trim(); // get rid of candidate string

        int junk_i = feats_str.indexOf("|||");
        if (junk_i >= 0) {
          feats_str = (feats_str.substring(0,junk_i)).trim();
        }

        String[] featVal_str = feats_str.split("\\s+");

        candidates[i][n] = candidate_str;
        for (int c = 0; c < numParams; ++c) {
          features[i][n][c] = Double.parseDouble(featVal_str[c]);
        }

      }
    }


    double[][] scores = new double[numSentences][cps];
    for (int i = 0; i < numSentences; ++i) {
      for (int n = 0; n < cps; ++n) {
        scores[i][n] = 0;
        for (int c = 0; c < numParams; ++c) {
          scores[i][n] += weights[c]*features[i][n][c];
        }
      }
    }


    FileOutputStream outStream = new FileOutputStream(outputFileName, false); // false: don't append
    OutputStreamWriter outStreamWriter = new OutputStreamWriter(outStream, "utf8");
    BufferedWriter outFile = new BufferedWriter(outStreamWriter);

    for (int i = 0; i < numSentences; ++i) {
      int[] indices = sort(scores,i);
      for (int n = 0; n < N; ++n) {
        String str = "" + i + " ||| " + candidates[i][indices[n]] + " |||";
        for (int c = 0; c < numParams; ++c) {
          str += " " + f3.format(features[i][indices[n]][c]);
        }
        str += " ||| " + f3.format(scores[i][indices[n]]);
        writeLine(str, outFile);
      }

    }

    outFile.close();

    System.exit(0);

  }

  private static int[] sort(double[][] scores, int i)
  {
    int numCands = scores[i].length;
    int[] retA = new int[numCands];
    double[] sc = new double[numCands];

    for (int n = 0; n < numCands; ++n) {
      retA[n] = n;
      sc[n] = scores[i][n];
    }

    for (int j = 0; j < numCands; ++j) {
      int best_k = j;
      double best_sc = sc[j];
      for (int k = j+1; k < numCands; ++k) {
        if (sc[k] > best_sc) {
          best_k = k;
          best_sc = sc[k];
        }
      }

      // switch j and best_k
      int temp_n = retA[best_k];
      retA[best_k] = retA[j];
      retA[j] = temp_n;

      double temp_sc = sc[best_k];
      sc[best_k] = sc[j];
      sc[j] = temp_sc;
    }

    return retA;
  }

  private static void sort(int[] keys, double[] vals, int start, int end)
  {
    if (end-start > 1) {
      int mid = (start+end)/2;
      sort(keys,vals,start,mid);
      sort(keys,vals,mid+1,end);

    }
  }

  private static int countLines(String fileName) throws Exception
  {
    BufferedReader inFile = new BufferedReader(new FileReader(fileName));

    String line;
    int count = 0;
    do {
      line = inFile.readLine();
      if (line != null) ++count;
    }  while (line != null);

    inFile.close();

    return count;
  }

  private static void writeLine(String line, BufferedWriter writer) throws IOException
  {
    writer.write(line, 0, line.length());
    writer.newLine();
    writer.flush();
  }

  private static void println(Object obj) { System.out.println(obj); }
  private static void print(Object obj) { System.out.print(obj); }

}
