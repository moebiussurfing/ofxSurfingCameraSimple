#pragma once
#include "ofMain.h"

#include "SurfingEasyCam.h"
#include "ofxSurfingImGui.h"

class SurfingEasyCamImGui : public SurfingEasyCam {
	SurfingGuiManager * ui = nullptr;

public:
	void setUiPtr(SurfingGuiManager * ui_) {
		ui = ui_;

		//--

		helpTextWidget.bGui.makeReferenceTo(bHelp);
		helpTextWidget.setTitle("Help Camera");
		helpTextWidget.setCustomFonts(ui->customFonts, ui->namesCustomFonts);

		//TODO: workflow. set mono-spaced font
		if (ui->customFonts.size() > 4 && ui->isDoneDefinedMonospacedFonts()) {
			helpTextWidget.setFontIndex(OFX_IM_FONT_DEFAULT_MONO);
		}
	}

	void drawImGui() {
		if (ui == nullptr) return;

		if (bGui) IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_LOCKED_RESIZE;

		if (ui->BeginWindow(bGui)) {
			ui->Add(bMouseCam, OFX_IM_TOGGLE_BIG_BORDER_BLINK);
			ui->AddSpacing();

			ui->Add(vLoadCamera, OFX_IM_BUTTON, 2, true);
			ui->Add(vSaveCamera, OFX_IM_BUTTON, 2);
			ui->Add(vResetCamera, OFX_IM_BUTTON);

			if (ui->isMaximized()) {
				ui->Add(bAutosave);
				ui->AddSpacing();

				ui->Add(bExtra, OFX_IM_TOGGLE_ROUNDED_MINI);
				if (bExtra) {
					ui->AddSpacing();
					ui->Add(bOrtho);
					ui->Add(bKeyModMomentary);
					ui->Add(bInertia);
					if (bInertia) {
						ui->Indent();
						ui->AddLabel(dragInertia.getName());
						ui->Add(dragInertia, OFX_IM_STEPPER_NO_LABEL);
						ui->Add(vResetInertia);
						ui->Unindent();
					}
					ui->AddSpacing();
					ui->Add(vResetSettings);
				}
				ui->AddSpacing();

				ui->Add(bDebug, OFX_IM_TOGGLE_ROUNDED_MINI);
				if (bDebug) {
					string s;
					ui->AddSpacing();
					bool b = ui->isMouseOverGui();
					s = "Mouse OverGui:\n";
					s += b ? "true" : "false";
					ui->AddLabel(s);
					s = "Control area:\n";
					s += ofToString(this->getControlArea());
					ui->AddLabel(s);
				}
				ui->Add(bKeys, OFX_IM_TOGGLE_ROUNDED_MINI);
				ui->Add(bHelp, OFX_IM_TOGGLE_ROUNDED_MINI);
			}

			ui->EndWindow();
		}

		if (bHelp) helpTextWidget.draw();
	}

	void end() override {
		ofGetCurrentRenderer()->unbind(*this);

		if (ui == nullptr) return;
		if (ui->isMouseOverGui()) {
			if (bMouseCam) disableMouseInput();
		} else {
			if (bMouseCam) enableMouseInput();
		}
	}

	void refreshGui() override { //optional for some ui's..
		if (ui == nullptr) return;
		ofLogNotice("SurfingEasyCam") << "refreshGui()";

		helpTextWidget.setText(sHelp);
	}

	void drawHelpText() override {
		//if (!bGui) return;
		//if (!bHelp) return;

		//helpTextWidget.draw();
	}

	HelpTextWidget helpTextWidget;
};