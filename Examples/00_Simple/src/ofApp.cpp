#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	// Parameters
	params.setName("paramsGroup"); // main container
	params2.setName("paramsGroup2"); // nested
	params3.setName("paramsGroup3"); // nested
	params.add(bPrevious.set("<"));
	params.add(bNext.set(">"));
	params.add(bEnable1.set("Enable1", false));
	params.add(bEnable2.set("Enable2", false));
	params.add(bEnable3.set("Enable3", false));
	params.add(lineWidth.set("lineWidth", 0.5, 0, 1));
	params.add(separation.set("separation", 50, 1, 100));
	params.add(speed.set("speed", 0.5, 0, 1));
	params.add(shapeType.set("shapeType", 0, -50, 50));
	params.add(size.set("size", 100, 0, 100));
	params.add(amount.set("amount", 10, 0, 25));
	params2.add(shapeType2.set("shapeType2", 0, -50, 50));
	params2.add(size2.set("size2", 100, 0, 100));
	params2.add(amount2.set("amount2", 10, 0, 25));
	params3.add(lineWidth3.set("lineWidth3", 0.5, 0, 1));
	params3.add(separation3.set("separation3", 50, 1, 100));
	params3.add(speed3.set("speed3", 0.5, 0, 1));
	params2.add(params3);
	params.add(params2);

	//--

	// Linear to logarithmic

	vIn.set("vIn", 0.5f, 0.f, 1.f);
	vOut1.set("vOut1", 0.5f, 0.f, 1.f);
	vOut2.set("vOut2", 0.5f, 0.f, 1.f);
	vOut3.set("vOut3", 0.5f, 0.f, 1.f);

	// Callback for vIn
	// and convert input linear to log
	listener = vIn.newListener([this](float& v) {

		vOut1 = ofxImGuiSurfing::reversedExponentialFunction(vIn * 10.f);
	vOut2 = ofxImGuiSurfing::exponentialFunction(vIn) / 10.f;
	vOut3 = ofxImGuiSurfing::squaredFunction(vIn);

	ofLogNotice() << v << " -> " << vOut1.get() << " : " << vOut2.get();
		});

	vIn = vIn; // refresh callback

	//--

	// Can be omitted in many scenarios..
	//ui.setup();

	//--

	// Optional help

	// enable internal help about how the addon works
	ui.setEnableHelpInfoInternal();

	// Set app help text.
	//ui.setEnableHelpInfoApp();//auto forced
	string s = "Examples/00_Simple \n\n";
	s += "This example shows learning to:\n\n";
	s += "- Populate many ofParameters.\n";
	s += "- Populate an ofParamterGroup.\n";
	s += "- Use exponential helpers for sliders.\n";
	s += "- Populate common internal bool toggles:\n";
	s += "  minimize, auto resize, help windows.\n";
	s += "- Set and show internal and app help windows.\n";
	ui.setHelpInfoApp(s);
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ui.Begin();
	{
		if (ui.BeginWindow(bGui))
		{
			ui.AddMinimizerToggle();
			ui.AddAutoResizeToggle();
			ui.AddKeysToggle();
			ui.AddSpacing();
			ui.AddHelpToggle();
			ui.AddHelpInternalToggle();

			ui.AddSpacingBigSeparated();

			//--

			// ImGui widgets are placed here.
			// ofParamaters widgets helpers be easy populate,
			// But you can populate raw ImGui too.

			// Check the state of the internal toggle minimize
			if (ui.isMaximized())
			{
				// This is an ofParameterGroup
				// contained params are populated 
				// as their default widgets styles
				ui.AddGroup(params, SurfingGuiGroupStyle_Collapsed);

				// This is a separator line 
				ui.AddSpacingBigSeparated();

				// This is a big param widget
				ui.Add(amount2, OFX_IM_VSLIDER);
			}
			else
			{
				// This is a default size param widget
				ui.Add(amount2);
			}

			//--

			// This is a param widget
			ui.Add(bPrevious, OFX_IM_TOGGLE_BIG_BORDER_BLINK);

			ui.AddSpacingBigSeparated();

			//--

			// Linear to logarithmic
			ui.AddLabelBig("LINEAR TO LOGARITHMIC");
			ui.Add(vIn, OFX_IM_HSLIDER);
			ui.AddTooltip("linear");

			ui.AddSpacingBig();

			//--

			// Some useful methods to help a bit on align

			//// Align right
			//{
			//	float sz = ImGui::CalcTextSize(">").x;
			//	ofxImGuiSurfing::AddSpacingRightAlign(sz);
			//	ui.AddLabelBig(">");
			//}

			// Align center
			{
				float sz = ImGui::CalcTextSize(">").x;
				float w = ui.getWidgetsWidth();
				ofxImGuiSurfing::AddSpacingRightAlign(sz + w / 2);
				ui.AddLabelBig(">");
			}

			ui.AddSpacingBig();

			ui.Add(vOut1, OFX_IM_HSLIDER_MINI);
			ui.AddTooltip("reversedExponential");
			ui.Add(vOut2, OFX_IM_HSLIDER_MINI);
			ui.AddTooltip("exponential");
			ui.Add(vOut3, OFX_IM_HSLIDER_MINI);
			ui.AddTooltip("squared");

			//--

			ui.EndWindow();
		}
	}
	ui.End();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'g') bGui = !bGui;
}