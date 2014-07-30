/*
 *  FrameHolder.cpp
 *  fileLoaderAndOnsetDetection
 *
 *  Created by Andrew on 25/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */

#include "FrameHolder.h"

FrameHolder::FrameHolder(){
	sizeOfFrame = 0;
	sizeOfFvec = 0;
	frame.reserve(4096); 
//	buffersize = 8192;
	frameCounter = 0;
//	vec = (fvec_t *)new_fvec(buffersize,1);
	//printf("initialise frameholder\n");
}


void FrameHolder::deleteFrame(){
	frame.clear();
	//printf("clear frame\n");
}


void FrameHolder::addToFrame(float* newAudio, const int& length){
	for (int i = 0;i < length;i++){
	//	printf("new audio %i is %f\n", i, newAudio[i]);
		frame.push_back(newAudio[i]);
	}
	
	sizeOfFrame = frame.size();
	//printf("framesize %i\n", sizeOfFrame);
}

bool FrameHolder::sizeEquals(const int& length){
	if (sizeOfFrame == length)
		return true;
	else 
		return false;
}
/*
void FrameHolder::addToFvec(float* newAudio, const int& length){
	//my way to write to fvec - hope it works!
	for (int i =;i < length;i++)
		vec->data[i+sizeOfFvec] = newAudio[i];
	
	sizeOfFvec += length;
	
//	fvec_write_sample(vec, frame[j], 0, pos);
}

void FrameHolder::addToFvec(float* newAudio, const int& length){
	for (int i = 0;i < length;i++)
		frame.push_back(newAudio[i]);
	
	size = frame.size();
}
*/
