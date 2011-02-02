/*
    Copyright (C) 2006, Xuan-Hieu Phan
    
    Email:	hieuxuan@ecei.tohoku.ac.jp
		pxhieu@gmail.com
    URL:	http://www.hori.ecei.tohoku.ac.jp/~hieuxuan
    
    Graduate School of Information Sciences,
    Tohoku University
*/

package crf.tagger;

public class Mathlib {
    public static void mult(int size, DoubleVector x, DoubleMatrix A, 
		DoubleVector y, boolean isTransposed) {
	// isTransposed = false:	x = A * y
	// isTransposed = true:		x^t = y^t * A^t
	
	int i, j;
	
	if (!isTransposed) {
	    // for beta
	    // x = A * y
	    for (i = 0; i < size; i++) {	
		x.vect[i] = 0;
		for (j = 0; j < size; j++) {
		    x.vect[i] += A.mtrx[i][j] * y.vect[j];
		}
	    }
	    
	} else {
	    // for alpha
	    // x^t = y^t * A^t
	    for (i = 0; i < size; i++) {
		x.vect[i] = 0;
		for (j = 0; j < size; j++) {
		    x.vect[i] += y.vect[j] * A.mtrx[j][i];
		}
	    }	    
	}	
    }

} // end of class Mathlib

