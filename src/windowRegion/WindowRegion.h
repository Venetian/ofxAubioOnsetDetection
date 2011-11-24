/*
 *  WindowRegion.h
 *  BTrackPortAudioMultipleAnalyser
 *
 *  Created by Andrew on 13/10/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */


#ifndef WINDOW_REGION_H
#define WINDOW_REGION_H


#include "ofMain.h"

struct WindowRegion {
public:
	WindowRegion();
	WindowRegion(int _x, int _y, int _width, int _height);
	void setWindowDimensions(int _x, int _y, int _width, int _height);
	void resetToFullScreen();
	void resized(int w, int h);
	void getRelativeSize();
	void setToRelativeSize();
	void setToRelativeSize(double _xr, double _yr, double _wr, double _hr);
	
	int x;
	int y;
	int width; 
	int height;
	
	double relativeX, relativeY, relativeWidth, relativeHeight;
	double fullScreenX;
	double fullScreenY;
	
	void drawOutline();
	
};
#endif
