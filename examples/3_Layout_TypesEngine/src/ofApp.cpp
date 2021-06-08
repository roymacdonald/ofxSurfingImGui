#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);

	//-

	// debug
	bCustom1 = 1; // inmediate customized when populating
	bCustom2 = 1; // pre configured to customize after (ie: inside a group)

	//-

	// prepare parameters
	params.setName("paramsGroup");// main group container
	params2.setName("paramsGroup2");// nested group
	params3.setName("paramsGroup3");// nested group
	params.add(lineWidth.set("lineWidth", 0.5, 0, 1));
	params.add(separation.set("separation", 50, 1, 100));
	params.add(speed.set("speed", 0.5, 0, 1));
	params.add(shapeType.set("shapeType", 0, -50, 50));
	params.add(size.set("size", 100, 0, 100));
	params.add(amount.set("amount", 10, 0, 25));
	params2.add(bMode1.set("Mode1", false));
	params2.add(bMode2.set("Mode2", false));
	params2.add(bMode3.set("Mode3", false));
	params2.add(bMode4.set("Mode4", false));
	params2.add(bPrevious.set("<", false));
	params2.add(bNext.set(">", false));
	params2.add(bEnable.set("Enable", false));
	params2.add(shapeType2.set("shapeType2", 0, -50, 50));
	params2.add(size2.set("size2", 100, 0, 100));
	params2.add(amount2.set("amount2", 10, 0, 25));
	params3.add(lineWidth3.set("lineWidth3", 0.5, 0, 1));
	params3.add(separation3.set("separation3", 50, 1, 100));
	params3.add(speed3.set("speed3", 0.5, 0, 1));
	params.add(params2);
	params.add(params3);

	//-

	guiManager.setup(); // this instantiates and configurates ofxImGui inside the class object.

	//-

	// Queue widgets styles to customize when they will be drawn inside an ofParameterGroup
	// if the parameter widget is not added explicitly, will be populated with the default appearance.
	if (bCustom2)
	{
		// bools
		widgetsManager.AddWidgetConf(bEnable, SurfingWidgetTypes::IM_TOGGLE_BIG);

		widgetsManager.AddWidgetConf(bPrevious, SurfingWidgetTypes::IM_BUTTON_SMALL, true, 2);
		widgetsManager.AddWidgetConf(bNext, SurfingWidgetTypes::IM_BUTTON_SMALL, false, 2, 20);
		
		//widgetsManager.AddWidgetConf(bMode4, SurfingWidgetTypes::IM_CHECKBOX, false, 1, 10);
		
		// floats
		//widgetsManager.AddWidgetConf(lineWidth, SurfingWidgetTypes::IM_SLIDER);
		widgetsManager.AddWidgetConf(separation, SurfingWidgetTypes::IM_STEPPER);
		widgetsManager.AddWidgetConf(speed, SurfingWidgetTypes::IM_DRAG, false, 1, 10);
		
		// ints
		widgetsManager.AddWidgetConf(shapeType, SurfingWidgetTypes::IM_SLIDER);
		widgetsManager.AddWidgetConf(size, SurfingWidgetTypes::IM_STEPPER);
		widgetsManager.AddWidgetConf(amount, SurfingWidgetTypes::IM_DRAG, false, 1, 10);
		
		// bools
		widgetsManager.AddWidgetConf(bMode1, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 2);
		widgetsManager.AddWidgetConf(bMode2, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 2);
		widgetsManager.AddWidgetConf(bMode3, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 2);
		widgetsManager.AddWidgetConf(bMode4, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 2);

		// hide some params from any on-param-group appearance
		widgetsManager.AddWidgetConf(speed3, SurfingWidgetTypes::IM_HIDDEN, false, -1, 50);
		widgetsManager.AddWidgetConf(size2, SurfingWidgetTypes::IM_HIDDEN, false, -1, 50);
		widgetsManager.AddWidgetConf(bPrevious, SurfingWidgetTypes::IM_HIDDEN);
		widgetsManager.AddWidgetConf(bNext, SurfingWidgetTypes::IM_HIDDEN);
		widgetsManager.AddWidgetConf(lineWidth, SurfingWidgetTypes::IM_HIDDEN);
	}

	guiManager.auto_resize = false;
}

//--------------------------------------------------------------
void ofApp::draw()
{
	guiManager.begin();
	{
		//TODO:
		// trying a workaround to fix getUniqueName troubles..
		widgetsManager.resetIndex();

		drawWindow0();
		drawWindow1();
		drawWindow2();
	}
	guiManager.end();
}

//--------------------------------------------------------------
void ofApp::drawWindow0() {

	// window 0 (main)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		if (guiManager.auto_resize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

		ImGui::Begin("Show Windows", &bOpen0, window_flags);
		{
			// round toggles
			ToggleRoundedButton("Show Window 1", &bOpen1);
			ToggleRoundedButton("Show Window 2", &bOpen2);

			ImGui::Dummy(ImVec2(0, 5));

			AddToggleRoundedButton(guiManager.auto_resize, ImVec2(50, 30));

			ImGui::Dummy(ImVec2(0, 20));// spacing
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 10));// spacing
			ImGui::Text("DEBUG TYPES");
			ToggleRoundedButton("bCustom1", &bCustom1);
			ToggleRoundedButton("bCustom2", &bCustom2);// readed on setup only, cant be updated on runtime
		}
		ImGui::End();
	}
}

//--------------------------------------------------------------
void ofApp::drawWindow1() {

	// window 1
	if (bOpen1)
	{
		// a window but using my ofxSurfing_ImGui_LayoutManager.h class helper

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		if (guiManager.auto_resize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

		guiManager.beginWindow("Window 1", &bOpen1, window_flags);
		{
			//-

			// 0. Default bool param
			ofxSurfing::AddParameter(bEnable);
			
			//-

			// 1. Single parameters (out of a paramGroup)
			// instant populate customized widgets
			if (bCustom1)
			{
				// 1.0 update sizes to current window shape
				widgetsManager.refreshPanelShape(); 

				//-

				// 1.1 Toggle full width
				widgetsManager.Add(bEnable, SurfingWidgetTypes::IM_TOGGLE_SMALL);

				// 1.2 Two buttons same line
				if (widgetsManager.Add(bPrevious, SurfingWidgetTypes::IM_BUTTON_SMALL, true, 2)) // half width + same line
				{
					float v = lineWidth.get() - 0.1f;
					lineWidth = v;
					if (bPrevious) bPrevious = false;// required bc no param callback
				}
				if (widgetsManager.Add(bNext, SurfingWidgetTypes::IM_BUTTON_SMALL, false, 2, 20)) // half width + 20px vert spacing
				{
					float v = lineWidth.get() + 0.1f;
					lineWidth = v;
					if (bNext) bNext = false;// required bc no param callback
				}

				// 1.3 A float param
				//ofxSurfing::AddParameter(lineWidth);// default
				//widgetsManager.Add(lineWidth, SurfingWidgetTypes::IM_SLIDER);
				//BUG: duplicated params collide bc UniqueName troubles..
				//widgetsManager.Add(lineWidth, SurfingWidgetTypes::IM_DRAG);
				widgetsManager.Add(lineWidth, SurfingWidgetTypes::IM_STEPPER, false, 2, 20);

				// 1.4 Three widgets in one same row
				// with 20px vert spacing at end
				//widgetsManager.refreshPanelShape(); // update sizes to current window shape
				widgetsManager.Add(bMode1, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 3);
				widgetsManager.Add(bMode2, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 3);
				widgetsManager.Add(bMode3, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 3, 2);

				// 1.5 A check box
				//this force the style no matter if a conf is added (AddWidgetConf) for this param
				widgetsManager.Add(bMode4, SurfingWidgetTypes::IM_CHECKBOX);
				//this will be affected if there's an added conf (AddWidgetConf) for this param
				//ofxSurfing::AddParameter(bMode4);

				// 1.6 spacing
				ImGui::Dummy(ImVec2(0, 20)); // spacing

				// 1.7 A row of four toggles
				widgetsManager.Add(bMode1, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 4);
				widgetsManager.Add(bMode2, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 4);
				widgetsManager.Add(bMode3, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 4);
				widgetsManager.Add(bMode4, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 4);
			}

			ImGui::Dummy(ImVec2(0, 10));// spacing

			//-

			// 2. Parameters inside an ofParameterGroup
			// queue params configs to populate after when drawing they container group
			{
				// group of parameters with customized tree/folder type
				// will be applied to all nested groups inside this parent
				// customization is defined above on setup(): widgetsManager.AddWidgetConf(..

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
				flags |= ImGuiTreeNodeFlags_Framed; // uncomment to draw dark tittle bar
				//flags |= ImGuiTreeNodeFlags_DefaultOpen; // comment to start closed

				//widgetsManager.refreshPanelShape(); // update sizes to current window shape
				ofxSurfing::AddGroup(params, flags);
			}

			//-

			// 3. Another widgets pack
			{
				static bool bMore = false;
				ImGui::Dummy(ImVec2(0, 20));// spacing
				ImGui::Separator();
				ImGui::Dummy(ImVec2(0, 20));// spacing
				ImGui::Text("MORE WIDGETS");
				ToggleRoundedButton("draw", &bMore);
				if (bMore) drawMoreWidgets();
			}
		}
		guiManager.endWindow();
	}
}

//--------------------------------------------------------------
void ofApp::drawWindow2() {

	// window 2
	if (bOpen2)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		if (guiManager.auto_resize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

		guiManager.beginWindow("Window 2", &bOpen2, window_flags);
		{
			// Helpers to calculate sizes
			float _w100;
			float _w50;
			float _h = WIDGETS_HEIGHT;
			_w100 = getImGui_WidgetWidth(1);
			_w50 = getImGui_WidgetWidth(2);

			// Two full width toggles
			//if (AddBigToggle(bEnable)) {}// this is full width (_w100) with standard height (_h)
			//if (AddBigToggle(bEnable, _w100, _h / 2)) {} // same width but half height

			// Two widgets same line/row with the 50% of window panel width 
			if (AddBigButton(bPrevious, _w50, _h)) {
				lineWidth -= 0.1;
				bPrevious = false;
			}
			ImGui::SameLine();
			if (AddBigButton(bNext, _w50, _h)) {
				lineWidth += 0.1;
				bNext = false;
			}

			//-

			ImGui::Dummy(ImVec2(0.0f, 5.0f));// spacing

			// do not updates for below group. Must call:
			//widgetsManager.refreshPanelShape();

			// group parameter with customized tree/folder type
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
			flags |= ImGuiTreeNodeFlags_Framed;
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
			ofxSurfing::AddGroup(params2, flags);

			//-

			// an advanced panels with some typical toggle for extra config (autosize, get mouse over, lock..etc)
			guiManager.drawAdvancedSubPanel();
		}
		guiManager.endWindow();
	}
}

//--------------------------------------------------------------
void ofApp::drawMoreWidgets() {

	// these are pure widgets without window/tree/container

	// add an ofParameterGroup

	// A
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
	flags |= ImGuiTreeNodeFlags_Framed; // uncomment to draw dark tittle bar
	flags |= ImGuiTreeNodeFlags_DefaultOpen; // comment to start closed
	ofxSurfing::AddGroup(params3, flags);

	//// B
	//auto mainSettings = ofxImGui::Settings();
	//ofxImGui::AddGroup(params3, mainSettings);

	// spacing
	ImGui::Dummy(ImVec2(0.0f, 2.0f));// spacing

	//-

	// some custom widgets

	// range_slider.h
	static float v1 = 0;
	static float v2 = 1;
	static float v_min = 0;
	static float v_max = 1;
	static float v3 = 0;
	static float v4 = 1;
	ImGui::RangeSliderFloat("range 1", &v1, &v2, v_min, v_max, "%.1f  %.1f", 1.0f);
	ImGui::RangeSliderFloat("range 2", &v3, &v4, v_min, v_max);

	// vanilla range slider
	static float begin = 10, end = 90;
	static int begin_i = 100, end_i = 1000;
	ImGui::DragFloatRange2("range float", &begin, &end, 0.25f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
	ImGui::DragIntRange2("range int", &begin_i, &end_i, 5, 0, 0, "%.0fcm", "%.0fcm");

	ImGui::Dummy(ImVec2(0.0f, 2.0f));
}
