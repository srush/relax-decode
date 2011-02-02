/*
    Copyright (C) 2006, Xuan-Hieu Phan
    
    Email:	hieuxuan@ecei.tohoku.ac.jp
		pxhieu@gmail.com
    URL:	http://www.hori.ecei.tohoku.ac.jp/~hieuxuan
    
    Graduate School of Information Sciences,
    Tohoku University
*/

package crf.tagger;

public class DoubleMatrix {
    public double[][] mtrx = null;
    public int rows = 0;
    public int cols = 0;
    
    public DoubleMatrix() {
    }
    
    public DoubleMatrix(int rows, int cols) {
	this.rows = rows;
	this.cols = cols;
	mtrx = new double[rows][cols];
    }
    
    public DoubleMatrix(int rows, int cols, double[][] mtrx) {
	this.rows = rows;
	this.cols = cols;	
	this.mtrx = new double[rows][cols];
    }
    
    public DoubleMatrix(DoubleMatrix dm) {
	rows = dm.rows;
	cols = dm.cols;	
	mtrx = new double[rows][cols];
	
	for (int i = 0; i < rows; i++) {
	    for (int j = 0; j < cols; j++) {
		mtrx[i][j] = dm.mtrx[i][j];
	    }
	}
    }
    
    public void assign(double val) {
	for (int i = 0; i < rows; i++) {
	    for (int j = 0; j < cols; j++) {
		mtrx[i][j] = val;
	    }
	}
    }
    
    public void assign(DoubleMatrix dm) {
	for (int i = 0; i < rows; i++) {
	    for (int j = 0; j < cols; j++) {
		mtrx[i][j] = dm.mtrx[i][j];
	    }
	}
    }    
    
} // end of class DoubleMatrix

