#include "testApp.h"



const double sampofxPortaudioSoundStreamlingFrequency = 44100.0;
	
//--------------------------------------------------------------
void testApp::setup(){

	/*
	 soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
	 */

	aubioOnsetDetect = new ofxAubioOnsetDetection();
	
	sampleIndex = 0;
	outputIsSameAsInput = true;
	
	infoString = "this shows two sound streams being used\npress 's' to unpause the audio\n'e' to pause the audio\n";
	infoString += "'i' to switch from using input and using sine wave for output stream";
	
	volHistory.assign(400, 0.0);
	bufferCounter	= 0;
	drawCounter		= 0;
	smoothedVol     = 0.0;
	scaledVol		= 0.0;
	
	setupFinished = false;
	ofBackground(255,255,255);

	bufferSize = 512;//important for the beat tracker analysis - this could be improved.

	leftAudioIn.assign(bufferSize, 0.0);
	rightAudioIn.assign(bufferSize, 0.0);
	
	leftAudioOut.assign(bufferSize, 0.0);
	rightAudioOut.assign(bufferSize, 0.0);

	
	//setting up input and output streams
	//	inputSoundStream.listDevices();	
	
	//ofxSoundStream - class version
	cout << "Input process setting up input with instance of ofxPortaudioSoundstream ..." << endl;
	inputDeviceID = 1;//the mic(0)/line input(1) of a mac book pro
	outputDeviceID = 2;//the headphones out of a macbook pro
	setInputDevice(inputDeviceID);//includes calling the setup routine etc
	ofAddListener(inputSoundStream.audioReceivedEvent, this, &testApp::audioInputListener);
	
	
	cout << "Output process, setting up output with instance of ofxPortaudioSoundstream ..." << endl;
	setOutputDevice(outputDeviceID);
	ofAddListener(outputSoundStream.audioRequestedEvent, this, &testApp::audioOutputListener);

	
	bufferCounter = 0;
	ofSetFrameRate(40);
	
	//loading audio file for click sound
	const char	*clickfilename = "cow2.wav";
	clickSound.loadSound(clickfilename);
	clickSound.setVolume(0.1);

	
	verdana30.loadFont("verdana.ttf", 30, true, true);
	verdana30.setLineHeight(34.0f);
	verdana30.setLetterSpacing(1.035);
	
	verdana14.loadFont("verdana.ttf", 14, true, true);
	verdana14.setLineHeight(14.0f);
	verdana14.setLetterSpacing(1.035);
	
	setupFinished = true;
	showingInputDevices = false;
	showingOutputDevices = false;
	
	deviceListHeightDistance = 30;
	deviceListHeightOffset = 20;
	

	
	
}



//--------------------------------------------------------------
void testApp::update(){
	//lets scale the vol up to a 0-1 range 
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
	
	//lets record the volume into an array
	volHistory.push_back( scaledVol );
	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofSetColor(0);
	
	verdana14.drawString("input: "+inputSoundStream.deviceName, 20, 20);
	verdana14.drawString("output: "+outputSoundStream.deviceName, 20, 60);
	
	if (showingInputDevices)
	showDeviceListWindow(inputSoundStream);
	
	if (showingOutputDevices)
		showDeviceListWindow(outputSoundStream);
	
	drawAudioInput();
	
	//aubioOnsetDetect->drawOnsetDetection();
}

void testApp::drawAudioInput(){
	
	ofSetColor(125);

	ofDrawBitmapString(infoString, 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(32, 170, 0);
	
	ofDrawBitmapString("Input Stream", 4, 18);
	
	ofSetLineWidth(1);	
	ofRect(0, 0, 512, 200);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);
	
	ofBeginShape();
	for (int i = 0; i < leftAudioIn.size(); i++){
		ofVertex(i, 100 -leftAudioIn[i]*100.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	// draw the right channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(32, 370, 0);
	
	ofSetColor(125);
	ofDrawBitmapString("Output Stream", 4, 18);
	
	ofSetLineWidth(1);	
	ofRect(0, 0, 512, 200);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);
	
	ofBeginShape();
	for (int i = 0; i < leftAudioOut.size(); i++){
		ofVertex(i, 100 - leftAudioOut[i]*100.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	// draw the average volume:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(565, 170, 0);
	
	ofSetColor(125);
	ofDrawBitmapString("Scaled average vol (0-100): " + ofToString(scaledVol * 100.0, 0), 4, 18);
	ofRect(0, 0, 400, 400);
	
	ofSetColor(245, 58, 135);
	ofFill();		
	ofCircle(200, 200, scaledVol * 190.0f);
	
	//lets draw the volume history as a graph
	ofBeginShape();
	for (int i = 0; i < volHistory.size(); i++){
		if( i == 0 ) ofVertex(i, 400);
		
		ofVertex(i, 400 - volHistory[i] * 70);
		
		if( i == volHistory.size() -1 ) ofVertex(i, 400);
	}
	ofEndShape(false);		
	
	ofPopMatrix();
	ofPopStyle();
	
	drawCounter++;
	
	ofSetColor(125);
	string reportString = "buffers received: "+ofToString(bufferCounter)+"\ndraw routines called: "+ofToString(drawCounter)+"\n";//ticks: " + ofToString(inputSoundStream.getTickCount());
	ofDrawBitmapString(reportString, 32, 589);
}


//--------------------------------------------------------------
void testApp::audioInputListener(ofxAudioEventArgs &args){	
	// samples are "interleaved"
	float curVol = 0.0;
	// samples are "interleaved"
	int numCounted = 0;	
	
	if(setupFinished && args.deviceID == inputDeviceID){
		
		for (int i = 0; i < args.bufferSize; i++){
			leftAudioIn[i] = args.buffer[i*2];
			rightAudioIn[i] = args.buffer[i*2+1];

			aubioOnsetDetect->processFrame(&leftAudioIn[0], bufferSize);
			
			curVol += leftAudioIn[i] * leftAudioIn[i];
			numCounted++;
		}
		
		curVol /= (float)numCounted;
		curVol = sqrt( curVol );
		smoothedVol *= 0.93;
		smoothedVol += 0.07 * curVol;

		bufferCounter++;
		
	}//end if we have finished set up the floats to hold the audio

}



void testApp::audioOutputListener(ofxAudioEventArgs &args){	
	
	if(setupFinished && args.deviceID == outputDeviceID){
		for (int i = 0; i < args.bufferSize; i++){
			if (outputIsSameAsInput){
			args.buffer[i*2] = leftAudioIn[i];
			args.buffer[i*2 + 1] = rightAudioIn[i];
			}
			else{
				//any old sine wave
				float sinVal =  ((height-mouseY)/(float)height)*sin(sampleIndex / (2 * PI * 5.0 ));
				args.buffer[i*2] = sinVal;
				args.buffer[i*2 + 1] = sinVal;
				sampleIndex++;
			}
			leftAudioOut[i] = args.buffer[i*2];
			rightAudioOut[i] = args.buffer[i*2 + 1];
		}
	}else{
		std::cout << "OUTPUT CALLED before finished - sending zeros" << endl;
			for (int i = 0; i < args.bufferSize; i++){
				args.buffer[i*2] = 0;
				args.buffer[i*2 + 1] = 0;
			
			}
	}
}

 
//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	
	if( key == 's' ){
		inputSoundStream.start();
		setupFinished = true;
	}
	
	if( key == 'e' ){
		inputSoundStream.stop();
		setupFinished = false;//stops it making "frozen" audio glitch sound and writes 0.0 to output sound buffer
	}
	
	if (key == '-'){
		volume -= 0.05;
		volume = MAX(volume, 0);
	} else if (key == '+'){
		volume += 0.05;
		volume = MIN(volume, 1);
	}
	

	if (key == 'u'){
		setupFinished = false;
		inputSoundStream.close();
		inputDeviceID = 0;
		setInputDevice(inputDeviceID);
		setupFinished = true;
	}
	
	if (key == 'j'){
		setupFinished = false;
		inputSoundStream.close();
		inputDeviceID = 1;
		setInputDevice(inputDeviceID);
		setupFinished = true;
	}


		
	if (key == 'p')
		clickSound.play();
	
	if (key == 'i')
		outputIsSameAsInput = !outputIsSameAsInput;
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){

}
//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	mouseX = x;
	mouseY = y;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

	

	
	if (showingInputDevices || showingOutputDevices){
		checkPressOnDeviceList(x, y);
	
	}
	else{
		if (abs(mouseY - 28) < 16 && mouseX < 360){
			showingInputDevices = !showingInputDevices;
			showingOutputDevices = false;
		}
		
		if (abs(mouseY - 48) < 16 && mouseX < 360){
			showingOutputDevices = !showingOutputDevices;
			showingInputDevices = false;
		}
	}
	
}


void testApp::checkPressOnDeviceList(int x, int y){
	if (mouseX > ofGetWidth() - 360){
	//check height
		float deviceNumber = y - deviceListHeightOffset/2;
		deviceNumber /= deviceListHeightDistance;
		int deviceInt = (int) round( deviceNumber)-1;
		if (showingInputDevices && inputSoundStream.deviceList[deviceInt].maxNumberOfInputs > 0){
			setInputDevice(deviceInt);
			showingInputDevices = false;
		}
		if (showingOutputDevices && outputSoundStream.deviceList[deviceInt].maxNumberOfOutputs > 0){
			setOutputDevice(deviceInt);
			showingOutputDevices = false;
		}
		
	
	}

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
width = w;
height = h;
	//aubioOnsetDetect->windowResize(w,h);
}



void testApp::setInputDevice(int i){
	setupFinished = false;
	inputSoundStream.close();
	inputDeviceID = i;
	inputSoundStream.setDeviceId(inputDeviceID);	//this now uses the audio input rather than mic input for mac 	
	inputSoundStream.setup(0, 2, this, 44100, bufferSize, 4);

	long latency;
//	latency = inputSoundStream.audio->getStreamLatency();
//	cout << "stream latency is " << latency << endl;

	setupFinished = true;
	inputSoundStream.deviceName = inputSoundStream.getDeviceNameFromID(inputSoundStream.deviceID);
}

void testApp::setOutputDevice(int i){
	outputSoundStream.close();
	outputDeviceID = i;
	outputSoundStream.setDeviceId(outputDeviceID);
	outputSoundStream.setup(2, 0, this, 44100, bufferSize, 4);
	outputSoundStream.deviceName = outputSoundStream.getDeviceNameFromID(outputSoundStream.deviceID);
}

void testApp::printInputSampleRate(){
	unsigned int sampleRateUsed;
//	sampleRateUsed = inputSoundStream.audio->sampleRate;//getStreamSampleRate();
//	cout << "using sample rate " << sampleRateUsed << endl;
}
  
void testApp::showDeviceListWindow(ofxPortaudioSoundStream& paSoundStream){
	ofSetColor(0,0,0);
	if (showingInputDevices)
	verdana14.drawString("select input", width - 300, deviceListHeightOffset);
	else 
	verdana14.drawString("select output", width - 300, deviceListHeightOffset);	

	for (int i = 0; i < paSoundStream.deviceList.size();i++){
		if (abs(mouseY - (deviceListHeightOffset + deviceListHeightDistance*(i+1))) < 16  && mouseX > width - 300)
			ofSetColor(255,0,0);
		else
			ofSetColor(0);
		if (showingInputDevices && paSoundStream.deviceList[i].maxNumberOfInputs > 0 )
		verdana14.drawString(paSoundStream.deviceList[i].name, width - 300 , deviceListHeightOffset + deviceListHeightDistance*(i+1));
	
		if (showingOutputDevices && paSoundStream.deviceList[i].maxNumberOfOutputs > 0 )
			verdana14.drawString(paSoundStream.deviceList[i].name, width - 300 , deviceListHeightOffset + deviceListHeightDistance*(i+1));
		
	}
}





void testApp::exit(){

	
	ofRemoveListener(inputSoundStream.audioReceivedEvent, this, &testApp::audioInputListener);
	ofRemoveListener(outputSoundStream.audioRequestedEvent, this, &testApp::audioOutputListener);
	
	leftAudioIn.clear();
	rightAudioIn.clear();
	leftAudioOut.clear();
	rightAudioOut.clear();

// I think this is dealt with in the ofxPortaudioSoundstream class, by closing / terminating there
// so dont want to send message twice
//	inputSoundStream.close();
//	outputSoundStream.close();
	
//	inputSoundStream.terminate();
//	outputSoundStream.terminate();
	delete aubioOnsetDetect;
	
}

