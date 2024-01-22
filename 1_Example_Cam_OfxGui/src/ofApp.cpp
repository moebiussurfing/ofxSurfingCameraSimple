#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	int r = 50;
	ofSetConeResolution(r, 1);
	ofSetSphereResolution(r);
	ofSetCylinderResolution(r, 1);

	ofxSurfing::setWindowAtMonitor(-1);

	//camera.setup(); // Optional

	gui.setup("ofApp");
	gui.add(bDrawGrid);
	gui.add(camera.bGui);
	ofxSurfing::setGuiPositionToLayout(gui, ofxSurfing::SURFING_LAYOUT_TOP_CENTER);
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(48);
	ofEnableDepthTest();

	camera.begin();
	{
		drawScene();
	}
	camera.end();

	drawGui();
}

//--------------------------------------------------------------
void ofApp::drawScene() {
	ofEnableDepthTest();

	ofPushStyle();
	ofFill();

	float sz = SURFING__SCENE_SIZE_UNIT / 4.f;

	// Right
	ofSetColor(ofColor::red);
	ofDrawCone(sz, 0, 0, sz / 2, sz);

	// Left
	ofSetColor(ofColor::white);
	ofDrawSphere(-sz, 0, 0, sz / 2);

	// Bottom
	ofSetColor(ofColor::blue);
	ofDrawBox(0, sz, 0, sz);

	// Top
	ofSetColor(ofColor::cyan);
	ofDrawCylinder(0, -sz, 0, sz / 2, sz);

	// Front
	ofSetColor(ofColor::yellow);
	ofDrawBox(0, 0, sz, sz);

	// Back
	ofSetColor(ofColor::magenta);
	ofDrawBox(0, 0, -sz, sz);

	// Grid
	if (bDrawGrid) {
		ofDrawGrid(sz * 2, 1, true, true, true, false);
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawGui() {
	ofDisableDepthTest();

	gui.draw();

	camera.drawGui();
}
