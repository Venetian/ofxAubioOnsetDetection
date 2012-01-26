/*
 *  AubioPitch.cpp
 *  fileLoaderAndOnsetDetection
 *
 *  Created by Andrew on 24/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */

#include "AubioPitch.h"

AubioPitch::AubioPitch(){
	bufsize   = 2048;
	hopsize   = bufsize / 2;
	pitch = 0.0;
	
	aubio_pitchdetection_type type_pitch = aubio_pitch_yinfft;
	aubio_pitchdetection_mode mode_pitch = aubio_pitchm_freq;
//	bufsize*4 below
	pitchDetect = new_aubio_pitchdetection(bufsize, hopsize, 1, 44100., type_pitch, mode_pitch);
	
	aubio_pitchdetection_set_yinthresh(pitchDetect, 0.7);
	
	vec = (fvec_t *)new_fvec(bufsize,1);//anr changed from hopsize
	pos = 0;
}

AubioPitch::~AubioPitch(){
	del_aubio_pitchdetection(pitchDetect);
	//delk fvec
	
}

bool AubioPitch::processFrame(float* frame, int size){//
	int j;
	bool pitchDetected = false;
	//smpl_t pitch;
	
	for (j=0;j<size;j++) {
		
		fvec_write_sample(vec, frame[j], 0, pos);

		if (pos == hopsize-1) {         
			// block loop /
			pitch = aubio_pitchdetection(pitchDetect, vec);
			pitchDetected = true;
//			printf("Pitch detected %f\n", pitch);
//			outlet_float(x->pitchOutlet, pitch);
			
			// end of block loop 
			pos = -1; // so it will be zero next j loop //
		}
		pos++;
		
	}
	return pitchDetected;
	
}

//this function is more useful here
//can process a frame - by converting it into an fvec_t type used by aubio
//but since only do this once, better for our purposes than the more iterative method above
//we just store the float frames and when needed, call this for our pitch calculation on the whole
//buffer - checking that length below is in fact bufsize

float AubioPitch::doPitchDetection(float* frame, const int& length){
	//fn to do pitch detection without all the buffering etc
	float newPitch = -1.0;
	if (length == bufsize){
		fvec_t* tmpVec;
		tmpVec = new_fvec(bufsize,1);
		for (int j =0;j < length;j++)
			fvec_write_sample(tmpVec, frame[j], 0, j);
		
		
		newPitch = aubio_pitchdetection(pitchDetect, tmpVec);
	//	printf("NEW PITCH FOUND %f\n", newPitch);
	}
	return newPitch;
}

/*This is the fvec_t defn from aubio:
 AUBIO f_vec
 struct _fvec_t {
 ba_uint_t length;   /**< length of buffer 
 ba_uint_t channels; /**< number of channels 
 smpl_t **data;   /**< data array of size [length] * [channels] 
 */
