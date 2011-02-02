/*
    Copyright (C) 2006, Xuan-Hieu Phan
    
    Email:	hieuxuan@ecei.tohoku.ac.jp
		pxhieu@gmail.com
    URL:	http://www.hori.ecei.tohoku.ac.jp/~hieuxuan
    
    Graduate School of Information Sciences,
    Tohoku University
*/

package crf.tagger;

public class DoubleVector {
    public double[] vect = null;
    public int len = 0;
    
    public DoubleVector() {
	len = 0;
	vect = null;
    }
    
    public DoubleVector(int len) {
	this.len = len;
	vect = new double[len];
    }
    
    public DoubleVector(int len, double[] vect) {
	this.len = len;
	this.vect = new double[len];
	for (int i = 0; i < len; i++) {
	    this.vect[i] = vect[i];
	}
    }
    
    public DoubleVector(DoubleVector dv) {
	len = dv.len;
	vect = new double[len];
	for (int i = 0; i < len; i++) {
	    vect[i] = dv.vect[i];
	}
    }
    
    public int size() {
	return len;
    }
    
    public void assign(double val) {
	for (int i = 0; i < len; i++) {
	    vect[i] = val;
	}
    }
    
    public void assign(DoubleVector dv) {
	for (int i = 0; i < len; i++) {
	    vect[i] = dv.vect[i];
	}
    }
    
    public double sum() {
	double res = 0.0;
	for (int i = 0; i < len; i++) {
	    res += vect[i];
	}
	return res;	
    }
    
    public void compMult(double val) {
	for (int i = 0; i < len; i++) {
	    vect[i] *= val;
	}
    }
    
    public void compMult(DoubleVector dv) {
	for (int i = 0; i < len; i++) {
	    vect[i] *= dv.vect[i];
	}
    }

} // end of class DoubleVector

