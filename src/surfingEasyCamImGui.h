#pragma once
#include "ofMain.h"

#include "SurfingEasyCam.h"
#include "ofxSurfingImGui.h"

//--------------------------------------------------------------

class SurfingEasyCamImGui : public SurfingEasyCam {
	SurfingGuiManager * ui = nullptr;

public:
	void setUiPtr(SurfingGuiManager * ui_) {
		ui = ui_;
	}

	//void drawImGui(SurfingGuiManager & ui_) {
	//}

	void drawImGui() {
		if (ui == nullptr) return;

		if (bGui) IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_LOCKED_RESIZE;

		if (ui->BeginWindow(bGui)) {
			ui->Add(bMouseCam, OFX_IM_TOGGLE_BIG_BORDER_BLINK);
			ui->AddSpacing();

			ui->Add(vLoadCamera, OFX_IM_BUTTON, 2, true);
			ui->Add(vSaveCamera, OFX_IM_BUTTON, 2);
			ui->Add(vResetCamera, OFX_IM_BUTTON);
			ui->Add(bEnableCameraAutosave);
			ui->AddSpacing();

			if (ui->isMaximized()) {
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
						if (ui->AddButton("Reset Inertia")) {
							dragInertia = 0.7f;
						}
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
					s = b ? "Mouse OverGui" : " ";
					ui->AddLabel(s);
				}
				ui->Add(bKeys, OFX_IM_TOGGLE_ROUNDED_MINI);
				ui->Add(bHelp, OFX_IM_TOGGLE_ROUNDED_MINI);
			}

			ui->EndWindow();
		}
	}

	/*
	void drawImGui(SurfingGuiManager & ui) {
		if (bGui) IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_LOCKED_RESIZE;

		if (ui.BeginWindow(bGui)) {
			ui.Add(bMouseCam, OFX_IM_TOGGLE_BIG_BORDER_BLINK);
			ui.AddSpacing();

			ui.Add(vLoadCamera, OFX_IM_BUTTON, 2, true);
			ui.Add(vSaveCamera, OFX_IM_BUTTON, 2);
			ui.Add(vResetCamera, OFX_IM_BUTTON);
			ui.Add(bEnableCameraAutosave, OFX_IM_TOGGLE_ROUNDED_MINI);
			ui.AddSpacing();

			//--

			if (ui.isMaximized()) {
				ui.Add(bExtra, OFX_IM_TOGGLE_ROUNDED_MINI);
				if (bExtra) {
					ui.AddSpacing();
					ui.Add(bKeyModMomentary);
					ui.Add(bOrtho);
					ui.Add(bInertia);
					if (bInertia) {
						ui.Indent();
						ui.AddLabel(dragInertia.getName());
						ui.Add(dragInertia, OFX_IM_STEPPER_NO_LABEL);
						if (ui.AddButton("Reset Inertia")) {
							dragInertia = 0.7f;
						}
						ui.Unindent();
					}
					ui.AddSpacing();
					ui.Add(vResetSettings);
				}
				ui.AddSpacing();
				ui.Add(bDebug, OFX_IM_TOGGLE_ROUNDED_MINI);
				if (bDebug) {
					string s;
					ui.AddSpacing();
					bool b = ui.isMouseOverGui();
					s = b ? "Mouse OverGui" : " ";
					ui.AddLabel(s);
				}
			}
			ui.Add(bKeys, OFX_IM_TOGGLE_ROUNDED_MINI);
			ui.Add(bHelp, OFX_IM_TOGGLE_ROUNDED_MINI);

			ui.EndWindow();
		}
	}
	*/

	void end() override {
		ofGetCurrentRenderer()->unbind(*this);

		if (ui == nullptr) return;
		if (ui->isMouseOverGui()) {
			if (bMouseCam) disableMouseInput();
		} else {
			if (bMouseCam) enableMouseInput();
		}
	}
};