/*
 *  AubioPitch.h
 *  fileLoaderAndOnsetDetection
 *
 *  Created by Andrew on 24/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */


#ifndef AUBIO_PITCH_H
#define  AUBIO_PITCH_H

#include "ofMain.h"
#include "aubio.h"

class AubioPitch {
public:
	AubioPitch();
	~AubioPitch();
	
	bool processFrame(float* frame, int size);
	
	int bufsize, hopsize;
	
	aubio_pitchdetection_type type_pitch;
	aubio_pitchdetection_mode mode_pitch;


	float threshold;	
	
	smpl_t pitch;
	
	aubio_pitchdetection_t *pitchDetect;
	fvec_t *vec;
	int pos;
	
	float doPitchDetection(float* frame, const int& length);
	
	float getPitch();
	void addToBuffer(float* tmpFrame, const int& n);
	
	//float getPitchDetectedFromBuffer(float* frame, const int& length);
	
};
#endif