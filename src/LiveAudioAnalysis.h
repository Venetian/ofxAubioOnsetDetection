/*
 *  LiveAudioAnalysis.h
 *  audioFileLoader4
 *
 *  Created by Andrew Robertson on 14/08/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

#ifndef LIVE_AUDIO_ANALYSIS_H
#define LIVE_AUDIO_ANALYSIS_H


#include "fftw3.h"
#include "ofMain.h"
#include "chromaGram.h"
#include "ChordDetect.h"
#include "sndfile.h"
#include "AubioOnsetDetector.h"
#include "OnsetDetectionFunction.h"
#include "BTrack_mod.h"
#include "WindowRegion.h"
#include "PlotFunction.h"

#define FRAMESIZE 512

//this does a chromagram analysis and aubio onset analysis
//held in double matrix and doubleVector respectively
//these are dynamic vectors, so size set by what's needed for the file



class LiveAudioAnalysis{

public:
	
	LiveAudioAnalysis();
	~LiveAudioAnalysis();
	
	typedef std::vector<double> DoubleVector;
	typedef std::vector<int> IntVector;
	
	typedef std::vector<DoubleVector> DoubleMatrix;
	
	void clearVectors();
	void drawAudioAnalysis();
	
	DoubleMatrix chromaMatrix;
	DoubleVector chromaMaximum;
	DoubleVector energyVector;
	DoubleVector aubioOnsetDetectionVector;
	DoubleVector aubioMedianVector;
	double aubioMedianValue;
//	DoubleVector aubioEnvelopeFollower;
	IntVector rootChord;
	IntVector beatSynchronisedChords;
	IntVector aubioOnsetFrameTimes;
	
	
	static const int aubioBufferSize = 512;//256;
	static const int chromaBufferSize = 8192;
	static const int chromaHopSize = 2048;
	
	int chromaConversionFactor;
	
//	AudioFile* audioHolder;
	void loadNewAudio(string soundFileName);//??
	
	Chromagram chromoGramm;
	int currentPlayingFrame;
		
	ChordDetect chord;
	string chordNames[12];
	void setChordNames();
	
	double secondsToFrames(float seconds);
	double framesToSeconds(float frames);
	double secondsToBPM(double seconds);
	void drawBeatTimesSeconds(DoubleVector& beatTimes);
	void getEnergyOfFrame(float* frame);
	
	void getChordsForBeatTimes(DoubleVector& beatTimes);
	
	double getEnergyOfFrame();
	int scrollWidth;
	//int totalFrames;
	void drawEnergyVectorFromPointer();
	void drawSpectralDifference();
	void drawDoubleMatrix(DoubleMatrix* dMatrix, int minIndexInFrames);
	
	void processFrame(float* frame, int framesize);
	double processAubioOnsetDetection(float* audioFrame, int framesize);
	void setNewTempoAtTime(long timeMillis);
	int getCurrentOnsetFrame();
	
	void printAudioData();
	double getEnergyOfFrame(float* frame, int framesize);
	AubioOnsetDetector			*onsetDetector;
	
	void drawVector(DoubleVector& energyVec, int minIndex);//, int maxIndex
	void drawVector(DoubleVector& energyVec, int minIndex, WindowRegion& window);//overloaded for window drawing
	void drawTempoCurve(WindowRegion window);
	double aubioOnsetDetectionVectorMaximum;
	double recentOnsetValue;
	
	Chromagram* chromaG;
	
	OnsetDetectionFunction onset;
	BTrack btrack;
	IntVector beatFrames;
	void printBeatTracking();
	void drawBeatStripes(int minIndex);
		void drawBeatStripes(int minIndex, WindowRegion window);
	void drawOnsetEvents(int minIndex);
	
	double medianSpeed;
	int aubioCutoffForRepeatOnsetsMillis ;
	double aubioLongTermAverage;
	int lastBeatTime;
	double bpmEstimate;
	
	DoubleVector tempoData;
	
	WindowRegion aubioWindow;
		WindowRegion fullScreen;
	void windowResized(int w, int h);
		double numberOfTempoToFitWindow;
	
	void updateBeatPeriodEstimate();
	double beatPeriodEstimate;
	double predictedNextBeat;
	double recentBeat;
	double marginOfError;
	
	PlotFunction plotter;
	
};
#endif



