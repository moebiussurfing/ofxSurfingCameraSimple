#pragma once
#include "ofMain.h"

#include "surfingEasyCamImGui.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void draw();
	void drawScene();
	void drawGui();

	ofxSurfingGui ui;
	ofParameter<bool> bDrawGrid { "Draw Grid", true };
	
	SurfingEasyCamImGui cam;
};
