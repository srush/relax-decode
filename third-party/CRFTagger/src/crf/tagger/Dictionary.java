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

public class Dictionary {
    public Map dict = null;			// map between context predicate and element
    
    Dictionary() {
	dict = new HashMap();
    }
    
    // read dictionary from model file
    public void readDict(BufferedReader fin) throws IOException {	
	// clear any previous content
	dict.clear();
	    
	String line;
	
	// get dictionary size
	if ((line = fin.readLine()) == null) {
	    System.out.println("No dictionary size information");
	    return;
	}	
	int dictSize = Integer.parseInt(line);
	if (dictSize <= 0) {
	    System.out.println("Invalid dictionary size");
	}
	
	System.out.println("Reading dictionary ...");
	
	// main loop for reading dictionary content
	for (int i = 0; i < dictSize; i++) {
	    line = fin.readLine();
	    if (line == null) {
		System.out.println("Invalid dictionary line");
		return;
	    }
	    
	    StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
	    int len = strTok.countTokens();
	    if (len < 2) {
		// invalid line
		continue;
	    }	    
	    
	    StringTokenizer cpTok = new StringTokenizer(strTok.nextToken(), ":");
	    int cp = Integer.parseInt(cpTok.nextToken());
	    int cpCount = Integer.parseInt(cpTok.nextToken());
	    
	    // create a new element
	    Element elem = new Element();
	    elem.count = cpCount;
	    elem.chosen = 1;
	    
	    while (strTok.hasMoreTokens()) {
		StringTokenizer lbTok = new StringTokenizer(strTok.nextToken(), ":");
		
		int order = Integer.parseInt(lbTok.nextToken());
		int label = Integer.parseInt(lbTok.nextToken());
		int count = Integer.parseInt(lbTok.nextToken());
		int fidx = Integer.parseInt(lbTok.nextToken());
		CountFeatureIdx cntFeaIdx = new CountFeatureIdx(count, fidx);
		
		if (order == Option.FIRST_ORDER) {
		    elem.lbCntFidxes.put(new Integer(label), cntFeaIdx);		
		} else if (order == Option.SECOND_ORDER) {
		    // do nothing, second-order Markov is not supported
		}		    
	    }

	    // insert the element to the dictionary
	    dict.put(new Integer(cp), elem);
	}
	
	System.out.println("Reading dictionary (" + Integer.toString(dict.size()) + " entries) completed!");
	
	// read the line ###...
	line = fin.readLine();
    }
    
    public int size() {
	if (dict == null) {
	    return 0;
	} else {
	    return dict.size();
	}
    }
    
} // end of class Dictionary

