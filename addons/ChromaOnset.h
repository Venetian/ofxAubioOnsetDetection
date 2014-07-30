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

/*
//could switch to a struct to hold this?
 
 struct OnsetEvent{
	double millisTime;
	int frameTime;
	
	bool chromaCalculated;
	float chromaValues[12];
	
	bool aubioPitchFound;
	float aubioPitch;
};
*/

class ChromaOnset {
	public:
	ChromaOnset();
	
	double millisTime;
	int frameTime;
	//chromagram chroma;
	bool chromaCalculated;
	int chromaSize;
	int pitchFrameCounter;
	
	//move chromagram out of this class 
	//Chromagram cgram;
	
	typedef std::vector<float> FloatVector;
	FloatVector chromaValues;
	FloatVector quantisedChromaValues;
	
	double getQuantisedDistance(ChromaOnset& secondChromaOnset);
	double getChromaQuantisedDistance(float* quantisedChromagramTwo);
	//also do pitch detection using this class
	//AubioPitch *aubioPitchDetector;
	bool aubioPitchFound;
	float aubioPitch;
	
	bool matched;//just for drawing

	
	bool processFrame(float* frame, const int& length);
//	void calculateChroma(float* frame, const int& length);

	double timeDistanceMillis(const ChromaOnset& secondChromaOnset);
	double pitchDistance(const ChromaOnset& secondChromaOnset);
	double chromaDotProductDistance(const ChromaOnset& secondChromaOnset);


	Chromagram* cgramPtr;
	void deleteChromagram();
	
	FrameHolder onsetFrame;
	
	int onsetIndex;//in terms of our onset detection fn - ofxAubioOnsetDetector
	void printInfo();
};
#endif
