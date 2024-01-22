#pragma once
#include "ofMain.h"

#include "SurfingEasyCam.h"
#include "ofxGui.h"

class surfingEasyCamOfxGui : public SurfingEasyCam {
public:
	ofxPanel gui;

private:
	void setupGui() override{
		ofLogNotice("SurfingEasyCam") << "setupGui()";
		gui.setup(parameters);

		gui.getGroup(paramsSettings.getName()).minimize();
		gui.getGroup(paramsInternal.getName()).minimize();
	}

public:
	void drawGui() override {
		if (!bGui) return;
		if (bDebug) drawInteractionArea();
		drawHelpText();
		gui.draw();
	}

	 void refreshGui() override {
		//ofLogNotice("SurfingEasyCam") << "refreshGui()";
	}
};