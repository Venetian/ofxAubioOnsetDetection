/*
 *  ChromaOnset.cpp
 *  fileLoaderAndOnsetDetection
 *
 *  Created by Andrew on 17/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */

#include "ChromaOnset.h"


ChromaOnset::ChromaOnset(){
	frameTime = 0;
	millisTime = 0;
	chromaCalculated = false;
	chromaSize = 2048;
	cgram.initialise(512, chromaSize);
//	cgram = new Chromagram();
}

bool ChromaOnset::processFrame(float* frame, const int& length){
	bool newlyCalculated = false;
	if (chromaCalculated == false){
		
		cgram.processframe(frame);
		
		if (cgram.chromaready){
			
			for (int i = 0;i < 12;i++){
				chromaValues.push_back(cgram.rawChroma[i]);
				printf("[%i] %f\n", i, chromaValues[i]);
			}
			chromaCalculated = true;
			newlyCalculated = true;
			//this would do chord detection
			//			chord.C_Detect(chromoGramm.chroma,chromoGramm.chroma_low);
			//			rootChord[chromaIndex] = chord.root;
			
		}
		
	}
			 
	return newlyCalculated;
}


void ChromaOnset::printInfo(){

	printf("ONSET : frametime %i, millis %f\n", frameTime, millisTime);
}