#pragma once
#include "ofMain.h"

#include "SurfingEasyCam.h"
#include "ofxGui.h"

class surfingEasyCamOfxGui : public SurfingEasyCam {
	ofxPanel gui;

public:
	void setupGui() override{
		ofLogNotice("SurfingEasyCam") << "setupGui()";
		gui.setup(parameters);
	}

	void drawGui() override {
		if (!bGui) return;
		if (bDebug) drawInteractionArea();
		drawHelpText();
		gui.draw();
	}
};