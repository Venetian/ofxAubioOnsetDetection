/*
 *  ofxAubioOnsetDetection.cpp
 *  ofxOnsetDetection
 *
 *  Created by Andrew on 24/11/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

#include "ofxAubioOnsetDetection.h"


//@ do: fix up reset - maybe think of using vectors

//tracktypes:
//there are three types
//0 and 2 - the onset only type
//1 - bass - ie.e uses aubio pitch
//3 - harmonic - such as guitar represented by a chromagram following the onset

//we calculate these by logging the time then filling up a window for the calculation.

const bool blackAndWhite = true;

const bool printChromaInformation = false;

ofxAubioOnsetDetection::ofxAubioOnsetDetection(){

	//onsetDetector = new AubioOnsetDetector();
	onsetDetector.initialise();
	
	initialiseValues();
	
	printf("ofxAubioOnsetDetector made\n");
	
	amplitudeNumber = 256;//number of amplitudes shown on screen
	maximumAubioPitch = 220;//max pitch shown inside window
	minimumAubioPitch = 55.0/2.0;
	trackType = 0;

	printf("max val on making is %f\n", onsetDetector.maximumDetectionValue);
}


ofxAubioOnsetDetection::~ofxAubioOnsetDetection(){
//	delete onsetDetector;
	printf("DELETE ONSET CALLED\n");
//	delete qmOnsetDetector;
}

void ofxAubioOnsetDetection::initialiseValues(){
	printf("ONSET DETECTOR INITIALISED\n");
	//	useMedianOnsetDetection = true;
	onsetIndex = 0;
	frameCountIndex = 0;
	playPositionFrames = 0;
	playPosition = 0;
	
	screenWidth = ofGetWidth();
	screenHeight = ofGetHeight();
	
	//each file has onset window to be displayed in
	window.setToRelativeSize(0.1, 0.1, 0.8, 0.3);
	//fullScreen.setToRelativeSize(0,0,1,1);
	
	drawParams.width = 5;
	drawParams.maximumValue = 10;
	drawParams.minimumValue = 0;
	drawParams.difference = 10;
	drawParams.scale_factor = 30;
	
	setDrawParams();
}

void ofxAubioOnsetDetection::reset(){
	onsetIndex = 0;
	frameCountIndex = 0;
	
	for (int i = 0;i < NUM_DETECTION_SAMPLES;i++){
		onsetFunction[i] = 0;
		medianOnsetFunction[i] = 0;
		highSlopeOnsetFunction[i] = 0;
		
	}
	
	
	highSlopeOnsetsFrames.clear();
	highSlopeOnsetsMillis.clear();//The high slope onsets are the ones stored as chromaOnsets
	//(i.e. have associated chroma and pitch to them)
	chromaOnsets.clear();
	
	onsetDetector.resetValues();
	
	printf("Resetting the ofxAubioOnsetDetector\n");
}


void ofxAubioOnsetDetection::processFrame(double* frame, const int& n){
	//bool onsetFound = false;
	// aubio onset detector then processes current frame - returns bool true when new detection is output
	//if buffer full and new result is processed (buffer is 1024 with hopsize 512 - can be set to other values)
	
	//for other class OnsetDetectionFunction : dfSample = qmonsetDetector.getDFsample(frame);
	
	if (onsetDetector.processframe(frame, n)){

		rawOnsetFunction[onsetIndex] = onsetDetector.rawDetectionValue;
		medianOnsetFunction[onsetIndex] = onsetDetector.medianDetectionValue;
//		aubioOnsetFunction[onsetIndex] = ??;
		highSlopeOnsetFunction[onsetIndex] = onsetDetector.bestSlopeValue;
		aubioLongTermAverage[onsetIndex] = onsetDetector.aubioLongTermAverage;
		
		//	outlet_float(x->detectionFunctionOutlet, x->onsetDetector.peakPickedDetectionValue);
		
		if (onsetDetector.aubioOnsetFound){
			aubioOnsetRecorded[onsetIndex] = true;
		}
		else{
			aubioOnsetRecorded[onsetIndex] = false;//anr chnage late on
		}
		
		
		if (onsetDetector.anrMedianProcessedOnsetFound){
			onsetFound = true;
			medianOnsetRecorded[onsetIndex] = true;
		}else{
			onsetFound = false;
			medianOnsetRecorded[onsetIndex] = false;
		}
		
		if (onsetDetector.anrBestSlopeOnset){
			highSlopeOnsetRecorded[onsetIndex] = true;
			highSlopeOnsetsFrames.push_back(frameCountIndex);
			highSlopeOnsetsMillis.push_back(framesToMillis(frameCountIndex));
			
			printf("onset frame %i is time %f \n", frameCountIndex, framesToMillis(frameCountIndex));
			
			
			chromaOnsetPtr = new ChromaOnset();
			chromaOnsetPtr ->frameTime = frameCountIndex;
			chromaOnsetPtr->millisTime = framesToMillis(frameCountIndex);
			chromaOnsetPtr->onsetIndex = onsetIndex;
			chromaOnsets.push_back(*chromaOnsetPtr);
			
			
		}
		else{
			highSlopeOnsetRecorded[onsetIndex] = false;
		}
		
		frameCountIndex++;//this one never loops
		onsetIndex++;
		if (onsetIndex == NUM_DETECTION_SAMPLES)
			onsetIndex = 0;//this loops round
		
	}//end if new aubio onset detection result
	
	printf("N is %i\n", n);
	float* tmpFrame;
	vector<float> tmpFloatvector;
	for (int i = 0;i < n;i++){
		//tmpFrame[i] = frame[i];
		tmpFloatvector.push_back(frame[i]);
	}
	
	checkChromaAndPitch(&tmpFloatvector[0], n);
	
	
 }

void ofxAubioOnsetDetection::checkChromaAndPitch(float* tmpFrame, const int& n){
	//new method for pitch
	//calculate always
	pitchDetector.addToBuffer(tmpFrame, n);
	
	//process frames into each onset's chromagram analyser
	for (int i = 0;i < chromaOnsets.size();i++){
		
		//aubio pitch using yin
		if (trackType == 1 && !chromaOnsets[i].aubioPitchFound ){//not yet done
			
			
			//dont actually need to add this to the frame anymore!
		//	chromaOnsets[i].onsetFrame.addToFrame(tmpFrame, n);//add to frame in the chromaOnset class
		//	chromaOnsets[i].onsetFrame.frameCounter += n;
			chromaOnsets[i].pitchFrameCounter += n;
			
			//used to use chromaOnsets[i].pitchFrameCounter
		//	printf("adding to frame for onset %i at frametime %i\n", i, frameCountIndex);
			if (chromaOnsets[i].pitchFrameCounter >= pitchDetector.bufsize){//enough to calculate yin with using aubio pitch detection
				
				if (printChromaInformation)
					printf("frame %i onset frame counter %i chromacounter %i\n", chromaOnsets[i].onsetFrame.sizeOfFrame, chromaOnsets[i].onsetFrame.frameCounter, chromaOnsets[i].pitchFrameCounter );
				
				
				//float originalPitchMethod = pitchDetector.doPitchDetection(&chromaOnsets[i].onsetFrame.frame[0], pitchDetector.bufsize);
				chromaOnsets[i].aubioPitch = pitchDetector.getPitch();//used to be originalPitchMethod;
				
				chromaOnsets[i].aubioPitchFound = true;
				chromaOnsets[i].onsetFrame.deleteFrame();
		//		printf("Aubio Pitch recieved for onset %i is %f\n", i, chromaOnsets[i].aubioPitch);
				
			
				
			
			}
				
		}
		
		
		//chroma
		if (chromaOnsets[i].processFrame(tmpFrame, n) && trackType == 1){
			//!= 3 : i.e. true when we dont need so we delete immediately
			//printf("onset %i (frametime %i) chroma is newly calculated at frame %i\n", i, chromaOnsets[i].frameTime, frameCountIndex);
			if (printChromaInformation)
				chromaOnsets[i].printInfo();

			chromaOnsets[i].deleteChromagram();
		}
		
	}//end for all onsets
}

void ofxAubioOnsetDetection::processFrame(float* frame, const int& n){
	//bool onsetFound = false;
	// aubio onset detector then processes current frame - returns bool true when new detection is output
	//if buffer full and new result is processed (buffer is 1024 with hopsize 512 - can be set to other values)
	
	//for other class OnsetDetectionFunction : dfSample = qmonsetDetector.getDFsample(frame);
	
	if (onsetDetector.processframe(frame, n)){
		
		rawOnsetFunction[onsetIndex] = onsetDetector.rawDetectionValue;
		medianOnsetFunction[onsetIndex] = onsetDetector.medianDetectionValue;
		//		aubioOnsetFunction[onsetIndex] = ??;
		highSlopeOnsetFunction[onsetIndex] = onsetDetector.bestSlopeValue;
		aubioLongTermAverage[onsetIndex] = onsetDetector.aubioLongTermAverage;
		
		//	outlet_float(x->detectionFunctionOutlet, x->onsetDetector.peakPickedDetectionValue);
		
		if (onsetDetector.aubioOnsetFound){
			aubioOnsetRecorded[onsetIndex] = true;
		}
		else{
			aubioOnsetRecorded[onsetIndex] = false;
		}
		
		
		if (onsetDetector.anrMedianProcessedOnsetFound){
			onsetFound = true;
			medianOnsetRecorded[onsetIndex] = true;
		}else{
			onsetFound = false;
			medianOnsetRecorded[onsetIndex] = false;
		}
		
		if (onsetDetector.anrBestSlopeOnset){
			//do
			highSlopeOnsetRecorded[onsetIndex] = true;
			highSlopeOnsetsFrames.push_back(frameCountIndex);
			highSlopeOnsetsMillis.push_back(framesToMillis(frameCountIndex));
			
			
			//did use the above but all info held in ChromaOnset class
		
			/*
			 ChromaOnset c;
			 c.frameTime = frameCountIndex;
			 c.millisTime = framesToMillis(frameCountIndex);
			 c.onsetIndex = onsetIndex;
			 */
			chromaOnsetPtr = new ChromaOnset();
			chromaOnsetPtr->frameTime = frameCountIndex;
			chromaOnsetPtr->onsetIndex = onsetIndex;
			chromaOnsetPtr->millisTime = framesToMillis(frameCountIndex);
			chromaOnsets.push_back(*chromaOnsetPtr);
			
		//	printf("frame %i is time %f \n", frameCountIndex, framesToMillis(frameCountIndex));
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
	return ((frameCount*onsetDetector.hopsize*1000.0)/44100.0);
}


void ofxAubioOnsetDetection::printOnsetList(){
	printf("PRINT ONSET LIST\n");
	for (int i = 0;i < chromaOnsets.size();i++){
		printf("%i:", i);
		chromaOnsets[i].printInfo();
	}
}



void ofxAubioOnsetDetection::drawOnsetDetection(){
	
	drawOnsetDetection(0, amplitudeNumber, window);
//	ofDrawBitmapString("amp no"+ofToString(amplitudeNumber),40,40);
	
}//end draw onset fn

void ofxAubioOnsetDetection::drawOnsetDetectionScrolling(){
	
	int startFrame = (int)(playPositionFrames/amplitudeNumber)* amplitudeNumber;
	//setDrawParams();
	
	drawParams.windowStartFrame = startFrame;
	
	drawOutlineAndSetParams(window);
	
	if (trackType == 0 || trackType == 2)
	drawOnsetDetection(startFrame, startFrame+amplitudeNumber, window);

	setDrawParams(window);
	drawChromaOnsetData(startFrame, startFrame+amplitudeNumber, window);
	
	drawScrollLine(startFrame, window);
}//end draw onset fn


void ofxAubioOnsetDetection::drawOutlineAndSetParams(const ofxWindowRegion& screenRegion){	
	//ofBackground(0);
	setDrawParams(screenRegion);
	ofSetColor(100,150,250);
	screenRegion.drawOutline();
}


void ofxAubioOnsetDetection::drawOnsetDetection(int startIndex, int endIndex, const ofxWindowRegion& screenRegion){
	

	
	int tmpIndex = onsetIndex;
	float width = screenRegion.width / (float) amplitudeNumber;	
	float maximumValue = onsetDetector.maximumDetectionValue;
	float minimumValue = 0;//minimumDetectionFunction ;
	float difference = maximumValue - minimumValue;
	float scale_factor = screenRegion.height/ difference;
	
	endIndex = min(endIndex, startIndex+amplitudeNumber);
	
//	drawChromaOnsetData(startIndex, endIndex, screenRegion);
	
	for (int Xvalue = startIndex;Xvalue < endIndex; Xvalue++){
		
		int Xindex = Xvalue;//(endIndex - Xvalue) ;
		
		int previousIndex = (Xindex-1);
		if (Xindex < 0){
			Xindex += NUM_DETECTION_SAMPLES;
			if (previousIndex < 0)
				previousIndex += NUM_DETECTION_SAMPLES;
		}
		
		//onsetFunction[] also processed	but not shown - Brossier's?
		bool fullInfo = false;
		int Xpos, previousXpos;
		
			ofSetColor(155);//,200,55);
			previousXpos = (int) (screenRegion.x + (width*(Xvalue - startIndex - 1)));
			Xpos = (int) (screenRegion.x +(width*(Xvalue - startIndex)));		
			
		if (fullInfo){	
			
			ofLine(previousXpos, screenRegion.y + screenRegion.height - (scale_factor*(rawOnsetFunction[previousIndex]- minimumValue)), 
				   Xpos,  screenRegion.y + screenRegion.height - (scale_factor*(rawOnsetFunction[Xindex]- minimumValue)) );
			
			//median of Onset fn
			ofSetColor(0,105,0);		
			ofLine(previousXpos, screenRegion.y + screenRegion.height - (scale_factor*(medianOnsetFunction[previousIndex]- minimumValue)), 
				   Xpos,  screenRegion.y + screenRegion.height - (scale_factor*(medianOnsetFunction[Xindex]- minimumValue)) );
			
			
			if (medianOnsetRecorded[Xindex] == true){
				ofSetColor(0,255,0);
				ofCircle(Xpos, screenRegion.y + screenRegion.height - (scale_factor*(medianOnsetFunction[Xindex]- minimumValue)) , 4);
			}
			
			//long term average in dull grey
			ofSetColor(100);
			ofLine(previousXpos, screenRegion.y + screenRegion.height - (scale_factor*(aubioLongTermAverage[previousIndex]- minimumValue)), 
				   Xpos,  screenRegion.y + screenRegion.height - (scale_factor*(aubioLongTermAverage[Xindex]- minimumValue)) );
		 
			
		}//end if full info
		
		ofSetColor(160);//was 0,0,160 blue
		ofLine(previousXpos, screenRegion.y + screenRegion.height - (scale_factor*(highSlopeOnsetFunction[previousIndex]- minimumValue)), 
			   Xpos,  screenRegion.y + screenRegion.height - (scale_factor*(highSlopeOnsetFunction[Xindex]- minimumValue)) );
		
		//bright blue - slope based onsets
		if (highSlopeOnsetRecorded[Xindex] == true){
			//ofSetColor(0,0,255);
			ofCircle(Xpos, screenRegion.y + screenRegion.height - (scale_factor*(highSlopeOnsetFunction[Xindex]- minimumValue)) , 4);
		}
		
		
		
		ofSetColor(255,100,0);
		
	}//end for Xvalue (across the recent observations of osc data)
	
	
	//some axis stuff deleted
	
	
//	string tmpInfoString = "(windowed)max val "+ofToString(onsetDetector.maximumDetectionValue,2);
//	tmpInfoString += " diff "+ofToString(difference);	
//	ofDrawBitmapString(tmpInfoString, 100,200);
	
}//end draw onset fn

void ofxAubioOnsetDetection::drawScrollLine(const int& startIndex, const ofxWindowRegion& screenRegion){
	//play position
	float width = screenRegion.width / (float) amplitudeNumber;	
	ofSetColor(255,255,255);
	playPositionFrames = playPosition * frameCountIndex;
	ofLine(screenRegion.x + width*(playPositionFrames-startIndex), screenRegion.y , screenRegion.x + width*(playPositionFrames-startIndex), screenRegion.y + screenRegion.height);
}


void ofxAubioOnsetDetection::drawChromaOnsetData(const int& startIndex, const int& endIndex, const ofxWindowRegion& screenRegion){
	
	int chromaIndex = 0;//chromaOnsets.size()/2;
	while (chromaIndex > 0 && chromaOnsets[chromaIndex].onsetIndex > startIndex)
		chromaIndex--;
	
	while (chromaIndex < chromaOnsets.size() && chromaOnsets[chromaIndex].onsetIndex < startIndex)
		chromaIndex++;
	
	switch (trackType) {
		case 1:
			if (blackAndWhite)
				ofSetColor(100);
			else
				ofSetColor(255,0,0);//red lines
			drawPitchLines(chromaIndex, endIndex, screenRegion);
			break;
		case 3:
				drawChromaStripes(chromaIndex, endIndex, screenRegion);
			break;
		case 0:
			if (blackAndWhite)
				ofSetColor(80);
			else
				ofSetColor(200,0,0);//red lines
//			ofSetColor(200,0,0);
			drawOnsetStripes(chromaIndex, endIndex, screenRegion);
			break;
		case 2:
			if (blackAndWhite)
				ofSetColor(120);
			else
				ofSetColor(200,200,0);//red lines
//			ofSetColor(200,200,0);
			drawOnsetStripes(chromaIndex, endIndex, screenRegion);
			break;	
	}//end switch
	


}//end draw chroma

void ofxAubioOnsetDetection::drawOnsetStripes(int chromaIndex, const int& frameEndIndex, const ofxWindowRegion& screenRegion){
			
			
	while (chromaIndex < chromaOnsets.size() && chromaOnsets[chromaIndex].chromaCalculated && chromaOnsets[chromaIndex].onsetIndex < frameEndIndex) {
				
				//ofSetColor(255,100,255);
				
				int Xindex = chromaOnsets[chromaIndex].onsetIndex;
				int Xvalue = frameEndIndex - Xindex;
				
		//		for (int j = 0;j < 12;j++){
					
				//	if (!chromaOnsets[chromaIndex].matched)
				//		ofSetColor(0,0,255*chromaOnsets[chromaIndex].chromaValues[11-j], 20);
				//	else
				//		ofSetColor(255*chromaOnsets[chromaIndex].chromaValues[11-j],0,0, 20);
					
					ofRect(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y, 4, screenRegion.height);
		//		}
				
		//		ofCircle(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y + screenRegion.height - (drawParams.scale_factor*(highSlopeOnsetFunction[Xindex]- drawParams.minimumValue)) , 4);
				
				chromaIndex++;
			}
			
}//end draw chroma		
		
void ofxAubioOnsetDetection::drawChromaStripes(int chromaIndex, const int& frameEndIndex, const ofxWindowRegion& screenRegion){
	
	
	while (chromaIndex < chromaOnsets.size() && chromaOnsets[chromaIndex].chromaCalculated && chromaOnsets[chromaIndex].onsetIndex < frameEndIndex) {
		
		ofSetColor(255,100,255);
		
		int Xindex = chromaOnsets[chromaIndex].onsetIndex;
		int Xvalue = frameEndIndex - Xindex;
		
		for (int j = 0;j < 12;j++){
			
			if (!chromaOnsets[chromaIndex].matched)
				ofSetColor(0,0,255*chromaOnsets[chromaIndex].chromaValues[11-j], 20);
			else
				ofSetColor(255*chromaOnsets[chromaIndex].chromaValues[11-j],0,0, 20);
			
			ofRect(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y + screenRegion.height*j/12.0, 20, screenRegion.height/12);
		}
		
		ofCircle(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y + screenRegion.height - (drawParams.scale_factor*(highSlopeOnsetFunction[Xindex]- drawParams.minimumValue)) , 4);
		
		chromaIndex++;
	}
	
}//end draw chroma

void ofxAubioOnsetDetection::drawPitchLines(int chromaIndex, const int& frameEndIndex, const ofxWindowRegion& screenRegion){
	
	ofSetLineWidth(2);
	
	while (chromaIndex < chromaOnsets.size() && chromaOnsets[chromaIndex].chromaCalculated && chromaOnsets[chromaIndex].onsetIndex < frameEndIndex) {
		
		int Xindex = chromaOnsets[chromaIndex].onsetIndex;
		int Xvalue = frameEndIndex - Xindex;
		
		//now do pitch in log freq
		float heightFactor = 0;
		if (chromaOnsets[chromaIndex].aubioPitch > 0)
			heightFactor = log(chromaOnsets[chromaIndex].aubioPitch/minimumAubioPitch) / log(maximumAubioPitch/minimumAubioPitch);
		heightFactor = 1 - heightFactor;
		//red lines for freq
		if (!blackAndWhite)
			ofSetColor(255,0,0);//red lines
		//ofSetLineWidth(8);
		ofLine(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y + screenRegion.height*heightFactor, 
			   screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue) + 50, screenRegion.y + screenRegion.height*heightFactor);
		
		if (!blackAndWhite)
			ofSetColor(255,0,255);
		
		ofDrawBitmapString(ofToString(chromaOnsets[chromaIndex].aubioPitch, 1) , screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue) + 4, screenRegion.y + screenRegion.height*heightFactor - 4);
		chromaIndex++;
	}
	
	ofSetLineWidth(1);
	
	
}//end draw pitch lines

void ofxAubioOnsetDetection::setDrawParams(){
	screenWidth = ofGetWidth();
	screenHeight = ofGetHeight();//justin case
	
	drawParams.width = screenWidth / (float) amplitudeNumber;	
	drawParams.maximumValue = onsetDetector.maximumDetectionValue;
	drawParams.minimumValue = 0;//minimumDetectionFunction ;
	drawParams.difference = drawParams.maximumValue - drawParams.minimumValue;
	drawParams.scale_factor = screenHeight/ drawParams.difference;
	
}

void ofxAubioOnsetDetection::setDrawParams(const ofxWindowRegion& screenRegion){
	
	drawParams.width = screenRegion.width / (float) amplitudeNumber;	
	drawParams.maximumValue = onsetDetector.maximumDetectionValue;
	drawParams.minimumValue = 0;//minimumDetectionFunction ;
	drawParams.difference = drawParams.maximumValue - drawParams.minimumValue;
	drawParams.scale_factor = screenRegion.height/ drawParams.difference;
}


void ofxAubioOnsetDetection::windowResized(const int& w, const int& h){
	screenWidth = ofGetWidth();
	screenHeight = ofGetHeight();
	window.resized(w, h);
	fullScreen.resized(w, h);
}


void ofxAubioOnsetDetection::printChromaInfo(){
	for (int i = 0;i < chromaOnsets.size();i++)
		chromaOnsets[i].printInfo();
	
	printf("max value on printing is %f\n", onsetDetector.maximumDetectionValue);
}

void ofxAubioOnsetDetection::aubioOnsetDetect_energy(){
	onsetDetector.onsetclass_energy();
	printf("Energy based onset detection now used by aubioOnsetDetect~.");
}

void ofxAubioOnsetDetection::aubioOnsetDetect_hfc(){
	/** High Frequency Content onset detection function
	 
	 This method computes the High Frequency Content (HFC) of the input spectral
	 frame. The resulting function is efficient at detecting percussive onsets.
	 
	 Paul Masri. Computer modeling of Sound for Transformation and Synthesis of
	 Musical Signal. PhD dissertation, University of Bristol, UK, 1996.*/

	onsetDetector.onsetclass_hfc();
	printf("High Frequency Content (Masri '96) detection now used by aubioOnsetDetect~.");
}


void ofxAubioOnsetDetection::aubioOnsetDetect_complex(){
	//Complex Domain Method onset detection function 
	//Christopher Duxbury, Mike E. Davies, and Mark B. Sandler. Complex domain
	//onset detection for musical signals. In Proceedings of the Digital Audio
	//Effects Conference, DAFx-03, pages 90-93, London, UK, 2003.
	onsetDetector.onsetclass_complex();
	printf("Complex domain onset detection (Duxbury et al., DaFx '03) now used by aubioOnsetDetect~.");
	
}

void ofxAubioOnsetDetection::aubioOnsetDetect_phase(){
	/** Phase Based Method onset detection function 
	 
	 Juan-Pablo Bello, Mike P. Davies, and Mark B. Sandler. Phase-based note onset
	 detection for music signals. In Proceedings of the IEEE International
	 Conference on Acoustics Speech and Signal Processing, pages 441­444,
	 Hong-Kong, 2003.*/

	onsetDetector.onsetclass_phase();
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
	onsetDetector.onsetclass_specdiff();
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
	
	onsetDetector.onsetclass_kl();
	printf("Kullback-Liebler (Hainsworth and McLeod, ICMC '03) detection now used by aubioOnsetDetect~.");
}

void ofxAubioOnsetDetection::aubioOnsetDetect_mkl(){
	/** Modified Kullback-Liebler onset detection function 
	 
	 Paul Brossier, ``Automatic annotation of musical audio for interactive
	 systems'', Chapter 2, Temporal segmentation, PhD thesis, Centre for Digital
	 music, Queen Mary University of London, London, UK, 2003.*/		
	onsetDetector.onsetclass_mkl();
	printf("Modified Kullback-Liebler (Brossier, PhD thesis '03) detection now used by aubioOnsetDetect~.");
}


/*
 axis stuff end
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
 */






void ofxAubioOnsetDetection::drawOnsetDetection(int startIndex, int endIndex){
	//ofBackground(0);
	setDrawParams();
	
	int tmpIndex = onsetIndex;
	float width = screenWidth / (float) amplitudeNumber;	
	float maximumValue = onsetDetector.maximumDetectionValue;
	float minimumValue = 0;//minimumDetectionFunction ;
	float difference = maximumValue - minimumValue;
	float scale_factor = screenHeight/ difference;
	
	//draw axis
	ofSetColor(255,0,255);
	ofDrawBitmapString(ofToString((startIndex)), 20, 20);
	
	ofLine(0, screenHeight - (scale_factor*(0 - minimumValue)), 
		   (int) (width*(amplitudeNumber)),  screenHeight - (scale_factor*(0 - minimumValue)) );
	
	
	
	endIndex = min(endIndex, startIndex+amplitudeNumber);
	
	drawChromaOnsetData(startIndex, endIndex);
	
	for (int Xvalue = startIndex;Xvalue < endIndex; Xvalue++){
		
		int Xindex = Xvalue;//(endIndex - Xvalue) ;
		
		int previousIndex = (Xindex-1);
		if (Xindex < 0){
			Xindex += NUM_DETECTION_SAMPLES;
			if (previousIndex < 0)
				previousIndex += NUM_DETECTION_SAMPLES;
		}
		
		//onsetFunction[] also processed	but not shown - Brossier's?
		
		
		ofSetColor(155);//,200,55);
		
		
		int previousXpos = (int) (width*(Xvalue - startIndex - 1));
		int Xpos = (int) (width*(Xvalue - startIndex));		
		
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
	
	//axis stuff at end
	
	//play position
	ofSetColor(255,255,255);
	playPositionFrames = playPosition * frameCountIndex;
	ofLine(width*(playPositionFrames-startIndex), 0, width*(playPositionFrames-startIndex), screenHeight);
	
	
	//ofDrawBitmapString("max val "+ofToString(onsetDetector.maximumDetectionValue,2), 100,200);
}//end draw onset fn



void ofxAubioOnsetDetection::drawChromaOnsetData(const int& startIndex, const int& endIndex){
	
	int chromaIndex = 0;//chromaOnsets.size()/2;
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
	
	ofSetColor(255,255,235);
	ofDrawBitmapString("full drawparams width "+ofToString(drawParams.width), 100, 100);
	ofDrawBitmapString("full x"+ofToString(fullScreen.x)+" width "+ofToString(fullScreen.width), 100, 120);
}//end draw chroma



/*
 DRAW CHROMA JUNK
 while (chromaIndex < chromaOnsets.size() && chromaOnsets[chromaIndex].chromaCalculated && chromaOnsets[chromaIndex].onsetIndex < endIndex) {
 
 ofSetColor(255,100,255);
 
 int Xindex = chromaOnsets[chromaIndex].onsetIndex;
 int Xvalue = endIndex - Xindex;
 
 for (int j = 0;j < 12;j++){
 
 if (!chromaOnsets[chromaIndex].matched)
 ofSetColor(0,0,255*chromaOnsets[chromaIndex].chromaValues[11-j], 20);
 else
 ofSetColor(255*chromaOnsets[chromaIndex].chromaValues[11-j],0,0, 20);
 
 ofRect(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y + screenRegion.height*j/12.0, 6, screenRegion.height/12);
 }
 
 ofCircle(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y + screenRegion.height - (drawParams.scale_factor*(highSlopeOnsetFunction[Xindex]- drawParams.minimumValue)) , 4);
 
 //now do pitch in log freq
 float heightFactor = 0;
 if (chromaOnsets[chromaIndex].aubioPitch > 0)
 heightFactor = log(chromaOnsets[chromaIndex].aubioPitch) / log(maximumAubioPitch);
 heightFactor = 1 - heightFactor;
 //red lines for freq
 ofSetColor(255,0,0);
 //ofSetLineWidth(8);
 ofLine(screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue), screenRegion.y + screenRegion.height*heightFactor, 
 screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue) + 50, screenRegion.y + screenRegion.height*heightFactor);
 
 ofSetColor(0,0,255);
 ofDrawBitmapString(ofToString(chromaOnsets[chromaIndex].aubioPitch, 1) , screenRegion.x + drawParams.width*(amplitudeNumber - Xvalue) + 4, screenRegion.y + screenRegion.height*heightFactor - 4);
 chromaIndex++;
 }
 */

//	ofSetColor(255,255,235);
//	ofDrawBitmapString("windowed drawparams width "+ofToString(drawParams.width), 100, 100);
//	ofDrawBitmapString("windowed x"+ofToString(screenRegion.x)+" width "+ofToString(screenRegion.width), 100, 120);

