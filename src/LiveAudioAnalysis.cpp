/*
 *  AudioAnalysis.cpp
 *  audioFileLoader4
 *
 *  Created by Andrew Robertson on 14/08/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

#include "LiveAudioAnalysis.h"



LiveAudioAnalysis::LiveAudioAnalysis(){
	chromaG = new Chromagram;
	chromaG->initialise(FRAMESIZE, chromaHopSize);
	chromaG->maximumChromaValue = 0;
	
	scrollWidth = 400;
	
	onsetDetector = new AubioOnsetDetector();
	onsetDetector->initialise();
	aubioOnsetDetectionVectorMaximum = 0.01;
	chromaConversionFactor = chromaHopSize*4 / aubioBufferSize;//(hopsize of aubio is half the buffer size)
	printf("chroma conversion factor is initialised to %i\n", chromaConversionFactor);
	setChordNames();
	recentOnsetValue = 0;
	aubioMedianValue = 0;
	aubioCutoffForRepeatOnsetsMillis = 150;
	aubioLongTermAverage = 1;
	marginOfError = 10;
	btrack.initialise(FRAMESIZE);
	lastBeatTime = 0;
	medianSpeed = 6;;
	bpmEstimate = 120.0;
	
	aubioWindow.setWindowDimensions(0, 200, ofGetWidth(),  ofGetHeight()- 200);
	
	numberOfTempoToFitWindow = 80;

	
	printf("aubio window %f", aubioWindow.relativeHeight);
	//chromaConversionFactor = 16;
}

LiveAudioAnalysis::~LiveAudioAnalysis(){
	cout << "Live audio analysis destructor called" << endl;
	delete chromaG;
	delete onsetDetector;
	tempoData.clear();
	
	beatFrames.clear();
	chromaMatrix.clear();
	chromaMaximum.clear();
	energyVector.clear();
	aubioOnsetDetectionVector.clear();
	aubioOnsetFrameTimes.clear();
	aubioMedianVector.clear();
	rootChord.clear();
	beatSynchronisedChords.clear();
	
/*
	Q: are these deleted okay anyhow?
 
	delete tempoData;

	delete beatFrames;
	delete chromaMatrix;
	delete chromaMaximum;
	delete energyVector;
	delete aubioOnsetDetectionVector;
	delete aubioMedianVector;
	delete rootChord;
	delete beatSynchronisedChords;
	delete aubioOnsetFrameTimes;
 */
}

void LiveAudioAnalysis::clearVectors(){
	
	chromaMatrix.clear();
	chromaMaximum.clear();
	
	energyVector.clear();
	
	beatFrames.clear();
	btrack.reset();
	btrack.unfixtempo();
	
	tempoData.clear();
	
	aubioOnsetDetectionVector.clear();
	aubioOnsetFrameTimes.clear();
	aubioOnsetDetectionVectorMaximum = 0.01;
	aubioMedianVector.clear();
	aubioMedianValue = 0;
	
	rootChord.clear();
	
	chromaG->initialise(FRAMESIZE, chromaHopSize);//framesize 512 and hopsize 2048 
	chromaG->maximumChromaValue = 0;
	lastBeatTime = 0;
	
}



void LiveAudioAnalysis::processFrame(float* frame, int framesize){
	
	
	//do onset analysis - this gets onset detection function value
	//also stores detected onsets w.r.t. frame number
	double dfvalue = processAubioOnsetDetection(frame, framesize);
	
	recentOnsetValue = dfvalue;
	//do beat tracking
	btrack.process(dfvalue);
	
	if (btrack.playbeat == 1){//i.e. beat is detected now
		beatFrames.push_back(aubioOnsetDetectionVector.size()-1);
		recentBeat = aubioOnsetDetectionVector.size()-1;
		predictedNextBeat = aubioOnsetDetectionVector.size()-1 + beatPeriodEstimate;
		
		updateBeatPeriodEstimate();
		
		tempoData.push_back(bpmEstimate);
	}
	
	chromaG->processframe(frame);
	
	if (chromaG->chromaready)
	{
		DoubleVector d;
		//cout << "chroma[" << chromaMatrix.size() << "]" ;
		for (int i = 0;i<12;i++){
			//note - I prefer use of the raw chroma as it has energy information
			//but for visualisation purposes, the normalised chroma will look better
			//and that's what's used here. Just delete the max value part if you want
			//actual chroma values and not normalised ones
			d.push_back(chromaG->rawChroma[i] / chromaG->maximumChromaValue);	
			//	cout << ": " << chromaG->rawChroma[i];
		}	
		cout << endl;
		
		chromaMatrix.push_back(d);
		chromaMaximum.push_back(chromaG->maximumChromaValue);
		
		//chord detection method 
		chord.C_Detect(chromoGramm.chroma,chromoGramm.chroma_low);
		rootChord.push_back(chord.root);
		//end chord detection
		
	}//end if chromagRamm ready
	
	//	frameIndex++;
	
	//used to look at simple energy measurement, but this is covered in aubio onset
	//get energy of the current frame and wait
	//	double energyValue = getEnergyOfFrame(frame, framesize);
	//	energyVector.push_back(energyValue);
	
	
	
}



void LiveAudioAnalysis::drawAudioAnalysis(){
	ofSetColor(200,0,0);
/*
	//some old info about what is being recorded
	ofDrawBitmapString(ofToString((int)aubioOnsetDetectionVector.size()), 20,200);
	ofDrawBitmapString(ofToString((int)chromaMatrix.size()), 20,240);
	ofDrawBitmapString(ofToString(aubioOnsetDetectionVectorMaximum, 2), 20,280);
	ofDrawBitmapString(ofToString(recentOnsetValue, 2), 20,320);
*/
 ofSetHexColor(0xFF0066);
	drawVector(aubioOnsetDetectionVector, aubioOnsetDetectionVector.size()-1, aubioWindow);//-(scrollWidth/2)
//,  aubioOnsetDetectionVector.size()-1
	ofSetHexColor(0x006600);
	//drawVector(aubioMedianVector, (int)aubioOnsetDetectionVector.size()-1);//-(scrollWidth/2)
	drawVector(aubioMedianVector, (int)aubioOnsetDetectionVector.size()-1, aubioWindow);
	drawOnsetEvents(aubioOnsetDetectionVector.size()-1);
	
	

	ofSetHexColor(0x0000FF);
	drawBeatStripes(aubioOnsetDetectionVector.size()-1, aubioWindow);
	
}


void LiveAudioAnalysis::drawBeatTimesSeconds(DoubleVector& beatTimes){

	
	
	ofSetHexColor(0xFFFFFF);
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();  
	float heightFactor = 8;
	int i, j, startingFrame;
	
	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	
	startingFrame *= scrollWidth;
	
	string beatsString;
	
	
	int beatIndex = 0;
	while (beatIndex < beatTimes.size() && secondsToFrames(beatTimes[beatIndex]) < startingFrame)
		beatIndex++;
	
	
	//	printf("draw beats %i, %i, satrt frame %i, beat frame %f \n", (int) beatTimes.size(), beatIndex, startingFrame, beatFrame);
	
	for (;beatIndex < beatTimes.size() && secondsToFrames(beatTimes[beatIndex]) < startingFrame + scrollWidth;beatIndex++){
		double beatFrame = secondsToFrames(beatTimes[beatIndex]);
		beatsString += ofToString(beatFrame, 2)+"  \n";;
		ofLine((beatFrame - startingFrame) * screenWidth/scrollWidth, 0, (beatFrame - startingFrame) * screenWidth/scrollWidth, screenHeight);

	
		ofDrawBitmapString(chordNames[beatSynchronisedChords[beatIndex]], (beatFrame - startingFrame) * screenWidth/scrollWidth, 60);
		
	//	printf("draw beats %i, %i, %f\n", (int) beatTimes.size(), beatIndex, beatFrame);
	}
	ofDrawBitmapString(beatsString, 20, 20);		
	//printf("end beats %i\n",  beatIndex);
	
}

double LiveAudioAnalysis::secondsToFrames(float seconds){
	double frames = seconds * 44100. / aubioBufferSize;//1024.;
	return frames;
}

double LiveAudioAnalysis::framesToSeconds(float frames){
	double seconds = frames * aubioBufferSize / 44100.;
	return seconds;
}

double LiveAudioAnalysis::secondsToBPM(double seconds){
	return (60./seconds);
}

void LiveAudioAnalysis::getChordsForBeatTimes(DoubleVector& beatTimes){
	beatSynchronisedChords.clear();
	int i = 0;
	double energyFrame;
	while (i < beatTimes.size()){
		energyFrame = secondsToFrames(beatTimes[i]);
		energyFrame /= chromaConversionFactor;
		int frameToCheck = (int) energyFrame;//rounds down
		printf("beat time %f gives onset %f chroma %i\n", beatTimes[i], secondsToFrames(beatTimes[i]), frameToCheck);
		frameToCheck++;
		if (frameToCheck < rootChord.size())
			beatSynchronisedChords.push_back(rootChord[frameToCheck]);
		
		i++;
	}
	
}


void LiveAudioAnalysis::drawTempoCurve(WindowRegion window){
	ofSetColor(0,0,0);

	int startIndex = (int)(tempoData.size()/numberOfTempoToFitWindow);
	startIndex *= numberOfTempoToFitWindow;
	if (tempoData.size() > startIndex){
	int endIndex = tempoData.size();					 
	double screenFraction	= window.width / numberOfTempoToFitWindow;	
	double maxTempo = 160;
	double lastTempoHeight = window.y + window.height * (maxTempo - tempoData[startIndex])/maxTempo;
	double tempoHeight = lastTempoHeight;
	int lastX = window.x;
	int nextX = lastX;
	for (int i = startIndex;i < endIndex;i++){
		if (i >= 1){
			nextX = (int)(window.x + screenFraction*(i-startIndex));
			tempoHeight = window.y + window.height * (maxTempo - tempoData[i])/maxTempo;	
			ofLine(lastX, lastTempoHeight, nextX, tempoHeight);
			lastTempoHeight = tempoHeight;
			lastX = nextX;
		}
	}
	}//end if size >  0 
	 
	 
}


void LiveAudioAnalysis::drawEnergyVectorFromPointer(){
	DoubleVector* energyVec;
	energyVec = &energyVector;
	//xxx above
	
	ofSetHexColor(0xFF0066);
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();  
	float heightFactor = 8;
	int i, j, startingFrame;
	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;
	
	for (i = 0; i < scrollWidth - 1; i++){
		j = i + startingFrame;
		if (j < (*energyVec).size())
			ofLine(i*screenWidth/scrollWidth, screenHeight - ((*energyVec)[j]*screenHeight/heightFactor),
				   screenWidth*(i+1)/scrollWidth, screenHeight - ((*energyVec)[j+1]*screenHeight/heightFactor));
		
	}
}



void LiveAudioAnalysis::drawBeatStripes(int minIndex){
	drawBeatStripes(minIndex, fullScreen);

}


void LiveAudioAnalysis::drawBeatStripes(int minIndex, WindowRegion window){

	float screenWidth = window.width;//ofGetWidth();  
	float screenHeight = window.height;//ofGetHeight() ;

	float heightFactor = 8;
	int i, j, startingFrame;
	startingFrame = minIndex / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;
	
	int beatIndex = 0;
	while (beatIndex < beatFrames.size() && beatFrames[beatIndex] < startingFrame)
		beatIndex++;
	
	
	
	for (int j = 0;beatIndex + j < beatFrames.size() && beatFrames[beatIndex + j] < startingFrame + scrollWidth;j++){
		int beatStripePosition = beatFrames[beatIndex + j];
		int screenPosition = window.x + (beatStripePosition - startingFrame)*screenWidth/scrollWidth;
		ofLine(window.x+screenPosition, window.y, window.x+screenPosition, window.y+screenHeight);
		if (beatIndex + j - 1 >= 0)
			ofDrawBitmapString(ofToString(beatFrames[beatIndex+j] - beatFrames[beatIndex+j-1]), window.x+screenPosition+ 5, window.y+30);
		
		if (beatIndex + j > 3){
			double bpm = framesToSeconds((beatFrames[beatIndex + j] - beatFrames[beatIndex+j-4])/4.);//
			bpm = secondsToBPM(bpm);
			ofDrawBitmapString(ofToString(bpm, 2), window.x+screenPosition+ 5, window.y+90);
		}
		
	}
	
	if ((aubioOnsetDetectionVector.size() - lastBeatTime) < 5){
		ofSetHexColor(0x0000DD);
		ofCircle(window.x+screenWidth/2, window.y+20, 20);
	}
	
	
	
}




void LiveAudioAnalysis::drawOnsetEvents(int minIndex){
	
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();  
	float heightFactor = 8;
	int i, j, startingFrame;
	startingFrame = minIndex / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;
	
	int onsetIndex = aubioOnsetFrameTimes.size()-1;
	while (onsetIndex > 0 && aubioOnsetFrameTimes[onsetIndex] > startingFrame)
		onsetIndex--;
	
	
	
	for (int j = 0;onsetIndex + j < aubioOnsetFrameTimes.size() && aubioOnsetFrameTimes[onsetIndex + j] < startingFrame + scrollWidth;j++){
		int onsetPosition = aubioOnsetFrameTimes[onsetIndex + j];
		int screenPosition = (onsetPosition - startingFrame)*screenWidth/scrollWidth;
		ofCircle(screenPosition, screenHeight - 30, 6);
				
	}
		
}






void LiveAudioAnalysis::drawVector(DoubleVector& energyVec, int minIndex){//, int maxIndex){

	drawVector(energyVec, minIndex, fullScreen);

}



void LiveAudioAnalysis::drawVector(DoubleVector& energyVec, int minIndex, WindowRegion& window){
	
	minIndex = max(0, minIndex);
	
	
	
	float screenHeight = window.height;//ofGetHeight() ;
	float screenWidth = window.width;//ofGetWidth();  
	float heightFactor = aubioOnsetDetectionVectorMaximum;
	int i, j, startingFrame;
	
	startingFrame = minIndex / scrollWidth;
	//version for static loaded audio is:
	//	currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;
	int lastHeight = 0;
	int newHeight;
	int xPosition;
	for (i = 0; i < scrollWidth - 1; i++){
		j = i + startingFrame;
		
		if (j < energyVec.size()){
			xPosition = window.x+i*screenWidth/scrollWidth;
			newHeight =	window.y + screenHeight - (energyVec[j]*screenHeight/heightFactor);
		
			//draw the tops
			ofLine(xPosition+window.x, window.y + screenHeight - (energyVec[j]*screenHeight/heightFactor),
				   window.x+screenWidth*(i+1)/scrollWidth, window.y + screenHeight - (energyVec[j]*screenHeight/heightFactor));

			ofLine(window.x+xPosition,lastHeight, window.x+xPosition, newHeight);
			lastHeight = newHeight;
		}
	}
}


void LiveAudioAnalysis::drawSpectralDifference(){
	DoubleMatrix* dMatrix;
	dMatrix = &chromaMatrix;
	//get rid of these!
	
	int matrixSize = (*dMatrix).size();
	if (matrixSize > 0){
		
		float screenHeight = ofGetHeight() ;
		float screenWidth = ofGetWidth();
		float heightFactor = 8;
		double difference;
		int i, j, startingFrame;
		startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
		startingFrame *= scrollWidth;//starting frame in terms of energy frames
		startingFrame /= chromaConversionFactor; //in terms of chroma frames
		
		float chromoLength = scrollWidth/chromaConversionFactor;
		for (i = 1; i < chromoLength; i++){//changed to add 1
			j = i + startingFrame;
			for (int y = 0;y < 12;y++){			
				
				if (j < matrixSize)
					difference = (*dMatrix)[j][11-y] - (*dMatrix)[j-1][11-y];
				else 
					difference = 0;
				
				if (difference < 0)
					difference = 0;//half wave rectify
				
				ofSetColor(0,0,255 * difference);//, 0;
				ofRect(i*screenWidth/chromoLength,y*screenHeight/12,screenWidth/chromoLength,screenHeight/12);
			}//end y
		}//end i
		
	}///end if matrix has content
	else{
		printf("Error - please load audio first");
	}
	
}



void LiveAudioAnalysis::drawDoubleMatrix(DoubleMatrix* dMatrix, int minIndexInFrames){
	//used to draw the chromagram matrix
	int matrixSize = (*dMatrix).size();
	if (matrixSize > 0){
		
		
		float screenHeight = ofGetHeight() ;
		float screenWidth = ofGetWidth();
		float heightFactor = 8;
		int i, j, startingFrame;
		//note this takes aubioOnsetDetectionLength as frames
		//then scales it to be chroma
		startingFrame = minIndexInFrames / scrollWidth;//i.e. number of scroll widths in
		startingFrame *= scrollWidth;//starting frame in terms of energy frames
	//	printf("chroma conv is %i\n", chromaConversionFactor);
		startingFrame /= chromaConversionFactor; //in terms of chroma frames
		
		float chromoLength = scrollWidth/chromaConversionFactor ;
		for (i = 0; i < chromoLength; i++){
			j = i + startingFrame;
			for (int y = 0;y < 12;y++){
				
				if (j < matrixSize)
					ofSetColor(0,0,255 * (*dMatrix)[j][11-y]);
				else 
					ofSetColor(0,0,0);
				
				ofRect(i*screenWidth/chromoLength,y*screenHeight/12,screenWidth/chromoLength,screenHeight/12);
			}//end y
			//print chord labels
			string chordName = ofToString(rootChord[j], 0);
			ofSetColor(200,255,0);
			ofDrawBitmapString(chordName,i*screenWidth/chromoLength,20);
			
		}//end i
		
	}///end if matrix has content
	else{
		printf("Error - please load audio first");
	}
	
	
}

void LiveAudioAnalysis::updateBeatPeriodEstimate(){
	
	//could use the beatPeriod estimate from btrack
	//but this changes slowly/reluctantly
	if (beatFrames.size() > 1){
	double newObservation = beatFrames[beatFrames.size()-1] - beatFrames[beatFrames.size()-2];
	
	double proportionOfNewValue = 0.3;
	beatPeriodEstimate *= (1 - proportionOfNewValue);
		beatPeriodEstimate += proportionOfNewValue * newObservation;//(btrack.anrBeatPeriodEstimate);
	bpmEstimate = secondsToBPM(framesToSeconds(beatPeriodEstimate));
	}else{//if at v v start
		beatPeriodEstimate = btrack.anrBeatPeriodEstimate;//the estimate in btrack
	}
	//using this gives a more 
	//beatFrames[beatFrames.size()-1] - beatFrames[beatFrames.size()-2] )

}



void LiveAudioAnalysis::setNewTempoAtTime(long timeMillis){
	btrack.setTempoByTapping(timeMillis);
	
	int frameNow = getCurrentOnsetFrame();
	double frameDifference = frameNow - beatFrames[beatFrames.size()-1];
	if (frameDifference > 0.5*beatPeriodEstimate){
		beatFrames.push_back(aubioOnsetDetectionVector.size()-1);
	}
}


int LiveAudioAnalysis::getCurrentOnsetFrame(){
	return aubioOnsetDetectionVector.size()-1;
}


void LiveAudioAnalysis::printAudioData(){
	
	printf("Max chroma value is %f \n", chromaG->maximumChromaValue);

	//normalise
	int length = chromaMatrix.size();
	printf("length of chromagram is %d frames and aubioOnset is %i\n", length, (int) aubioOnsetDetectionVector.size());
	length = (chromaMatrix[0]).size();
	printf("height of dmatrix is %d\n", length);
	
	for (int i = 0; i < chromaMatrix.size();i++){
		for (int j = 0; j < (chromaMatrix[0]).size();j++){
			chromaMatrix[i][j] /= chromaG->maximumChromaValue;	
		}
	}
  
	printf("size of energy vector is %d \n", (int) energyVector.size());
	//chromaConversionFactor = (int)round( energyVector.size() /  chromaMatrix.size());
	
	printf("after calculation, chroma conversionFactor set to %i\n", chromaConversionFactor);
	
	printf("NOW PRINTING BEAT TIMES\n");
	printBeatTracking();
}

double LiveAudioAnalysis::getEnergyOfFrame(float* frame, int framesize){
	
	float totalEnergyInFrame = 0;
	
	for (int i = 0;i<framesize;i++){
		
		totalEnergyInFrame += (frame[i] * frame[i]);
		
	}
	totalEnergyInFrame = sqrt(totalEnergyInFrame);
	
	return totalEnergyInFrame;
}



double LiveAudioAnalysis::processAubioOnsetDetection(float* audioFrame, int framesize){
	double dfvalue = 0;
	
	bool onsetDetected = false;
//	aubioBufferSize
	if (onsetDetector->processframe(audioFrame, framesize)){
		
		onsetDetected = onsetDetector->aubioOnsetFound;
		
		aubioOnsetDetectionVector.push_back(onsetDetector->rawDetectionValue);
		dfvalue = onsetDetector->rawDetectionValue;
		
		if (onsetDetector->rawDetectionValue > aubioOnsetDetectionVectorMaximum)
			aubioOnsetDetectionVectorMaximum = onsetDetector->rawDetectionValue;
			
		//check for onset relative to our rising and falling median threshold
		if (dfvalue > aubioMedianValue * 1.05 && 
			dfvalue > aubioLongTermAverage &&
			(aubioOnsetFrameTimes.size() == 0 || 
			1000*framesToSeconds((aubioOnsetDetectionVector.size() - aubioOnsetFrameTimes[aubioOnsetFrameTimes.size()-1])) > aubioCutoffForRepeatOnsetsMillis)
			){
			aubioOnsetFrameTimes.push_back(aubioOnsetDetectionVector.size()-1);//the current frame
			}
			
		aubioLongTermAverage *= 0.999;
		aubioLongTermAverage += 0.001*(dfvalue - aubioLongTermAverage);
		
		if (dfvalue > aubioMedianValue)
			aubioMedianValue = dfvalue;
		else 
			aubioMedianValue += 0.01*medianSpeed*(dfvalue - aubioMedianValue);
		
		aubioMedianVector.push_back(aubioMedianValue);
	}
	//printf("aubio val %f\n", dfvalue);
	//returns onset analysis according to Paul brossier's aubio onset detection method
	//we also store the raw onset detection function result
	//using methods recommended by Bello et al. (2005)
	return dfvalue;
}


void LiveAudioAnalysis::printBeatTracking(){
	for (int i = 0;i < beatFrames.size();i++){
		printf("onset[%i] : %i\n", i, beatFrames[i]);
	}

}

void LiveAudioAnalysis::setChordNames(){
	chordNames[0] = "C";
	chordNames[1] = "C#";
	chordNames[2] = "D";
	chordNames[3] = "D#";	
	chordNames[4] = "E";	
	chordNames[5] = "F";	
	chordNames[6] = "F#";	
	chordNames[7] = "G";
	chordNames[8] = "G#";
	chordNames[9] = "A";
	chordNames[10] = "A#";
	chordNames[11] = "B";

}

void LiveAudioAnalysis::windowResized(int w, int h){
	fullScreen.resized(w, h);
	aubioWindow.resized(w, h);

}


