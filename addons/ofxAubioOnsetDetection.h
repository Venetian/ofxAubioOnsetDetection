/*
 *  ofxAubioOnsetDetection.h
 *  ofxOnsetDetection
 *
 *  Created by Andrew Robertson on 24/11/2011.
 *  Copyright 2011 Centre for Digital Music, QMUL. All rights reserved.
 *
 */


#ifndef OFX_AUBDIO_ONSET_DETECTION_H_
#define OFX_AUBIO_ONSET_DETECTION_H_

#pragma once

#define NUM_DETECTION_SAMPLES 24000
#define TEXT_HEIGHT 16

#include "ofMain.h"
#include "AubioOnsetDetector.h"
#include "ChromaOnset.h"
#include "ofxWindowRegion.h"
#include "AubioPitch.h"
#include "Chromagram.h"

//#include "OnsetDetectionFunction.h"

//this builds on aubioOsetDetector class
//but that doesn't store any values
//for drawing, perhpas we need to


struct DrawOnsetParameters{
	float width;//= screenWidth / (float) amplitudeNumber;	
	float maximumValue;// = onsetDetector->maximumDetectionValue;
	float minimumValue;// = 0;//minimumDetectionFunction ;
	float difference;// = maximumValue - minimumValue;
	float scale_factor;// = screenHeight/ difference;
	int windowStartFrame;
};

class ofxAubioOnsetDetection{
public:
	ofxAubioOnsetDetection();
	~ofxAubioOnsetDetection();
	AubioOnsetDetector	onsetDetector;
	//OnsetDetectionFunction *qmOnsetDetector;
	
	void reset();
	void initialiseValues();
	double dfSample;
	
	void processFrame(double* frame, const int& n);
	void processFrame(float* frame, const int& n);
	
	void printOnsetList();//print the vector of chromaOnset times - onsets with timing and chroma info
	
	//switch between different onset methods
	void aubioOnsetDetect_energy();
	void aubioOnsetDetect_complex();
	void aubioOnsetDetect_kl();
	void aubioOnsetDetect_hfc();
	void aubioOnsetDetect_specdiff();
	void aubioOnsetDetect_phase();//this is pretty bad I think
	void aubioOnsetDetect_mkl();

	void drawOnsetDetection();
	void drawOnsetDetectionScrolling();//float of how far thru file - e.g 0.6 60% through
	void drawScrollLine(const int& startIndex, const ofxWindowRegion& screenRegion);
	
	void drawOnsetDetection(int startIndex, int endIndex);
	void drawOnsetDetection(int startIndex, int endIndex, const ofxWindowRegion& screenRegion);//overloaded
	int onsetIndex, frameCountIndex;
	
	
	float			onsetFunction[NUM_DETECTION_SAMPLES];
	bool			aubioOnsetRecorded[NUM_DETECTION_SAMPLES];
	
	float			rawOnsetFunction[NUM_DETECTION_SAMPLES];		
	bool			highSlopeOnsetRecorded[NUM_DETECTION_SAMPLES];//true/false for onset

	float			medianOnsetFunction[NUM_DETECTION_SAMPLES];		
	bool			medianOnsetRecorded[NUM_DETECTION_SAMPLES];//true/false for onset
	
	float			highSlopeOnsetFunction[NUM_DETECTION_SAMPLES];
	
	float			aubioLongTermAverage[NUM_DETECTION_SAMPLES];
	
	float			maximumDetectionFunction;
	float			minimumDetectionFunction;		
	
	float			maxValue;
	
	int				amplitudeNumber;
	bool			onsetFound;
	
	typedef std::vector<double> DoubleVector;
	DoubleVector highSlopeOnsetsFrames;
	DoubleVector highSlopeOnsetsMillis;
	double framesToMillis(const double& frameCount);
	double playPosition, playPositionFrames;//play position is between 0 and 1 of the file 
	
	typedef std::vector<ChromaOnset> ChromaOnsetVector;
	ChromaOnsetVector chromaOnsets;
	
	int trackType;
	
	void printChromaInfo();
	
	void checkChromaAndPitch(float* tmpFrame, const int& n);
	
	void drawChromaOnsetData(const int& startIndex, const int& endIndex);
	void drawChromaOnsetData(const int& startIndex, const int& endIndex, const ofxWindowRegion& screenRegion);
	
	void drawOnsetStripes(int chromaIndex, const int& frameEndIndex, const ofxWindowRegion& screenRegion);
	void drawChromaStripes(int chromaIndex, const int& frameEndIndex, const ofxWindowRegion& screenRegion);
	void drawPitchLines(int chromaIndex, const int& frameEndIndex, const ofxWindowRegion& screenRegion);
	
	
	DrawOnsetParameters drawParams ;
	void setDrawParams();
	void setDrawParams(const ofxWindowRegion& screenRegion);
	void drawOutlineAndSetParams(const ofxWindowRegion& screenRegion);
	
	ChromaOnset* chromaOnsetPtr;
	
	AubioPitch pitchDetector;
	//AubioPitch pitchDetectorTwo;
	float	maximumAubioPitch, minimumAubioPitch;
	
	//basic screen stuff
	float screenWidth;//= ofGetWidth();
	float screenHeight;// = ofGetHeight();
	void windowResized(const int& w, const int& h);
	
	ofxWindowRegion window;
	ofxWindowRegion fullScreen;
	
};


#endif