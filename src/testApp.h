#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "stdio.h"
#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib> 

#include "ofxPortaudioSoundStream.h"
#include "ofxAubioOnsetDetection.h"

#define TEMPO_SCREEN 2;
//main commands:

//see the key pressed function for other bits - e.g. can view spectral difference


//#define FRAMESIZE 512

//length in terms of frames (at 512 samples per frame - there are 90 per second) => 900: 10 seconds




class testApp : public ofBaseApp{

	public:


		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		void showDeviceListWindow(ofxPortaudioSoundStream& paSoundStream);

	
		ofxPortaudioSoundStream inputSoundStream;
		ofxPortaudioSoundStream outputSoundStream;

		void audioInputListener (ofxAudioEventArgs &args);
		void audioOutputListener (ofxAudioEventArgs &args);
	
	vector <float> leftAudioOut;
	vector <float> rightAudioOut;
	
	vector <double> leftAudioIn;
	vector <double> rightAudioIn;

	
	float 	volume;
	ofSoundPlayer clickSound;
	int clickIndex;	
	
	float width, height;

	int bufferCounter;
	int bufferSize;
	
	void setInputDevice(int i);
	void setOutputDevice(int i);
	
	int inputDeviceID;
	int outputDeviceID;
	bool setupFinished;
	
	ofTrueTypeFont verdana30;
	ofTrueTypeFont verdana14;
	string BPMstring;
	
	string inputDeviceName, outputDeviceName;
	void printInputSampleRate();
	
	int mouseX, mouseY;
	float screenWidth;
	bool showingInputDevices, showingOutputDevices;
	int deviceListHeightDistance, deviceListHeightOffset;
	
	void checkPressOnDeviceList(int x, int y);
	
	void drawAudioInput();
	int drawCounter;
	float smoothedVol;
	float scaledVol;
	vector <float> volHistory;
	
	bool outputIsSameAsInput;
	int sampleIndex;
	string infoString;
	
	ofxAubioOnsetDetection* aubioOnsetDetect;
	int screenToDraw;
	};

#endif
