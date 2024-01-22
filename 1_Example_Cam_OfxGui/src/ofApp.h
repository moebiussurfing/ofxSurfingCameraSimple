#pragma once
#include "ofMain.h"

#include "surfingEasyCamOfxGui.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void draw();
	void drawScene();
	void drawGui();

	ofxPanel gui;
	ofParameter<bool> bDrawGrid { "Draw Grid", true };
	
	surfingEasyCamOfxGui camera;
};
