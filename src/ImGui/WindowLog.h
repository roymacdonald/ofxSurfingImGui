#pragma once

/*
	This code is mainly based on the snippet:
	https://github.com/ocornut/imgui/issues/5796#issuecomment-1288128069
	from @karl0st: https://github.com/karl0st
*/

/*

	TODO:

	improve implemented filter. 
		could be a little slow bc it handles strings.
		for some situations we should replace by:

	filter search from ImGui Demo
	https://github.com/ocornut/imgui/issues/300
	Better from the ImGui Demo

*/

//----

#include "ofMain.h"

#include "ofxImGui.h"
#include "imgui_stdlib.h"

//TODO: some widget fails..
//#include "ofHelpers.h"
//#include "ofxSurfingImGui.h"

namespace ofxImGuiSurfing
{
	class SurfingLog
	{
		// formatting
	private:

		const int maxLenght = 8; // first or tag column width. to make right aligned
		const string strSpacer = "   "; // space between tag column and the second of message column

	public:

		ofParameterGroup params{ "Log Settings" };

	private:

		ofParameter<bool> bOptions{ "OPTIONS", false };
		ofParameter<bool> bLimitedBuffered{ "Limited" , false };
		ofParameter<bool> bPause{ "PAUSE" , false };
		ofParameter<bool> bTight{ "Tight" , true };
		ofParameter<bool> bOneLine{ "OneLine" , false };
		ofParameter<bool> bAutoFit{ "AutoFit" , true };
		ofParameter<int> sizeLogBuffered{ "AmountLines", 20, 1, 100 };
		ofParameter<bool> bAutoScroll{ "AutoScroll" , true };
		ofParameter<bool> bFilter{ "Filter", true };
		ofParameter<string> strSearch{ "Search", "" };

	public:

		//--------------------------------------------------------------
		SurfingLog()
		{
			// Infinite mode
			this->clear();

			// Buffered mode
			//sizeLogBuffered = 20;
			queLogBuffered = std::deque<std::string>();
			this->clearBuffered();

			buildTagsDefault();

			params.add(bPause, bTight, bOneLine, bAutoScroll, bLimitedBuffered, sizeLogBuffered, bOptions, bAutoFit, bFilter, strSearch);

			ofAddListener(params.parameterChangedE(), this, &SurfingLog::Changed_Params);
		};

		//--------------------------------------------------------------
		~SurfingLog()
		{
			this->clear();
			this->clearBuffered();

			ofRemoveListener(params.parameterChangedE(), this, &SurfingLog::Changed_Params);
		};


	private:

		void Changed_Params(ofAbstractParameter& e)
		{
			std::string n = e.getName();
			ofLogNotice("SurfingLog") << n << ": " << e;

			if (n == sizeLogBuffered.getName())
			{
				int diff = queLogBuffered.size() - sizeLogBuffered.get();
				// reduce resize
				if (diff > 0) {
					for (size_t i = 0; i < diff; i++)
					{
						queLogBuffered.pop_front();
					}
				}
			}
		}

		//----

		// Infinite mode

	public:

		struct tagData
		{
			string name;
			ofColor color;
		};

	private:

		vector<tagData> tags;

		void buildTagsDefault()
		{
			// to be used when log a message without tag.
			// will use common text color
			strEmpty = "";
			for (size_t i = 0; i < maxLenght; i++)
			{
				strEmpty += " ";
			}

			AddTag({ "INFO", ofColor::white });
			AddTag({ "VERBOSE", ofColor::white });
			AddTag({ "NOTICE", ofColor::green });
			AddTag({ "WARNING", ofColor::yellow });
			AddTag({ "ERROR", ofColor::red });
		}

		string strEmpty = "";

		string strAlign(string s) const
		{
			//TODO:
			// to make right aligned
			int l = s.length();
			int diff = maxLenght - l;
			if (diff > 0) {
				string spre = "";
				for (size_t i = 0; i < diff; i++)
				{
					spre += " ";
				}
				s = spre + s;
			}

			//s += " ";//end spacing

			return s;
		}

	public:

		// add custom tags passing name and color
		void AddTag(tagData tag)
		{
			////TODO:
			//// to make right aligned
			//const int maxLenght = 10;
			//int l = tag.name.length();
			//int diff = maxLenght - l;
			//if (diff > 0) {
			//	string spre = "";
			//	for (size_t i = 0; i < diff; i++)
			//	{
			//		spre += " ";
			//	}
			//	tag.name = spre + tag.name;
			//}

			tag.name = strAlign(tag.name);

			tags.push_back(tag);
		}

		//--

	private:

		void addBuffered(string msg)
		{
			queLogBuffered.emplace_back(msg);
			if (sizeLogBuffered.get() < queLogBuffered.size())
			{
				queLogBuffered.pop_front();
			}

			if (bAutoScroll)
				this->scroll_to_bottom = true;
		}

		//void add(const char* fmt, ...) 
		void add(string msg, ...)
		{
			const char* fmt = msg.c_str();
			char buf[1024];
			va_list args;
			va_start(args, fmt);
			vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
			buf[IM_ARRAYSIZE(buf) - 1] = 0;
			va_end(args);
			buff.push_back(strdup(buf));

			if (bAutoScroll)
				this->scroll_to_bottom = true;
		};

		void clear() {
			for (int i = 0; i < buff.Size; i++)
				free(buff[i]);
			buff.clear();
		};

		void drawUnlimited() {
			float p = 0;
			float w = ofxImGuiSurfing::getWidgetsWidth(1);
			float h = ofxImGuiSurfing::getWindowHeightFree();
			bool bBorder = true;//used?

			drawUnlimited("Logger", ImGui::GetCursorPos(), { w - p, h - p }, bBorder);
		};

		void drawUnlimited(const char* str_id, const ImVec2 pos, const ImVec2 size, const bool border)
		{
			ImGui::SetCursorPos(pos);

			ImGuiWindowFlags flags = ImGuiWindowFlags_None;

			// border not used?
			ImGui::BeginChild(str_id, size, border, flags);
			{
				for (int i = 0; i < buff.Size; i++)
				{
					const char* item = buff[i];

					//--

					// filter
					if (bFilter) {
						bool bFound = true;
						string s1 = strSearch.get();
						string s2(item);
						if (s1 != "")
						{
							bFound = ofIsStringInString(s2, s1);
						}
						if (!bFound) continue; // skip!
					}

					//--

					ImVec4 color;
					bool has_color = false;

					for (auto t : tags)
					{
						if (strstr(item, t.name.c_str()))
						{
							color = t.color;
							has_color = true;
						}
					}

					if (!has_color) {
						string s = strEmpty + string(item);
						item = s.c_str();
					}

					if (has_color)
						ImGui::PushStyleColor(ImGuiCol_Text, color);

					//--

					// draw text
					if (bTight) ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
					if (bOneLine) ImGui::TextUnformatted(item);
					else ImGui::TextWrapped(item);
					if (bTight) ImGui::PopStyleVar();

					//--

					if (has_color)
						ImGui::PopStyleColor();
				}

				//--

				if (bAutoScroll)
				{
					if (this->scroll_to_bottom)
						ImGui::SetScrollHereY(1.0f);
					this->scroll_to_bottom = false;
				}
			}
			ImGui::EndChild();
		};

		void drawBuffered()
		{
			ImGui::BeginChild("LoggerBuffered");
			{
				// force log size to current window size
				if (bAutoFit)
				{
					ImGuiContext& g = *GImGui;
					float h = g.FontSize;
					if (!bTight) h += g.Style.FramePadding.y;
					float hFree = ofxImGuiSurfing::getWindowContentHeight();
					int amountLines = hFree / h;
					sizeLogBuffered = amountLines;
				}

				//--

				for (auto& m : queLogBuffered)
				{
					// macOS bug
					//for each (string l in logs)

					const char* item = m.c_str();

					//--

					// filter
					if (bFilter) {
						bool bFound = true;
						string s1 = strSearch.get();
						if (s1 != "")
						{
							bFound = ofIsStringInString(m, s1);
						}
						if (!bFound) continue; // skip!
					}

					//--

					ImVec4 color;
					bool has_color = false;

					for (auto t : tags)
					{
						if (strstr(item, t.name.c_str()))
						{
							color = t.color;
							has_color = true;
						}
					}

					if (!has_color) {
						string s = strEmpty + m;
						item = s.c_str();
					}

					if (has_color)
						ImGui::PushStyleColor(ImGuiCol_Text, color);

					//--

					// draw text
					if (bTight) ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
					if (bOneLine) ImGui::TextUnformatted("%s", m.c_str());
					else ImGui::TextWrapped("%s", m.c_str());
					if (bTight) ImGui::PopStyleVar();

					//--

					if (has_color)
						ImGui::PopStyleColor();
				}

				//--

				//if (bAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				//	ImGui::SetScrollHereY(1.0f);

				if (bAutoScroll)
				{
					if (this->scroll_to_bottom)
						ImGui::SetScrollHereY(1.0f);
					this->scroll_to_bottom = false;
				}
			}
			ImGui::EndChild();
		};

	private:

		ImVector<char*> buff;
		bool scroll_to_bottom;

		static char* strdup(const char* s) {
			IM_ASSERT(s);
			size_t len = strlen(s) + 1;
			void* buf = malloc(len);
			IM_ASSERT(buf);
			return (char*)memcpy(buf, (const void*)s, len);
		};

		//--

		// Buffered mode

	public:

		//--------------------------------------------------------------
		//void setLogBufferedSize(int size) { sizeLogBuffered = size; };

		//--------------------------------------------------------------
		void clearBuffered() { queLogBuffered.clear(); };

	private:

		std::deque<std::string> queLogBuffered;

	public:

		//TODO:
		//--------------------------------------------------------------
		void Add(std::string msg, string nameTag)
		{
			// A. search if tag exists
			for (size_t i = 0; i < tags.size(); i++)
			{
				nameTag = strAlign(nameTag);

				if (nameTag == tags[i].name) {
					Add(msg, (int)i);
					return;
				}
			}

			// B. tag do not exists
			// print as default
			ofLogWarning("SurfingLog") << "The tag " << nameTag << " do not exist. We will use the default tag.";
			Add(msg);
		}

		//--------------------------------------------------------------
		void Add(std::string msg, int itag = -1)
		{
			if (bPause) return;

			string s = "";
			string stag = "";

			if (itag < tags.size())
			{
				stag = tags[itag].name;
				s += stag;
				s += strSpacer;
			}
			s += msg;

			//--

			if (!bLimitedBuffered) {
				add(s);
			}
			else {
				addBuffered(s);
			}
		};

	public:

		//--------------------------------------------------------------
		void drawImGui(ofParameter<bool>& bGui)
		{
			if (!bGui) return;

			std::string name = bGui.getName();
			ImGuiWindowFlags flags;
			flags = ImGuiWindowFlags_None;
			flags |= ImGuiWindowFlags_NoScrollbar;

			// Window shape
			{
				const int LOG_WINDOW_SIZE = 240;//minimal width
				float hmin = (bOptions.get() ? 200 : 130);//minimal height
				ImGuiCond cond = ImGuiCond_FirstUseEver;

				// app window
				float w = ofGetWidth();
				float h = ofGetHeight();
				ImGui::SetNextWindowPos(ImVec2(w - LOG_WINDOW_SIZE - 10, 20), cond);
				ImGui::SetNextWindowSize(ImVec2(LOG_WINDOW_SIZE, h - 100), cond);

				// constraints
				ImVec2 size_min = ImVec2(LOG_WINDOW_SIZE, hmin);
				ImVec2 size_max = ImVec2(FLT_MAX, FLT_MAX);
				ImGui::SetNextWindowSizeConstraints(size_min, size_max);
			}

			//--

			if (!ImGui::Begin(name.c_str(), (bool*)&bGui.get(), flags))
			{
				ImGui::End();
				return;
			}

			ofxImGuiSurfing::AddSpacing();

			float _h = 1.5f * ofxImGuiSurfing::getWidgetsHeightUnit();
			float _w = ofxImGuiSurfing::getWidgetsWidth(1);
			float _spx = ofxImGuiSurfing::getWidgetsSpacingX();

			//--

			float _hh = ofxImGuiSurfing::getWidgetsHeightUnit();
			ofxImGuiSurfing::AddToggleRoundedButton(bOptions, 1.25 * _hh, true);
			ImGui::Spacing();
			if (bOptions)
			{
				float _w2 = ofxImGuiSurfing::getWidgetsWidth(2);
				ofxImGuiSurfing::AddBigToggle(bPause, ImVec2(_w2, _h), true, true);
				ImGui::SameLine();
				if (ImGui::Button("CLEAR", ImVec2(_w2, _h)))
				{
					this->clear();
					this->clearBuffered();
					//this->ClearFilter();
				}
				ofxImGuiSurfing::AddSpacing();

				//ImGuiStyle * style = &ImGui::GetStyle();
				//float h3 = style[FrameRect]
				float w3 = 80;
				ofxImGuiSurfing::AddBigToggleNamed(bLimitedBuffered, w3, 0.65*_h, "LIMITED", "UNLIMITED");
				//ofxImGuiSurfing::AddBigToggle(bLimitedBuffered, 50, _hh, true);
				//ofxImGuiSurfing::AddToggleRoundedButton(bLimitedBuffered, _hh, true);
				//ofxImGuiSurfing::AddCheckBox(bLimitedBuffered);
				{
					string s = bLimitedBuffered ? "Buffer size of lines is limited" : "Buffer size of lines is unlimited.";
					ofxImGuiSurfing::AddTooltip2(s);
				}

				//--

				if (bLimitedBuffered)
				{
					ofxImGuiSurfing::SameLine();
					ofxImGuiSurfing::AddCheckBox(bAutoFit);
					string s = "Resize buffer to fit window size \nas amount of expected text lines.";
					s += "\nAmount Lines: " + ofToString(queLogBuffered.size());
					ofxImGuiSurfing::AddTooltip2(s);
				}
				if (bLimitedBuffered && !bAutoFit)
				{
					ofxImGuiSurfing::SameLine();

					//TODO: not working
					//ImGui::DragInt("Size", &sizeLogBuffered, 1, 100);
					//ofxImGuiSurfing::AddParameter(sizeLogBuffered);
					// fix
					{
						ImGui::PushItemWidth(90);
						string name = sizeLogBuffered.getName();
						auto tmpRefi = sizeLogBuffered.get();
						string n = "##STEPPERint" + name;// +ofToString(1);
						const ImU32 u32_one = 1;
						ImGui::PushID(n.c_str());
						if (ImGui::InputScalar(sizeLogBuffered.getName().c_str(), ImGuiDataType_S32, (int*)&tmpRefi, &u32_one, NULL, "%d"))
						{
							tmpRefi = ofClamp(tmpRefi, sizeLogBuffered.getMin(), sizeLogBuffered.getMax());
							sizeLogBuffered.set(tmpRefi);
						}
						ImGui::PopID();
						ImGui::PopItemWidth();
					}
				}

				//--

				if (!bLimitedBuffered)
				{
					ofxImGuiSurfing::SameLine();
					ofxImGuiSurfing::AddCheckBox(bAutoScroll);
					//ImGui::Dummy({ 0,10 });
					//ofxImGuiSurfing::AddToggleRoundedButton(bAutoScroll, _hh, true);
					string s = "Amount Lines: " + ofToString(buff.size());
					ofxImGuiSurfing::AddTooltip2(s);
				}

				ofxImGuiSurfing::SameLine();
				ofxImGuiSurfing::AddCheckBox(bTight);

				ofxImGuiSurfing::SameLine();
				ofxImGuiSurfing::AddCheckBox(bOneLine);

				//string s;
				//if (bLimitedBuffered)s = "Limited Size";
				//else s = "Infinite Size";
				//ofxImGuiSurfing::AddTooltip(s);

				//--

				ImGui::SameLine();
				bool copy = ImGui::Button("Copy");
				string s = "Copy Log to Clipboard";
				ofxImGuiSurfing::AddTooltip2(s);
				if (copy) ImGui::LogToClipboard();

				//--

				// filter

				ImGui::SameLine();
				ofxImGuiSurfing::AddCheckBox(bFilter);
				{
					string s = bFilter ? "Write your pattern\nto filter log lines" : "Disabled";
					ofxImGuiSurfing::AddTooltip2(s);
				}
				if (bFilter) {
					ImGui::SameLine();
					static bool bReturn;
					auto& tmpRef = strSearch.get();
					{
						float _w = 60;
						//float _w = getWidgetsWidth() * 0.9f;
						string s = tmpRef.c_str();
						ImGui::PushItemWidth(_w);
						{
							bReturn = ImGui::InputText("Pattern", &s);
							if (bReturn)
							{
								ofLogNotice("ofxSurfingImGui") << "InputText:" << s.c_str();
								strSearch.set(s);
							}
						}
						ImGui::PopItemWidth();
					}
				}
			}

			//--

			ofxImGuiSurfing::AddSpacingSeparated();
			ofxImGuiSurfing::AddSpacing();

			//--

			if (!bLimitedBuffered)
			{
				drawUnlimited();
			}
			else
			{
				drawBuffered();
			}

			//--

			ImGui::End();
		}
	};

}




//---




/*
ImGuiStyle* style = &ImGui::GetStyle();
const ImVec4 c = style->Colors[ImGuiCol_Text];
ofFloatColor c1 = ofColor::green;
ofFloatColor c2 = ofColor::yellow;
ofFloatColor c3 = ofColor::red;
ofFloatColor c4 = ofColor::orange;

if (strstr(item, "[INFO]")) {
	color = c;
	has_color = true;
}

else if (strstr(item, "[VERBOSE]")) {
	color = c;
	has_color = true;
}

else if (strstr(item, "[NOTICE]")) {
	color = ImVec4(c1);
	has_color = true;
}

else if (strstr(item, "[WARNING]")) {
	color = ImVec4(c2);
	has_color = true;
}

else if (strstr(item, "[ERROR]")) {
	color = ImVec4(c3);
	has_color = true;
}

else if (strncmp(item, "# ", 2) == 0) {
	color = ImVec4(c4);
	has_color = true;
}
*/

//----


// TODO:
// Save log file to disk
// from ofxMyUtil from https://github.com/Iwanaka

////--------------------------------------------------------------
//void SurfingLog::AddTextToFile(std::string str, std::string path, bool append, bool withTimeStamp) {
//	AddText(str);
//	if (withTimeStamp) str = ofGetTimestampString() + " " + str;
//	TextToFile(path, str, append);
//}

//namespace ofxImGuiSurfing {
//
//	//--------------------------------------------------------------
//	inline bool TextToFile(const string& Path, const stringstream& Args, bool Append)
//	{
//		//ofFile f(path, ofFile::ReadWrite);
//		//if (!f.exists()) f.create();
//
//		filebuf fb;
//		if (Append) fb.open(Path, ios::app);
//		else fb.open(Path, ios::out);
//
//		if (!fb.is_open()) return false;
//
//		ostream os(&fb);
//		stringstream ss(Args.str());
//
//		if (ss.fail()) return false;
//
//		string temp;
//		while (getline(ss, temp))
//		{
//			if (temp != "")
//			{
//				os << temp << endl;
//			}
//		}
//
//		fb.close();
//		return true;
//	}
//	//--------------------------------------------------------------
//	inline bool TextToFile(const string& Path, const char *Args, bool Append)
//	{
//		stringstream ss; ss << Args;
//		return TextToFile(Path, ss, Append);
//	}
//	//--------------------------------------------------------------
//	inline bool TextToFile(const std::string& Path, std::string Args, bool Append)
//	{
//		stringstream ss; ss << Args;
//		return TextToFile(Path, ss, Append);
//	}
//}