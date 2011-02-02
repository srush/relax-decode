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

public class Option {
    
    static public final int FIRST_ORDER = 1;
    // second-order Markov is not supported currently
    static public final int SECOND_ORDER = 2;
    
    public static final String inputSeparator = "/";
    public static final String outputSeparator = "/";
    
    // model directory, default is current dir
    public String modelDir = ".";
    // model file (mapping, dictionary, and features)
    public final String modelFile = "model.txt";
    // option file
    public final String optionFile = "option.txt";
    
    public int order = FIRST_ORDER;	// 1: first-order Markov; 2: second-order Markov
    
    public Option() {
    }
    
    public Option(String modelDir) {
	if (modelDir.endsWith(File.separator)) {
	    this.modelDir = modelDir.substring(0, modelDir.length() - 1);
	} else {
	    this.modelDir = modelDir;
	}
    }
    
    public boolean readOptions() {
	String filename = modelDir + File.separator + optionFile;
	BufferedReader fin = null;
	String line;
	
	try {
	    fin = new BufferedReader(new FileReader(filename));

	    System.out.println("Reading options ...");
	    	    
	    // read option lines
	    while ((line = fin.readLine()) != null) {
		String trimLine = line.trim();
		if (trimLine.startsWith("#")) {
		    // comment line
		    continue;
		}
		
		StringTokenizer strTok = new StringTokenizer(line, "= \t\r\n");
		int len = strTok.countTokens();
		if (len != 2) {
		    // invalid parameter line, ignore it
		    continue;
		}
		
		String strOpt = strTok.nextToken();
		String strVal = strTok.nextToken();
		
		if (strOpt.compareToIgnoreCase("order") == 0) {
		    int numTemp = Integer.parseInt(strVal);
		    order = numTemp;
		}		
	    }
	    
	    System.out.println("Reading options completed!");
	
	} catch (IOException e) {
	    System.out.println("Couldn't open and read option file: " + optionFile);
	    System.out.println(e.toString());
	    return false;
	}
	
	return true;	
    }
    
    public BufferedReader openModelFile() {
	String filename = modelDir + File.separator + modelFile;
	BufferedReader fin = null;
	
	try {
	    fin = new BufferedReader(new FileReader(filename));
	    
	} catch (IOException e) {
	    System.out.println("Couldn't open model file: " + filename);
	    System.out.println(e.toString());
	    fin = null;
	}
	
	return fin;
    }

} // end of class Option

