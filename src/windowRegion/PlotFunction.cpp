/*
 *  PlotFunction.cpp
 *  BTrackPortAudioMultipleAnalyser
 *
 *  Created by Andrew on 14/11/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

#include "PlotFunction.h"
/*
void PlotFunction::drawVector(DoubleVector& energyVec, int minIndex, int maxIndex){

}
*/

PlotFunction :: PlotFunction(){
	fullScreen.resetToFullScreen();
}

PlotFunction :: ~PlotFunction(){

}


void PlotFunction::drawVector(DoubleVector& energyVec, int minIndex, int maxIndex, const WindowRegion& window, const double& maxNumberOfIndexPoints, const double& maxValue){

	float screenHeight = window.height;
	float screenWidth = window.width;  
	
	double  numberOfIndexPoints = min(maxNumberOfIndexPoints, (double) maxIndex - minIndex);
	double indicesPerStep = (maxIndex - minIndex) / numberOfIndexPoints;
	double pixelsPerStep = window.width / numberOfIndexPoints;
	
	int i, j;
	
	double heightScalar = window.height / (1.1*maxValue);
	
	int lastHeight = window.y + screenHeight - (energyVec[minIndex]*heightScalar);;
	int newHeight;
	int xPosition;
	int lastXposition = window.x;
	
	double exactIndex;
	for (exactIndex = minIndex; exactIndex < maxIndex; exactIndex += indicesPerStep){
		j = round(exactIndex);
		i = j - minIndex;
		
		if (j < energyVec.size()){
			xPosition = window.x + i*pixelsPerStep;
			newHeight =	window.y + screenHeight - (energyVec[j]*heightScalar);
			
			ofLine(lastXposition, lastHeight, xPosition, newHeight);
			
			lastHeight = newHeight;
			lastXposition = xPosition;
			
		}
	}
}


void PlotFunction::drawVector(DoubleVector& energyVec, int minIndex, int maxIndex, const WindowRegion& window, const double& maxResolution){
	
	
	minIndex = max(0, minIndex);
	maxIndex = min((int)energyVec.size()-1, maxIndex);
	int tmpTwenty = 20;
	double maximumValue = 1.1*getMaximum(energyVec, minIndex, maxIndex, tmpTwenty);
	drawVector(energyVec, minIndex, maxIndex, window, maxResolution, maximumValue);

}



void PlotFunction::drawArray(float* energyArray, int minIndex, int maxIndex, const WindowRegion& window, const double& maxResolution){
	
	minIndex = max(0, minIndex);
	//	maxIndex = min((int)sizeof(energyArray)-1, maxIndex);
	int tmpStepVal = 100;
	double maximumValue = 1;
	if (maxIndex > minIndex)
	maximumValue = getMaximumArray(energyArray, minIndex, maxIndex, tmpStepVal);
	
	drawArray(energyArray, minIndex, maxIndex, window, maxResolution, maximumValue);
}

void PlotFunction::drawArray(float* energyArray, int minIndex, int maxIndex, const WindowRegion& window, const double& maxNumberOfIndexPoints, const double& maximumValue){
	
	
	float screenHeight = window.height;
	float screenWidth = window.width;  
	
	double  numberOfIndexPoints= min(maxNumberOfIndexPoints, (double) maxIndex - minIndex);
	int indicesPerStep = (int) round((maxIndex - minIndex) / numberOfIndexPoints);
	indicesPerStep = max(1, indicesPerStep);
	double pixelsPerStep = window.width / numberOfIndexPoints;
	
	string pixelstring = ofToString(indicesPerStep);
	ofDrawBitmapString("max val "+pixelstring, 500,100);
	
	int i, j;
	
	double heightScalar = window.height / (1.2*maximumValue);
	
	int lastHeight = window.y + screenHeight - (energyArray[minIndex]*heightScalar);;
	int newHeight;
	int xPosition;
	int lastXposition = window.x;
	
	int exactIndex;
	for (exactIndex = minIndex - minIndex%indicesPerStep; exactIndex < maxIndex; exactIndex += indicesPerStep){
		j = exactIndex;// - exactIndex % indicesPerStep ;
		i = j - minIndex;

			xPosition = window.x + i*pixelsPerStep;
			if (j >= 0)
			newHeight =	window.y + screenHeight - (energyArray[j]*heightScalar);

			if (j < 16)
				cout << "j is " << j << ", i is " << i << endl;
		
			ofLine(lastXposition, lastHeight, xPosition, newHeight);
			//			   window.x+screenWidth*(i+1)/scrollWidth, window.y + screenHeight - (energyVec[j]*screenHeight/heightFactor));
			
			lastHeight = newHeight;
			lastXposition = xPosition;
			
		
	}
	 
	
}


double PlotFunction::getMaximum(DoubleVector& energyVec, const int& minIndex, const int& maxIndex, int& numberOfSteps){
	
	int step = max(1, (int)((maxIndex - minIndex) / (float) numberOfSteps));
	double maximumValue = energyVec[minIndex];
	int index = minIndex;
	while (index < maxIndex && index < energyVec.size()){
		index += step;
		if (energyVec[index] > maximumValue){
			maximumValue = energyVec[index];
		}
	}
	return maximumValue;
	
}



double PlotFunction::getMaximumArray(float* energyArray, const int& minIndex, const int& maxIndex, int& numberOfSteps){
	
	int step = max(1, (int)((maxIndex - minIndex) / (float) numberOfSteps));
	double maximumValue = energyArray[minIndex];
	int index = minIndex;
	while (index < maxIndex ){//&& index < sizeof(energyArray)
		index += step;
		if (energyArray[index] > maximumValue){
			maximumValue = energyArray[index];
		}
	}
	return maximumValue;
	
}

