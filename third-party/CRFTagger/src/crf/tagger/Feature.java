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

public class Feature {
    // feature types; second-order Markov is not supported
    static public final int UNKNOWN_FEATURE = 0;
    static public final int EDGE_FEATURE1 = 1;
    // static public final int EDGE_FEATURE2 = 2;
    static public final int STAT_FEATURE1 = 3;
    // static public final int STAT_FEATURE2 = 4;
    
    int ftype = UNKNOWN_FEATURE; // feature type
    int idx = -1;	// feature index
    String strId = "";	// string identifier
    int y = -1;		// current label
    int yp = -1;	// previous label
    int cp = -1;	// context predicate
    float val = 1;	// feature value
    double wgt = 0.0;	// feature weight
    
    // edge feature (type 1) initialization
    public void eFeature1Init(int y, int yp) {
	ftype = EDGE_FEATURE1;
	idx = -1;
	this.y = y;
	this.yp = yp;
	val = 1;
	wgt = 0.0;	
	strId = "e1_" + Integer.toString(y) + "_" + Integer.toString(yp);
    }
    
    public void eFeature1Init(int y, int yp, Map fmap) {
	eFeature1Init(y, yp);
	strId2IdxAdd(fmap);
    }    
    
    // state feature (type 1) initialization
    public void sFeature1Init(int y, int cp) {
	ftype = STAT_FEATURE1;
	idx = -1;
	this.y = y;
	this.cp = cp;
	val = 1;
	wgt = 0.0;
	strId = "s1_" + Integer.toString(y) + "_" + Integer.toString(cp);
    }
    
    public void sFeature1Init(int y, int yp, Map fmap) {
	sFeature1Init(y, cp);
	strId2IdxAdd(fmap);
    }
    
    public Feature() {
    }
    
    // feature constructor that parses an input line
    public Feature(String line, Map cpStr2Int, Map lbStr2Int) {

	StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
	int len = strTok.countTokens();
	
	String strIdStr = strTok.nextToken();
	int idx = Integer.parseInt(strTok.nextToken());
	float val = 1;
	double wgt = Double.parseDouble(strTok.nextToken());
	
	// parsing string identifier
	StringTokenizer strIdTok = new StringTokenizer(strIdStr, "_");
	String prefix = strIdTok.nextToken();
	
	if (prefix.compareToIgnoreCase("e1") == 0) {
	    // edge feature type 1
	    Integer yInt = (Integer)lbStr2Int.get(strIdTok.nextToken());
	    Integer ypInt = (Integer)lbStr2Int.get(strIdTok.nextToken());
	    
	    if (yInt != null && ypInt != null) {
		eFeature1Init(yInt.intValue(), ypInt.intValue());
	    }
	
	} else if (prefix.compareToIgnoreCase("s1") == 0) {
	    // state feature type 1
	    Integer yInt = (Integer)lbStr2Int.get(strIdTok.nextToken());
	    Integer cpInt = (Integer)cpStr2Int.get(strIdTok.nextToken());
	    
	    if (yInt != null && cpInt != null) {
		sFeature1Init(yInt.intValue(), cpInt.intValue());
	    }
			    
	} 
	
	this.idx = idx;
	this.val = val;
	this.wgt = wgt;	
    }
    
    // feature constructor that parses an input line (adding to the feature map)
    public Feature(String line, Map cpStr2Int, Map lbStr2Int, Map fmap) {

	StringTokenizer strTok = new StringTokenizer(line, " \t\r\n");
	int len = strTok.countTokens();
	
	String strIdStr = strTok.nextToken();
	int idx = Integer.parseInt(strTok.nextToken());
	float val = 1;
	double wgt = Double.parseDouble(strTok.nextToken());
	
	// parsing string identifier
	StringTokenizer strIdTok = new StringTokenizer(strIdStr, "_");
	String prefix = strIdTok.nextToken();
	
	if (prefix.compareToIgnoreCase("e1") == 0) {
	    // edge feature type 1
	    Integer yInt = (Integer)lbStr2Int.get(strIdTok.nextToken());
	    Integer ypInt = (Integer)lbStr2Int.get(strIdTok.nextToken());
	    
	    if (yInt != null && ypInt != null) {
		eFeature1Init(yInt.intValue(), ypInt.intValue());
	    }
	
	} else if (prefix.compareToIgnoreCase("s1") == 0) {
	    // state feature type 1
	    Integer yInt = (Integer)lbStr2Int.get(strIdTok.nextToken());
	    Integer cpInt = (Integer)cpStr2Int.get(strIdTok.nextToken());
	    
	    if (yInt != null && cpInt != null) {
		sFeature1Init(yInt.intValue(), cpInt.intValue());
	    }
	    
	}
	
	this.idx = idx;
	this.val = val;
	this.wgt = wgt;	
    
	strId2IdxAdd(fmap);
    }
    
    // mapping from string identifier to feature index
    public int strId2Idx(Map fmap) {
	Integer idxInt = (Integer)fmap.get(strId);
	if (idxInt != null) {
	    this.idx = idxInt.intValue();
	}
	
	return this.idx;
    }
    
    // mapping from string identifier to feature index (adding feature to the map
    // if the mapping does not exist
    public int strId2IdxAdd(Map fmap) {
	strId2Idx(fmap);
	
	if (idx < 0) {
	    idx = fmap.size();
	    fmap.put(strId, new Integer(idx));
	}    
	
	return idx;
    }
    
    // return the feature index
    public int index() {
	return idx;
    }
    
    // return the feature index (lookup the map)
    public int index(Map fmap) {
	return strId2Idx(fmap);
    }
    
    // convert feature to string "<identifier> <index> <weight>"
    public String toString(Map cpInt2Str, Map lbInt2Str) {
	String str = "";
	
	if (ftype == EDGE_FEATURE1) {
	    // edge feature type 1
	    str = "e1_";
	    
	    String yStr = (String)lbInt2Str.get(new Integer(y));
	    if (yStr != null) {
		str += yStr + "_";
	    }
	    
	    String  ypStr = (String)lbInt2Str.get(new Integer(yp));
	    if (ypStr != null) {
		str += ypStr;
	    }	    
	
	} else if (ftype == STAT_FEATURE1) {
	    // state feature type 1
	    str = "s1_";
	    
	    String yStr = (String)lbInt2Str.get(new Integer(y));
	    if (yStr != null) {
		str += yStr + "_";
	    }
	    
	    String cpStr = (String)cpInt2Str.get(new Integer(cp));
	    if (cpStr != null) {
		str += cpStr;
	    }
	    
	} 
	
	str += " " + Integer.toString(idx) + " " + Double.toString(wgt);
	return str;
    }
        
} // end of class Feature

