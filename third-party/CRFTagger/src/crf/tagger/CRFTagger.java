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

public class CRFTagger {
    public static void main(String[] args) {
	displayCopyright();
	
	if (!checkArgs(args)) {
	    displayHelp();
	    return;
	}
	
	String modelDir = args[1];
	boolean isInputFile = true;
	if (args[2].compareToIgnoreCase("-inputfile") != 0) {
	    isInputFile = false;
	}

        boolean genTrain = false;
	//if (args[3].compareToIgnoreCase("-train") != 0) {
        //genTrain = true;
	//}

        String knownWordsFile = "";
        if (args[4].compareToIgnoreCase("-knownwords") == 0) {
            knownWordsFile = args[5];
	}

	String inputFile = "";
	String inputDir = "";
	if (isInputFile) {
	    inputFile = args[3];
	} else {
	    inputDir = args[3];
	}
	
	Option taggerOpt = new Option(modelDir);
	if (!taggerOpt.readOptions()) {
	    return;
	}
	
	Maps taggerMaps = new Maps();
	Dictionary taggerDict = new Dictionary();
	FeatureGen taggerFGen = new FeatureGen(taggerMaps, taggerDict);
	Viterbi taggerVtb = new Viterbi();
        System.err.println("Known words: " +knownWordsFile);
	Model taggerModel = new Model(taggerOpt, taggerMaps, taggerDict, taggerFGen, taggerVtb);
	if (!taggerModel.init()) {
	    System.out.println("Couldn't load the model");
	    System.out.println("Check the <model directory> and the <model file> again");
	    return;
	}
	
	TaggingData taggerData = new TaggingData();

        if (genTrain) {
            //taggerData.readData(inputFile);
	    //taggerData.cpGen(taggerMaps.cpStr2Int);
            taggerData.writeTaggedData(taggerMaps.cpStr2Int, taggerMaps.lbInt2Str,inputFile, inputFile + ".tagged");
            return;
        }
	
	if (isInputFile) {
            
            taggerData.readKnownWords(knownWordsFile);
	    taggerData.readData(inputFile);
	    taggerData.cpGen(taggerMaps.cpStr2Int);

	    // inference
	    taggerModel.inferenceAll(taggerData.data);
	    
	    taggerData.writeData(taggerMaps.lbInt2Str, inputFile + ".pos");
	}
	
	if (!isInputFile) {
	    if (inputDir.endsWith(File.separator)) {
		inputDir = inputDir.substring(0, inputDir.length() - 1);
	    }
	    
	    File dir = new File(inputDir);	    
	    String[] children = dir.list();
	    
	    for (int i = 0; i < children.length; i++) {
		String filename = inputDir + File.separator + children[i];
		if ((new File(filename)).isDirectory()) {
		    continue;
		}
		
		taggerData.readData(filename);
		taggerData.cpGen(taggerMaps.cpStr2Int);
		
		// inference
		taggerModel.inferenceAll(taggerData.data);
		
		taggerData.writeData(taggerMaps.lbInt2Str, filename + ".pos");
	    }
	}
	
    } // end of the main method
    
    public static boolean checkArgs(String[] args) {
	// case 1: CRFTagger -modeldir <model directory> -inputfile <input data file>
	// case 2: CRFTagger -modeldir <model directory> -inputdir <input data directory>
	
	if (args.length < 4) {	    
	    return false;
	}
	
	if (args[0].compareToIgnoreCase("-modeldir") != 0) {
	    return false;
	}
	
	if (!(args[2].compareToIgnoreCase("-inputfile") == 0 ||
		    args[2].compareToIgnoreCase("-inputdir") == 0)) {
	    return false;
	}
	
	return true;
    }   
    
    public static void displayCopyright() {
	System.out.println("English CRFTagger:");
	System.out.println("\tTrain on sections 01-24 of Wall Street Journal corpus");
	System.out.println("\tusing first-order Markov Conditional Random Fields");
	System.out.println("\ttesting on section 00 with the highest accuracy of 97.00%");
	System.out.println("Copyright (C) by Xuan-Hieu Phan");
	System.out.println("Graduate School of Information Sciences, Tohoku University");
	System.out.println("Email: hieuxuan@ecei.tohoku.ac.jp");
	System.out.println();
    }
    
    public static void displayHelp() {
	System.out.println("Usage:");
	System.out.println("\tCase 1: CRFTagger -modeldir <model directory> -inputfile <input data file>");
	System.out.println("\tCase 2: CRFTagger -modeldir <model directory> -inputdir <input data directory>");
	System.out.println("Where:");
	System.out.println("\t<model directory> is the directory contain the model and option files");
	System.out.println("\t<input data file> is the file containing input sentences that need to");
	System.out.println("\tbe tagged (each sentence on a line)");
	System.out.println("\t<input data directory> is the directory containing multiple input data files");
	System.out.println();
    }
     
} // end of class CRFTagger

