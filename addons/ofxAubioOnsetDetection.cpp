/*
 *  ofxAubioOnsetDetection.cpp
 *  ofxOnsetDetection
 *
 *  Created by Andrew on 24/11/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

#include "ofxAubioOnsetDetection.h"


ofxAubioOnsetDetection::ofxAubioOnsetDetection(){
	onsetDetector = new AubioOnsetDetector();
	onsetDetector->initialise();
	
	amplitudeNumber = 256;//number of amplitudes shown on screen


//	useMedianOnsetDetection = true;
	onsetIndex = 0;
	frameCountIndex = 0;
	playPositionFrames = 0;
	playPosition = 0;
	
	maximumAubioPitch = 1600;
	
	screenWidth = ofGetWidth();
	screenHeight = ofGetHeight();
}
	



ofxAubioOnsetDetection::~ofxAubioOnsetDetection(){
	delete onsetDetector;
//	delete qmOnsetDetector;
}

void ofxAubioOnsetDetection::reset(){
	onsetIndex = 0;
	frameCountIndex = 0;
	highSlopeOnsetsFrames.clear();
	highSlopeOnsetsMillis.clear();//The high slope onsets are the ones stored as chromaOnsets
	//(i.e. have associated chroma and pitch to them)
	chromaOnsets.clear();
	printf("Resetting the ofxAubioOnsetDetector\n");
}


void ofxAubioOnsetDetection::processFrame(double* frame, const int& n){
	//bool onsetFound = false;
	// aubio onset detector then processes current frame - returns bool true when new detection is output
	//if buffer full and new result is processed (buffer is 1024 with hopsize 512 - can be set to other values)
	
	//for other class OnsetDetectionFunction : dfSample = qmOnsetDetector->getDFsample(frame);
	
	if (onsetDetector->processframe(frame, n)){

		rawOnsetFunction[onsetIndex] = onsetDetector->rawDetectionValue;
		medianOnsetFunction[onsetIndex] = onsetDetector->medianDetectionValue;
//		aubioOnsetFunction[onsetIndex] = ??;
		highSlopeOnsetFunction[onsetIndex] = onsetDetector->bestSlopeValue;
		aubioLongTermAverage[onsetIndex] = onsetDetector->aubioLongTermAverage;
		
		//	outlet_float(x->detectionFunctionOutlet, x->onsetDetector->peakPickedDetectionValue);
		
		if (onsetDetector->aubioOnsetFound){
			aubioOnsetRecorded[onsetIndex] = true;
		}
		else{
			aubioOnsetRecorded[onsetIndex] = true;
		}
		
		
		if (onsetDetector->anrMedianProcessedOnsetFound){
			onsetFound = true;
			medianOnsetRecorded[onsetIndex] = true;
		}else{
			onsetFound = false;
			medianOnsetRecorded[onsetIndex] = false;
		}
		
		if (onsetDetector->anrBestSlopeOnset){
			highSlopeOnsetRecorded[onsetIndex] = true;
			highSlopeOnsetsFrames.push_back(frameCountIndex);
			highSlopeOnsetsMillis.push_back(framesToMillis(frameCountIndex));
			
			printf("onset frame %i is time %f \n", frameCountIndex, framesToMillis(frameCountIndex));
			ChromaOnset c;
			c.frameTime = frameCountIndex;
			c.millisTime = framesToMillis(frameCountIndex);
			chromaOnsets.push_back(c);

		}
		else{
			highSlopeOnsetRecorded[onsetIndex] = false;
		}
		
		frameCountIndex++;//this one never loops
		onsetIndex++;
		if (onsetIndex == NUM_DETECTION_SAMPLES)
			onsetIndex = 0;//this loops round
		
	}//end if new aubio onset detection result
	
	float* tmpFrame;
	for (int i = 0;i < n;i++)
		tmpFrame[i] = frame[i];
	
	checkChromaAndPitch(tmpFrame, n);
	
	
 }

void ofxAubioOnsetDetection::checkChromaAndPitch(float* tmpFrame, const int& n){
	//process frames into each onset's chromagram analyser
	for (int i = 0;i < chromaOnsets.size();i++){
		
		//aubio pitch using yin
		if (!chromaOnsets[i].aubioPitchFound ){//not yet done
			chromaOnsets[i].onsetFrame.addToFrame(tmpFrame, n);//add to frame in the chromaOnset class
			printf("adding to frame for onset %i at frametime %i\n", i, frameCountIndex);
			if (chromaOnsets[i].onsetFrame.sizeOfFrame >= pitchDetector.bufsize){//enough to calculate yin with using aubio pitch detection
				chromaOnsets[i].aubioPitch = pitchDetector.doPitchDetection(&chromaOnsets[i].onsetFrame.frame[0], pitchDetector.bufsize);
				chromaOnsets[i].aubioPitchFound = true;
				printf("Aubio Pitch recieved for onset %i is %f\n", i, chromaOnsets[i].aubioPitch);
			}
				
		}
		
		
		//chroma
		if (chromaOnsets[i].processFrame(tmpFrame, n)){
			printf("onset %i (frametime %i) chroma is newly calculated at frame %i\n", i, chromaOnsets[i].frameTime, frameCountIndex);
			chromaOnsets[i].printInfo();
		}
		
	}//end for all onsets
}

void ofxAubioOnsetDetection::processFrame(float* frame, const int& n){
	//bool onsetFound = false;
	// aubio onset detector then processes current frame - returns bool true when new detection is output
	//if buffer full and new result is processed (buffer is 1024 with hopsize 512 - can be set to other values)
	
	//for other class OnsetDetectionFunction : dfSample = qmOnsetDetector->getDFsample(frame);
	
	if (onsetDetector->processframe(frame, n)){
		
		rawOnsetFunction[onsetIndex] = onsetDetector->rawDetectionValue;
		medianOnsetFunction[onsetIndex] = onsetDetector->medianDetectionValue;
		//		aubioOnsetFunction[onsetIndex] = ??;
		highSlopeOnsetFunction[onsetIndex] = onsetDetector->bestSlopeValue;
		aubioLongTermAverage[onsetIndex] = onsetDetector->aubioLongTermAverage;
		
		//	outlet_float(x->detectionFunctionOutlet, x->onsetDetector->peakPickedDetectionValue);
		
		if (onsetDetector->aubioOnsetFound){
			aubioOnsetRecorded[onsetIndex] = true;
		}
		else{
			aubioOnsetRecorded[onsetIndex] = true;
		}
		
		
		if (onsetDetector->anrMedianProcessedOnsetFound){
			onsetFound = true;
			medianOnsetRecorded[onsetIndex] = true;
		}else{
			onsetFound = false;
			medianOnsetRecorded[onsetIndex] = false;
		}
		
		if (onsetDetector->anrBestSlopeOnset){
			//do
			highSlopeOnsetRecorded[onsetIndex] = true;
			highSlopeOnsetsFrames.push_back(frameCountIndex);
			highSlopeOnsetsMillis.push_back(framesToMillis(frameCountIndex));
			
			
			//did use the above but all info held in ChromaOnset class
			
			ChromaOnset c;
			c.frameTime = frameCountIndex;
			c.millisTime = framesToMillis(frameCountIndex);
			c.onsetIndex = onsetIndex;
		//	c.aubioPitchFound = false;c.chromaCalculated = false;
			chromaOnsets.push_back(c);
			
			printf("frame %i is time %f \n", frameCountIndex, framesToMillis(frameCountIndex));
		}
		else{
			highSlopeOnsetRecorded[onsetIndex] = false;
		}
		
		frameCountIndex++;//this one never loops
		onsetIndex++;
		if (onsetIndex == NUM_DETECTION_SAMPLES)
			onsetIndex = 0;//this loops round
		
	}//end if new aubio onset detection result
	
	checkChromaAndPitch(frame, n);
	
}



double ofxAubioOnsetDetection::framesToMillis(const double& frameCount){
	return ((frameCount*onsetDetector->hopsize*1000.0)/44100.0);
}
 

void ofxAubioOnsetDetection::drawOnsetDetection(){
	drawOnsetDetection(0, amplitudeNumber);

	
}//end draw onset fn

void ofxAubioOnsetDetection::drawOnsetDetectionScrolling(){
	
	int startFrame = (int)(playPositionFrames/amplitudeNumber)* amplitudeNumber;
	
	drawOnsetDetection(startFrame, startFrame+amplitudeNumber);
		
}//end draw onset fn


void ofxAubioOnsetDetection::printOnsetList(){
	printf("PRINT ONSET LIST");
	for (int i = 0;i < chromaOnsets.size();i++){
		printf("%i:", i);
		chromaOnsets[i].printInfo();
	}
}

void ofxAubioOnsetDetection::drawOnsetDetection(int startIndex, int endIndex){
	ofBackground(0);
	setDrawParams();
	
	int tmpIndex = onsetIndex;
	float width = screenWidth / (float) amplitudeNumber;	
	float maximumValue = onsetDetector->maximumDetectionValue;
	float minimumValue = 0;//minimumDetectionFunction ;
	float difference = maximumValue - minimumValue;
	float scale_factor = screenHeight/ difference;
	
	//draw axis
	ofSetColor(255,255,255);
	ofLine(0, screenHeight - (scale_factor*(0 - minimumValue)), 
		   (int) (width*(amplitudeNumber)),  screenHeight - (scale_factor*(0 - minimumValue)) );
	
	drawChromaOnsetData(startIndex, endIndex);
	
	for (int Xvalue = startIndex;Xvalue < min(endIndex, amplitudeNumber); Xvalue++){
		
		int Xindex = (endIndex - Xvalue) ;

		int previousIndex = (Xindex-1);
		if (Xindex < 0){
			Xindex += NUM_DETECTION_SAMPLES;
			if (previousIndex < 0)
				previousIndex += NUM_DETECTION_SAMPLES;
		}
		
		//onsetFunction[] also processed	but not shown - Brossier's?
			
		ofSetColor(155);//,200,55);

		//raw detection value we use for post processing
		int previousXpos = (int) (width*(amplitudeNumber - Xvalue + startIndex - 1));
		int Xpos = (int) (width*(amplitudeNumber - Xvalue + startIndex));				  
		
		ofLine(previousXpos, screenHeight - (scale_factor*(rawOnsetFunction[previousIndex]- minimumValue)), 
			   Xpos,  screenHeight - (scale_factor*(rawOnsetFunction[Xindex]- minimumValue)) );
		

		
		//median of Onset fn	
		ofSetColor(0,105,0);		
		ofLine(previousXpos, screenHeight - (scale_factor*(medianOnsetFunction[previousIndex]- minimumValue)), 
			   Xpos,  screenHeight - (scale_factor*(medianOnsetFunction[Xindex]- minimumValue)) );
		
		
		if (medianOnsetRecorded[Xindex] == true){
			ofSetColor(0,255,0);
			ofCircle(Xpos, screenHeight - (scale_factor*(medianOnsetFunction[Xindex]- minimumValue)) , 4);
		}
		
		
		ofSetColor(0,0,160);
		ofLine(previousXpos, screenHeight - (scale_factor*(highSlopeOnsetFunction[previousIndex]- minimumValue)), 
			   Xpos,  screenHeight - (scale_factor*(highSlopeOnsetFunction[Xindex]- minimumValue)) );
		
		//bright blue - slope based onsets
		if (highSlopeOnsetRecorded[Xindex] == true){
			ofSetColor(0,0,255);
			ofCircle(Xpos, screenHeight - (scale_factor*(highSlopeOnsetFunction[Xindex]- minimumValue)) , 4);
		}
		
		//long term average in dull grey
		ofSetColor(100);
		ofLine(previousXpos, screenHeight - (scale_factor*(aubioLongTermAverage[previousIndex]- minimumValue)), 
			   Xpos,  screenHeight - (scale_factor*(aubioLongTermAverage[Xindex]- minimumValue)) );
	
		
		ofSetColor(255,100,0);
		
	}//end for Xvalue (across the recent observations of osc data)
	


	//label y axis
	int axisHeight, stepSize;
	ofSetColor(255,255,255);
	stepSize = 1000;
	
	while((difference / stepSize) < 3)
		stepSize /= 2;
	
	while ((difference / stepSize) > 7)// maximum 6 numbers to display
		stepSize *= 2;
	
	
	for (axisHeight = 0; axisHeight < maximumDetectionFunction; axisHeight += stepSize){
		ofDrawBitmapString( ofToString((int)axisHeight), ofGetWidth()-50,
						   (int) ((TEXT_HEIGHT/2) +(screenHeight - (scale_factor*(axisHeight- minimumValue)))) );
	}
	
	for (axisHeight = max(0, (int)minimumDetectionFunction); axisHeight > min(0, (int)minimumDetectionFunction); axisHeight -= stepSize){
		ofDrawBitmapString( ofToString((int)axisHeight), ofGetWidth()-50,
						   (int) ((TEXT_HEIGHT/2) +(screenHeight - (scale_factor*(axisHeight- minimumValue)))) );
	}
	
	//label x axis
	stepSize = 20;//need to make sure not too many of these:
	
	while((amplitudeNumber / stepSize) < 4)
		stepSize /= 2;
	
	while ((amplitudeNumber / stepSize) > 8)
		stepSize *= 2;
	
	int labelIndex = onsetIndex - (onsetIndex % stepSize);
	for (int y = labelIndex; y > onsetIndex - amplitudeNumber; y -= stepSize){
		ofDrawBitmapString( ofToString((int)y), (int) (width*(amplitudeNumber - (onsetIndex - y))), (int) ((TEXT_HEIGHT+2) + (screenHeight - (scale_factor*(0 - minimumValue)))) );
	}
	 
	
	//play position
	ofSetColor(255,255,255);
	playPositionFrames = playPosition * frameCountIndex;
	ofLine(width*(playPositionFrames-startIndex), 0, width*(playPositionFrames-startIndex), screenHeight);

	
}//end draw onset fn


void ofxAubioOnsetDetection::drawChromaOnsetData(const int& startIndex, const int& endIndex){
		
	int chromaIndex = chromaOnsets.size()/2;
	while (chromaIndex > 0 && chromaOnsets[chromaIndex].onsetIndex > startIndex)
		chromaIndex--;
	
	while (chromaIndex < chromaOnsets.size() && chromaOnsets[chromaIndex].onsetIndex < startIndex)
		chromaIndex++;

	
	while (chromaIndex < chromaOnsets.size() && chromaOnsets[chromaIndex].chromaCalculated && chromaOnsets[chromaIndex].onsetIndex < endIndex) {
		
		ofSetColor(255,100,255);
		
		int Xindex = chromaOnsets[chromaIndex].onsetIndex;
		int Xvalue = endIndex - Xindex;
		
		for (int j = 0;j < 12;j++){
			ofSetColor(0,0,255*chromaOnsets[chromaIndex].chromaValues[11-j], 20);
			ofRect(drawParams.width*(amplitudeNumber - Xvalue), screenHeight*j/12.0, 6, screenHeight/12);
		}
		
		ofCircle(drawParams.width*(amplitudeNumber - Xvalue), screenHeight - (drawParams.scale_factor*(highSlopeOnsetFunction[Xindex]- drawParams.minimumValue)) , 4);
				
		//now do pitch in log freq
		float heightFactor = 0;
		if (chromaOnsets[chromaIndex].aubioPitch > 0)
			heightFactor = log(chromaOnsets[chromaIndex].aubioPitch) / log(maximumAubioPitch);
		heightFactor = 1 - heightFactor;
		ofSetColor(255,0,0);
		//ofSetLineWidth(8);
		ofLine(drawParams.width*(amplitudeNumber - Xvalue), heightFactor*screenHeight, drawParams.width*(amplitudeNumber - Xvalue) + 50, heightFactor*screenHeight);
		
		ofSetColor(0,0,255);
		ofDrawBitmapString(ofToString(chromaOnsets[chromaIndex].aubioPitch, 1) , drawParams.width*(amplitudeNumber - Xvalue), heightFactor*screenHeight-4);
		chromaIndex++;
	}

	
	
}//end draw chroma


void ofxAubioOnsetDetection::setDrawParams(){
	
	drawParams.width = screenWidth / (float) amplitudeNumber;	
	drawParams.maximumValue = onsetDetector->maximumDetectionValue;
	drawParams.minimumValue = 0;//minimumDetectionFunction ;
	drawParams.difference = drawParams.maximumValue - drawParams.minimumValue;
	drawParams.scale_factor = screenHeight/ drawParams.difference;
}


void ofxAubioOnsetDetection::windowResized(const int& w, const int& h){
	screenWidth = ofGetWidth();
	screenHeight = ofGetHeight();
}


void ofxAubioOnsetDetection::aubioOnsetDetect_energy(){
	onsetDetector->onsetclass_energy();
	printf("Energy based onset detection now used by aubioOnsetDetect~.");
}

void ofxAubioOnsetDetection::aubioOnsetDetect_hfc(){
	/** High Frequency Content onset detection function
	 
	 This method computes the High Frequency Content (HFC) of the input spectral
	 frame. The resulting function is efficient at detecting percussive onsets.
	 
	 Paul Masri. Computer modeling of Sound for Transformation and Synthesis of
	 Musical Signal. PhD dissertation, University of Bristol, UK, 1996.*/

	onsetDetector->onsetclass_hfc();
	printf("High Frequency Content (Masri '96) detection now used by aubioOnsetDetect~.");
}


void ofxAubioOnsetDetection::aubioOnsetDetect_complex(){
	//Complex Domain Method onset detection function 
	//Christopher Duxbury, Mike E. Davies, and Mark B. Sandler. Complex domain
	//onset detection for musical signals. In Proceedings of the Digital Audio
	//Effects Conference, DAFx-03, pages 90-93, London, UK, 2003.
	onsetDetector->onsetclass_complex();
	printf("Complex domain onset detection (Duxbury et al., DaFx '03) now used by aubioOnsetDetect~.");
	
}

void ofxAubioOnsetDetection::aubioOnsetDetect_phase(){
	/** Phase Based Method onset detection function 
	 
	 Juan-Pablo Bello, Mike P. Davies, and Mark B. Sandler. Phase-based note onset
	 detection for music signals. In Proceedings of the IEEE International
	 Conference on Acoustics Speech and Signal Processing, pages 441­444,
	 Hong-Kong, 2003.*/

	onsetDetector->onsetclass_phase();
	printf("Phase-based detection (Bello et al., IEEE '03) now used by aubioOnsetDetect~.");
}

void ofxAubioOnsetDetection::aubioOnsetDetect_specdiff(){
	/* Spectral difference method onset detection function 
	 Jonhatan Foote and Shingo Uchihashi. The beat spectrum: a new approach to
	 rhythm analysis. In IEEE International Conference on Multimedia and Expo
	 (ICME 2001), pages 881­884, Tokyo, Japan, August 2001.
	 */
	//aubio_onsetdetection_type
	//aubio_onsetdetection_free (x->o);
	onsetDetector->onsetclass_specdiff();
	printf("Spectral Difference (Foote and Shingo Uchihashi, ICME '01) detection now used by aubioOnsetDetect~.");
	
	
}

void ofxAubioOnsetDetection::aubioOnsetDetect_kl(){
	//aubio_onsetdetection_type
	//aubio_onsetdetection_free (x->o);
	/** Kullback-Liebler onset detection function 
	 
	 Stephen Hainsworth and Malcom Macleod. Onset detection in music audio
	 signals. In Proceedings of the International Computer Music Conference
	 (ICMC), Singapore, 2003.
	 */
	
	onsetDetector->onsetclass_kl();
	printf("Kullback-Liebler (Hainsworth and McLeod, ICMC '03) detection now used by aubioOnsetDetect~.");
}

void ofxAubioOnsetDetection::aubioOnsetDetect_mkl(){
	/** Modified Kullback-Liebler onset detection function 
	 
	 Paul Brossier, ``Automatic annotation of musical audio for interactive
	 systems'', Chapter 2, Temporal segmentation, PhD thesis, Centre for Digital
	 music, Queen Mary University of London, London, UK, 2003.*/		
	onsetDetector->onsetclass_mkl();
	printf("Modified Kullback-Liebler (Brossier, PhD thesis '03) detection now used by aubioOnsetDetect~.");
}


