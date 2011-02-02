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

public class FeatureGen {
    List features = null;	// list of features
    Map fmap = null;		// feature map
    Maps maps = null;		// context predicate and label maps
    Dictionary dict = null;	// dictionary
        
    public FeatureGen(Maps maps, Dictionary dict) {
	this.maps = maps;
	this.dict = dict;	
    }
    
    // adding a feature
    public void addFeature(Feature f) {
	f.strId2IdxAdd(fmap);
	features.add(f);
    }
    
    public int numFeatures() {
	if (features == null) {
	    return 0;
	} else {
	    return features.size();
	}
    }
    
    public void readFeatures(BufferedReader fin) throws IOException {
	if (features != null) {
	    features.clear();
	} else {
	    features = new ArrayList();
	}
	
	if (fmap != null) {
	    fmap.clear(); 
	} else {
	    fmap = new HashMap();
	}
	
	if (eFeatures != null) {
	    eFeatures.clear();
	} else {
	    eFeatures = new ArrayList();
	}
	
	if (sFeatures != null) {
	    sFeatures.clear();
	} else {
	    sFeatures = new ArrayList();
	}
	
	String line;
	
	// get the number of features
	if ((line = fin.readLine()) == null) {
	    System.out.println("Unknown number of features");
	    return;
	}
	int numFeatures = Integer.parseInt(line);
	if (numFeatures <= 0) {
	    System.out.println("Invalid number of features");
	    return;
	}
	
	System.out.println("Reading features ...");
	
	// main loop for reading features
	for (int i = 0; i < numFeatures; i++) {
	    line = fin.readLine();
	    if (line == null) {
		// invalid feature line, ignore it
		continue;
	    }
	    
	    StringTokenizer strTok = new StringTokenizer(line, " ");
	    if (strTok.countTokens() != 3) {
		// invalid feature line, ignore it
		continue;
	    }
	    
	    // create a new feature by parsing the line
	    Feature f = new Feature(line, maps.cpStr2Int, maps.lbStr2Int);
	    
	    Integer fidx = (Integer)fmap.get(f.strId);
	    if (fidx == null) {
		// insert the feature into the feature map
		fmap.put(f.strId, new Integer(f.idx));
		features.add(f);
		
		if (f.ftype == Feature.EDGE_FEATURE1) {
		    eFeatures.add(f);
		}
	    }
	}
	
	System.out.println("Reading " + Integer.toString(features.size()) + " features completed!");
	
	// read the line ###...
	line = fin.readLine();
    }
    
    // start to scan features at a particular position in a data sequence
    public void startScanFeaturesAt(List seq, int pos) {
	startScanSFeaturesAt(seq, pos);
	startScanEFeatures();
    }
    
    public boolean hasNextFeature() {
	return (hasNextSFeature() || hasNextEFeature());
    }
    
    public Feature nextFeature() {
	Feature f = null;
    
	if (hasNextSFeature()) {
	    f = nextSFeature();
	} else if (hasNextEFeature()) {
	    f = nextEFeature();
	} else {
	    // do nothing
	}
	
	return f;
    }
    
    // start to scan state features
    List sFeatures = null;
    int sFeatureIdx = 0;
    
    void startScanSFeaturesAt(List seq, int pos) {	
	sFeatures.clear();
	sFeatureIdx = 0;
	
	Observation obsr = (Observation)seq.get(pos);
	    
	// scan over all context predicates
	for (int i = 0; i < obsr.cps.length; i++) {
	    Element elem = (Element)dict.dict.get(new Integer(obsr.cps[i]));
	    if (elem == null) {
		continue;
	    }
	    
	    if (!(elem.isScanned)) {
		// scan all labels for state feature
		Iterator it = elem.lbCntFidxes.keySet().iterator();
		while (it.hasNext()) {
		    Integer label = (Integer)it.next();
		    CountFeatureIdx cntFidx = (CountFeatureIdx)elem.lbCntFidxes.get(label);

		    if (cntFidx.fidx >= 0) {
			Feature sF = new Feature();
			sF.sFeature1Init(label.intValue(), obsr.cps[i]);
			sF.idx = cntFidx.fidx;
			
			elem.cpFeatures.add(sF);
		    }	    
		}		
		
		elem.isScanned = true;
	    }
	    
	    for (int j = 0; j < elem.cpFeatures.size(); j++) {
		sFeatures.add(elem.cpFeatures.get(j));
	    }
	}		
    }    
    
    boolean hasNextSFeature() {
	return (sFeatureIdx < sFeatures.size());
    }
    
    Feature nextSFeature() {
	Feature sF = (Feature)sFeatures.get(sFeatureIdx);
	sFeatureIdx++;
	return sF;
    }
    
    // start to scan edge features
    List eFeatures = null;
    int eFeatureIdx = 0;
    
    void startScanEFeatures() {
	eFeatureIdx = 0;
    }
    
    boolean hasNextEFeature() {
	return (eFeatureIdx < eFeatures.size());
    }
    
    Feature nextEFeature() {
	Feature eF = (Feature)eFeatures.get(eFeatureIdx);
	eFeatureIdx++;
	return eF;
    }
    
} // end of class FeatureGen

