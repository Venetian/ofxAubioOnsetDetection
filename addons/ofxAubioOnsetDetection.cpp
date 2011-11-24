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
	aubioOnsetDetect_energy();
	amplitudeNumber = 256;//number of amplitudes shown on screen
	
	
//	threshold = 1;
//	threshold2 = -70.;
//	bufsize   = 1024;//using fixed buffer size here.
//	hopsize   = bufsize / 2;
	
	//set this up in AubioOnsetDetector class instead
	
/*
 x->onsetDetector->buffersize = bufsize;
	x->onsetDetector->hopsize = hopsize;
	x->onsetDetector->threshold = threshold;
	x->onsetDetector->threshold2 = threshold2;
	x->onsetDetector->initialise();
*/
//	useMedianOnsetDetection = true;
	onsetIndex = 0;
	
}
	

ofxAubioOnsetDetection::~ofxAubioOnsetDetection(){
	delete onsetDetector;
}


void ofxAubioOnsetDetection::processFrame(float* frame, const int& n){
	//bool onsetFound = false;
	// aubio onset detector then processes current frame - returns bool true when new detection is output
	//if buffer full and new result is processed (buffer is 1024 with hopsize 512 - can be set to other values)
	
	if (onsetDetector->processframe(frame, n)){
/*
		rawOnsetFunction[onsetIndex] = onsetDetector->rawDetectionValue;
		medianOnsetFunction[onsetIndex] = onsetDetector->medianDetectionValue;
		onsetFunction[onsetIndex] = onsetDetector->bestSlopeValue;
		
		//	outlet_float(x->detectionFunctionOutlet, x->onsetDetector->peakPickedDetectionValue);
		
		if (onsetDetector->anrMedianProcessedOnsetFound){
		//	onsetFound = true;
			medianOnsetRecorded[onsetIndex] = true;
		//	outlet_bang(x->medianBangOutlet);
		}else{
			medianOnsetRecorded[onsetIndex] = false;
		}
		
		if (onsetDetector->anrBestSlopeOnset){
		//	outlet_bang(x->bangoutlet);
			onsetRecorded[onsetIndex] = true;
			
		}
		else{
			onsetRecorded[onsetIndex] = false;
		}
		
		onsetIndex++;
		if (onsetIndex == NUM_DETECTION_SAMPLES)
			onsetIndex = 0;
		
 */
	}//end if new aubio onset detection result
	//return onsetFound;
}


void ofxAubioOnsetDetection::drawOnsetDetection(){
	
	float screenWidth = ofGetWidth();
	float screenHeight = ofGetHeight();
	
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
	
	
	for (int Xvalue = 0;Xvalue < amplitudeNumber; Xvalue++){
		
		int Xindex = (onsetIndex-Xvalue) ;
		int previousIndex = (Xindex-1);
		
		ofSetColor(55,100,255);
		
		ofLine((int) (width*(amplitudeNumber - Xvalue - 1)), screenHeight - (scale_factor*(onsetFunction[previousIndex]- minimumValue)), 
			   (int) (width*(amplitudeNumber - Xvalue)),  screenHeight - (scale_factor*(onsetFunction[Xindex]- minimumValue)) );
		
		if (onsetRecorded[Xindex] == true){
			ofSetColor(255,100,255);
			ofCircle(width*(amplitudeNumber - Xvalue), screenHeight - (scale_factor*(onsetFunction[Xindex]- minimumValue)) , 3);
		}
		
		//specDiffOnsets	
		ofSetColor(55,100,55);
		Xindex = (onsetIndex-Xvalue) ;
		previousIndex = (Xindex-1);
		
		ofLine((int) (width*(amplitudeNumber - Xvalue - 1)), screenHeight - (scale_factor*(rawOnsetFunction[previousIndex]- minimumValue)), 
			   (int) (width*(amplitudeNumber - Xvalue)),  screenHeight - (scale_factor*(rawOnsetFunction[Xindex]- minimumValue)) );
		
		//median of Onset fn	
		ofSetColor(205,0,0);
		Xindex = (onsetIndex-Xvalue) ;
		previousIndex = (Xindex-1);
		
		ofLine((int) (width*(amplitudeNumber - Xvalue - 1)), screenHeight - (scale_factor*(medianOnsetFunction[previousIndex]- minimumValue)), 
			   (int) (width*(amplitudeNumber - Xvalue)),  screenHeight - (scale_factor*(medianOnsetFunction[Xindex]- minimumValue)) );
		
		
		
		if (rawOnsetRecorded[Xindex] == true){
			ofSetColor(255,100,0);
			ofCircle(width*(amplitudeNumber - Xvalue), screenHeight - (scale_factor*(rawOnsetFunction[Xindex]- minimumValue)) , 3);
		}
		
		if (medianOnsetRecorded[Xindex] == true){
			ofSetColor(255,0,0);
			ofCircle(width*(amplitudeNumber - Xvalue), screenHeight - (scale_factor*(medianOnsetFunction[Xindex]- minimumValue)) , 3);
		}
		
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
	
}//end draw onset fn




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

