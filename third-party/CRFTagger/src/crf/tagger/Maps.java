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

public class Maps {
    public Map cpStr2Int = null;
    public Map cpInt2Str = null;
    public Map lbStr2Int = null;
    public Map lbInt2Str = null;
    
    public Maps() {
    }
    
    public void readCpMaps(BufferedReader fin) throws IOException {
	if (cpStr2Int != null) {
	    cpStr2Int.clear();
	} else {
	    cpStr2Int = new HashMap();
	}
	
	if (cpInt2Str != null) {
	    cpInt2Str.clear();
	} else {
	    cpInt2Str = new HashMap();
	}
	
	String line;
	
	// get size of context predicate map
	if ((line = fin.readLine()) == null) {
	    System.out.println("No context predicate map size information");
	    return;
	}
	int numCps = Integer.parseInt(line);
	if (numCps <= 0) {
	    System.out.println("Invalid mapping size");
	    return;
	}
	
	System.out.println("Reading the context predicate maps ...");
		
	for (int i = 0; i < numCps; i++) {
	    line = fin.readLine();
	    if (line == null) {
		System.out.println("Invalid context predicate mapping line");
		return;
	    }
	    
	    StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
	    if (strTok.countTokens() != 2) {
		continue;
	    }
	    
	    String cpStr = strTok.nextToken();
	    String cpInt = strTok.nextToken();
	    
	    cpStr2Int.put(cpStr, new Integer(cpInt));
	    cpInt2Str.put(new Integer(cpInt), cpStr);
	}
	
	System.out.println("Reading context predicate maps (" + Integer.toString(cpStr2Int.size()) + 
		    " entries) completed!");
	
	// read the line ###...
	line = fin.readLine();
    }
    
    public void readLbMaps(BufferedReader fin) throws IOException {
	if (lbStr2Int != null) {
	    lbStr2Int.clear();
	} else {
	    lbStr2Int = new HashMap();
	}
	
	if (lbInt2Str != null) {
	    lbInt2Str.clear();
	} else {
	    lbInt2Str = new HashMap();
	}

	String line;
	
	// get size of label map
	if ((line = fin.readLine()) == null) {
	    System.out.println("No label map size information");
	    return;
	}
	int numLabels = Integer.parseInt(line);
	if (numLabels <= 0) {
	    System.out.println("Invalid label map size");
	    return;
	}
	
	System.out.println("Reading label maps ...");
	
	for (int i = 0; i < numLabels; i++) {
	    line = fin.readLine();
	    if (line == null) {
		System.out.println("Invalid label map line");
		return;
	    }
	    
	    StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
	    if (strTok.countTokens() != 2) {
		continue;
	    }
	    
	    String lbStr = strTok.nextToken();
	    String lbInt = strTok.nextToken();
	    
	    lbStr2Int.put(lbStr, new Integer(lbInt));
	    lbInt2Str.put(new Integer(lbInt), lbStr);
	}
	
	System.out.println("Reading label maps (" + Integer.toString(lbStr2Int.size()) + 
		    " entries) completed!");
	
	// read the line ###...
	line = fin.readLine();
    }
    
    public int numCps() {
	if (cpStr2Int == null) {
	    return 0;
	} else {
	    return cpStr2Int.size();
	}
    }
    
    public int numLabels() {
	if (lbStr2Int == null) {
	    return 0;
	} else {
	    return lbStr2Int.size();
	}
    }
    
} // end of class Maps

