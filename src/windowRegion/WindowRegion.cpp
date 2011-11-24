/*
 *  WindowRegion.cpp
 *  BTrackPortAudioMultipleAnalyser
 *
 *  Created by Andrew Robertson on 13/10/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

//WARNING - if window is resized, you must make a manual check function in testApp that calls 
//yourWindow.resize(w, h);
//it does not use a listener (which would be preferable)

#include "WindowRegion.h"

WindowRegion::WindowRegion(){
	x = 0;
	y = 0;
	width = ofGetWidth();
	height = ofGetHeight();
	
	fullScreenX = ofGetWidth();
	fullScreenY = ofGetHeight();

	getRelativeSize();//store in case we resize!
	printf("set window with x %i, y %i, width %i, height %i\n", x, y, width, height);
}

WindowRegion::WindowRegion(int _x, int _y, int _width, int _height){
	if (_x >= 0 && _x <= ofGetWidth())
		x = _x;
	
	if (_y >= 0 && _y <= ofGetHeight())
		y = _y;
	
	if (_width > 0)
		width = _width;
	
	if (_height > 0)
		height = _height;
	
	fullScreenX = ofGetWidth();
	fullScreenY = ofGetHeight();
	
	getRelativeSize();
	printf("set window with x %i, y %i, width %i, height %i\n", x, y, width, height);
}

void WindowRegion::getRelativeSize(){
	relativeX = (double)x / fullScreenX;
	relativeY = (double)y / fullScreenY;
	relativeWidth = (double)width / fullScreenX;
	relativeHeight = (double)height / fullScreenY;
}

void WindowRegion::setToRelativeSize(){
	x = relativeX * fullScreenX;
	y = relativeY * fullScreenY;
	width = relativeWidth * fullScreenX;
	height = relativeHeight * fullScreenY;
}

void WindowRegion::setToRelativeSize(double _xr, double _yr, double _wr, double _hr){
	relativeX = _xr;
	relativeY = _yr;;
	relativeWidth = _wr;;
	relativeHeight = _hr;
	setToRelativeSize();
}

void WindowRegion::resized(int w, int h){
	fullScreenX = w;
	fullScreenY = h;
	setToRelativeSize();
}


void WindowRegion::setWindowDimensions(int _x, int _y, int _width, int _height){
	if (_x >= 0 && _x <= ofGetWidth())
		x = _x;
	
	if (_y >= 0 && _y <= ofGetHeight())
		y = _y;
	
	if (_width > 0)
		width = _width;
	
	if (_height > 0)
		height = _height;
	
	getRelativeSize();
	printf("set window with x %i, y %i, width %i, height %i\n", x, y, width, height);
}

void WindowRegion::resetToFullScreen(){
	x = 0;
	y = 0;
	width = ofGetWidth();
	height = ofGetHeight();
}

void WindowRegion::drawOutline(){
	ofLine(x, y, x+width, y);
	ofLine(x+width, y, x+width, y+height);
	ofLine(x, y+height, x+width, y+height);
	ofLine(x, y, x, y+height);

}