#pragma once
#include "ofMain.h"

#include "ofxCameraSaveLoad.h"
#include "ofxSurfingHelpersLite.h"

/*
	SurfingEasyCam.h

	Extends ofEasyCam with:

	- Implements a mouse control workflow. 
		Ctrl key toggles mode.
	- Momentary modes:
		True: toggles mouse control state and stay.
		False: enables mouse control state momentary.
	- Inertia and drag settings and config.
	- Help info and key commands.
	- ImGui required but it could be removed easily.
*/

/*
	TODO
	- add floor class
	- add extra cam with easing
	-
*/

#define SURFING__SCENE_SIZE_UNIT 1000.f

#define SURFING__CAMERA_FAR_CLIP 1000000.f
#define SURFING__CAMERA_NEAR_CLIP -1000000.f

#include <functional>
using callback_t = std::function<void()>;

//--------------------------------------------------------------

class SurfingEasyCam : public ofEasyCam {
public:
	SurfingEasyCam() {
		ofAddListener(ofEvents().update, this, &SurfingEasyCam::update);
		ofAddListener(ofEvents().keyPressed, this, &SurfingEasyCam::keyPressed);
		ofAddListener(ofEvents().keyReleased, this, &SurfingEasyCam::keyReleased);
		ofAddListener(ofEvents().windowResized, this, &SurfingEasyCam::windowResized);

		this->reset();

		//this->setNearClip(SURFING__CAMERA_NEAR_CLIP);
		this->setFarClip(SURFING__CAMERA_FAR_CLIP);
	}

	~SurfingEasyCam() {
		ofRemoveListener(ofEvents().update, this, &SurfingEasyCam::update);
		ofRemoveListener(paramsCamera.parameterChangedE(), this, &SurfingEasyCam::ChangedCamera);
		ofRemoveListener(ofEvents().keyPressed, this, &SurfingEasyCam::keyPressed);
		ofRemoveListener(ofEvents().keyReleased, this, &SurfingEasyCam::keyReleased);
		ofRemoveListener(ofEvents().windowResized, this, &SurfingEasyCam::windowResized);

		if (!bDoneExit) exit();
	}

public:
	void exit() {
		ofLogNotice("SurfingEasyCam") << "exit()";

		save();
		bDoneExit = true;
	}

private:
	bool bDoneExit = false;

	void ChangedCamera(ofAbstractParameter & e) {

		std::string name = e.getName();

		ofLogNotice("SurfingEasyCam") << "ChangedCamera: " << name << ": " << e;

		//--

		if (name == vSaveCamera.getName()) {
			doSaveCamera();
		}

		else if (name == vLoadCamera.getName()) {
			doLoadCamera();
		}

		else if (name == vResetCamera.getName()) {
			doResetCamera();

			//// workflow: save
			//if (bEnableCameraAutosave) doSaveCamera();
		}
	}

	//--

public:
	ofParameterGroup paramsCamera;
	ofParameter<bool> bEnableCameraAutosave;
	ofParameter<void> vSaveCamera;
	ofParameter<void> vLoadCamera;

private:
	string pathCamera = "Camera.txt";

public:
	ofParameter<void> vResetCamera;

	ofParameter<void> vResetSettings;

private:
	ofEventListener listenerResetSettings;

private:
	SurfingAutoSaver autoSaver;

	//--

public:
	ofParameterGroup parameters;
	ofParameterGroup paramsInternal;
	ofParameterGroup paramsSettings;

private:
	ofEventListener listenerParameters;
	string path = "Camera.json";

public:
	ofParameter<bool> bGui { "Camera", true };
	ofParameter<bool> bHelp { "Help", false };
	ofParameter<bool> bExtra { "Extra", true };
	ofParameter<bool> bKeys { "Keys", true };
	ofParameter<bool> bDebug { "Debug", false };

	string sHelp;

	//TODO: listener and ctrl to toggle/enable workflow
	ofParameter<bool> bMouseCam { "Mouse Camera", false };
	ofParameter<bool> bInertia { "Inertia", false };
	ofParameter<void> vResetInertia { "Reset Inertia" };
	ofParameter<float> dragInertia { "Drag Inertia", .7f, 0.2f, 1.0f };
	ofParameter<bool> bOrtho { "Ortho", false };

private:
	ofEventListener listenerMouseCam;
	ofEventListener listenerIntertia;
	ofEventListener listenerResetIntertia;
	ofEventListener listenerOrtho;
	ofEventListener listenerDrag;

private:
	bool bFlagBuildHelp = false;

protected:
	ofParameter<bool> bKeyMod { "KeyMod", false };
	ofEventListener listenerKeyMod;

	ofParameter<bool> bKeyModMomentary { "Momentary", false };
	ofEventListener listenerKeyModMomentary;
	// true: mouse cam enabled while key mod pressed.
	// false: mouse cam state switch when key mod press.

	bool bDoneSetup = false;
	bool bDoneFixControlArea = false;

public:
	void setup() {
		ofLogNotice("SurfingEasyCam") << "setup()";

		vResetSettings.set("Reset Settings");
		vLoadCamera.set("Load");
		vSaveCamera.set("Save");
		bEnableCameraAutosave.set("Auto Save", true);
		vResetCamera.set("Reset Camera");
		bOrtho.setSerializable(false);

		paramsInternal.setName("Internal");
		paramsInternal.add(bGui);
		paramsInternal.add(bHelp);
		paramsInternal.add(bExtra);
		paramsInternal.add(bDebug);
		paramsInternal.add(bKeys);

		paramsSettings.setName("Settings");
		paramsSettings.add(vResetSettings);
		paramsSettings.add(bMouseCam);
		paramsSettings.add(bOrtho);
		paramsSettings.add(bInertia);
		paramsSettings.add(dragInertia);
		paramsSettings.add(bInertia);

		paramsCamera.setName("Camera Preset");
		paramsCamera.add(vLoadCamera);
		paramsCamera.add(vSaveCamera);
		paramsCamera.add(vResetCamera);
		paramsCamera.add(bEnableCameraAutosave);

		parameters.setName("Camera");
		parameters.add(paramsCamera);
		parameters.add(paramsSettings);
		parameters.add(paramsInternal);

		//--

		setupCallbacks();

		startup();

		//--

		bDoneSetup = true;
	}

private:
	void setupCallbacks() {
		ofLogNotice("SurfingEasyCam") << "setupCallbacks()";

		//TODO: how to know when ofEasyCam changed internally to trig save?
		//this->listenerDrag

		//--

		listenerMouseCam = bMouseCam.newListener([&](bool & b) {
			string s = "Mouse Camera " + ofToString(b ? "True" : "False");
			ofLogNotice("SurfingEasyCam") << s;

			//TODO: fix (sometimes wrong state) with a workaround in update.
			if (b)
				this->enableMouseInput();
			else
				this->disableMouseInput();
		});

		//--

		listenerKeyMod = bKeyMod.newListener([&](bool & b) {
			static bool b_ = !bKeyMod.get();
			if (b != b_) {
				// changed
				b_ = b;

				if (!bKeyModMomentary) {
					if (b) {
						// MODE A: switch
						// toggle mouse cam if bKeyMod changed but to true
						bMouseCam.set(!bMouseCam.get());
					}
				} else {
					// MODE B: latch
					// mouse cam enabled while key mod pressed.
					bMouseCam = b;
				}

				string s = "ModKey " + ofToString(b ? "ENABLED" : "DISABLED");
				ofLogNotice("SurfingEasyCam") << s;
			}
		});

		//--

		listenerKeyModMomentary = bKeyModMomentary.newListener([&](bool & b) {
			static bool b_ = !bKeyModMomentary.get();
			if (b != b_) { // changed
				b_ = b;

				if (bKeyModMomentary) {
					bMouseCam.set(bKeyMod);
				} else {
				}
			}
		});

		//--

		listenerIntertia = bInertia.newListener([&](bool & b) {
			if (b)
				this->enableInertia();
			else
				this->disableInertia();

			this->setDrag(dragInertia);
		});

		listenerDrag = dragInertia.newListener([&](float & v) {
			this->setDrag(v);
		});

		listenerResetIntertia = vResetInertia.newListener([&](void) {
			dragInertia = 0.7f;
		});

		listenerOrtho = bOrtho.newListener([&](bool & b) {
			if (b)
				this->enableOrtho();
			else
				this->disableOrtho();
		});

		listenerResetSettings = vResetSettings.newListener([&](void) {
			doResetSettings();
		});

		//--

		ofAddListener(paramsCamera.parameterChangedE(), this, &SurfingEasyCam::ChangedCamera);

		//--

		listenerParameters = parameters.parameterChangedE().newListener([this](ofAbstractParameter & e) {
			std::string name = e.getName();
			ofLogVerbose("SurfingEasyCam") << "Changed: " << name << ": " << e;

			if (e.isSerializable()) {
				autoSaver.saveSoon();
			}

			bFlagBuildHelp = true;
		});

		//--

		callback_t f = std::bind(&SurfingEasyCam::save, this);
		autoSaver.setFunctionSaver(f);
	}

	void startup() {
		ofLogNotice("SurfingEasyCam") << "startup()";

		load();

		//TODO
		this->setControlArea(ofGetCurrentViewport());
	}

private:
	void update(ofEventArgs & args) {
		if (!bDoneSetup) {
			setup();
		}

		//TODO
		if (!bDoneFixControlArea && ofGetFrameNum() > 0) {
			this->setControlArea(ofGetCurrentViewport());
			bDoneFixControlArea = true;
		}

		if (bFlagBuildHelp) {
			bFlagBuildHelp = false;
			buildHelp();
		}
	}

private:
	void keyPressed(int key) {
		if (!bKeys) return;

		switch (key) {

		case OF_KEY_LEFT_CONTROL:
			bKeyMod = true;
			break;

		case ' ':
			bOrtho = !bOrtho;
			break;
		case 'H':
		case 'h':
			bHelp ^= true;
			break;

		case 'I':
		case 'i':
			this->getInertiaEnabled() ? this->disableInertia() : this->enableInertia();
			break;

		case 'Y':
		case 'y':
			this->setRelativeYAxis(!this->getRelativeYAxis());
			break;
		}
	}

	void keyReleased(int key) {
		if (!bKeys) return;
		switch (key) {
		case OF_KEY_LEFT_CONTROL:
			bKeyMod = false;
			break;
		}
	}

	//--

public:
	void drawGui() {
		if (bDebug) drawInteractionArea();
		drawHelpText();
	}

	void drawInteractionArea() {
		ofRectangle vp = ofGetCurrentViewport();
		float r = std::min<float>(vp.width, vp.height) * 0.5f;
		float x = vp.width * 0.5f;
		float y = vp.height * 0.5f;

		ofPushStyle();
		ofSetCircleResolution(300);
		ofSetLineWidth(3);
		ofColor c = ofColor(0);
		ofSetColor(c.r, c.g, c.b, 24);
		ofNoFill();
		glDepthMask(false);
		ofDrawCircle(x, y, r);
		glDepthMask(true);
		ofPopStyle();
	}

	void buildHelp() {
		ofLogNotice("SurfingEasyCam") << "buildHelp()";

		stringstream ss;
		ss << "Camera\n"
		   << endl;
		ss << "h:     Help" << endl;
		ss << "Ctrl:  Mouse Input" << endl;
		ss << "Space: Projection\n";
		ss << "       Mode " << (bOrtho ? "Ortho" : "Perspective") << endl;
		ss << "i:     Camera Inertia" << endl;
		ss << "y:     Relative y axis" << endl;
		ss << "m:     Translation" << endl;
		ss << endl;
		ss << "Mouse Input:     " << (this->getMouseInputEnabled() ? "Enabled" : "Disabled") << endl;
		ss << "Momentary:       " << (bKeyModMomentary ? "True" : "False") << endl;
		ss << "Mode:            " << (this->getOrtho() ? "Ortho" : "Perspective") << endl;
		ss << "Inertia:         " << (this->getInertiaEnabled() ? "True" : "False") << endl;
		ss << "Relative y axis: " << (this->getRelativeYAxis() ? "True" : "False") << endl;
		ss << endl;
		if (bDebug) {
			ss << "x,y rotation" << endl;
			ss << "Left Mouse button \ndrag inside circle" << endl;
			ss << endl;
			ss << "z rotation or roll" << endl;
			ss << "Left Mouse button \ndrag outside circle" << endl;
			ss << endl;
		}
		ss << "Move over x,y axis" << endl;
		ss << "Left Mouse button drag" << endl;
		ss << endl;
		ss << "Middle Mouse button press" << endl;
		ss << "Truck and Boom" << endl;
		ss << endl;
		ss << "Move over z axis" << endl;
		ss << "Right Mouse button drag" << endl;
		ss << "Vertical mouse wheel" << endl;
		ss << "Dolly/Zoom In/Out" << endl;
		if (bOrtho) {
			ss << endl;
			ss << "Notice that in Mode Ortho \n";
			ss << "zoom will be centered at the mouse position.";
		}

		sHelp = ss.str();

		buildHelpGui();
	}

	virtual void buildHelpGui() { //optional for some ui's..
		//ofLogNotice("SurfingEasyCam") << "buildHelpGui()";
	}

	virtual void drawHelpText() {
		if (!bGui) return;
		if (!bHelp) return;

		ofxSurfing::ofDrawBitmapStringBox(sHelp, ofxSurfing::SURFING_LAYOUT_BOTTOM_RIGHT);
	}

	//--

	void doSaveSettings() {
		ofLogNotice("SurfingEasyCam") << "doSaveSettings() > " << path;

		bOrtho.set(this->getOrtho());
		bInertia.set(this->getInertiaEnabled());

		//TODO
		//bMouseCam.set(this->getMouseInputEnabled());//fails bc auto saver

		ofxSurfing::saveSettings(parameters, path);
	}

	bool doLoadSettings() {
		ofLogNotice("SurfingEasyCam") << "doLoadSettings()  > " << path;

		bool b = ofxSurfing::loadSettings(parameters, path);
		return b;
	}

	//--

public:
	bool getSettingsFileFoundForCamera() {
		bool b = ofxSurfing::checkFileExist(pathCamera);
		if (b) {
			ofLogNotice("SurfingEasyCam") << "Found CAMERA settings file: " << pathCamera;
		} else {
			ofLogWarning("SurfingEasyCam") << "CAMERA settings file: " << pathCamera << " not found!";
		}
		return b;
	}

	void doLoadCamera() {
		ofLogNotice("SurfingEasyCam") << "doLoadCamera()";

		ofxLoadCamera(*this, pathCamera);
	}

	void doSaveCamera() {
		ofLogNotice("SurfingEasyCam") << "doSaveCamera()";

		ofxSaveCamera(*this, pathCamera);
	}

	void doResetCamera() {
		ofLogNotice("SurfingEasyCam") << "doResetCamera()";

		this->reset();

		//this->setNearClip(SURFING__CAMERA_NEAR_CLIP);
		this->setFarClip(SURFING__CAMERA_FAR_CLIP);

		//this->setVFlip(true);

		float d = SURFING__SCENE_SIZE_UNIT;
		this->setPosition({ 0, d, d });
		this->lookAt(glm::vec3(0, 0, 0));
	}

	void doResetSettings() {
		ofLogNotice("SurfingEasyCam") << "doResetSettings()";

		bEnableCameraAutosave.set(true);
		bMouseCam.set(false);
		bInertia.set(false);
		dragInertia.set(.7f);
		bOrtho.set(false);
		bKeyModMomentary.set(false);
		bKeyMod.set(false);
	}

	//--

public:
	void save() {
		ofLogNotice("SurfingEasyCam") << "Save()";

		doSaveSettings();
		doSaveCamera();
	}

	bool load() {
		ofLogNotice("SurfingEasyCam") << "Load()";

		bool b1, b2;
		autoSaver.pause();
		{
			b1 = doLoadSettings();

			b2 = this->getSettingsFileFoundForCamera();
			if (!b2) {
				doResetCamera();
			} else {
				if (bEnableCameraAutosave) doLoadCamera();
			}

			//TODO
			// refresh
			bInertia.set(bInertia.get());
			bMouseCam.set(bMouseCam.get());

			//TODO
			bOrtho.set(this->getOrtho());
			//bOrtho.set(bOrtho.get());
		}
		autoSaver.start();

		bFlagBuildHelp = true;

		return (b1 && b2);
	}

	//--

private:
	void keyPressed(ofKeyEventArgs & eventArgs) {
		const int key = eventArgs.key;

		//// modifiers
		//bool bKeyModCommand = eventArgs.hasModifier(OF_KEY_COMMAND);
		//bool bKeyModControl = eventArgs.hasModifier(OF_KEY_CONTROL);
		//bool bKeyModAlt = eventArgs.hasModifier(OF_KEY_ALT);
		//bool bKeyModShift = eventArgs.hasModifier(OF_KEY_SHIFT);

		ofLogVerbose("SurfingEasyCam") << "keyPressed: " << key;

		keyPressed(key);
	}

	void keyReleased(ofKeyEventArgs & eventArgs) {
		const int key = eventArgs.key;

		//// modifiers
		//bool bKeyModCommand = eventArgs.hasModifier(OF_KEY_COMMAND);
		//bool bKeyModControl = eventArgs.hasModifier(OF_KEY_CONTROL);
		//bool bKeyModAlt = eventArgs.hasModifier(OF_KEY_ALT);
		//bool bKeyModShift = eventArgs.hasModifier(OF_KEY_SHIFT);

		ofLogVerbose("SurfingEasyCam") << "keyReleased: " << key;

		keyReleased(key);
	}

	void windowResized(ofResizeEventArgs & resize) {
		ofRectangle r = { 0.f, 0.f, (float)resize.width, (float)resize.height };
		this->setControlArea(r);
	}
};