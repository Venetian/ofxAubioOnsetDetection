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

#define NUM_DETECTION_SAMPLES 640
#define TEXT_HEIGHT 16

#include "ofMain.h"
#include "AubioOnsetDetector.h"
#include "OnsetDetectionFunction.h"

//this builds on aubioOsetDetector class
//but that doesn't store any values
//for drawing, perhpas we need to


class ofxAubioOnsetDetection{
public:
	ofxAubioOnsetDetection();
	~ofxAubioOnsetDetection();
	AubioOnsetDetector	*onsetDetector;
	//OnsetDetectionFunction *qmOnsetDetector;
	
	double dfSample;
	
	void processFrame(double* frame, const int& n);
	
	//switch between different onset methods
	void aubioOnsetDetect_energy();
	void aubioOnsetDetect_complex();
	void aubioOnsetDetect_kl();
	void aubioOnsetDetect_hfc();
	void aubioOnsetDetect_specdiff();
	void aubioOnsetDetect_phase();//this is pretty bad I think
	void aubioOnsetDetect_mkl();

	void drawOnsetDetection();
	
	int onsetIndex;
	
	float			onsetFunction[NUM_DETECTION_SAMPLES];
	bool			aubioOnsetRecorded[NUM_DETECTION_SAMPLES];
	
	float			rawOnsetFunction[NUM_DETECTION_SAMPLES];		
	bool			highSlopeOnsetRecorded[NUM_DETECTION_SAMPLES];

	float			medianOnsetFunction[NUM_DETECTION_SAMPLES];		
	bool			medianOnsetRecorded[NUM_DETECTION_SAMPLES];
	
	float			highSlopeOnsetFunction[NUM_DETECTION_SAMPLES];
	
	float			aubioLongTermAverage[NUM_DETECTION_SAMPLES];
	
	float			maximumDetectionFunction;
	float			minimumDetectionFunction;		
	
	float			maxValue;
	
	int				amplitudeNumber;
	bool			onsetFound;

};


#endif