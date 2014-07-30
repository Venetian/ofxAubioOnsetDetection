
#ifndef AUBIOONSETDETECTOR_H
#define AUBIOONSETDETECTOR_H

#include "aubio.h"

class AubioOnsetDetector
{
public:		
	AubioOnsetDetector();
	~AubioOnsetDetector();
	void initialise();
	void resetValues();
	
	bool processframe(double* frame, const int& n);
	bool processframe(float* frame, const int& n);
	
	void onsetclass_energy();	
	void onsetclass_complex();
	void onsetclass_specdiff();
	void onsetclass_kl();
	void onsetclass_mkl();
	void onsetclass_hfc();
	void onsetclass_phase();
		
	bool checkForMedianOnset(const float& dfvalue);
	void postProcessing();
	
	float getRawDetectionFrame();
	float getPeakPickedDetectionFrame();
	double framesToSeconds(const float& frames);
//	double framesToMillis(const float& frames);
//	boolean getIsOnset();
	
	
	aubio_onsetdetection_t		*o;
	aubio_pvoc_t				*pv;
	aubio_pickpeak_t			*parms;
	
		
	fvec_t						*vec;
	fvec_t						*onset;
	cvec_t						*fftgrain;//complex vector of buffersize, 1
	
	bool aubioOnsetFound;//method used by Paul Brossier
	//see relevant papers - eg fast transients 2005
	int buffersize, hopsize, pos;
	float threshold,  threshold2;
	float rawDetectionValue, peakPickedDetectionValue;

	static const int numberOfDetectionValues = 16;
	float recentRawDetectionValues[numberOfDetectionValues];
	
	int recentValueIndex;
	
	float medianDetectionValue, aubioLongTermAverage ;
	bool anrMedianProcessedOnsetFound;//simple median method by Andrew Robertson
	//requires a fast rise, slower fall
	long lastMedianOnsetFrame, currentFrame;
	float thresholdRelativeToMedian;
	long cutoffForRepeatOnsetsMillis;
	float medianSpeed;
	float lastDfValue;
	float bestSlopeValue;
	double getBestSlopeValue(const float& dfvalue);
	double bestSlopeMedian;
	bool anrBestSlopeOnset;
	long lastSlopeOnsetFrame;
	bool slopeFallenBelowMedian;
	bool checkForSlopeOnset(const float& bestValue);
	
	void updateDetectionTriggerThreshold(const float& val);
	
	float detectionTriggerThreshold, detectionTriggerRatio;
	float maximumDetectionValue;
	
	
};

#endif