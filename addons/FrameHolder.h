/*
 *  FrameHolder.h
 *  fileLoaderAndOnsetDetection
 *
 *  Created by Andrew on 25/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */


#ifndef FRAME_HOLDER_H
#define  FRAME_HOLDER_H

#include "ofMain.h"
#include "chromaGram.h"
#include "AubioPitch.h"
#include "aubio.h"

class FrameHolder {
public:
	FrameHolder();
	
	int sizeOfFrame, sizeOfFvec;
	std::vector<float> frame;
	
	void deleteFrame();
	void addToFrame(float* newAudio, const int& length);
	
	bool sizeEquals(const int& length);
	
	//_fvec_t aubioFvec;
//	fvec_t *vec;
	
	/*
	 struct _fvec_t {
	 ba_uint_t length;   /**< length of buffer
	ba_uint_t channels; /**< number of channels 
	smpl_t **data;   /**< data array of size [length] * [channels] 
	*/
	
};
#endif
	