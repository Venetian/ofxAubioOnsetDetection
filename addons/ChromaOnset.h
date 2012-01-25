/*
 *  ChromaOnset.h
 *  fileLoaderAndOnsetDetection
 *
 *  Created by Andrew on 17/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */

#ifndef CHROMA_ONSET_H
#define  CHROMA_ONSET_H

#include "ofMain.h"
#include "chromaGram.h"
//#include "AubioPitch.h"
#include "FrameHolder.h"

class ChromaOnset {
	public:
	ChromaOnset();
	
	double millisTime;
	int frameTime;
	//chromagram chroma;
	bool chromaCalculated;
	int chromaSize;
	typedef std::vector<float> FloatVector;
	FloatVector chromaSamples;
	bool processFrame(float* frame, const int& length);
//	void calculateChroma(float* frame, const int& length);

	Chromagram cgram;
	
	FloatVector chromaValues;
	
	float holder[512];
	
	//also do pitch detection using this class
	//AubioPitch *aubioPitchDetector;
	bool aubioPitchFound;
	float aubioPitch;
	
	FrameHolder onsetFrame;
	
	
	void printInfo();
};
#endif
