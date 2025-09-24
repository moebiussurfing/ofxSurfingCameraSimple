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
#define SURFING__CAMERA_NEAR_CLIP 0.1f

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

		this->setFarClip(SURFING__CAMERA_FAR_CLIP);
		this->setNearClip(SURFING__CAMERA_NEAR_CLIP);
	}

	~SurfingEasyCam() {
		ofRemoveListener(ofEvents().update, this, &SurfingEasyCam::update);
		ofRemoveListener(paramsCamera.parameterChangedE(), this, &SurfingEasyCam::ChangedCamera);
		ofRemoveListener(ofEvents().keyPressed, this, &SurfingEasyCam::keyPressed);
		ofRemoveListener(ofEvents().keyReleased, this, &SurfingEasyCam::keyReleased);
		ofRemoveListener(ofEvents().windowResized, this, &SurfingEasyCam::windowResized);

		if (!bDoneExit) exit();
	}

private:
	callback_t f_Reset = nullptr;

public:
	void setFunctionReset(callback_t f = nullptr) {
		f_Reset = f;
	}

public:
	void exit() {
		ofLogNotice("SurfingEasyCam") << "exit()";

		save();
		bDoneExit = true;
	}

private:
	bool bDoneExit = false;

	//--

public:
	ofParameterGroup paramsCamera { "Camera Preset" };
	ofParameter<bool> bAutosave { "Auto Save", true };
	ofParameter<void> vSaveCamera { "Save" };
	ofParameter<void> vLoadCamera { "Load" };

public:
	ofParameter<void> vResetCamera { "Reset Camera" };
	ofParameter<void> vResetSettings { "Reset Settings" };

private:
	ofEventListener listenerResetSettings;

private:
	SurfingAutoSaver autoSaver;

	//--

public:
	ofParameterGroup parameters { "Camera" };
	ofParameterGroup paramsInternal { "Internal" };
	ofParameterGroup paramsSettings { "Settings" };

private:
	ofEventListener listenerParameters;
	string path = "Camera.json"; //some main settings
	string pathCamera = "Camera.txt"; //full camera internal

	string name = "";

public:
	void setName(const string & name_) { name = name_; }

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

		//--

		setupParameters();
		setupCallbacks();
		setupGui();

		startup();

		//--

		bDoneSetup = true;
	}

	void setupParameters() {
		ofLogNotice("SurfingEasyCam") << "setupParameters()";

		if (name != "") {
			string n = "";
			n += "UI ";
			n += name;
			bGui.setName(n);
			parameters.setName(name);
		}

		//TODO
		bOrtho.setSerializable(false); //handled by ofxCameraSaveLoad/pathCamera/"Camera.txt"

		paramsCamera.add(vLoadCamera);
		paramsCamera.add(vSaveCamera);
		paramsCamera.add(bAutosave);
		paramsCamera.add(vResetCamera);

		paramsSettings.add(bOrtho);
		paramsSettings.add(bInertia);
		paramsSettings.add(dragInertia);
		paramsSettings.add(vResetSettings);

		paramsInternal.add(bGui);
		paramsInternal.add(bExtra);

		parameters.add(bMouseCam);
		parameters.add(paramsCamera);
		parameters.add(paramsSettings);
		parameters.add(paramsInternal);
		parameters.add(bDebug);
		parameters.add(bKeys);
		parameters.add(bHelp);
	}

	virtual void setupGui() {
		ofLogNotice("SurfingEasyCam") << "setupGui()";
		//to be used when using ofxGui
	}

private:
	void setupCallbacks() {
		ofLogNotice("SurfingEasyCam") << "setupCallbacks()";

		//TODO: how to know when ofEasyCam changed internally to trig save?
		//this->listenerDrag

		//--

		ofAddListener(paramsCamera.parameterChangedE(), this, &SurfingEasyCam::ChangedCamera);

		//--

		listenerParameters = parameters.parameterChangedE().newListener([this](ofAbstractParameter & e) {
			if (bAttending) return;

			std::string name = e.getName();
			ofLogNotice("SurfingEasyCam") << "Changed listenerParameters: " << name << ": " << e;

			if (e.isSerializable()) {
				autoSaver.saveSoon();
			}

			bFlagBuildHelp = true;
		});

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

		callback_t f = std::bind(&SurfingEasyCam::save, this);
		autoSaver.setFunctionSaver(f);
	}

	bool bAttending = false;

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
			//if (bAutosave) doSaveCamera();
		}
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

		case 'G':
		case 'g':
			bGui ^= true;
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
	virtual void drawGui() {
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
		ss << "CAMERA\n"
		   << endl;
		if (bKeys) {
			ss << "h: Help" << endl;
			ss << "g: Gui" << endl;
			ss << endl;
			ss << "Ctrl: Mouse Input\n"
			   << (this->getMouseInputEnabled() ? "ENABLED" : "DISABLED") << endl;
			ss << endl;
			ss << "Space: Projection Mode\n"
			   << (this->getOrtho() ? "ORTHO" : "PERSPECTIVE") << endl;
			ss << endl;
			ss << "i: Camera Inertia\n"
			   << (this->getInertiaEnabled() ? "TRUE" : "FALSE") << endl;
			ss << endl;
			ss << "y: Relative y axis\n"
			   << (this->getRelativeYAxis() ? "TRUE" : "FALSE") << endl;
			ss << endl;
			ss << "m: Translation key" << endl;
			ss << endl;
		} else {
			ss << "Mouse Input: " << (this->getMouseInputEnabled() ? "ENABLED" : "DISABLED") << endl;
			ss << "Momentary: " << (bKeyModMomentary ? "TRUE" : "FALSE") << endl;
			ss << "Mode: " << (this->getOrtho() ? "ORTHO" : "PERSPECTIVE") << endl;
			ss << "Inertia: " << (this->getInertiaEnabled() ? "TRUE" : "FALSE") << endl;
			ss << "Relative y axis: " << (this->getRelativeYAxis() ? "TRUE" : "FALSE") << endl;
			ss << endl;
		}
		if (bDebug) {
			ss << "X,Y ROTATION" << endl;
			ss << "Left Mouse button \ndrag inside circle" << endl;
			ss << endl;
			ss << "Z ROTATION OR ROLL" << endl;
			ss << "Left Mouse button \ndrag outside circle" << endl;
			ss << endl;
		}
		ss << "MOVE OVER X,Y AXIS" << endl;
		ss << "Left Mouse button drag" << endl;
		ss << endl;
		ss << "TRUCK AND BOOM" << endl;
		ss << "Middle Mouse button press" << endl;
		ss << "Left Mouse button drag + m" << endl;
		ss << endl;
		ss << "MOVE OVER Z AXIS" << endl;
		ss << "Right Mouse button drag" << endl;
		ss << "Vertical mouse wheel" << endl;
		ss << "Dolly/Zoom In/Out" << endl;
		if (bOrtho) {
			ss << endl;
			ss << "Notice that in Mode Ortho\n";
			ss << "zoom will be centered\nat the mouse position.";
		}
		if (bDebug) {
			ss << endl
			   << endl;
			ss << "Control area:\n";
			ss << ofToString(this->getControlArea());
		}

		sHelp = ss.str();

		refreshGui();
	}

	virtual void refreshGui() {
		//ofLogNotice("SurfingEasyCam") << "refreshGui()";
	}

	virtual void drawHelpText(ofxSurfing::SURFING_LAYOUT layout = ofxSurfing::SURFING_LAYOUT_BOTTOM_RIGHT) {
		if (!bGui) return;
		if (!bHelp) return;

		ofxSurfing::ofDrawBitmapStringBox(sHelp, layout);
	}

	//--

	void doSaveSettings() {
		ofLogNotice("SurfingEasyCam") << "doSaveSettings() > " << path;

		//TODO: fix mirroring both settings..
		bAttending = true;
		bOrtho.set(this->getOrtho());
		bInertia.set(this->getInertiaEnabled());
		bAttending = false;

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
		this->setFarClip(SURFING__CAMERA_FAR_CLIP);
		this->setNearClip(SURFING__CAMERA_NEAR_CLIP);

		//this->setVFlip(true);

		if (f_Reset != nullptr) {
			f_Reset();
		} else {
			float d = SURFING__SCENE_SIZE_UNIT;
			this->setPosition({ 0, d, d });
			this->lookAt(glm::vec3(0, 0, 0));
		}
	}

	void doResetSettings() {
		ofLogNotice("SurfingEasyCam") << "doResetSettings()";

		//bMouseCam.set(false);
		bKeyModMomentary.set(false);
		bAutosave.set(true);
		bInertia.set(false);
		dragInertia.set(.7f);
		bOrtho.set(false);
		bKeyMod.set(false);
	}

	//--

public:
	void save() {
		ofLogNotice("SurfingEasyCam") << "save()";

		doSaveSettings();
		doSaveCamera();
	}
	void setEnableMouse(const bool b = true) {
		ofLogNotice("SurfingEasyCam") << "setEnableMouse()" << b;
		bMouseCam = b;
	}

	bool load() {
		ofLogNotice("SurfingEasyCam") << "load()";

		bool b1, b2;
		autoSaver.pause();
		{
			b1 = doLoadSettings();

			b2 = this->getSettingsFileFoundForCamera();
			if (!b2) {
				doResetCamera();
			} else {
				if (bAutosave) doLoadCamera();
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

		//bFlagBuildHelp = true;

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
