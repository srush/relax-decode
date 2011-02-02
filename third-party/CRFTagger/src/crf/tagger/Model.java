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

public class Model {
    public Option taggerOpt = null;
    public Maps taggerMaps = null;
    public Dictionary taggerDict = null;
    public FeatureGen taggerFGen = null;
    public Viterbi taggerVtb = null;
    
    // feature weight
    double[] lambda = null;
    
    public Model() {
    }
    
    public Model(Option taggerOpt, Maps taggerMaps, Dictionary taggerDict, 
		FeatureGen taggerFGen, Viterbi taggerVtb) {
	this.taggerOpt = taggerOpt;
	this.taggerMaps = taggerMaps;
	this.taggerDict = taggerDict;
	this.taggerFGen = taggerFGen;
	this.taggerVtb = taggerVtb;	
    }
    
    // load the model
    public boolean init() {
	// open model file to load model here ... complete later
	BufferedReader fin = null;
	String modelFile = taggerOpt.modelDir + File.separator + taggerOpt.modelFile;
	
	try {
	    fin = new BufferedReader(new FileReader(modelFile));
	    
	    // read context predicate map and label map
	    taggerMaps.readCpMaps(fin);
	    
	    System.gc();

	    taggerMaps.readLbMaps(fin);
	    
	    System.gc();
	    
	    // read dictionary 
	    taggerDict.readDict(fin);
	    
	    System.gc();
	    
	    // read features
	    taggerFGen.readFeatures(fin);
	    
	    System.gc();
	    
	    // close model file
	    fin.close();
	    
	} catch (IOException e) {
	    System.out.println("Couldn't open model file: " + modelFile);
	    System.out.println(e.toString());
	    
	    return false;	    
	}
	
	// update feature weights
	if (lambda == null) {
	    int numFeatures = taggerFGen.numFeatures();
	    lambda = new double[numFeatures];
	    for (int i = 0; i < numFeatures; i++) {
		Feature f = (Feature)taggerFGen.features.get(i);
		lambda[f.idx] = f.wgt;
	    }
	}
    
	// call init method of Viterbi object
	if (taggerVtb != null) {
	    taggerVtb.init(this);
	}
	
	return true;
    }
    
    public void inference(List seq) {
	taggerVtb.viterbiInference(seq);
    }
    
    public void inferenceAll(List data) {
	System.out.println("Starting inference ...");

	long start, stop, elapsed;
	start = System.currentTimeMillis();
	
	for (int i = 0; i < data.size(); i++) {
	    System.out.println("sequence " + Integer.toString(i + 1));
	    List seq = (List)data.get(i);
	    inference(seq);	    
	}
	
	stop = System.currentTimeMillis();
	elapsed = stop - start;
	
	System.out.println("Inference " + Integer.toString(data.size()) + " sequences completed!");
	System.out.println("Inference time: " + Double.toString((double)elapsed / 1000) + " seconds");
    }
    
} // end of class Model

