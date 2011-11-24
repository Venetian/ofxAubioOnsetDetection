/*
 *  AubioOnsetDetector.cpp
 *  aubioonset~
 *
 *  Created by Andrew Robertson on 13/08/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "AubioOnsetDetector.h"

AubioOnsetDetector :: AubioOnsetDetector(){
	buffersize = 1024;
	hopsize = 512;
		//aubio related setup
		o = new_aubio_onsetdetection(aubio_onset_complex, buffersize, 1);//initially in complex mode
		pv = (aubio_pvoc_t *)new_aubio_pvoc(buffersize, hopsize, 1);
		parms = new_aubio_peakpicker(threshold);
		vec = (fvec_t *)new_fvec(hopsize,1);
		
		threshold = 1;
		threshold2 = -70.;

	resetValues();
	thresholdRelativeToMedian = 1.1;
	cutoffForRepeatOnsetsMillis = 100;
	medianSpeed = 15;
	pos = 0;
	
	detectionTriggerRatio = 0.5f;
	detectionTriggerThreshold = 10;
}

AubioOnsetDetector :: ~AubioOnsetDetector(){
		aubio_onsetdetection_free (o);
		
}

void AubioOnsetDetector :: resetValues(){
	rawDetectionValue = 1;
	peakPickedDetectionValue = 1;
	medianDetectionValue = 10.;
	lastMedianOnsetFrame = 0;
	currentFrame = 0;
	aubioLongTermAverage = 10.;
	lastDfValue = 0;
	bestSlopeValue = 0;
	recentValueIndex = 0;
	lastSlopeOnsetFrame = 0;
	bestSlopeMedian = 10;
	slopeFallenBelowMedian = true;
	maximumDetectionValue = 10.0;
	
	for (int i = 0;i< numberOfDetectionValues;i++)
		recentRawDetectionValues[i] = 1;
	
}


void AubioOnsetDetector :: initialise(){
	//reinitialises our object
		o = new_aubio_onsetdetection(aubio_onset_complex, buffersize, 1);//initially in complex mode
		pv = (aubio_pvoc_t *)new_aubio_pvoc(buffersize, hopsize, 1);
		parms = new_aubio_peakpicker(threshold);
		vec = (fvec_t *)new_fvec(hopsize,1);
		pos = 0;
				fvec_write_sample(vec, 0.234, 0, pos);
				fftgrain  = (cvec_t *)new_cvec(buffersize,1);
				onset = (fvec_t *)new_fvec(1,1);
		}



bool AubioOnsetDetector :: processframe(double* frame, const int& n){
	bool newFrameResult = false;
	//Paul Brossier's aubioonsetclass~ code ported from Pd
		int j,isonset;
	for (j=0;j<n;j++) {
		
		// write input to datanew 
		fvec_write_sample(vec, frame[j], 0, pos);//vec->data[0][pos] = frame[j]
		//time for fft
	
		if (pos == hopsize-1) {  //hopsize is 512       
			newFrameResult = true;
			aubioOnsetFound = false;
		
			 // block loop 
			aubio_pvoc_do (pv,vec, fftgrain);
		
			fftgrain->norm[0][0] = fabs(fftgrain->norm[0][0]);
			//added hack to solve bug that norm[0][0] is negative sometimes.
			
			aubio_onsetdetection(o, fftgrain, onset);
			rawDetectionValue = onset->data[0][0];
		//Paul Brossier's method to return value of peak picking process
		
				
			postProcessing();
				
	//	smpl_t my_sample_value;
	peakPickedDetectionValue = aubio_peakpick_pimrt_getval(parms); 
	//peakPickedDetectionValue = my_sample_value;

		
			isonset = aubio_peakpick_pimrt(onset,parms);
			if (isonset) {
				// test for silence 
				if (aubio_silence_detection(vec, threshold2)==1){
					isonset=0;
					}
				else{
	//				outlet_bang(x->bangoutlet);
					aubioOnsetFound = true;
					}
			}//end if (isonset)
		
			
			
			// end of block loop 
			pos = -1; // so it will be zero next j loop 
		}
		pos++;
	
	}//end for j 
//end of Paul's code

return newFrameResult;
		
}


bool AubioOnsetDetector :: processframe(float* frame, const int& n){
	bool newFrameResult = false;
	//Paul Brossier's aubioonsetclass~ code ported from Pd
	int j,isonset;
	for (j=0;j<n;j++) {
		
		// write input to datanew 
		fvec_write_sample(vec, frame[j], 0, pos);//vec->data[0][pos] = frame[j]
		//time for fft
		
		if (pos == hopsize-1) {  //hopsize is 512       
			newFrameResult = true;
			aubioOnsetFound = false;
			
			// block loop 
			aubio_pvoc_do (pv,vec, fftgrain);
			
			fftgrain->norm[0][0] = fabs(fftgrain->norm[0][0]);
			//added hack to solve bug that norm[0][0] is negative sometimes.
			
			aubio_onsetdetection(o, fftgrain, onset);
			rawDetectionValue = onset->data[0][0];
			//Paul Brossier's method to return value of peak picking process
			
			
			postProcessing();
			
			//	smpl_t my_sample_value;
			//peakPickedDetectionValue = aubio_peakpick_pimrt_getval(parms); 
			//peakPickedDetectionValue = my_sample_value;
			
			//Paul Brossier's onset detection method
			isonset = aubio_peakpick_pimrt(onset,parms);
			if (isonset) {
				// test for silence 
				if (aubio_silence_detection(vec, threshold2)==1){
					isonset=0;
				}
				else{
					//				outlet_bang(x->bangoutlet);
					aubioOnsetFound = true;
				}
			}//end if (isonset)
			
			
			
			// end of block loop 
			pos = -1; // so it will be zero next j loop 
		}
		pos++;
		
	}//end for j 
	//end of Paul's code
	
	return newFrameResult;
	
}


 
 void AubioOnsetDetector :: postProcessing(){
	 //anrMedia processed has conditions below
	 anrMedianProcessedOnsetFound = checkForMedianOnset(rawDetectionValue);
	 
	 //the slope idea is we need both a high rise and a high / steep slope and we
	 //look for both of these
	 bestSlopeValue = getBestSlopeValue(rawDetectionValue); 
	 anrBestSlopeOnset = checkForSlopeOnset(bestSlopeValue);
 }
 
bool AubioOnsetDetector :: checkForMedianOnset(const float& dfvalue){
	 bool onsetDetected = false;
	 //check for onset relative to our rising and falling median threshold
	
	 if (dfvalue > medianDetectionValue * thresholdRelativeToMedian && 
		 dfvalue > aubioLongTermAverage &&
		 //lastDfValue < medianDetectionValue &&
		 1000*framesToSeconds(currentFrame - lastMedianOnsetFrame) > cutoffForRepeatOnsetsMillis){
		
		onsetDetected = true;
		lastMedianOnsetFrame = currentFrame;
		//printf("frame diff between onsets %6.1f", (1000*framesToSeconds(currentFrame - lastMedianOnsetFrame)) );
	 }
 
//	aubioLongTermAverage *= 0.995;
	aubioLongTermAverage += 0.01*(dfvalue - aubioLongTermAverage);
 
	if (dfvalue > medianDetectionValue)
		medianDetectionValue = dfvalue;
	else 
		medianDetectionValue += 0.01*medianSpeed*(dfvalue - medianDetectionValue);
 
	if (dfvalue > maximumDetectionValue)
		maximumDetectionValue = dfvalue;
 
 
 currentFrame++;
 lastDfValue = dfvalue;
 
 
return onsetDetected;
}
 
 
 
 double AubioOnsetDetector::getBestSlopeValue(const float& dfvalue){
 //the idea is we want a high slope
 recentRawDetectionValues[recentValueIndex] = dfvalue;
 double bestValue = 0;
 for (int i = 1;i < numberOfDetectionValues;i++){
 double angle = 0;
 int otherIndex = (recentValueIndex - i + numberOfDetectionValues)%numberOfDetectionValues;
 double testValue = 0;
 if (otherIndex > 0 && recentRawDetectionValues[otherIndex] > 0){
 angle = atan((float)(i * dfvalue)/ (numberOfDetectionValues*(dfvalue-recentRawDetectionValues[otherIndex])) );
 testValue = (dfvalue - recentRawDetectionValues[otherIndex]) * cos(angle);
 }
 
 if (testValue > bestValue)
 bestValue = testValue;
 }
 
 recentValueIndex++;
 
 if (recentValueIndex == numberOfDetectionValues)
 recentValueIndex = 0;
 
 
 return bestValue;
 
 }
 
 
 
 
 bool AubioOnsetDetector :: checkForSlopeOnset(const float& bestValue){
 bool onsetDetected = false;
 //check for onset relative to our processed slope function
 //a mix between increase in value and the gradient of that increase
 
 if (bestValue > bestSlopeMedian * thresholdRelativeToMedian && //better than recent average 
 1000*framesToSeconds(currentFrame - lastSlopeOnsetFrame) > cutoffForRepeatOnsetsMillis //after cutoff time
 && slopeFallenBelowMedian // has had onset and fall away again
 && bestValue > detectionTriggerThreshold * detectionTriggerRatio //longer term ratio of winning onsets 
 ){
 //	printf("frame diff between onsets %6.1f", (1000*framesToSeconds(currentFrame - lastMedianOnsetFrame)) );
 onsetDetected = true;
 lastSlopeOnsetFrame = currentFrame;
 slopeFallenBelowMedian = false;
 
 updateDetectionTriggerThreshold(bestValue);
 }
 
 
 if (bestValue > bestSlopeMedian)
 bestSlopeMedian += (bestValue - bestSlopeMedian)*0.02;//was 1.1
 else{
 bestSlopeMedian *= 0.99;
 slopeFallenBelowMedian = true;;
 }
 return onsetDetected;
 }
 
 
 void AubioOnsetDetector::updateDetectionTriggerThreshold(const float& val){
 float detectionAdaptSpeed = 0.05;//moving average, roughly last twenty onsets
 detectionTriggerThreshold *= 1- detectionAdaptSpeed;
 detectionTriggerThreshold += (val * detectionAdaptSpeed);
 }
 

double  AubioOnsetDetector::framesToSeconds(float frames){
	double seconds = frames * buffersize / 44100.;
	return seconds;
}

float AubioOnsetDetector :: getRawDetectionFrame(){
return rawDetectionValue;	
}

float AubioOnsetDetector :: getPeakPickedDetectionFrame(){
return peakPickedDetectionValue;	
}


void AubioOnsetDetector :: onsetclass_energy(){
		//aubio_onsetdetection_type
		aubio_onsetdetection_free (o);
		o = new_aubio_onsetdetection(aubio_onset_energy, buffersize, 1);
}

void AubioOnsetDetector :: onsetclass_hfc(){
/** High Frequency Content onset detection function
 
  This method computes the High Frequency Content (HFC) of the input spectral
  frame. The resulting function is efficient at detecting percussive onsets.

  Paul Masri. Computer modeling of Sound for Transformation and Synthesis of
  Musical Signal. PhD dissertation, University of Bristol, UK, 1996.*/
		aubio_onsetdetection_free (o);
		o = new_aubio_onsetdetection(aubio_onset_hfc, buffersize, 1);
}


void AubioOnsetDetector :: onsetclass_complex(){
		//aubio_onsetdetection_type
		//Complex Domain Method onset detection function 
		//Christopher Duxbury, Mike E. Davies, and Mark B. Sandler. Complex domain
		//onset detection for musical signals. In Proceedings of the Digital Audio
		//Effects Conference, DAFx-03, pages 90-93, London, UK, 2003.
		aubio_onsetdetection_free (o);
		o = new_aubio_onsetdetection(aubio_onset_complex, buffersize, 1);
}

void AubioOnsetDetector :: onsetclass_phase(){
/** Phase Based Method onset detection function 

  Juan-Pablo Bello, Mike P. Davies, and Mark B. Sandler. Phase-based note onset
  detection for music signals. In Proceedings of the IEEE International
  Conference on Acoustics Speech and Signal Processing, pages 441¬≠444,
  Hong-Kong, 2003.*/
  		aubio_onsetdetection_free (o);
		o = new_aubio_onsetdetection(aubio_onset_phase, buffersize, 1);
		
}

void AubioOnsetDetector :: onsetclass_specdiff(){
		/* Spectral difference method onset detection function 
		Jonhatan Foote and Shingo Uchihashi. The beat spectrum: a new approach to
		rhythm analysis. In IEEE International Conference on Multimedia and Expo
		(ICME 2001), pages 881¬≠884, Tokyo, Japan, August 2001.
		*/
		//aubio_onsetdetection_type
		aubio_onsetdetection_free (o);
		o = new_aubio_onsetdetection(aubio_onset_specdiff, buffersize, 1);
}

void AubioOnsetDetector :: onsetclass_kl(){
		/** Kullback-Liebler onset detection function 
  
			Stephen Hainsworth and Malcom Macleod. Onset detection in music audio
			signals. In Proceedings of the International Computer Music Conference
			(ICMC), Singapore, 2003.
		*/
		aubio_onsetdetection_free (o);
		o = new_aubio_onsetdetection(aubio_onset_kl, buffersize, 1);
}

void AubioOnsetDetector :: onsetclass_mkl(){

		/** Modified Kullback-Liebler onset detection function 

		Paul Brossier, ``Automatic annotation of musical audio for interactive
		systems'', Chapter 2, Temporal segmentation, PhD thesis, Centre for Digital
		music, Queen Mary University of London, London, UK, 2003.*/		
		aubio_onsetdetection_free (o);
		o = new_aubio_onsetdetection(aubio_onset_hfc, buffersize, 1);
}

