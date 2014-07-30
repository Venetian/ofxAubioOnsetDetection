/*
 *  ChromaOnset.cpp
 *  fileLoaderAndOnsetDetection
 *
 *  Created by Andrew on 17/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */

#include "ChromaOnset.h"

#include "assert.h"

//stores the information about our onset event
//when it happened
//what the chromagram was immeditaely after 
//what the pitch was using AubioPitch class

//FrameHolder holds the audio samples needed to do the harmonic analysis after the fact

ChromaOnset::ChromaOnset(){
	frameTime = 0;
	millisTime = 0;
	chromaCalculated = false;
	chromaSize = 2048;
	
	cgramPtr = new Chromagram();
	cgramPtr->initialise(512, chromaSize);
	
	onsetIndex = 0;

	matched = false;
	//we also store an aubio pitch result - using a Frameholder to hold the frames we need to calculate this on
	aubioPitch = 0;
	aubioPitchFound = false;
	
	pitchFrameCounter = 0;//counts til time to do pitch detection
	
	
//	aubioPitchDetector = new AubioPitch();

}

void ChromaOnset::deleteChromagram(){
	delete cgramPtr;
	//printf("chromagram deleted\n");
}


bool ChromaOnset::processFrame(float* frame, const int& length){
	bool newlyCalculated = false;
	if (chromaCalculated == false){
		
		cgramPtr->processframe(frame);
		
		if (cgramPtr->chromaready){
			
			float tmpMax = max(cgramPtr->maximumChromaValue, 1.0f);

			for (int i = 0;i < 12;i++){
				chromaValues.push_back(cgramPtr->rawChroma[i]/tmpMax);
				quantisedChromaValues.push_back(cgramPtr->quantisedChromagram[i]);
			//	printf("chroma [%i] %f max %f quantised %i\n", i, cgramPtr->rawChroma[i], cgramPtr->chroma[i], cgramPtr->quantisedChromagram[i] );
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

double ChromaOnset::timeDistanceMillis(const ChromaOnset& secondChromaOnset){
	
	return (abs(secondChromaOnset.millisTime - millisTime));
}

double ChromaOnset::pitchDistance(const ChromaOnset& secondChromaOnset){

	return (abs(secondChromaOnset.aubioPitch - aubioPitch));
}

double ChromaOnset::getQuantisedDistance(ChromaOnset& secondChromaOnset){
	
	return getChromaQuantisedDistance(&secondChromaOnset.quantisedChromaValues[0]);
	
}

double ChromaOnset::getChromaQuantisedDistance(float* quantisedChromagramTwo){
	if (quantisedChromaValues.size() > 0){
		double cosineDistance = 0;
		double quantLengthOne = 0;
		double quantLengthTwo = 0; 


		for (int i = 0;i < 12;i++){	
			double testOne = quantisedChromaValues[i];
			double testVarTwo = quantisedChromagramTwo[i];
			
			cosineDistance += quantisedChromaValues[i] * quantisedChromagramTwo[i];
			
			quantLengthOne += quantisedChromaValues[i] * quantisedChromaValues[i];
			quantLengthTwo += quantisedChromagramTwo[i] * quantisedChromagramTwo[i];
			
		}
		
		if (quantLengthOne > 0 && quantLengthTwo > 0)
			cosineDistance /= sqrt(quantLengthOne * quantLengthTwo);
		else 
			cosineDistance = 0;
		
		return cosineDistance;
	} else {
		printf("CHROMA PROBLEM!!\n");
		return 0;
	}

	
}

double ChromaOnset::chromaDotProductDistance(const ChromaOnset& secondChromaOnset){
	double dotProductSum = 0;
	double chromaOneSum = 0;
	double chromaTwoSum = 0;
	if (chromaValues.size() == 12 && secondChromaOnset.chromaValues.size() == 12){
		for (int i = 0;i < 12;i++){
			dotProductSum += chromaValues[i]*secondChromaOnset.chromaValues[i];
			chromaOneSum += chromaValues[i]*chromaValues[i];
			chromaTwoSum += secondChromaOnset.chromaValues[i]*secondChromaOnset.chromaValues[i];
		}
		chromaOneSum = sqrt(chromaOneSum);
		chromaTwoSum = sqrt(chromaTwoSum);
		if (chromaOneSum > 0 && chromaTwoSum > 0)
			dotProductSum /= (chromaOneSum*chromaTwoSum);
		else
			dotProductSum = 0;
	}
	return (dotProductSum);
}

void ChromaOnset::printInfo(){

	printf("ONSET : frametime %i, millis %f pitch %f\n", frameTime, millisTime, aubioPitch);
	printf("chroma: ");
	
	for(int i = 0;i < chromaValues.size();i++)
			printf("[%i] %1.3f, ", i, chromaValues[i]);
	
	printf(".\n");
	
}