/*
    Copyright (C) 2006, Xuan-Hieu Phan
    
    Email:	hieuxuan@ecei.tohoku.ac.jp
		pxhieu@gmail.com
    URL:	http://www.hori.ecei.tohoku.ac.jp/~hieuxuan
    
    Graduate School of Information Sciences,
    Tohoku University
*/

package crf.tagger;

import java.io.*;
import java.util.*;

public class TaggingData {
    List data = null;
    Set knownWords = new HashSet();
    public void readKnownWords(String dataFile) {
	BufferedReader fin = null;
	
	try {
	    fin = new BufferedReader(new FileReader(dataFile));	
            String line = null;
	    // start to read sentences => sequences
	    while ((line = fin.readLine()) != null) {
		StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
                String known = strTok.nextToken();
                System.err.println("know" + known);
                knownWords.add(known);
            }
        } catch (IOException e) {
	    System.out.println("Couldn't open data file" + dataFile);
	    return;
	}
    }

    // each sentence on one line
    public void readData(String dataFile) {
	if (data != null) {
	    data.clear();
	} else {
	    data = new ArrayList();
	}
	
	// open data file
	BufferedReader fin = null;
	
	try {
	    fin = new BufferedReader(new FileReader(dataFile));	
	    
	    System.out.println("Reading input data ...");
	    
	    String line = null;
	    // start to read sentences => sequences
	    while ((line = fin.readLine()) != null) {
		//line = PennTokenizer.tokenize(line);
	    
		StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
		
		if (strTok.countTokens() == 0) {
		    // skip this blank line
		    continue;
		}
		
		// create new data sequence
		List seq = new ArrayList();
		
		while (strTok.hasMoreTokens()) {
		    Observation obsr = new Observation();
                    String dat = strTok.nextToken();
                    System.err.println(dat);
                    // if (dat.charAt(0) == '/' && dat.charAt(1) == '/'j) {
                    //     StringTokenizer locTok = new StringTokenizer(dat, "/");
                    //     obsr.originalData = "/";
                    //     obsr.originalPos = locTok.nextToken();

                    // } else {
                    //StringTokenizer locTok = new StringTokenizer(dat, "~");
                    obsr.originalData = dat;
                    System.out.println("Data is " + dat + " " + knownWords.contains(dat));
                    obsr.knownWord = knownWords.contains(dat);
                    //locTok.nextToken();
                        //obsr.originalPos = locTok.nextToken();
                        //                    }
		    seq.add(obsr);
		}
		
		data.add(seq);
	    }
	    
	    System.out.println("Reading input data (" + Integer.toString(data.size()) + 
			" sequences) completed!");
	    
	} catch (IOException e) {
	    System.out.println("Couldn't open data file" + dataFile);
	    return;
	}
    }
    
    // write output, each sentence on a line
    // <word1>/<postag1> <word2>/<postag2> ...
    public void writeData(Map lbInt2Str, String outputFile) {
	if (data == null) {
	    return;
	}
	
	PrintWriter fout = null;
	
	try { 
	    fout = new PrintWriter(new FileWriter(outputFile));
	
	    // main loop for writing
	    for (int i = 0; i < data.size(); i++) {
		List seq = (List)data.get(i);
		for (int j = 0; j < seq.size(); j++) {
		    Observation obsr = (Observation)seq.get(j);
		    fout.print(obsr.toString(lbInt2Str) + " ");
                    
		}
		fout.println();
	    }
	
	    fout.close();
	    
	} catch(IOException e) {
	    System.out.println("Couldn't create file: " + outputFile);
	    return;
	}
    }

    // write output, each cpgen list on a line
   

    public void writeTaggedData(Map cpStr2Int, Map lbInt2Str, String inputFile, String outputFile) {
	//if (data == null) {
        //  return;
	//}
	
	PrintWriter fout = null;
	BufferedReader fin = null;
	
	try { 

	    fin = new BufferedReader(new FileReader(inputFile));	
	    fout = new PrintWriter(new FileWriter(outputFile));

	    
	    System.out.println("Reading input data ...");
	    
	    String line = null;
	    // start to read sentences => sequences
	    while ((line = fin.readLine()) != null) {
		//line = PennTokenizer.tokenize(line);
	    
		StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
		
		if (strTok.countTokens() == 0) {
		    // skip this blank line
		    continue;
		}
		
		// create new data sequence
		List seq = new ArrayList();
		
		while (strTok.hasMoreTokens()) {
		    Observation obsr = new Observation();
                    String dat = strTok.nextToken();
                    System.err.println(dat);
                    // if (dat.charAt(0) == '/' && dat.charAt(1) == '/'j) {
                    //     StringTokenizer locTok = new StringTokenizer(dat, "/");
                    //     obsr.originalData = "/";
                    //     obsr.originalPos = locTok.nextToken();

                    // } else {
                        StringTokenizer locTok = new StringTokenizer(dat, "~");
                        obsr.originalData = locTok.nextToken();
                        obsr.originalPos = locTok.nextToken();
                        //                    }
		    seq.add(obsr);
		}
		
	    	
		cpGen(cpStr2Int, seq);
                for (int j = 0; j < seq.size(); j++) {
		    Observation obsr = (Observation)seq.get(j);
                    for (int k = 0; k < obsr.cpsStr.length; k++) {
                        fout.print(obsr.cpsStr[k] + " ");
                    }
                    fout.print(obsr.originalPos);
                    System.err.println(obsr.originalPos);
                    fout.println();
		}
		fout.println();
	    }
	
	    fout.close();
	    fin.close();
	} catch(IOException e) {
	    System.out.println("Couldn't create file: " + outputFile);
	    return;
	}
    }

    
    // context predicate generation for each position
    public void cpGen( Map cpStr2Int, List seq, int i) {
	List tempCps = new ArrayList();
	int j;
	int seqLen = seq.size();
	
	if (i < 0 || i > seqLen - 1) {
	    return;
	}
	
	// single word
	for (j = -2; j <= 2; j++) {
	    if (i + j >= 0 && i + j < seqLen) {	    
		// 1 = w:
		String cp = "1:";
		cp += Integer.toString(j) + ":" + ((Observation)seq.get(i + j)).originalData;
		tempCps.add(cp.toLowerCase());
	    }
	}
	
	// prefixes
	for (j = 0; j <= 0; j++) {
	    if (i + j >= 0 && i + j < seqLen) {
		String currentToken = ((Observation)seq.get(i + j)).originalData;
		
		int prefixLen = currentToken.length() - 2;
		if (prefixLen > 4) {
		    prefixLen = 4;
		}
		
		for (int count = 1; count <= prefixLen; count++) {
		    String prefix = currentToken.substring(0, count);		    
		    // 2 = p:
		    String cp = "2:";
		    cp += Integer.toString(j) + ":" + prefix;
		    tempCps.add(cp);
		}
	    }
	}
	
	// suffixes
	for (j = 0; j <= 0; j++) {
	    if (i + j >= 0 && i + j < seqLen) {
		String currentToken = ((Observation)seq.get(i + j)).originalData;
		
		int suffixLen = currentToken.length() - 2;
		if (suffixLen > 4) {
		    suffixLen = 4;
		}	
		
		for (int count = 1; count <= suffixLen; count++) {
		    String suffix = currentToken.substring(currentToken.length() - count, 
				currentToken.length());		    
		    // 3 = s:
		    String cp = "3:";
		    cp += Integer.toString(j) + ":" + suffix;
		    tempCps.add(cp);
		}			
	    }
	}
	
	// two consecutive words
	for (j = -1; j <= 0; j++) {
	    if (i + j >= 0 && i + j + 1 < seqLen) {
		// 4 = ww:
		String cp = "4:";
		cp += Integer.toString(j) + ":" + Integer.toString(j + 1) + ":" +
			    ((Observation)seq.get(i + j)).originalData + ":" +
			    ((Observation)seq.get(i + j + 1)).originalData;
		tempCps.add(cp.toLowerCase());	
	    }
	}
	
	for (j = 0; j <= 0; j++) {
	    if (i + j >= 0 && i + j < seqLen) {
		int k;
		
		String currentToken = ((Observation)seq.get(i + j)).originalData;
		int tokenLen = currentToken.length();
		
		boolean isAllCap = true;
		k = 0;
		while (k < tokenLen) {
		    if (!(Character.isUpperCase(currentToken.charAt(k)))) {
			isAllCap = false;
			break;
		    } 
		    k++;
		}
		
		if (isAllCap) {
		    // 5 = i:allc
		    String cp = "5:" + Integer.toString(j);		    
		    tempCps.add(cp);
		    
		    if (currentToken.endsWith("S")) {
			// 6 = i:allcs
			cp = "6:" + Integer.toString(j);
			tempCps.add(cp);
		    }
		}
		
		if (!isAllCap && Character.isUpperCase(currentToken.charAt(0))) {
		    // 7 = i:fstc
		    String cp = "7:" + Integer.toString(j);
		    tempCps.add(cp);
		    
		    String preToken = null;
		    if (i + j > 0) {
			preToken = ((Observation)seq.get(i + j - 1)).originalData;
		    }
		    
		    if (i + j == 0 || (i + j > 0 && preToken.compareTo("``") == 0)) {
			// 8 = i:fstsfstc
			cp = "8:" + Integer.toString(j);
			tempCps.add(cp);
			
		    } else {
			// 9 = i:nfstsfstc
			cp = "9:" + Integer.toString(j);
			tempCps.add(cp);
		    }
		    
		    if (currentToken.endsWith("s")) {
			// 10 = i:fstcs
			cp = "10:" + Integer.toString(j);
			tempCps.add(cp);
			
			if (i + j == 0 || (i + j > 0 && preToken.compareTo("``") == 0)) {
			    // 11 = i:fstsfstcs
			    cp = "11:" + Integer.toString(j);
			    tempCps.add(cp);
			    
			} else {
			    // 12 = i:nfstsfstcs
			    cp = "12:" + Integer.toString(j);
			    tempCps.add(cp);
			}
		    }
		}
		
		boolean hasNumber = false;
		k = 0;
		while (k < tokenLen) {
		    if (Character.isDigit(currentToken.charAt(k))) {
			hasNumber = true;
			break;
		    }
		    k++;
		}		
		
		boolean isAllNumber = true;
		k = 0;
		while (k < tokenLen) {
		    if (!(Character.isDigit(currentToken.charAt(k))) && 
			    currentToken.charAt(k) != '.' && currentToken.charAt(k) != ',') {
			isAllNumber = false;
			break;
		    }
		    k++;
		}
		if (!hasNumber) {
		    isAllNumber = false;
		}	
		
		if (isAllNumber) {
		    // 13 = n:alln
		    String cp = "13:" + Integer.toString(j);
		    tempCps.add(cp);
		}
		
		if (!isAllNumber && hasNumber) {
		    // 14 = n:hasn
		    String cp = "14:" + Integer.toString(j);
		    tempCps.add(cp);
		}
		
		boolean hasHyphen = false;
		k = 0;
		while (k < tokenLen) {
		    if (currentToken.charAt(k) == '-') {
			hasHyphen = true;
			break;
		    }
		    k++;
		}
		
		if (hasHyphen) {
		    //15 = h:hyph
		    String cp = "15:" + Integer.toString(j);
		    tempCps.add(cp);
		}
	    }
	}
	
	List tempCpsInt = new ArrayList();
	
	for (int k = 0; k < tempCps.size(); k++) {
	    Integer cpInt = (Integer)cpStr2Int.get((String)tempCps.get(k));
	    if (cpInt == null) {
		continue;
	    }
	    tempCpsInt.add(cpInt);
	}
	
	Observation obsr = (Observation)seq.get(i);
	obsr.cps = new int[tempCpsInt.size()];
	obsr.cpsStr = new String[tempCpsInt.size()];
	
	for (int k = 0; k < tempCpsInt.size(); k++) {
	    obsr.cps[k] = ((Integer)tempCpsInt.get(k)).intValue();
            obsr.cpsStr[k] = (String)tempCps.get(k);
	}
    }
    
    // context predicate generation for each sequence
    public void cpGen(Map cpStr2Int, List seq) {
	for (int i = 0; i < seq.size(); i++) {
	    cpGen(cpStr2Int, seq, i);
	}
    }    
    
    // context predicate generation
    public void cpGen(Map cpStr2Int) {
	System.out.println("Generating context predicates for input data ...");
	for (int i = 0; i < data.size(); i++) {
	    cpGen(cpStr2Int, (List)data.get(i));
	}
	System.out.println("Generating context predicates for input data completed!");
    }
    
} // end of class TaggingData

