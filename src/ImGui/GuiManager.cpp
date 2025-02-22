
#include "GuiManager.h"

//--------------------------------------------------------------
SurfingGuiManager::SurfingGuiManager()
{
	//TODO:
	// Simplify namespaces!
	namespace ofxSurfingImGui = ofxImGuiSurfing;

	path_Global = "Gui/";
	// default path that will be used appended,
	// or alone if setName(.. is not called!

	//----

	// ofApp / core callbacks

	//TODO:
	// Fix exit exceptions on RF..
	int minValue = std::numeric_limits<int>::min();
	ofAddListener(ofEvents().exit, this, &SurfingGuiManager::exit, minValue);

	//--

	ofAddListener(ofEvents().keyPressed, this, &SurfingGuiManager::keyPressed);

	// Auto call draw. Only to draw help boxes / OF native info. ?
	ofAddListener(ofEvents().update, this, &SurfingGuiManager::update, OF_EVENT_ORDER_AFTER_APP);
	//ofAddListener(ofEvents().update, this, &SurfingGuiManager::update, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().draw, this, &SurfingGuiManager::draw, OF_EVENT_ORDER_AFTER_APP);

	//----

	// App callbacks

	ofAddListener(params_AppSettings.parameterChangedE(), this, &SurfingGuiManager::Changed_Params);
	ofAddListener(params_bGuiToggles.parameterChangedE(), this, &SurfingGuiManager::Changed_Params);

	//--

	// These params are handled into file settings!
	params_Advanced.add(bAutoResize);
	params_Advanced.add(bExtra);
	params_Advanced.add(bMinimize);
	params_Advanced.add(bAdvanced);
	params_Advanced.add(bGui_GameMode);
	params_Advanced.add(bSolo_GameMode);
	params_Advanced.add(bKeys);
	params_Advanced.add(bMouseWheel);
	params_Advanced.add(bMouseWheelFlip);
	params_Advanced.add(bHelp);
	params_Advanced.add(bHelpInternal);
	params_Advanced.add(bDebug);
	params_Advanced.add(bThemeUiAlt);

	params_Advanced.add(bDebugDebugger);
#ifdef OFX_USE_DEBUGGER
	debugger.bGui.makeReferenceTo(bDebugDebugger);
	params_Advanced.add(debugger.params);
#endif

#ifdef OFX_USE_NOTIFIER
	params_Advanced.add(notifier.bGui);
#endif

	params_Advanced.add(bLog);
	params_Advanced.add(bLogKeys);
	params_Advanced.add(log.params);
	params_Advanced.add(bNotifier);
	params_Advanced.add(bReset);//TODO:
	params_Advanced.add(bReset_Window);//TODO:
	params_Advanced.add(bLockMove);//TODO:
	params_Advanced.add(bNoScroll);//TODO:

	params_Advanced.add(bSolo);//used on layout presets engine
	//params_Advanced.add(bMinimize_Presets);//need to rename

	//params_Advanced.add(bLinkGlobal);
	//params_Advanced.add(bLinked);
	//params_Advanced.add(windowsOrganizer.pad);
	//params_Advanced.add(bLandscape);//TODO:

	//--

	//TODO: For functions not implemented yet.

	// Exclude from settings
	//bAdvanced.setSerializable(false);
	//bExtra.setSerializable(false);
	bLockMove.setSerializable(false);
	bNoScroll.setSerializable(false);
	bReset.setSerializable(false);
	bReset_Window.setSerializable(false);

	//--

	////TODO: BUG?
	//// it seems than requires to be false when using multi-context/instances
	//// if is settled to true, sometimes it hangs and gui do not refresh/freezes.
	//bAutoDraw = false;
}

//--------------------------------------------------------------
SurfingGuiManager::~SurfingGuiManager() {
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << "Destructor!";

	if (!bDoneExit)
	{
		exit();

		ofLogWarning("ofxSurfingImGui") << "exit() was not called yet as expected.";
		ofLogWarning("ofxSurfingImGui") << "Now forcing exit() here in destructor!";
	}
	else
	{
		ofLogWarning("ofxSurfingImGui") << "exit() was already called before as expected.";
		ofLogWarning("ofxSurfingImGui") << "It was already done!";
		ofLogWarning("ofxSurfingImGui") << "So we successfully omitted calling exit() herre in destructor.";
	}

	//TODO:
	//// Delete pointers
	//delete customFont;
	//for (size_t i = 0; i < customFonts.size(); i++) delete customFonts[i];

	ofRemoveListener(ofEvents().exit, this, &SurfingGuiManager::exit);
}

//--------------------------------------------------------------
void SurfingGuiManager::exit(ofEventArgs& e)
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " > exit(ofEventArgs& e)";
	ofLogNotice("ofxSurfingImGui") << "Called by ofEvents().exit with maximum priority! Now calling exit().";
	ofLogNotice("ofxSurfingImGui") << "Then we avoid that is been called by the object destructor.";

	exit();
}

//--------------------------------------------------------------
void SurfingGuiManager::exit()
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << "exit()";
	//return;//TODO: fixing crash

	ofRemoveListener(ofEvents().keyPressed, this, &SurfingGuiManager::keyPressed);

	ofRemoveListener(ofEvents().update, this, &SurfingGuiManager::draw, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().draw, this, &SurfingGuiManager::draw, OF_EVENT_ORDER_BEFORE_APP);

	ofRemoveListener(params_LayoutPresetsStates.parameterChangedE(), this, &SurfingGuiManager::Changed_Params);
	ofRemoveListener(params_AppSettings.parameterChangedE(), this, &SurfingGuiManager::Changed_Params);
	ofRemoveListener(params_bGuiToggles.parameterChangedE(), this, &SurfingGuiManager::Changed_Params);

	ofLogNotice("ofxSurfingImGui") << "Listener has been removed. Now we are going to save the session settings.";
	saveAppSettings();

	bDoneExit = true;
}

//--

//--------------------------------------------------------------
void SurfingGuiManager::setup(ofxImGuiSurfing::SurfingGuiMode mode) {
	if (bDoneSetup)
	{
		ofLogWarning(__FUNCTION__) << "Setup was already done. Skipping this call!";
	}

	surfingImGuiMode = mode;

	switch (surfingImGuiMode)
	{

	case ofxImGuiSurfing::IM_GUI_MODE_UNKNOWN:
		// nothing to do

		break;

	case ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED:
		setAutoSaveSettings(true);
		//setImGuiAutodraw(true);
		// This instantiates and configures ofxImGui inside the class object.
		setupInitiate();

		break;

	case ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING:
		numPresetsDefault = DEFAULT_AMOUNT_PRESETS;
		setAutoSaveSettings(true);
		setupDocking();
		setupInitiate();

		break;

	case ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_SINGLE:
		setAutoSaveSettings(true);
		//setImGuiAutodraw(true);
		// This instantiates and configures ofxImGui inside the class object.
		setupInitiate();

		break;

		//	//TODO:
		//case ofxImGuiSurfing::IM_GUI_MODE_SPECIAL_WINDOWS:
		//	setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
		//	setup();
		//	break;

	case ofxImGuiSurfing::IM_GUI_MODE_REFERENCED: //TODO:
		setAutoSaveSettings(false);

		break;

		// ui.Begin(); it's bypassed internally then can remain uncommented.
	case ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED:
		setAutoSaveSettings(false);

		break;

	}

	//--

	// Clear

	params_Layouts.clear();
	params_LayoutsExtra.clear();
	params_LayoutsVisible.clear();

	//--

	bDoneSetup = true;
}

//--------------------------------------------------------------
void SurfingGuiManager::setup() // We will use the most common mode, to avoid to have to require any argument.
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__);

	setup(IM_GUI_MODE_INSTANTIATED);
}

//--------------------------------------------------------------
void SurfingGuiManager::setupDocking()
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__);

	surfingImGuiMode = ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING;

	setAutoSaveSettings(true);
	setImGuiDocking(true);
	setImGuiDockingModeCentered(true);
	//setImGuiAutodraw(true);
}

//--

//--------------------------------------------------------------
void SurfingGuiManager::setupInitiate()
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__);

	// For using internal instantiated GUI.
	// Called by all modes except when using the external scope modes aka not instantiated.
	// In that case we will only use the widgets helpers into a parent/external ImGui context!
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) {

		ofLogWarning("ofxSurfingImGui") << "Aborted setupIntitiate() bc surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED";
		return;
	}

	//--

	// MouseWheel link
	_ui.bMouseWheel.makeReferenceTo(bMouseWheel);
	_ui.bMouseWheelFlip.makeReferenceTo(bMouseWheelFlip);

	// MouseWheel link
	windowsOrganizer.bDebug.makeReferenceTo(bDebug);

	// Minimizes link
	bMinimize_Presets.makeReferenceTo(bMinimize);
	//bMinimize_Panels.makeReferenceTo(bMinimize);

	//--

	//TODO:
	// When using docking/presets mode
	// we force enable special windows by default
	if (surfingImGuiMode == IM_GUI_MODE_INSTANTIATED_DOCKING)
	{
		this->setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);

		// Add LINK to extra params
		// that allows that each presets could have his own link state enabled or disabled.
		// to allow linking or floating windows.
		this->addExtraParamToLayoutPresets(bLinked);
	}

	//--

	// Main initialization for ImGui object!

	setupImGui();

	//--

	// Settings
	{
		// Default root path to contain all the file settings!
		//path_Global = "Gui/";

		// Layout Presets
		path_ImLayouts = path_Global + "Presets/";

		// Create folders if required
		CheckFolder(path_Global);

		if (bUseLayoutPresetsManager) CheckFolder(path_ImLayouts);

		//--

		// Some internal settings
		path_AppSettings = path_Global + nameLabel + "_UI_" + bGui_LayoutsPanels.getName() + ".json";
		// this allow multiple add-ons instances with non shared settings.

		params_AppSettings.add(params_Advanced);
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::setup(ofxImGui::Gui& _gui) { //TODO: should be tested. For using external instantiated ImGui
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__);

	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//-

	guiPtr = &_gui;

	setupImGui();
}

//--------------------------------------------------------------
void SurfingGuiManager::setupImGuiFonts()
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__);

	std::string _fontName;
	float _fontSizeParam;

	// WARNING: will not crash or notify you if the font files are not present!
	_fontName = FONT_DEFAULT_FILE;
	_fontSizeParam = FONT_DEFAULT_SIZE;

	std::string _path = "assets/fonts/";
	// assets folder

	// To check if default font file exists
	ofFile fileToRead(_path + _fontName);
	bool b = fileToRead.exists();

	// If font not located..
	// We can set an alternative font like a legacy font

	if (!b)
	{
		_fontName = FONT_DEFAULT_FILE_LEGACY;
		_fontSizeParam = FONT_DEFAULT_SIZE_LEGACY;
	}

	// Then check if legacy font file exists

	ofFile fileToRead2(_path + _fontName);
	bool b2 = fileToRead2.exists();
	if (b2)
	{
		//gui.begin();

		// Font default
		pushFont(_path + _fontName, _fontSizeParam); // queue default font too

		// Font big
		pushFont(_path + _fontName, _fontSizeParam * 1.5f); // queue big font too

		// Font huge
		pushFont(_path + _fontName, _fontSizeParam * 2.5f); // queue huge font too

		// Font huge xxl
		pushFont(_path + _fontName, _fontSizeParam * 5.f); // queue huge xxl font too

		//TODO: 
		// WARNING! 
		// these names could be copied to GuiManager too!
		// take care if both sizes fonts/names changed! 
		// this is hardcoded now!
		// Font sizes
		namesCustomFonts.clear();
		namesCustomFonts.push_back("DEFAULT");
		namesCustomFonts.push_back("BIG");
		namesCustomFonts.push_back("HUGE");
		namesCustomFonts.push_back("HUGE_XXL");

		//--

		// Set default
		addFont(_path + _fontName, _fontSizeParam);

		//gui.end();
	}

	// Legacy not found neither,
	else
	{
		ofLogError("ofxSurfingImGui:setupImGuiFonts") << " Expected file fonts not found!";
		ofLogError("ofxSurfingImGui") << "ImGui will use his own bundled default font.";
		ofLogError("ofxSurfingImGui") << "Some ofxSurfingImGui styles will be omitted.";
	}

	/*
	//TODO:
	// Our last attempt is to try the a default bundled oF font
	else
	{
		//ofTrueTypeFont::lo
		//		//TODO: how to get the absolute path of OF_TTF_MONO?
		//		//string path = std::filesystem::absolute(OF_TTF_MONO).string();
		//		string path = winFontPathByName(OF_TTF_MONO.string());
		//
		//////#elif defined(TARGET_WIN32)
		////if (fontname == OF_TTF_SANS) {
		////	fontname = "Arial";
		////}
		////else if (fontname == OF_TTF_SERIF) {
		////	fontname = "Times New Roman";
		////}
		////else if (fontname == OF_TTF_MONO) {
		////	fontname = "Courier New";
		////}
		////filename = winFontPathByName(fontname.string());
		//////#endif

		_fontSizeParam = FONT_DEFAULT_SIZE_LEGACY;

		// Font default
		pushFont(path, _fontSizeParam); // queue default font too

		// Font big
		pushFont(path, _fontSizeParam * 1.5f); // queue big font too

		// Font huge
		pushFont(path, _fontSizeParam * 2.5f); // queue huge font too

		//--

		// Set default
		addFont(path, _fontSizeParam);
	}
	*/

	// If we don't found any of the font,
	// we will let ImGui to use his default bundled font,
	// The label sizes widgets, and the 3 font types will not be working,
	// all styles will be the same.
}

//--------------------------------------------------------------
void SurfingGuiManager::setupImGui()
{
	ofLogNotice("ofxSurfingImGui:setupImGui");

	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//--

	ImGuiConfigFlags flags = ImGuiConfigFlags_None;

	// Hard coded settings

	if (bDockingLayoutPresetsEngine) flags += ImGuiConfigFlags_DockingEnable;
	if (bViewport) flags += ImGuiConfigFlags_ViewportsEnable;

	// Setup ImGui with the appropriate config flags

	if (guiPtr != nullptr) guiPtr->setup(nullptr, bAutoDraw, flags, bRestoreIniSettings, bMouseCursorFromImGui);
	else gui.setup(nullptr, bAutoDraw, flags, bRestoreIniSettings, bMouseCursorFromImGui);

	// Uncomment below to perform docking with SHIFT key
	// Gives a better user experience, matter of opinion.

	if (bDockingLayoutPresetsEngine) ImGui::GetIO().ConfigDockingWithShift = true;

	// Uncomment below to "force" all ImGui windows to be standalone
	//ImGui::GetIO().ConfigViewportsNoAutoMerge=true;

	//--

	// Fonts
#ifdef OFX_IMGUI_USE_FONTS
	setupImGuiFonts();
#endif

	//--

	//TODO:
	//setupImGuiTheme();
}

//--------------------------------------------------------------
void SurfingGuiManager::setupImGuiTheme()
{
	ofLogNotice("ofxSurfingImGui:setupImGuiTheme");

	//--

//	// Fonts
//#ifdef OFX_IMGUI_USE_FONTS
//	setupImGuiFonts();
//#endif

	//--

	// Theme
	// Colors and widgets/spacing sizes

	//// A. Hardcoded
	//ofxImGuiSurfing::ImGui_ThemeMoebiusSurfingV2();

	//TODO:
	// B. Loading a file
	string pNight = THEME_NAME_NIGHT;
	string pDay = THEME_NAME_DAY;
	bool bLoaded = false;

	//TODO: Why do not works?
	// fails when including "SurfingThemeEditor.h" into GuiManager.h.
	//if (ofxImGuiSurfing::SurfingThemes::themeStyle == THEME_NIGHT)
	//	bLoaded = ofxImGuiSurfing::SurfingThemes::loadThemeFileByName(pNight);
	//else if (ofxImGuiSurfing::SurfingThemes::themeStyle == THEME_DAY)
	//	bLoaded = ofxImGuiSurfing::SurfingThemes::loadThemeFileByName(pDay);

	string name;
	if (bThemeUiAlt) name = pDay;
	else name = pNight;
	string pathThemes = path_Global + "themes/";
	//string pathThemes = "Gui/themes/";
	string pathTheme = pathThemes + name;
	string p = ofToDataPath(pathTheme);
	ofLogNotice("ofxSurfingImGui::loadThemeFileByName") << "Load from " << p;
	ImGui::LoadStyleFrom(p.c_str());
	ofFile f;
	bLoaded = f.doesFileExist(p);

	// If theme files are not found, then it will load a hardcoded theme!
	if (!bLoaded) {
		if (!bThemeUiAlt) ofxImGuiSurfing::ImGui_ThemeMoebiusSurfingV2();//dark
		else ofxImGuiSurfing::ImGui_ThemeDearImGuiLight();//light
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::startup()
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__);

	//--

	setupImGuiTheme();

	//--

	//TODO:
	//workflow
	// enable organizer by default.
	// no need to call manually.
	setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);

	//--

	// Finally the last initialization process step

	if (bDockingLayoutPresetsEngine)
	{
		// Default Layout with 4 presets.
		setupLayout(DEFAULT_AMOUNT_PRESETS);

		//--

		appLayoutIndex = appLayoutIndex;

		//--

		////TODO:
		// //// workflow
		//// Force enable special windows always
		//this->setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	}

	//--

	// Special Windows Organizer

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		initiateWindowsOrganizer();

		//--

		if (surfingImGuiMode == IM_GUI_MODE_INSTANTIATED_DOCKING)
		{
			//// workflow
			//windowsOrganizer.setHideWindows(true);
		}

		// Docking mode has the GUI toggles in other panels..
		else
		{
			//// workflow
			//// force disable to avoid collide settings layout!
			//windowsOrganizer.bGui_Organizer = false;
		}

		//TODO: disabled bc must fix behavior if enabled
		// workflow
		//bLinked = false;
	}

	//--

	//if (surfingImGuiMode != IM_GUI_MODE_INSTANTIATED_DOCKING)
	//{
	//}

	//--

	// Log
	// pass fonts to allow styles switching
	log.setCustomFonts(customFonts, namesCustomFonts);
	//log.setCustomFontsNames(namesCustomFonts);
	//log.bGui.makeReferenceTo(bLog);

	//TODO: trying to redirect all logs to the imgui log window.
	//log.setRedirectConsole();

	//--

	// Notifier
#ifdef OFX_USE_NOTIFIER

	notifier.setPath(path_Global);
	notifier.setup();
	//notifier.setDuration(4000);

	//notifier.setIndexFont(0);
	//notifier.setIndexFont(1);
	//notifier.setIndexFont(2);
	//notifier.setIndexFont(3);

#endif

	//--

	// Two Help Boxes
	{
		// A. Help Text Box internal

		helpInternal.setCustomFonts(customFonts);
		//helpInternal.setName(bHelpInternal.getName());
		helpInternal.bGui.makeReferenceTo(bHelpInternal);
		helpInternal.setTitle(bHelpInternal.getName());

		doBuildHelpInfo();

		//--

		// B. Help Text Box app

		helpApp.setCustomFonts(customFonts);
		//helpApp.setName(bHelp.getName());
		helpApp.bGui.makeReferenceTo(bHelp);
		helpApp.setTitle(bHelp.getName());

		//--

		//setEnableHelpInfoInternal(true);
	}

	//----

	// Startup

	//--

	// Load App Session Settings

	// When opening for first time,
	// we set some default settings.

	// Load some internal settings
	bool bNoSettingsFound = !(loadAppSettings());

	// Will return false if settings file do not exist.
	// That happens when started for first time or after OF_APP/bin cleaning!
	if (bNoSettingsFound)
	{
		ofLogWarning("ofxSurfingImGui") << "No file settings found!";
		ofLogWarning("ofxSurfingImGui") << "Probably the app is opening for the the first time.";
		ofLogWarning("ofxSurfingImGui") << "We will reset the windows layout to avoid overlap of windows.";

		// hide all special windows, if they are queued.
		setShowAllPanels(false);

		// forced default params
		bHelpInternal = true;
		bMinimize = false;

		//// help
		//helpApp.setPosition(400, 10);
		//helpInternal.setPosition(800, 10);

		// workflow
		bDoForceStartupResetLayout = true;
	}

	//--

	bDoneStartup = true;

	//windowsOrganizer.bGui_Global = true;
}

//----

// Help (Internal)

//--------------------------------------------------------------
void SurfingGuiManager::doBuildHelpInfo()
{
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__);

	// we recreate the help info during runtime when some variable changed

	string l1 = "-----------------------------------\n";//divider
	string l2 = "\n" + l1 + "\n";//spaciated divider
	//left indent
	//string l3 = "  ";
	string l3 = "";
	string l4 = "     ";//spacing 1st column

	//--

	helpInfo = "";

	//if(!helpInternal.bHeader) helpInfo += "HELP INTERNAL \n\n";

	//helpInfo += "Gui Manager \n\n";
	//helpInfo += "Double click to EDIT/LOCK \n\n";
	//helpInfo += l;
	//helpInfo += "\n";

	//TODO: check mode
	//if (0) {
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING) {
		helpInfo += "LAYOUTS PRESETS ENGINE \n";
		helpInfo += "\n";
		//helpInfo += l2;
	}

	helpInfo += l3 + "KEY COMMANDS \n";
	helpInfo += "\n";

	string st = "  ";

	//if (!bMinimize)
	{
		if (bKeys)
			helpInfo += " " + l4 + "Keys          " + st + " ON  \n";
		else
			helpInfo += " " + l4 + "Keys          " + st + " OFF \n";

		if (bMinimize)
			helpInfo += string(bKeys ? "`" : " ") + l4 + "Minimize      " + st + " ON  \n";
		else
			helpInfo += string(bKeys ? "`" : " ") + l4 + "Minimize      " + st + " OFF \n";

		if (bDebug)
			helpInfo += string(bKeys ? "D" : " ") + l4 + "Debug         " + st + " ON  \n";
		else
			helpInfo += string(bKeys ? "D" : " ") + l4 + "Debug         " + st + " OFF \n";

		if (bExtra)
			helpInfo += string(bKeys ? "E" : " ") + l4 + "Extra         " + st + " ON  \n";
		else
			helpInfo += string(bKeys ? "E" : " ") + l4 + "Extra         " + st + " OFF \n";

		if (bLog)
			helpInfo += string(bKeys ? "L" : " ") + l4 + "Log           " + st + " ON  \n";
		else
			helpInfo += string(bKeys ? "L" : " ") + l4 + "Log           " + st + " OFF \n";

		if (bHelp)
			helpInfo += string(bKeys ? "H" : " ") + l4 + "Help App      " + st + " ON  \n";
		else
			helpInfo += string(bKeys ? "H" : " ") + l4 + "Help App      " + st + " OFF \n";

		if (bHelpInternal)
			helpInfo += string(bKeys ? "I" : " ") + l4 + "Help Internal " + st + " ON";
		else
			helpInfo += string(bKeys ? "I" : " ") + l4 + "Help Internal " + st + " OFF";


		//helpInfo += "\n";
		//helpInfo += l2;
	}
	//else helpInfo += "\n";

	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING) {
		if (bDockingLayoutPresetsEngine)
		{
			helpInfo += l3 + "PRESETS \n";
			//helpInfo += "\n";

			helpInfo += "F1 F2 F3 F4 \n";
			helpInfo += "\n";

			helpInfo += l3 + "SECTIONS \n";
			//helpInfo += "\n";

			helpInfo += "F5          LAYOUTS \n";
			helpInfo += "F6          PANELS \n";
			if (!bMinimize) helpInfo += "F7          MANAGER \n";
			helpInfo += "\n";

			helpInfo += l3 + "PANELS \n";
			//helpInfo += "\n";

			helpInfo += "Ctrl+ \n";
			helpInfo += "F1 .. F8    Panel # \n";
			helpInfo += "A           All  \n";
			helpInfo += "N           None \n";

			if (bSolo)
				helpInfo += "S           Solo          " + st + " ON  \n";
			else
				helpInfo += "S           Solo          " + st + " OFF \n";

			//--

			if (!bMinimize)
			{
				helpInfo += l2;

				helpInfo += l3 + "HOW TO \n";
				//helpInfo += "\n";

				helpInfo += "1. Click on P1 P2 P3 P4 \nto pick a PRESET \n";
				//helpInfo += "\n";
				helpInfo += "2. Toggle the PANELS \nthat you want to be visible \nor hidden \n";
				//helpInfo += "\n";
				helpInfo += "3. Layout the PANELS around \nthe App view port \n";
				//helpInfo += "\n";
				helpInfo += "4. Pick another PRESET \n";

				helpInfo += "\n";
				//helpInfo += l2;

				helpInfo += l3 + "MORE TIPS \n";
				//helpInfo += "\n";

				helpInfo += "- Disable the Minimize toggle \nto show more controls. \n";
				//helpInfo += "\n";
				helpInfo += "- Explore more deep into \nLAYOUT, PANELS \nand MANAGER Windows. \n";
				//helpInfo += "\n";
				helpInfo += "- Each PRESET can be defined \nas a particular App Mode \nor an activated section. \n";
				//helpInfo += "\n";
				helpInfo += "- When no PRESET is enabled \nall PANELS will be hidden. \n";
				//helpInfo += "\n";
				helpInfo += "- On Docking Mode, \npress Shift when dragging \na window \nto lock to some viewport zone. \n";

				//helpInfo += "\n";
			}
		}
	}

	helpInternal.setText(helpInfo);
}

//----

// Fonts

//--------------------------------------------------------------
void SurfingGuiManager::setDefaultFontIndex(int index)
{
	if (customFonts.size() == 0) return;

	currFont = ofClamp(index, 0, customFonts.size() - 1);
	customFont = customFonts[currFont];
}

//--------------------------------------------------------------
void SurfingGuiManager::setDefaultFont()//will apply the first added font file
{
	setDefaultFontIndex(0);
}

//TODO: make it work on runtime..
//--------------------------------------------------------------
void SurfingGuiManager::clearFonts()
{
	customFonts.clear();

	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
}

// API user:
// workflow during setup not in draw.

//TODO: could return an int with the current index.
// Maybe could be useful to help push / changing default font.
//--------------------------------------------------------------
bool SurfingGuiManager::pushFont(std::string path, int size)
{
#ifndef OFX_IMGUI_USE_FONTS
	return false;
#endif

	//TODO:
	// It could be a vector with several customFont
	// to allow hot reloading..
	// if not, last added font will be used as default.

	ofLogNotice("ofxSurfingImGui:pushFont") << " " << path << " : " << size;

	auto& io = ImGui::GetIO();
	auto normalCharRanges = io.Fonts->GetGlyphRangesDefault();

	ofFile fileToRead(path);
	// a file that exists
	bool b = fileToRead.exists();

	if (b)
	{
		ImFont* _customFont = nullptr;
		if (guiPtr != nullptr)
		{
			_customFont = guiPtr->addFont(path, size, nullptr, normalCharRanges);
		}
		else
		{
			_customFont = gui.addFont(path, size, nullptr, normalCharRanges);
		}

		if (_customFont != nullptr)
		{
			customFonts.push_back(_customFont);
			customFont = _customFont;
			currFont = customFonts.size() - 1;
		}
	}
	else
	{
		ofLogError("ofxSurfingImGui:pushFont") << " FILE FONT " << path << " NOT FOUND!";
	}

	if (customFont != nullptr) io.FontDefault = customFont;

	return b;
}

// API user: workflow during setup not in draw.

//--------------------------------------------------------------
bool SurfingGuiManager::addFont(std::string path, int size)
{
#ifndef OFX_IMGUI_USE_FONTS
	return false;
#endif

	//TODO:
	// should be a vector with several customFont to allow hot reloading..
	// if not, last added font will be used

	auto& io = ImGui::GetIO();
	auto normalCharRanges = io.Fonts->GetGlyphRangesDefault();

	ofFile fileToRead(path); // a file that exists
	bool b = fileToRead.exists();
	if (b)
	{
		if (guiPtr != nullptr) customFont = guiPtr->addFont(path, size, nullptr, normalCharRanges);
		else customFont = gui.addFont(path, size, nullptr, normalCharRanges);
	}

	if (customFont != nullptr) io.FontDefault = customFont;

	return b;
}

//--

// API: workflow during draw to switch between font styles

//--------------------------------------------------------------
void SurfingGuiManager::pushStyleFont(int index)
{
#ifndef OFX_IMGUI_USE_FONTS
	return;
#endif

	if (index < customFonts.size())
	{
		if (customFonts[index] != nullptr)
			ImGui::PushFont(customFonts[index]);
	}
	else
	{
		bIgnoreNextPopFont = true; // workaround to avoid crashes
	}
	}
//--------------------------------------------------------------
void SurfingGuiManager::popStyleFont()
{
#ifndef OFX_IMGUI_USE_FONTS
	return;
#endif

	//TODO: will crash if not pushed..
	//workaround to avoid crashes
	if (bIgnoreNextPopFont)
	{
		bIgnoreNextPopFont = false;

		return;
	}

	ImGui::PopFont();
}

// NEW API: this is the preferred or recommended method!
//--------------------------------------------------------------
void SurfingGuiManager::PushFont(SurfingFontTypes style) {
#ifndef OFX_IMGUI_USE_FONTS
	return;
#endif

	int index = SurfingFontTypes(style);
	this->pushStyleFont(index);
}
//--------------------------------------------------------------
void SurfingGuiManager::PopFont() {
#ifndef OFX_IMGUI_USE_FONTS
	return;
#endif

	this->popStyleFont();
}


//--

//--------------------------------------------------------------
void SurfingGuiManager::processOpenFileSelection(ofFileDialogResult openFileResult, int size = 10) {

	std::string path = openFileResult.getPath();

	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " Name:" << openFileResult.getName();
	ofLogNotice("ofxSurfingImGui") << " Path:" << path;

	ofFile file(path);

	if (file.exists())
	{
		ofLogNotice("ofxSurfingImGui") << (" The file exists - now checking the type via file extension");
		std::string fileExtension = ofToUpper(file.getExtension());

		// We only want ttf/otf
		if (fileExtension == "TTF" || fileExtension == "OTF") {

			ofLogNotice("ofxSurfingImGui") << (" TTF or OTF found!");

			pushFont(path, size);
		}
		else ofLogError("ofxSurfingImGui") << "\n" << ("TTF or OTF not found!");
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::openFontFileDialog(int size)
{
	// Open the Open File Dialog
	ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a font file, TTF or OTF to add to ImGui", false, ofToDataPath(""));

	// Check if the user picked a file
	if (openFileResult.bSuccess) {

		ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << (" User selected a file");

		// We have a file, check it and process it
		processOpenFileSelection(openFileResult, size);
	}
	else {
		ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << (" User hit cancel");
	}
}

//----

//--------------------------------------------------------------
void SurfingGuiManager::update(ofEventArgs& args) {
	update();
}

//--------------------------------------------------------------
void SurfingGuiManager::update() { // -> Not being used by default
	//if (ofGetFrameNum() == 1)
	//{
	//	appLayoutIndex = appLayoutIndex;
	//}

	// Force call Startup
	//if ((ofGetFrameNum() <= 1) && (!bDoneStartup || !bDoneSetup)
	if (!bDoneSetup || !bDoneStartup)
	{
		setupStartupForced();
	}

#ifdef OFX_USE_DEBUGGER
	if (bDebugDebugger) {
		debugger.updateProfileTasksCpu();//call after (before) main ofApp update 
		debugger.update();
	}
#endif
}

//--------------------------------------------------------------
void SurfingGuiManager::draw()
{
	//TODO:
	//if (!bAutoDraw) if (customFont == nullptr) gui.draw();

#ifdef OFX_USE_DEBUGGER
	if (bDebugDebugger) debugger.updateProfileTasksGpu();//call after main ofApp draw
#endif
}

//--------------------------------------------------------------
void SurfingGuiManager::draw(ofEventArgs& args) // -> Auto called on each frame
{
	draw();
}

//--------------------------------------------------------------
void SurfingGuiManager::updateLayout() {

	// Layouts

	if (ini_to_load)
	{
		ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " LOAD! " << ini_to_load;

		loadLayoutImGuiIni(ini_to_load);

		ini_to_load = NULL;
	}

	if (ini_to_save)
	{
		ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " SAVE! " << ini_to_save;

		if (ini_to_save != "-1")
		{
			saveLayoutPreset(ini_to_save);
		}

		ini_to_save = NULL;
	}
}

//----

//--------------------------------------------------------------
void SurfingGuiManager::drawLayoutsManager()
{
	ImGuiWindowFlags flagsMng = ImGuiWindowFlags_None;

	// Exclude from imgui.ini settings
	flagsMng |= ImGuiWindowFlags_NoSavedSettings;

	if (bAutoResize) flagsMng |= ImGuiWindowFlags_AlwaysAutoResize;

	//--

	// Lock to the right of the Presets Window (tittled as Layouts)

	// Skip if the anchor window is hidden! to avoid being locked!

	if (bGui_LayoutsPresetsSelector)
	{
		ImGuiCond mngCond;
		mngCond = ImGuiCond_Appearing;

		bool blocked = true;
		if (blocked)
		{
			int _pad = windowsOrganizer.pad;

			glm::vec2 pos = rectangles_Windows[0].get().getTopRight();
			ofRectangle r = rectangles_Windows[2];
			r.setPosition(pos.x + _pad, pos.y);
			r.setWidth(rectangles_Windows[0].get().getWidth());

			//TODO:
			// set same height than Layouts panel if visible
			if (bGui_LayoutsPanels) r.setHeight(rectangles_Windows[0].get().getHeight());

			rectangles_Windows[2] = r;
			mngCond = ImGuiCond_Always;
		}

		const int i = 2;
		ImGui::SetNextWindowPos(ofVec2f(rectangles_Windows[i].get().getX(), rectangles_Windows[i].get().getY()), mngCond);
	}
	else
	{
	}

	//-

	if (bGui_LayoutsManager) IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_SMALL;

	if (BeginWindow(bGui_LayoutsManager, flagsMng))
	{
		const int i = 2;
		rectangles_Windows[i].setWithoutEventNotifications(ofRectangle(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));

		//-

		float _w = ofxImGuiSurfing::getWidgetsWidth(1);
		float _h = 2 * ofxImGuiSurfing::getWidgetsHeightRelative();

		//--

		if (!bGui_LayoutsPanels)
		{
			// Panels
			AddBigToggle(bGui_LayoutsPanels, _w, _h, false);
			this->AddTooltip("F6");

			// Presets
			AddBigToggle(bGui_LayoutsPresetsSelector, _w, _h, false);
			this->AddTooltip("F5");

			this->AddSpacingSeparated();
		}

		//--

		// Panels

		// Show a mini version when the main panel is hidden!

		if (!bGui_LayoutsPanels)
		{
			static bool bOpen = false;
			ImGuiColorEditFlags _flagw = (bOpen ? ImGuiWindowFlags_NoCollapse : ImGuiWindowFlags_None);

			if (ImGui::CollapsingHeader("PANELS", _flagw))
			{
				this->AddSpacing();

				// All the queued special windows aka panels

				for (int i = 0; i < windows.size(); i++)
				{
					AddToggleRoundedButton(windows[i].bGui);
				}

				this->AddSpacing();

				// All, None

				float _w2 = ofxImGuiSurfing::getWidgetsWidth(2);
				if (ImGui::Button("All", ImVec2(_w2, _h / 2)))
				{
					setShowAllPanels(true);
				}
				ImGui::SameLine();
				if (ImGui::Button("None", ImVec2(_w2, _h / 2)))
				{
					setShowAllPanels(false);
				}
			}
		}

		//--

		// Window Layouts

		if (ImGui::TreeNode("LAYOUTS"))
		{
			this->refreshLayout();
			Add(bAutoResize_PresetsWindows, OFX_IM_TOGGLE_ROUNDED_SMALL);
			Add(bReset_PresetsWindow, OFX_IM_BUTTON_SMALL);

			ImGui::TreePop();
		}

		//--

		// Window Panels

		if (ImGui::TreeNode("PANELS"))
		{
			this->refreshLayout();
			Add(bAutoResize_Panels, OFX_IM_TOGGLE_ROUNDED_SMALL);
			Add(bReset_WindowPanels, OFX_IM_BUTTON_SMALL);
			//if (bAutoResize_Panels) Add(bReset_WindowPanels, OFX_IM_BUTTON_SMALL);

			// Landscape
			//Add(bLandscape, OFX_IM_TOGGLE_ROUNDED);//TODO:

			ImGui::TreePop();
		}

		//--

		// Extra Params

		if (!bMinimize_Presets)
		{
			if (!bGui_LayoutsPanels) this->AddSpacingSeparated();
			this->AddGroup(params_LayoutsExtra);
		}

		//--

		// Advanced toggles bundled

		if (!bMinimize_Presets)
		{
			this->AddSpacingSeparated();

			// a toggle that expands the other widgets
			DrawAdvancedBundle();
		}

		this->EndWindow();
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::drawLayoutsPresetsEngine() {

	// Draws all sections except drawLayoutsManager() and drawLayoutsPresetsManualWidgets();

	//TODO:
	// How to make all windows dockeable in the same space ?
	if (bGui_LayoutsPresetsSelector)
	{
		drawLayoutsLayoutPresets(); // main presets clicker

		//if (!bMinimize_Presets) if (bGui_LayoutsPresetsManual) drawLayoutsPresetsManualWidgets();
	}

	if (bGui_LayoutsPanels) {
		drawLayoutsPanels();
	}

	//// Log
	//if (appLayoutIndex != -1) DrawWindowLogIfEnabled();
}

//--------------------------------------------------------------
void SurfingGuiManager::drawLayoutPresetsEngine() {

	if (bUseLayoutPresetsManager && !bMinimize_Presets)
		if (bGui_LayoutsManager) drawLayoutsManager();

	//----

	if (bUseLayoutPresetsManager)
	{
		updateLayout(); // to attend save load flags

		//----

		if (bDockingLayoutPresetsEngine)
		{
			ImGuiID dockNodeID;
			ImGuiDockNode* dockNode;
			//ImGuiDockNode* centralNode;
			//ImGuiDockNodeFlags dockingFlags;

			//----

			// a. Define the ofWindow as a docking space

			//ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0)); // Fixes imgui to expected behaviour. Otherwise add in ImGui::DockSpace() [~line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
			//ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_PassthruCentralNode);
			//ImGui::PopStyleColor();

			//-

			// b. Lockable settings

			// Fixes imgui to expected behavior. Otherwise add in ImGui::DockSpace() [~line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
			//ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
			//ImGuiDockNodeFlags flagsDock;
			//flagsDock = ImGuiDockNodeFlags_PassthruCentralNode;
			//if (bModeLockControls)
			//{
			//	flagsDock |= ImGuiDockNodeFlags_NoResize;
			//	flagsDock |= ImGuiDockNodeFlags_NoCloseButton;
			//	//flagsDock |= ImGuiDockNodeFlags_NoTabBar;
			//	//flagsDock |= ImGuiDockNodeFlags_NoWindowMenuButton;
			//	//flagsDock |= ImGuiDockNodeFlags_NoMove__;
			//}

			//TODO:
			//dockNodeID = ImGui::DockSpaceOverViewport(NULL, flagsDock);
			//dockNodeID = ImGui::GetID("MyDockSpace");
			//dockNodeID = ImGui::GetID("DockSpace");

			//ImGui::PopStyleColor();

			//----

			// Get check free space
			// central inter docks rectangle

			ImGuiDockNodeFlags flagsDock = ImGuiDockNodeFlags_None;
			//flagsDock += ImGuiDockNodeFlags_DockSpace;
			flagsDock += ImGuiDockNodeFlags_PassthruCentralNode;

			// A
			dockNodeID = ImGui::DockSpaceOverViewport(NULL, flagsDock);
			dockNode = ImGui::DockBuilderGetNode(dockNodeID);

			// B
			//ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockNodeID);

			if (dockNode)
			{
				ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockNodeID);

				// Verifies if the central node is empty (visible empty space for oF)
				if (centralNode && centralNode->IsEmpty())
				{
					ImRect availableSpace = centralNode->Rect();
					//availableSpace.Max = availableSpace.Min + ImGui::GetContentRegionAvail();
					//ImGui::GetForegroundDrawList()->AddRect(availableSpace.GetTL() + ImVec2(8, 8), availableSpace.GetBR() - ImVec2(8, 8), IM_COL32(255, 50, 50, 255));

					ImVec2 viewCenter = availableSpace.GetCenter();
					// Depending on the viewports flag, the XY is either absolute or relative to the oF window.
					//if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) viewCenter = viewCenter - ImVec2(ofGetWindowPositionX(), ofGetWindowPositionY());

					float ww = availableSpace.GetSize().x;
					float hh = availableSpace.GetSize().y;
					rectangle_Central_MAX = ofRectangle(viewCenter.x, viewCenter.y, ww, hh);

					bool bDebug_ = bDrawView2.get();
					if (bDebug_)
					{
						int _wl = 2;
						int pad = 10;

						ofPushStyle();
						ofSetRectMode(OF_RECTMODE_CENTER);

						int g = 0;
						ofColor cl = ofColor::white;
						//ofColor cl = ofColor::orange;

						//int g = 255 * Bounce(0.5);
						int a = 255.f * ofMap(Bounce(1), 0.0f, 1.0f, 0.2f, 1.0f, true);
						ofColor c = ofColor(cl.r, cl.g, cl.b, a);
						//ofColor c = ofColor(g, a);
						ofSetColor(c);

						ofNoFill();
						ofSetLineWidth(_wl);

						float ww = availableSpace.GetSize().x - pad;
						float hh = availableSpace.GetSize().y - pad;

						ofRectangle rDebug;
						rDebug = ofRectangle(viewCenter.x, viewCenter.y, ww, hh);
						ofDrawRectangle(rDebug);

						//ofDrawRectangle(rectangle_Central_MAX);
						ofSetRectMode(OF_RECTMODE_CORNER);
						ofPopStyle();
					}
					// move to left corner mode
					rectangle_Central_MAX.translate(-ww / 2, -hh / 2);

					//-

					static ofRectangle rectangle_Central_MAX_PRE;

					{
						if (rectangle_Central_MAX_PRE != rectangle_Central_MAX)
						{ // updates when layout changes..
							rectangle_Central_MAX_PRE = rectangle_Central_MAX;

							bool bSkip = false;
							bSkip = (rectangle_Central.getWidth() == 0 || rectangle_Central.getHeight() == 0);
							bSkip += (rectangle_Central_MAX.getWidth() == 0 || rectangle_Central_MAX.getHeight() == 0);
							if (!bSkip)
							{
								// fit exact rectangle to borders and scaled to fit
								//rectangle_Central = DEMO3_Svg.getRect();
								//if (rectangle_Central_MAX.getWidth() != 0 && rectangle_Central_MAX.getHeight() != 0) // avoid crash
								rectangle_Central.scaleTo(rectangle_Central_MAX, OF_ASPECT_RATIO_KEEP, OF_ALIGN_HORZ_CENTER, OF_ALIGN_VERT_CENTER);

								//// rescaled rectangle a bit
								//float _scale = 0.7f;
								//rectangle_Central_Transposed = rectangle_Central;
								//rectangle_Central_Transposed.scaleFromCenter(_scale, _scale);//scale down to fit layout spacing better
								//rectangle_Central_Transposed.translateY(rectangle_Central.getHeight() * 0.07);//move down a bit
								//DEMO3_Svg.setRect(rectangle_Central_Transposed);
							}
						}
					}
				}
			}
		}

		//----

		//// b. Lockable settings

		//// Fixes imgui to expected behavior. Otherwise add in ImGui::DockSpace() [~line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
		////ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
		//ImGuiDockNodeFlags flagsDock;
		//flagsDock = ImGuiDockNodeFlags_PassthruCentralNode;
		//if (bModeLockControls)
		//{
		//	flagsDock |= ImGuiDockNodeFlags_NoResize;
		//	flagsDock |= ImGuiDockNodeFlags_NoCloseButton;
		//	//flagsDock |= ImGuiDockNodeFlags_NoTabBar;
		//	//flagsDock |= ImGuiDockNodeFlags_NoWindowMenuButton;
		//	//flagsDock |= ImGuiDockNodeFlags_NoMove__;
		//}
		//ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(NULL, flagsDock);
		////ImGui::PopStyleColor();
	}
}

#ifdef FIXING_DRAW_VIEWPORT
//--------------------------------------------------------------
void SurfingGuiManager::drawViewport_oFNative() {

	//TODO: debug viewport. freew space for OF drawing

	ImGuiDockNodeFlags __dockingFlags;
	__dockingFlags = ImGuiDockNodeFlags_PassthruCentralNode;

	//ImGuiViewport* viewport = ImGui::GetMainViewport();

	//auto dockNodeID = ImGui::DockSpaceOverViewport(NULL, __dockingFlags);
	//auto dockNodeID = ImGui::GetID("DockSpace");
	auto dockNodeID = ImGui::GetID("MyDockSpace");

	ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockNodeID);
	if (dockNode)
	{
		ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockNodeID);
		if (centralNode)
			//if (centralNode && centralNode->IsEmpty())
		{
			ImRect availableSpace = centralNode->Rect();
			//availableSpace.Max = availableSpace.Min + ImGui::GetContentRegionAvail();
			//ImGui::GetForegroundDrawList()->AddRect(availableSpace.GetTL() + ImVec2(8, 8), availableSpace.GetBR() - ImVec2(8, 8), IM_COL32(255, 50, 50, 255));

			ImVec2 viewCenter = availableSpace.GetCenter();

			// Depending on the viewports flag, the XY is either absolute or relative to the oF window.
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) viewCenter = viewCenter - ImVec2(ofGetWindowPositionX(), ofGetWindowPositionY());

			// create rectangle
			rectangle_Central = ofRectangle(viewCenter.x, viewCenter.y, availableSpace.GetWidth(), availableSpace.GetHeight());
			float v = Bounce(1);
			rectangle_Central.setSize(availableSpace.GetWidth() * v, availableSpace.GetHeight() * v);
		}
	}
	else // get the OF viewport
	{
		auto view = ofGetCurrentViewport();
		auto viewCenter = view.getCenter();
		rectangle_Central = ofRectangle(viewCenter.x, viewCenter.y, view.getWidth(), view.getHeight());
		float v = Bounce(1);
		rectangle_Central.setSize(view.getWidth() * v, view.getHeight() * v);
	}

	ofPushStyle();
	{
		ofSetRectMode(OF_RECTMODE_CENTER);
		ofSetLineWidth(4);
		ofColor cl = ofColor::white;
		//ofColor cl = ofColor::yellow;
		int a = 255.f * ofMap(Bounce(1), 0.0f, 1.0f, 0.2f, 1.0f, true);
		ofColor c = ofColor(cl.r, cl.g, cl.b, a);
		ofSetColor(c);
		ofFill();
		ofDrawCircle(rectangle_Central.getCenter().x, rectangle_Central.getCenter().y, 3);
		ofNoFill();
		ofDrawRectangle(rectangle_Central);

		ofSetRectMode(OF_RECTMODE_CORNER);
	}
	ofPopStyle();
}
#endif

//--------------------------------------------------------------
void SurfingGuiManager::setupStartupForced()
{
	ofLogWarning(__FUNCTION__);

	if (!bDoneSetup)
	{
		ofLogWarning("ofxSurfingImGui") << "Setup() was not called/done!";
		ofLogWarning("ofxSurfingImGui") << "Force a default Setup() call!";

		setup();
	}

	// Force call startup().
	// Maybe user forgets to do it or to speed up the API setup in some scenarios.
	// i.e. when not using special windows or layout engine
	if (!bDoneStartup)
	{
		ofLogWarning("ofxSurfingImGui") << "Startup() was not called after initiation process.";
		ofLogWarning("ofxSurfingImGui") << "Auto force call Startup()!";

		startup();
	}

	//TODO:
	{
		appLayoutIndex = appLayoutIndex;
		if (rectangles_Windows.size() > 0) rectangles_Windows[1] = rectangles_Windows[1];
	}
}

//----

// Global ImGui being/end like ofxImGui
//
// All the ImGui Stuff goes in between here,
// The RAW ImGui widgets and the API / Engine handled stuff too!
//
//--------------------------------------------------------------
void SurfingGuiManager::Begin()
{
	// Check that it's property initialized!
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED)
	{
		ofLogError("ofxSurfingImGui") << (__FUNCTION__) << "\n" <<
			("Initialization was not done properly. \nCheck the examples / documentation.");

		return;
	}

	//--

	//TODO:
	//// Force call Startup
	//if (ofGetFrameNum() <= 1) 
	//	if (!bDoneStartup || !bDoneSetup)
	//{
	//	setupStartupForced();
	//}

	//--

	//TODO:
	_indexLastBegin = -1;

	//--

	// Reset unique names
	// Here it handles the queued ofParams names
	// to avoid colliding by using push/pop id's tags.
	resetUniqueNames();

	// This handles the name to Push/Pop widgets IDs
	// Then we can use several times the same ofParameter with many styles,
	// into the same window without colliding!
	// That collide happens when using the original legacy ofxImGui ofParam Helpers!

	//--

	//TODO:
	// Sometimes we could use an ofxImGui external or from a parent scope..
	if (guiPtr != nullptr) guiPtr->begin();
	else gui.begin();

	//--

	// Fonts

	if (customFont != nullptr) ImGui::PushFont(customFont);

	// Reset font to default.
	// this clear all the push/pop queue.
	setDefaultFont();

	//--

	//TODO:
	// Fix
	//if (!bDockingLayoutPresetsEngine)
	//if (bMenu) drawMenu();

	//----

	// Extra Tools / Engines

	// 1. Layout Presets Engine

	if (bDockingLayoutPresetsEngine) drawLayoutPresetsEngine();
	// false by default as/when docking has not been initiated.

	//----

	// 2. Special Windows Engine Window Panel

	// Organizer

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		// Main Panels Controller
		if (windowsOrganizer.isInitiated())
		{
			// Organizer
			if (bGui_Organizer) drawWindowOrganizer();

			// Special windows toggles
			if (bGui_SpecialWindows) drawWindowSpecialWindows();
		}
	}

	//----

	// 3. Aligners

	if (bGui_Aligners) drawWindowAlignHelpers();

#ifndef OFX_USE_DEBUGGER
	if (bDebugDebugger) ImGui::ShowMetricsWindow();
#endif
}

//--------------------------------------------------------------
void SurfingGuiManager::drawWindowsExtraManager() {

	// Auto handles drawing of extra windows. Not required to draw manually!

	// Log
	DrawWindowLogIfEnabled();

	// Notifier
#ifdef OFX_USE_NOTIFIER
	DrawNotifierIfEnabled();
#endif

	//--

#ifdef FIXING_DRAW_VIEWPORT
	if (bDrawView1) drawViewport_oFNative();
#endif

	//--

	// Draw Help windows

	// Internal
	if (bUseHelpInfoInternal) {
		if (helpInternal.bGui) IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_MEDIUM;
		helpInternal.draw();
	}

	// App
	if (bUseHelpInfoApp) {
		if (helpApp.bGui) IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_MEDIUM;
		helpApp.draw();
	}

#ifdef OFX_USE_DEBUGGER
	if (bDebugDebugger) debugger.drawImGui();
	//if (bDebugDebugger) debugger.draw(this);//TODO: how to pass ui?
#endif
}

//--------------------------------------------------------------
void SurfingGuiManager::End()
{
	// Check that it's property initialized!
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//--

	//TODO:
	//workflow
	// if there's no settings files, it means that the app it's opened by first time,
	// then probably the windows will be overlapped..
	if (ofGetFrameNum() > 1)
		if (!bDisableStartupReset && bDoForceStartupResetLayout && !bDoneDoForceStartupResetLayout)
		{
			doResetLayout();
			bDoneDoForceStartupResetLayout = true;
		}

	//--

	drawWindowsExtraManager();

	//--

	// Font

	//TODO: could set the default font instead of Pop..
	// bc that will be prophylactic if pushed too many fonts by error!
	if (customFont != nullptr) ImGui::PopFont();

	//--

	// Mouse and Keyboard
	doCheckOverGui();

	//--

	// ImGui End

	//TODO:
	// Sometimes we could use an ofxImGui external or from a parent scope.
	// This should be tested. bc it's kind of a deprecated idea/feature.
	if (guiPtr != nullptr) guiPtr->end();
	else gui.end();

	//--

	//TODO: should add a new variable like bDrawInfront to draw back/foreground...
	//TODO: maybe it's an ofxImGui feature..
	if (!bAutoDraw) gui.draw();
}

//--

// Begin Window Methods

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(char* name)
{
	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	bool b = BeginWindow((string)name, NULL, fg);
	// required to avoid exceptions when minimizing the window.
	if (!b) this->EndWindow();

	return b;
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(char* name, ImGuiWindowFlags window_flags)
{
	std::string n = name;
	bool b = BeginWindow(n, NULL, window_flags);

	return b;
}

////--------------------------------------------------------------
//bool SurfingGuiManager::BeginWindow(char* name, ImGuiWindowFlags window_flags, ImGuiCond cond)
//{
//	std::string n = name;
//	bool b = BeginWindow(n, NULL, window_flags, cond);
//
//	return b;
//}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(std::string name)
{
	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	bool b = BeginWindow(name, NULL, fg);
	// required to avoid exceptions when minimizing the window.
	if (!b) this->EndWindow();

	return b;
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(std::string name, bool* p_open)
{
	if (!&p_open) return false;

	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	bool b = BeginWindow(name, p_open, fg);
	// required to avoid exceptions when minimizing the window.
	if (!b) this->EndWindow();

	return b;
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(ofParameter<bool>& p)
{
	if (!p.get()) return false; // p is used as the "visible toggle"

	if (p.getName() == "") {
		ofLogWarning("ofxSurfingImGui") << (__FUNCTION__);
		ofLogWarning("ofxSurfingImGui") << "Current bool parameter have NO NAME!";
		ofLogWarning("ofxSurfingImGui") << "Take care and set a name to it.";
		p.setName("__NONAME__");
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
	if (bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize; // windows can be auto resized or not

	//TODO:
	// workaround
	// fix to trig the ofParams callbacks
	// when closed window by the x on the upper right corner.
	bool tmp = p.get();
	bool b = BeginWindow(p.getName().c_str(), (bool*)&tmp, window_flags);
	// trig if changed
	if (p.get() != tmp) p.set(tmp);

	// required to avoid exceptions when minimizing the window.
	//fix crashes when foldering
	if (!b) this->EndWindow();

	return b;
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(std::string name, ofParameter<bool>& p)
{
	if (!p.get()) return false;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
	if (bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	return BeginWindow(name.c_str(), (bool*)&p.get(), window_flags);
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(std::string name, ofParameter<bool>& p, ImGuiWindowFlags window_flags)
{
	if (!p.get()) return false;

	return BeginWindow(name.c_str(), (bool*)&p.get(), window_flags);
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(ofParameter<bool>& p, ImGuiWindowFlags window_flags)
{
	if (!p.get()) return false;

	//bool b = BeginWindow(p.getName().c_str(), (bool*)&p.get(), window_flags);

	//TODO:
	// workaround
	// fix to trig the ofParams callbacks
	// when closed window by the x on the upper right corner.
	bool tmp = p.get();
	bool b = BeginWindow(p.getName().c_str(), (bool*)&tmp, window_flags);
	// trig if changed
	if (p.get() != tmp) p.set(tmp);

	//TODO:
	//fix crashes when foldering
	if (!b) this->EndWindow();

	return b;
}

// This is the main BeginWindow.
// All above methods will call this one!

////--------------------------------------------------------------
//bool SurfingGuiManager::BeginWindow(std::string name = "Window", bool* p_open = NULL, ImGuiWindowFlags window_flags = ImGuiWindowFlags_None, ImGuiCond cond)
//{
//}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindow(std::string name = "Window", bool* p_open = NULL, ImGuiWindowFlags window_flags = ImGuiWindowFlags_None)
{
	ImGuiCond cond = ImGuiCond_None;

	//TODO:
	//if (bLockMove) window_flags |= ImGuiWindowFlags_NoMove;

	//TODO:
	//if (bReset_Window) {
	//	bReset_Window = false;
	//	resetWindowImGui(false, true);
	//}

	//--

	// Reset unique names
	// This is to handle the widgets ID to avoid repeat an used name,
	// avoiding colliding between them
	// when params are re used on the same window/scope.

	resetUniqueNames();

	//--

	// Default constraints
	//IMGUI_SUGAR__WINDOWS_CONSTRAINTS;
	//IMGUI_SUGAR__WINDOWS_CONSTRAINTS_SMALL;

	//--

	bool b = ImGui::Begin(name.c_str(), p_open, window_flags);

	//--

	////TODO: crashes ?
	//// Early out if the window is collapsed, as an optimization.
	//if (!b)
	//{
	//	ImGui::End();
	//	return false;
	//}

	// When we are instantiating ImGui externally,
	// not inside this addon,
	// we don't handle the font and theme.
	if (surfingImGuiMode != ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED)
	{
		// Set default font
		setDefaultFont();
	}

	// Refresh layout
	// Calculates sizes related to current window shape/size.
	_ui.refreshLayout();

	return b;
}

//--

// Begin Special Windows

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindowSpecial(string name)
{
	int _index = getWindowSpecialIndexForName(name);

	if (_index != -1)
	{
		return BeginWindowSpecial(_index);
	}
	else
	{
		ofLogError("ofxSurfingImGui") << (__FUNCTION__) << " Special Window with name '" << name << "' not found!";
		return false;
	}
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindowSpecial(int index)
{
	//TODO:
	// workflow
	_indexLastBegin = index;

	//--

	// Skip if there's no queued special windows
	if (index > windows.size() - 1 || index == -1)
	{
		ofLogError("ofxSurfingImGui") << (__FUNCTION__) << " Out of range index for queued windows, " << index;
		return false;
	}

	//--

	//TODO:
	// Skip window if hidden
	// (bGui = false)
	if (!windows[index].bGui.get())
		return false;

	//--

	ImGuiWindowFlags flags = ImGuiWindowFlags_None;

	//--

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		// All can be disabled by the Global toggle
		if (!windowsOrganizer.bGui_Global.get()) return false;

		//--

		// Run the linking engine when this is enabled
		if (windowsOrganizer.bLinked)
		{
			//TODO:
			// make refresh faster
			// can be moved to global begin() to reduce calls ?
			// maybe is better like that bc it's called many times.. ?
			windowsOrganizer.refreshUpdate();
			windowsOrganizer.runShapeState(index);
		}

		//--

		// Header
		if (!windowsOrganizer.bHeaders) flags += ImGuiWindowFlags_NoDecoration;
	}

	//--

	// Auto resize

	// global
	//if (bAutoResize) flags += ImGuiWindowFlags_AlwaysAutoResize;

	if (windows[index].bAutoResize) flags += ImGuiWindowFlags_AlwaysAutoResize;
	// independent for each window

	//--

	bool b = BeginWindow(windows[index].bGui, flags);

	//--

	//bool b = BeginWindow(windows[index].bGui);
	//if (windows[index].bMasterAnchor.get()) // window
	//{
	//	if (windows[index].bAutoResize.get()) {
	//		flags |= ImGuiWindowFlags_AlwaysAutoResize;
	//	}
	//}
	//bool b = BeginWindow(windows[index].bGui.getName().c_str(), (bool*)&windows[index].bGui.get(), flags);

	//--

	return b;
}

//--------------------------------------------------------------
bool SurfingGuiManager::BeginWindowSpecial(ofParameter<bool>& _bGui)
{
	if (!_bGui) return false;

	int i = getWindowSpecialIndexForToggle(_bGui);

	if (i != -1)
	{
		return BeginWindowSpecial(i);
	}
	else
	{
		ofLogError("ofxSurfingImGui") << (__FUNCTION__) << " Special Window toggle not found! " << _bGui.getName();

		//TODO:
		// detect if there is no special window with that name,
		// and then call a normal window to avoid missing drawing.
		// being prophylactic.
		//return BeginWindow(_bGui);

		return false;
	}
}

//--------------------------------------------------------------
int SurfingGuiManager::getWindowSpecialIndexForName(string name)
{
	for (size_t i = 0; i < windows.size(); i++)
	{
		string _name = windows[i].bGui.getName();

		if (name == _name)
		{
			return  i;
		}
	}

	ofLogVerbose("ofxSurfingImGui") << (__FUNCTION__) << "\n" << "Special Window with name '" << name << "' not found!";

	return -1;
}

//--------------------------------------------------------------
int SurfingGuiManager::getWindowSpecialIndexForToggle(ofParameter<bool>& _bGui)
{
	string name = _bGui.getName();

	for (size_t i = 0; i < windows.size(); i++)
	{
		string _name = windows[i].bGui.getName();

		if (name == _name)
		{
			return  i;
		}
	}

	ofLogError("ofxSurfingImGui") << (__FUNCTION__) << "\n" << "Special Window toggle not found! " << _bGui.getName();

	return -1;
}

//TODO:
//--------------------------------------------------------------
void SurfingGuiManager::EndWindowSpecial(ofParameter<bool>& _bGui)
{
	string name = _bGui.getName();
	int i = getWindowSpecialIndexForName(name);

	if (i == -1) {
		ofLogError("ofxSurfingImGui") << (__FUNCTION__) << "\n" << "Special Window with bool param with name '" << name << "' not found!";

		return;
	}

	EndWindowSpecial(i);

	return;
}

//--------------------------------------------------------------
void SurfingGuiManager::EndWindowSpecial(int index)
{
	if (index == -1) index = _indexLastBegin; // workaround

	//--

	if (index > windows.size() - 1)
	{
		ofLogError("ofxSurfingImGui") << (__FUNCTION__) << "\n" << "Out of range index for queued windows, " << index;
		return;
	}

	//--

	//TODO: crash ?
	//// Skip window if hidden (bGui = false)
	//if (!windows[index].bGui.get()) return;

	//--

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		if (!windowsOrganizer.bGui_Global.get()) return;
	}

	//--

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		if (windowsOrganizer.bLinked)
		{
			// Reads the window shape before end
			windowsOrganizer.getShapeState(index);

			//TODO: make refresh faster
			//windowsOrganizer.refreshUpdate();
		}
	}

	this->EndWindow();
}

//--------------------------------------------------------------
void SurfingGuiManager::EndWindow()
{
	ImGui::End();
}

//--

// Docking Helpers

#ifdef FIXING_DOCKING
//--------------------------------------------------------------
void SurfingGuiManager::beginDocking()
{
	// Make windows transparent, to demonstrate drawing behind them.
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(200, 200, 200, 128)); // This styles the docked windows

	ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_PassthruCentralNode; // Make the docking space transparent
	// Fixes imgui to expected behaviour, having a transparent central node in passthru mode.
	// Alternative: Otherwise add in ImGui::DockSpace() [~line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));

	//dockingFlags |= ImGuiDockNodeFlags_NoDockingInCentralNode; // Uncomment to always keep an empty "central node" (a visible oF space)
	//dockingFlags |= ImGuiDockNodeFlags_NoTabBar; // Uncomment to disable creating tabs in the main view

	// Define the ofWindow as a docking space
	ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(NULL, dockingFlags); // Also draws the docked windows
	//ImGui::PopStyleColor(2);
}
#endif

#ifndef FIXING_DOCKING
//--------------------------------------------------------------
void SurfingGuiManager::BeginDocking()
{
	//dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	//ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	if (bMenu) window_flags |= ImGuiWindowFlags_MenuBar;

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// fit full viewport
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	window_flags
		|= ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove;

	window_flags
		|= ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	//ImGui::Begin("MyDockSpace", nullptr, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	//----

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	else
	{
		// Docking is DISABLED - Show a warning message
		//ShowDockingDisabledMessage();
	}

	//----

	// All windows goes here before endDocking()

	drawLayoutsPresetsEngine();
}
#endif

//--------------------------------------------------------------
void SurfingGuiManager::EndDocking()
{
#ifdef FIXING_DOCKING
	return;
#endif

	if (bMenu) drawMenuDocked();

	//--

	//ImGuiIO& io = ImGui::GetIO();
	//if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	//{
	//}

	// End the parent window that contains the Dockspace:
	ImGui::End(); // ?
}

//----

// Layouts presets management
//--------------------------------------------------------------
void SurfingGuiManager::setupLayout(int numPresets) //-> must call manually after adding windows and layout presets
{
	numPresetsDefault = numPresets; // default is 4 presets with names P0, P1, P2, P3

	//-

	//// Clear
	//params_Layouts.clear();
	//params_LayoutsExtra.clear();
	//params_LayoutsVisible.clear();

	//--

	// 1.1 Store all the window panels show toggles
	// we will remember, on each layout preset, if a window is visible or not!

	for (int i = 0; i < windows.size(); i++)
	{
		params_LayoutsVisible.add(windows[i].bGui);
	}

	//--

	//// 1.2 Add other settings that we want to store into each presets

	//-

	// Extra params that will be included into each preset.
	// Then can be different and memorized in different states too,
	// like the common panels.

	params_LayoutsExtra.add(bMenu);
	params_LayoutsExtra.add(bLog);
	params_LayoutsExtra.add(bNotifier);
	//TODO: should be removed if handled by preset engine..


	//params_LayoutsExtraInternal.clear();
	//params_LayoutsExtraInternal.add(bMenu);
	//params_LayoutsExtraInternal.add(bLog);
	//params_LayoutsExtra.add(params_LayoutsExtraInternal);

	//--

	// 1.2.2 Special Windows Helpers

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		this->addExtraParamToLayoutPresets(this->getGuiToggleLinked());
		//params_LayoutsExtra.add(windowsOrganizer.getParamsSettings());
	}

	//--

	// 1.3 Applied to control windows

	//params_LayoutsExtra.add(bModeFree);
	//params_LayoutsExtra.add(bModeForced);
	//params_LayoutsExtra.add(bModeLock1);
	//params_LayoutsExtra.add(bModeLockControls);

	//--

	// 1.4 Pack both groups

	params_Layouts.add(params_LayoutsVisible);
	params_Layouts.add(params_LayoutsExtra);

	//--

	// 2. Initiate

	bLayoutPresets.clear();
	params_LayoutPresetsStates.clear();
	appLayoutIndex.setMax(0);

	//--

	// 3. Populate some presets

	for (int i = 0; i < numPresetsDefault; i++)
	{
		if (namesPresets.size() == 0) {//if names are not defined will be setted by default P0-P1-P2-P3
			createLayoutPreset();
		}
		else {
			if (i < namesPresets.size()) createLayoutPreset(namesPresets[i]);
			else createLayoutPreset();
		}
	}

	//--

	// 4. App states for the next session

	// The main control windows

	params_AppSettings.add(bGui_LayoutsManager);

	//params_AppSettingsLayout.add(bModeFree);
	//params_AppSettingsLayout.add(bModeForced);
	//params_AppSettingsLayout.add(bModeLock1);
	//params_AppSettingsLayout.add(bModeLockControls);
	//params_AppSettingsLayout.add(bModeLockPreset);

	params_AppSettingsLayout.add(bGui_LayoutsPresetsSelector);
	params_AppSettingsLayout.add(bGui_LayoutsPanels);
	//params_AppSettingsLayout.add(bGui_LayoutsPresetsManual);

	//params_AppSettingsLayout.add(bGui_Organizer);
	//params_AppSettingsLayout.add(bGui_SpecialWindows);

	params_AppSettingsLayout.add(bAutoSave_Layout);
	params_AppSettingsLayout.add(bDrawView1);
	params_AppSettingsLayout.add(appLayoutIndex);
	params_AppSettingsLayout.add(bSolo);

	params_AppSettings.add(params_AppSettingsLayout);

	//--

	// Exclude from settings but to use the grouped callback
	//bSolo.setSerializable(false);

	//---------------

	// Engine Windows

	// Initiate the 3 control windows
	// We store the shapes using ofRectangles to "split" them from ImGui .ini store manager...

	float x, y, w, h, pad;
	x = ofGetWidth() * 0.6;
	y = 30;
	w = 200;
	h = 1;
	pad = windowsOrganizer.pad;

	//--

	rect0_Presets.set(ofRectangle(10, y, w, h));
	rect1_Panels.set(ofRectangle(x, y, w, h));
	rect2_Manager.set(ofRectangle(x + 2 * (pad + w), y, w, h));

	//rect1_Panels.set(ofRectangle(x + (pad + w), y, w, h));
	//rect0_Presets.set(ofRectangle(x, y, w, h));


	//--

	rectangles_Windows.clear();
	rectangles_Windows.emplace_back(rect0_Presets);
	rectangles_Windows.emplace_back(rect1_Panels);
	rectangles_Windows.emplace_back(rect2_Manager);

	// To store settings to disk
	params_RectPanels.clear();
	params_RectPanels.add(rect0_Presets);
	params_RectPanels.add(rect1_Panels);
	params_RectPanels.add(rect2_Manager);

	params_WindowsEngine.clear();
	params_WindowsEngine.add(params_RectPanels);

	//--

	// Presets and Panels Windows

	params_WindowPresets.add(bReset_PresetsWindow);
	params_WindowPresets.add(bAutoResize_PresetsWindows);
	params_WindowPresets.add(bMinimize_Presets);

	params_WindowPanels.add(bReset_WindowPanels);
	params_WindowPanels.add(bAutoResize_Panels);

	//params_WindowPresets.add(bMinimize_Panels);

	params_WindowsEngine.add(params_WindowPresets);
	params_WindowsEngine.add(params_WindowPanels);

	params_AppSettings.add(params_WindowsEngine);

	//----

	//TODO: Simplify calls merging to one group only...

	// Callbacks
	ofAddListener(params_LayoutPresetsStates.parameterChangedE(), this, &SurfingGuiManager::Changed_Params);

	//--

	//// Gui - > which panels enabled but overwritten by Layout Presets Engine
	//params_AppSettings.add(params_bGuiToggles);

	//--

	setImGuiLayoutPresets(true);
}

//--------------------------------------------------------------
bool SurfingGuiManager::loadAppSettings()
{
	bool b = false;
	if (bAutoSaveSettings) b = loadGroup(params_AppSettings, path_AppSettings, true);

	return b;

	// Will return false if settings file do not exist. That happens when started for first time or after OF_APP/bin cleaning
}

//--------------------------------------------------------------
void SurfingGuiManager::saveAppSettings()
{
	ofLogNotice("ofxSurfingImGui") << "saveAppSettings()";

	if (bAutoSaveSettings)
	{
		//TODO:
		// Double check again that folder exist.
		// This is already made on setup or when a custom setName is made.
		CheckFolder(path_Global);
		// Could use:
		//ofFilePath::getEnclosingDirectory(O)

		saveGroup(params_AppSettings, path_AppSettings);
		ofLogNotice("ofxSurfingImGui") << "saveAppSettings() DONE!";
	}
	else {
		ofLogWarning("ofxSurfingImGui") << "By passed saveAppSettings()";
		ofLogWarning("ofxSurfingImGui") << "bAutoSaveSettings was disabled!";
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::saveAppLayout(int _index)
{
	if (_index == -1) return;

	ini_to_save_Str = getLayoutName(_index);

	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " " << ini_to_save_Str;

	if (ini_to_save_Str == "-1") return; // skip

	// Flag to save .ini on update
	ini_to_save = ini_to_save_Str.c_str();

	// Save group
	saveLayoutPresetGroup(ini_to_save);
}

//--------------------------------------------------------------
void SurfingGuiManager::loadAppLayout(int _index)
{
	if (_index == -1) return;

	//if (appLayoutIndex == _index) return; // skip

	appLayoutIndex = ofClamp(_index, appLayoutIndex.getMin(), appLayoutIndex.getMax());

	std::string _name = getLayoutName(appLayoutIndex.get());
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " " << appLayoutIndex << ":" << _name;

	//std::string _label = APP_RELEASE_NAME;
	std::string _label = "";
	_label += "App Layout ";
	_label += " "; // spacing

	if (!bLayoutPresets[appLayoutIndex.get()])
	{
		for (int i = 0; i < bLayoutPresets.size(); i++) {
			bLayoutPresets[i].set(false);
		}
		bLayoutPresets[_index].set(true);
	}

	// ini
	ini_to_load_Str = _name;
	ini_to_load = ini_to_load_Str.c_str();

	// Group
	loadLayoutPresetGroup(ini_to_load_Str);

	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " ------------------------------------";
	ofLogNotice("ofxSurfingImGui") << (__FUNCTION__) << " ini_to_load    : " << ini_to_load;
}

//--------------------------------------------------------------
void SurfingGuiManager::drawLayoutsLayoutPresets() // That's the window tittled as "Layout"
{
	flags_wPr = ImGuiWindowFlags_None;
	flags_wPr += ImGuiWindowFlags_NoSavedSettings;

	if (bAutoResize_PresetsWindows) flags_wPr += ImGuiWindowFlags_AlwaysAutoResize;

	//--

	/*
	//// Viewport Center
	////
	//// is excluded from .ini
	//float xw, yw, ww, hw;
	//{
	//	if (bModeForced)
	//	{
	//		// Forced inside a free viewport
	//		//// Upper left
	//		//glm::vec2 p = rectangle_Central_MAX.getTopLeft() + glm::vec2(-1, -1);
	//		// Center upper left
	//		int _pad = 10;
	//		int _xx = rectangle_Central_MAX.getTopLeft().x + _pad;
	//		int _yy = rectangle_Central_MAX.getTopLeft().y + rectangle_Central_MAX.getHeight() / 2 - hw / 2;
	//		glm::vec2 p = glm::vec2(_xx, _yy);
	//		// Shape
	//		xw = p.x;
	//		yw = p.y;
	//		flagCond = ImGuiCond_Always;
	//		ImGui::SetNextWindowPos(ofVec2f(xw, yw), flagCond);
	//		//ImGui::SetNextWindowSize(ofVec2f(ww, hw), flagCond); // exclude shape
	//	}
	//}
	*/

	//--

	// Reset trigger

	ImGuiCond prCond = ImGuiCond_None;
	prCond += ImGuiCond_Appearing;

	if (bReset_PresetsWindow)
	{
		bReset_PresetsWindow = false;

		const int i = 0;
		ofRectangle r = rectangles_Windows[i];
		r.setWidth(100);
		r.setHeight(100);
		rectangles_Windows[i].set(r);
		prCond = ImGuiCond_Always;

		// workflow
		bAutoResize_PresetsWindows = true;
		bMinimize_Presets = true;
		bGui_LayoutsManager = false;
	}

	//----

	// Position and size the window

	if (bGui_LayoutsPresetsSelector)
	{
		const int i = 0;
		ImGui::SetNextWindowPos(ImVec2(rectangles_Windows[i].get().getX(), rectangles_Windows[i].get().getY()), prCond);
		ImGui::SetNextWindowSize(ImVec2(rectangles_Windows[i].get().getWidth(), rectangles_Windows[i].get().getHeight()), prCond);
	}

	//--

	// Window

	if (BeginWindow(bGui_LayoutsPresetsSelector, flags_wPr))
	{
		float _h = 2 * ofxImGuiSurfing::getWidgetsHeightUnit();
		float _w1 = ofxImGuiSurfing::getWidgetsWidth(1);
		float _w2 = ofxImGuiSurfing::getWidgetsWidth(2);
		float _w = _w2;

		//--

		const int i = 0;
		rectangles_Windows[i].setWithoutEventNotifications(ofRectangle(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));

		//--

		// 1. Minimize (global)

		this->Add(bMinimize_Presets, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
		this->AddSpacingSeparated();

		//--

		// Panels

		ofxImGuiSurfing::AddBigToggle(bGui_LayoutsPanels, _w1, (bMinimize_Presets ? _h * 0.75f : _h));
		this->AddTooltip("F6");

		// Manager

		//if (!bMinimize_Presets) ofxImGuiSurfing::AddBigToggle(bGui_LayoutsManager, _w1, (bMinimize_Presets ? _h / 2 : _h));
		if (!bMinimize_Presets) {
			this->Add(bGui_LayoutsManager, OFX_IM_TOGGLE_ROUNDED);
			this->AddTooltip("F7");
		}

		this->AddSpacingSeparated();

		//--

		// 2. The 4 Preset Toggles

		// label
		if (!bMinimize_Presets) {
			this->AddLabelBig("PRESETS");
			this->AddSpacing();
		}

		// toggles
		for (int i = 0; i < bLayoutPresets.size(); i++)
		{
			ofxImGuiSurfing::AddBigToggle(bLayoutPresets[i], _w2, _h);

			string s = "F" + ofToString(i + 1);
			this->AddTooltip(s);

			if (i % 2 == 0) ImGui::SameLine();//two toggles per row
		}

		//--

		// 3. Save / Load

		if (!bMinimize_Presets)
		{
			this->AddSpacingSeparated();

			// Auto save

			ofxImGuiSurfing::AddBigToggle(bAutoSave_Layout, _w1, 0.5 * _h, true);

			// Save Button

			if (!bAutoSave_Layout.get())
			{
				ImGui::PushID("##SaveLayout");
				if (ImGui::Button("Save", ImVec2(_w1, 0.5 * _h)))
				{
					saveAppLayout(appLayoutIndex.get());
				}
				ImGui::PopID();
			}

			this->AddSpacing();

			//--

			// Manual

			drawLayoutsPresetsManualWidgets();

			this->AddSpacingSeparated();
			this->AddSpacing();
		}

		//--

		if (!bMinimize_Presets)//maximized
		{
			// Organizer panel visible toggle

			this->Add(bGui_Organizer, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
			this->AddSpacing();

			// Link mode toggle

			this->Add(this->getGuiToggleLinked());
		}

		//--

		if (!bMinimize_Presets)
		{
			// Extra Params

			// when is minimized or manager window hidden
			if (bMinimize_Presets || !bGui_LayoutsManager)
			{
				this->AddSpacingSeparated();
				this->AddGroup(params_LayoutsExtra, SurfingGuiGroupStyle_Collapsed);
			}
		}

		this->EndWindow();
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::drawLayoutsPresetsManualWidgets()
{
	bool bMin = false; // hide load buttons to simplify
	float max = (bMin ? 150 : 175);

	//----

	{
		float _h = ofxImGuiSurfing::getWidgetsHeightUnit();
		float _w1 = ofxImGuiSurfing::getWidgetsWidth(1);
		float _w2 = ofxImGuiSurfing::getWidgetsWidth(2);

		//--

		if (!bMinimize_Presets)
		{
			if (ImGui::CollapsingHeader("Manual", ImGuiWindowFlags_None))
			{
				ImVec2 bb{ (bMin ? _w1 : _w2), _h };

				//--

				int _id = 0;
				for (int i = 0; i < bLayoutPresets.size(); i++)
				{
					std::string _name = (bLayoutPresets[i].getName());

					ImGui::Text(_name.c_str());
					if (!bMin)
					{
						ImGui::PushID(_id++);
						if (ImGui::Button("Load", bb))
						{
							appLayoutIndex = i;
						}
						ImGui::PopID();

						ImGui::SameLine();
					}

					ImGui::PushID(_id++);

					if (ImGui::Button("Save", bb))
					{
						ini_to_save_Str = _name;
						ini_to_save = ini_to_save_Str.c_str();
					}

					ImGui::PopID();
				}

				//--

				this->AddSpacingSeparated();

				if (ImGui::Button("Reset", ImVec2(_w2, _h)))
				{
					//TODO: to trig an external function..
					if (bResetPtr != nullptr) {
						*bResetPtr = true;
					}

					// Toggle panels to true
					for (int i = 0; i < windows.size(); i++) {
						windows[i].bGui.set(true);
					}

					saveAppLayout((appLayoutIndex.get()));
				}
				this->AddTooltip("Reset all the Presets");

				ImGui::SameLine();

				if (ImGui::Button("Clear", ImVec2(_w2, _h)))
				{
					doRemoveDataFiles();
				}
				this->AddTooltip("Clear all the Presets");
			}
		}

		//--

		/*
		//TODO: not storing new presets yet!
		//presets amounts can be defined on setupLayout(4);
		// create a new layout preset
		if (ImGui::Button("Create", ImVec2(_w100, _h)))
		{
			createLayoutPreset();
		}
		*/
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::Changed_Params(ofAbstractParameter& e)
{
	std::string name = e.getName();

	bool bskip = true;
	if (name != "position" &&
		name != log.amountLinesLimitedBuffered.getName() && //workaround
		name != "rect_Manager")
	{
		bskip = false;
		ofLogNotice("ofxSurfingImGui") << "Changed: " << name << ": " << e;
	}
	if (bskip) return;

	//--

	if (0) {}

	//--

	// Update Help Info

	// Help internal
	else if (name == bHelpInternal.getName() && bHelpInternal)
	{
		doBuildHelpInfo();//recreate info
		return;
	}

	// Help App / global. To be handled externally
	else if (name == bHelp.getName())
	{
		doBuildHelpInfo();
		return;
	}

	// Debug
	else if (name == bDebug.getName())
	{
		doBuildHelpInfo();
		return;
	}

	// Extra
	else if (name == bExtra.getName())
	{
		doBuildHelpInfo();
		return;
	}

	// Log
	else if (name == bLog.getName())
	{
		doBuildHelpInfo();
		return;
	}

	// Solo
	else if (name == bSolo.getName())
	{
		doBuildHelpInfo();
		return;
	}

	// Keys
	else if (name == bKeys.getName())
	{
		doBuildHelpInfo();
		return;
	}

	// Minimize
	else if (name == bMinimize.getName())
	{
		doBuildHelpInfo();
		return;
	}

	// Theme
	else if (name == bThemeUiAlt.getName())
	{
		setupImGuiTheme();
		return;
	}

	//else if (name == bMinimize_Presets.getName())
	//{
	//	doBuildHelpInfo();
	//}

	//----

	//TODO:
	// Skip below callbacks
	// when not using the Layout Presets Engine!
	// to improve the performance a bit
	// by skipping the not required callbacks.
	if (surfingImGuiMode != ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING)
		if (!bDockingLayoutPresetsEngine) return;

	//--

	if (0) {}

	// Gui layout
	else if (name == bGui_LayoutsPresetsSelector.getName())
	{
		return;
	}

	//--

	// Reset
	// This toggle/flag is "sent" to the parent scope (ofApp), to resets something in our apps.
	// Example: to resets the layout.
	// That pointer must be created externally!
	else if (name == bReset.getName() && bReset.get())
	{
		bReset = false;

		if (bResetPtr != nullptr) {
			*bResetPtr = true;
		}
		return;
	}

	else if (name == bReset_Window.getName() && bReset_Window.get())
	{
		bReset_Window = false;
		return;
	}

	//--

	// Solo Panel

	else if (name == bSolo.getName() && bSolo.get())
	{
		// workflow
		appLayoutIndex = -1;

		// disable preset
		for (int i = 0; i < bLayoutPresets.size(); i++)
		{
			bLayoutPresets[i].setWithoutEventNotifications(false);
		}
		return;
	}

	//--

	// Layout preset index

	else if (name == appLayoutIndex.getName())
	{
		//appLayoutIndex = ofClamp(appLayoutIndex.get(), appLayoutIndex.getMin(), appLayoutIndex.getMax());

		//if (appLayoutIndex != appLayoutIndex_PRE /*&& appLayoutIndex_PRE != -1*/)

		if (appLayoutIndex != appLayoutIndex_PRE)
		{
			ofLogNotice("ofxSurfingImGui") << "Changed: " << appLayoutIndex;

			//-

			// 1. Auto save

			if (bAutoSave_Layout)
			{
				// workaround:
				// must save here bc usually we use the fallged on update save...
				// only once per cycle allowed this way.
				// force to ensure save bc update chain load and save below
				//saveAppLayout(AppLayouts(appLayoutIndex_PRE));
				std::string __ini_to_save_Str = getLayoutName(appLayoutIndex_PRE);

				if (__ini_to_save_Str != "-1")
				{
					const char* _iniSave = NULL;
					_iniSave = __ini_to_save_Str.c_str(); // flags to save on update

					if (_iniSave != "-1")
					{
						saveLayoutPreset(_iniSave);
					}
				}
			}

			appLayoutIndex_PRE = appLayoutIndex.get();
		}

		//-

		// We get notified of special window toggles changes here!
		// not inside the WindowsOrganizer object!

		// 2. index preset selected is -1
		// Hide all panels

		if (appLayoutIndex == -1) // When no preset selected, puts all panels to false
		{
			for (int i = 0; i < windows.size(); i++)
			{
				if (windows[i].bGui)
					windows[i].bGui.set(false);
			}

			return; // not required bc loadAppLayout will be skipped when passed -1
		}

		//-

		// 2. Load layout
		loadAppLayout(appLayoutIndex.get());

		return;
	}

	//-

	// Presets Selector
	// exclusive toggles
	// Solo behavior
	{
		bool bSomeTrue = false;

		// iterate each preset toggle
		for (int i = 0; i < bLayoutPresets.size(); i++)
		{
			// if that toggle changed
			if (name == bLayoutPresets[i].getName())
			{
				// is changed to true
				if (bLayoutPresets[i].get())
				{
					// workflow
					if (bSolo.get()) bSolo = false;

					appLayoutIndex = i;
					bSomeTrue = true;

					continue; // bc only one will be changed at once. no need to check the others.
				}

				// is changed to false
				else
				{
					// avoid all false
					bool bAllFalse = true;
					for (int i = 0; i < bLayoutPresets.size(); i++)
					{
						if (bLayoutPresets[i].get()) {
							bAllFalse = false;
						}
					}
					if (bAllFalse)
					{
						// workflow A
						////force back to true if it's there's no other enabled..
						//bLayoutPresets[appLayoutIndex].set(true);

						// workflow B
						//set to -1
						//appLayoutIndex = -1;

						//TODO:
						appLayoutIndex.set(-1);
						//appLayoutIndex.setWithoutEventNotifications(-1);
					}
				}
			}
		}

		if (bSomeTrue)
		{
			for (int i = 0; i < bLayoutPresets.size(); i++)
			{
				// the preset toggle that currently changed has been to true
				// put the other toggles to false
				if (i != appLayoutIndex.get())
				{
					bLayoutPresets[i].set(false);
					//bLayoutPresets[i].setWithoutEventNotifications(false);
				}
			}
			return;
		}
	}

	//-

	//TODO: bug
	// fix

	// Solo Panels Selectors behavior

	if (bSolo.get())
	{
		// iterate all panels
		// search for which one changed and to true
		for (int i = 0; i < windows.size(); i++)
		{
			// if that one has changed and it goes to true
			if (name == windows[i].bGui.getName() && windows[i].bGui)
			{
				// set the others to false and return
				for (int k = 0; k < windows.size(); k++)
				{
					// i is the index of the panel toggle that just changed
					if (k != i)//put the others to false
					{
						if (windows[k].bGui)
							windows[k].bGui.set(false);
					}
				}
				return;
			}
		}
	}

	//--

	//TODO:
	//// Rectangles
	//{
	//	for (int i = 0; i < rectangles_Windows.size(); i++)
	//	{
	//		if (name == rectangles_Windows[i].getName())
	//		{
	//		}
	//	}
	//}
}

//----

// Layout Preset Loaders / Savers

//--------------------------------------------------------------
void SurfingGuiManager::saveLayoutPreset(std::string path)
{
	saveLayoutImGuiIni(path);
	saveLayoutPresetGroup(path);
}

//--------------------------------------------------------------
void SurfingGuiManager::loadLayoutPreset(std::string path)
{
	loadLayoutImGuiIni(ini_to_load);
	loadLayoutPresetGroup(path);
}

//--------------------------------------------------------------
void SurfingGuiManager::saveLayoutImGuiIni(std::string path)
{
	ImGui::SaveIniSettingsToDisk(ofToDataPath(path_ImLayouts + path + ".ini", true).c_str());
}

//--------------------------------------------------------------
void SurfingGuiManager::loadLayoutImGuiIni(std::string path)
{
	ImGui::LoadIniSettingsFromDisk(ofToDataPath(path_ImLayouts + path + ".ini", true).c_str());
}

//--------------------------------------------------------------
void SurfingGuiManager::saveLayoutPresetGroup(std::string path)
{
	saveGroup(params_Layouts, path_ImLayouts + path + ".json");
}

//--------------------------------------------------------------
void SurfingGuiManager::loadLayoutPresetGroup(std::string path)
{
	loadGroup(params_Layouts, path_ImLayouts + path + ".json");
}

//----

//--------------------------------------------------------------
void SurfingGuiManager::createLayoutPreset(std::string namePreset)
{
	std::string n;
	//int i = bLayoutPresets.size();
	int i = bLayoutPresets.size() + 1;

	if (namePreset == "-1") n = "P" + ofToString(i);
	else n = namePreset;

	ofParameter<bool> _b = ofParameter<bool>{ n, false };
	bLayoutPresets.push_back(_b);
	appLayoutIndex.setMax(bLayoutPresets.size() - 1);
	params_LayoutPresetsStates.add(bLayoutPresets[i - 1]);
	//params_LayoutPresetsStates.add(bLayoutPresets[i]);
}

//----

//--------------------------------------------------------------
void SurfingGuiManager::drawLayoutsPanels()
{
	flags_wPanels = ImGuiWindowFlags_None;
	flags_wPanels += ImGuiWindowFlags_NoSavedSettings;

	if (bAutoResize_Panels) flags_wPanels += ImGuiWindowFlags_AlwaysAutoResize;

	//--

	// Used to read the window shape below
	// that's done to re arrange the toggles distribution with a responsive behavior.
	static bool _bLandscape = false;

	// Docking mode ignores these constraints... ?

#ifdef OFX_IMGUI_CONSTRAIT_WINDOW_SHAPE

	// Notice that _bLandscape on first frame is not updated!

	const int NUM_WIDGETS = windows.size(); // expected num widgets
	float ww = 450;
	float hh = 300;

	// set variable sizes to make all text (all/none) visible!
	if (NUM_WIDGETS > 5) {
		ww = 600 + 200;
		hh = 450;
	}
	else if (NUM_WIDGETS > 4) {
		ww = 600 + 100;
		hh = 450;
	}
	else if (NUM_WIDGETS > 3) {
		ww = 600;
		hh = 450;
	}

	// Landscape
	if (_bLandscape) ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(ww, 100));

	//if (_bLandscape) ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(450, 100));
	//if (_bLandscape) ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(700, 150));

	// Portrait
	else ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(95, hh));

	//else ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(95, 300));
	//else ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(105, 300));

#endif

	//--

	// Reset window

	ImGuiCond pnCond = ImGuiCond_None;
	pnCond += ImGuiCond_Appearing;

	if (bReset_WindowPanels)
	{
		bReset_WindowPanels = false;

		const int i = 1;
		ofRectangle r = rectangles_Windows[i];
		r.setWidth(150);
		r.setHeight(150);
		rectangles_Windows[i].set(r);
		pnCond = ImGuiCond_Always;
	}

	//-

	// Position and Shape

	const int i = 1;

	if (bGui_LayoutsPanels)
	{
		ImGui::SetNextWindowPos(ofVec2f(rectangles_Windows[i].get().getX(), rectangles_Windows[i].get().getY()), pnCond);
		ImGui::SetNextWindowSize(ofVec2f(rectangles_Windows[i].get().getWidth(), rectangles_Windows[i].get().getHeight()), pnCond);
	}

	//-

	// Draw window

	if (BeginWindow(bGui_LayoutsPanels, flags_wPanels))
	{
		const int i = 1;
		rectangles_Windows[i].setWithoutEventNotifications(ofRectangle(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));

		_bLandscape = false;
		float __w = ImGui::GetWindowWidth();
		float __h = ImGui::GetWindowHeight();
		if (__w > __h) _bLandscape = true;

		//-

		const int NUM_WIDGETS = windows.size(); // expected num widgets
		const int NUM_WIDGETS_EXTRA_LANDSCAPE = 5;
		//const int NUM_WIDGETS_EXTRA_LANDSCAPE = 6; // with autoresize

		float _spcx = ImGui::GetStyle().ItemSpacing.x;
		float _spcy = ImGui::GetStyle().ItemSpacing.y;
		float _h100 = ImGui::GetContentRegionAvail().y;

		float _w;
		float _h;
		float _hWid;

		//-

		// A. Landscape

		if (_bLandscape)
		{
			//_w = ofxImGuiSurfing::getWidgetsWidth(NUM_WIDGETS + 2);
			const int amntColumns = NUM_WIDGETS + 2;
			float __w100 = ImGui::GetContentRegionAvail().x - (3 * _spcx);
			// remove extra columns x spacing added!
			_w = (__w100 - _spcx * (amntColumns - 1)) / amntColumns;

			_h = _h100 - _spcy;
			_hWid = (_h - _spcy) / 2;
			//_hWid = (_h - _spcy) / 3; // with autoResize
		}

		//-

		// B. Portrait

		else
		{
			_hWid = ofxImGuiSurfing::getWidgetsHeightRelative();
			float _hTotal = _h100 - (_hWid * NUM_WIDGETS_EXTRA_LANDSCAPE + (NUM_WIDGETS_EXTRA_LANDSCAPE + 1) * _spcy);
			_w = ofxImGuiSurfing::getWidgetsWidth();
			_h = _hTotal / NUM_WIDGETS - _spcy;
		}

		//-

		// Landscape

		if (_bLandscape)
		{
			ImGui::Columns(3, "", false);
			//ImGui::Columns(3);
			ImGui::SetColumnWidth(0, _spcx + (_w + _spcx) * NUM_WIDGETS);
		}

		//-

		// 1. Populate all windows (aka panels) toggles

		for (int i = 0; i < windows.size(); i++)
		{
			ofxImGuiSurfing::AddBigToggle(windows[i].bGui, _w, _h);
			if (_bLandscape) ImGui::SameLine();
		}

		//-

		float _w100;
		float _w50;

		// Landscape

		if (_bLandscape)
		{
			ImGui::SameLine();
			_w100 = _w;
			_w50 = (_w - _spcx) / 2.0f;
		}

		//-

		// Portrait

		else
		{
			//this->AddSpacing();
			this->AddSeparator();
			this->AddSpacing();

			_w100 = ofxImGuiSurfing::getWidgetsWidth(1);
			_w50 = ofxImGuiSurfing::getWidgetsWidth(2);
		}

		//-

		// Landscape

		if (_bLandscape) {
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, _w100 + 2 * _spcx);
		}

		//-

		// 2. Extra widgets

		if (ImGui::Button("All", ImVec2(_w50, _hWid)))
		{
			// workflow
			if (bSolo) bSolo.set(false);

			bool b = true;
			setShowAllPanels(b);
		}

		ImGui::SameLine();

		if (ImGui::Button("None", ImVec2(_w50, _hWid)))
		{
			bool b = false;
			setShowAllPanels(b);
		}

		ofxImGuiSurfing::AddBigToggle(bSolo, _w100, _hWid, true);

		//-

		// 3. Panels Toggles

		// Landscape
		if (_bLandscape)
		{
			ImGui::NextColumn();
			ImGui::SetColumnWidth(2, _w100 + _spcx + _spcx);
		}

		// Portrait
		else
		{
			//this->AddSpacing();
			this->AddSeparator();
			this->AddSpacing();
		}

		//-

		// Layout

		float _hUnit = ofxImGuiSurfing::getWidgetsHeightRelative();

		ofxImGuiSurfing::AddBigToggle(bGui_LayoutsPresetsSelector, _w100,
			(bMinimize_Presets ? ofxImGuiSurfing::getPanelHeight() : _hWid), false);
		this->AddTooltip("F5");

		if (!bMinimize_Presets) {
			ofxImGuiSurfing::AddBigToggle(bGui_LayoutsManager, _w100, _hWid, false);
			this->AddTooltip("F7");
		}

		//--

		// Landscape

		if (_bLandscape)
		{
			ImGui::Columns();
		}

		this->EndWindow();
	}

	//-

#ifdef OFX_IMGUI_CONSTRAIT_WINDOW_SHAPE
	ImGui::PopStyleVar();
#endif
}

//--

// Keys
//--------------------------------------------------------------
void SurfingGuiManager::keyPressed(ofKeyEventArgs& eventArgs)
{
	if (!bKeys || this->bOverInputText) return;

	const int& key = eventArgs.key;
	ofLogNotice("ofxSurfingImGui") << "keyPressed: " << (char)key;

	// Modifiers
	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

	// Log
	if (bLogKeys)
		if (key != OF_KEY_SHIFT && !mod_COMMAND && !mod_CONTROL && !mod_ALT && !mod_SHIFT)
		{
			std::string ss = "KEY " + ofToString((char)key) + "";
			log.Add(ss, 3);
		}

	//-

	if (0)
	{
		ofLogNotice("ofxSurfingImGui") << "mod_COMMAND : " << (mod_COMMAND ? "ON" : "OFF");
		ofLogNotice("ofxSurfingImGui") << "mod_CONTROL : " << (mod_CONTROL ? "ON" : "OFF");
		ofLogNotice("ofxSurfingImGui") << "mod_ALT     : " << (mod_ALT ? "ON" : "OFF");
		ofLogNotice("ofxSurfingImGui") << "mod_SHIFT   : " << (mod_SHIFT ? "ON" : "OFF");
	}

	//----

	// Help App
	if (key == 'H')
	{
		bHelp = !bHelp;
	}
	// Help Internal
	else if (key == 'I')
	{
		bHelpInternal = !bHelpInternal;
	}

	// Minimize
	else if (key == '`')
	{
		bMinimize = !bMinimize;
	}

	// Extra
	else if (key == 'E' && !mod_CONTROL)
	{
		bExtra = !bExtra;
	}

	// Debug
	else if (key == 'D' && !mod_CONTROL)
	{
		bDebug = !bDebug;
	}

	// Log
	else if (key == 'L' && !mod_CONTROL)
	{
		bLog = !bLog;
	}

	//--

	// Layout Presets Engine
	{
		if (!bDockingLayoutPresetsEngine && !bUseLayoutPresetsManager) return;//skip is not enabled!

		//--

		if (!mod_CONTROL) // CTRL not pressed
		{
			switch (key)
			{
			case OF_KEY_F1: bLayoutPresets[0] = !bLayoutPresets[0]; break;
			case OF_KEY_F2: bLayoutPresets[1] = !bLayoutPresets[1]; break;
			case OF_KEY_F3: bLayoutPresets[2] = !bLayoutPresets[2]; break;
			case OF_KEY_F4: bLayoutPresets[3] = !bLayoutPresets[3]; break;
				//TODO: amount of presets is hard-coded
			}

			//--

			if (key == OF_KEY_F5) // Presets
			{
				bGui_LayoutsPresetsSelector = !bGui_LayoutsPresetsSelector;
			}

			else if (key == OF_KEY_F6) // Panels
			{
				bGui_LayoutsPanels = !bGui_LayoutsPanels;
			}

			else if (key == OF_KEY_F7) // Manager
			{
				bGui_LayoutsManager = !bGui_LayoutsManager;
			}

			//else if (key == OF_KEY_F8) // Tools
			//{
			//	bGui_LayoutsPresetsManual = !bGui_LayoutsPresetsManual;
			//}
		}
		else // CTRL pressed
		{
			switch (key)
			{
			case OF_KEY_F1: doSpecialWindowToggleVisible(0); break;
			case OF_KEY_F2: doSpecialWindowToggleVisible(1); break;
			case OF_KEY_F3: doSpecialWindowToggleVisible(2); break;
			case OF_KEY_F4: doSpecialWindowToggleVisible(3); break;
			case OF_KEY_F5: doSpecialWindowToggleVisible(4); break;
			case OF_KEY_F6: doSpecialWindowToggleVisible(5); break;
			case OF_KEY_F7: doSpecialWindowToggleVisible(6); break;
			case OF_KEY_F8: doSpecialWindowToggleVisible(7); break;
				//case OF_KEY_F9: doSpecialWindowToggleVisible(8); break;
				//TODO: amount of panels is hard-coded
			}
		}

		//else if (key == OF_KEY_F9) // Minimize
		//{
		//	bMinimize_Presets = !bMinimize_Presets;
		//}

		//--

		//TODO: not working ctrl

		// Solo
		if ((key == 's' && mod_CONTROL) || key == 19)
		{
			bSolo = !bSolo;
		}
		//TODO: Bug: collides with some other keys like shift + drag docking...
		//// All
		//if ((key == 'a' && mod_CONTROL) || key == 1)
		//{
		//	setShowAllPanels(true);
		//}
		// None
		if ((key == 'n' && mod_CONTROL) || key == 14)
		{
			setShowAllPanels(false);
		}

		//// Unlock Dock
		//else if (key == 'l')
		//{
		//	bModeLock1 = !bModeLock1;
		//}

		//else if (key == 'L')
		//{
		//	bModeLockControls = !bModeLockControls;
		//}

		//----

		//// Layout Modes

		//else if (key == OF_KEY_TAB && !mod_CONTROL)
		//{
		//	if (appLayoutIndex > appLayoutIndex.getMin()) appLayoutIndex++;
		//	else if (appLayoutIndex == appLayoutIndex.getMin()) appLayoutIndex = appLayoutIndex.getMax();
		//	//if (appLayoutIndex < 3) loadAppLayout(AppLayouts(appLayoutIndex + 1));
		//	//else if (appLayoutIndex == 3) loadAppLayout(AppLayouts(0));
		//}
	}
}

//--------------------------------------------------------------
void SurfingGuiManager::keyReleased(ofKeyEventArgs& eventArgs)
{
	if (!bKeys || this->bOverInputText) return;

	const int& key = eventArgs.key;
	ofLogNotice("ofxSurfingImGui") << "keyReleased: " << (char)key;

	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);
}

//--

//--------------------------------------------------------------
void SurfingGuiManager::drawMenuDocked()
{
	static bool opt_fullscreen = true;
	static bool* p_open = NULL;
	static bool opt_exit = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	//-

	// Menu bar

	// This is not operative. just for testing menus!

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			ofWindowMode windowMode = ofGetCurrentWindow()->getWindowMode();
			if (windowMode == OF_WINDOW) opt_fullscreen = false;
			else if (windowMode == OF_FULLSCREEN) opt_fullscreen = true;
			else if (windowMode == OF_GAME_MODE) opt_fullscreen = false;

			if (ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen))
			{
				ofSetFullscreen(opt_fullscreen);
			}

			this->AddSpacingSeparated();

			if (ImGui::MenuItem("Exit", NULL, &opt_exit))
			{
				ofExit();
				//*opt_exit = false;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy", NULL)) {}
			if (ImGui::MenuItem("Paste", NULL)) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Layouts"))
		{
			for (int i = 0; i < bLayoutPresets.size(); i++)
			{
				if (ImGui::MenuItem(bLayoutPresets[i].getName().c_str(), "", (bool*)&bLayoutPresets[i].get()))
				{
					bLayoutPresets[i] = bLayoutPresets[i]; // to trig
				}
			}
			//this->AddSpacingSeparated();
			//if (ImGui::MenuItem("All", NULL)) { setShowAllPanels(true); }
			//if (ImGui::MenuItem("None", NULL)) { setShowAllPanels(false); }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Docking"))
		{
			if (ImGui::MenuItem("WARNING", "")) {};
			ofxImGuiSurfing::AddTooltip2("Don't pay attention for this! \nThis is not operative here. \nJust for testing menus!\nPotential CRASH!");

			dockspace_flags = ImGui::GetIO().ConfigFlags;

			if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
			if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
			if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
			if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
			//if (ImGui::MenuItem("Flag: ConfigDockingWithShift", "", (dockspace_flags & ImGuiDockNodeFlags_) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
			//this->AddSpacingSeparated();

			ImGui::GetIO().ConfigFlags = dockspace_flags;
			//ImGui::GetIO().ConfigDockingWithShift = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About"))
		{
			ofxImGuiSurfing::AddTooltipHelp(
				"WARNING\nDon't pay attention for this text! \nThis is not operative here. \nJust for testing menus!" "\n\n"
				"When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
				"- Drag from window title bar or their tab to dock/undock." "\n"
				"- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
				"- Hold SHIFT to enable dragging docking." "\n"
				"This demo app has nothing to do with it!" "\n\n"
				"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window. This is useful so you can decorate your main application window (e.g. with a menu bar)." "\n\n"
				"ImGui::DockSpace() comes with one hard constraint: it needs to be submitted _before_ any window which may be docked into it. Therefore, if you use a dock spot as the central point of your application, you'll probably want it to be part of the very first window you are submitting to imgui every frame." "\n\n"
				"(NB: because of this constraint, the implicit \"Debug\" window can not be docked into an explicit DockSpace() node, because that window is submitted as part of the NewFrame() call. An easy workaround is that you can create your own implicit \"Debug##2\" window after calling DockSpace() and leave it in the window stack for anyone to use.)"
			);

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

//--

//--------------------------------------------------------------
void SurfingGuiManager::drawMenu()
{
	static bool opt_fullscreen = true;
	static bool* p_open = NULL;
	static bool opt_exit = false;

	//-

	// Menu bar

	// This is not operative. just for testing menus!

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			ofWindowMode windowMode = ofGetCurrentWindow()->getWindowMode();
			if (windowMode == OF_WINDOW) opt_fullscreen = false;
			else if (windowMode == OF_FULLSCREEN) opt_fullscreen = true;
			else if (windowMode == OF_GAME_MODE) opt_fullscreen = false;

			if (ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen))
			{
				ofSetFullscreen(opt_fullscreen);
			}

			this->AddSpacingSeparated();

			if (ImGui::MenuItem("Exit", NULL, &opt_exit))
			{
				ofExit();
				//*opt_exit = false;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy", NULL)) {}
			if (ImGui::MenuItem("Paste", NULL)) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About"))
		{
			ofxImGuiSurfing::AddTooltipHelp(
				"This is not operative here. Just for testing menus!" "\n\n"
				"When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
				"- Drag from window title bar or their tab to dock/undock." "\n"
				"- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
				"- Hold SHIFT to enable dragging docking." "\n"
				"This demo app has nothing to do with it!" "\n\n"
				"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window. This is useful so you can decorate your main application window (e.g. with a menu bar)." "\n\n"
				"ImGui::DockSpace() comes with one hard constraint: it needs to be submitted _before_ any window which may be docked into it. Therefore, if you use a dock spot as the central point of your application, you'll probably want it to be part of the very first window you are submitting to imgui every frame." "\n\n"
				"(NB: because of this constraint, the implicit \"Debug\" window can not be docked into an explicit DockSpace() node, because that window is submitted as part of the NewFrame() call. An easy workaround is that you can create your own implicit \"Debug##2\" window after calling DockSpace() and leave it in the window stack for anyone to use.)"
			);

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

//--

//--------------------------------------------------------------
void SurfingGuiManager::doResetLayout()
{
	ofLogNotice("ofxSurfingImGui") << "Reset layout of windows.";

	windowsOrganizer.doAlignWindowsReset();

	ofLogNotice("ofxSurfingImGui") << "The more lefted window is named: " << windowsOrganizer.getWindowMoreLefted();
}

//----

/*
//TODO:
//--------------------------------------------------------------
void SurfingGuiManager::drawSpecialWindowsPanel()
{
	if (ImGui::TreeNode("Special Windows"))
	{
		//ImGui::Text("Special Windows");

		//ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		//if (bAutoResize_Panels) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		//if (BeginWindow("Specials", NULL, window_flags))
		{
			const int NUM_WIDGETS = windows.size(); // expected num widgets

			float _spcx = ImGui::GetStyle().ItemSpacing.x;
			float _spcy = ImGui::GetStyle().ItemSpacing.y;
			//float _h100 = ImGui::GetContentRegionAvail().y;

			// 1. populate all toggles

			const int _amnt = 1;

			float _w = ofxImGuiSurfing::getWidgetsWidth(_amnt);
			float _h = 1 * ofxImGuiSurfing::getWidgetsHeightRelative();
			//float _w = ofxImGuiSurfing::getWidgetsWidth(windows.size());

			for (int i = 0; i < NUM_WIDGETS; i++)
			{
				if (i > windows.size() - 1) continue;

				ofxImGuiSurfing::AddBigToggle(windows[i].bGui, _w, _h);

				//if ((i + 1) % _amnt != 0 && i < NUM_WIDGETS - 1) ImGui::SameLine();
			}
		}

		ImGui::TreePop();
	}
}*/

//--

// Special behavior to control windows

//// Free layout
//else if (name == bModeFree.getName())
//{
//	if (bModeFree.get())
//	{
//		flagsWindowsModeFreeStore = ImGuiWindowFlags_NoSavedSettings;
//	}
//	else
//	{
//		flagsWindowsModeFreeStore = ImGuiWindowFlags_None;
//	}
//}

//-

//// Lock layout
//else if (name == bModeLock1.getName())
//{
//	if (!bModeLock1)
//	{
//		flagsWindowsLocked1 = ImGuiWindowFlags_None;
//	}
//	else
//	{
//		flagsWindowsLocked1 = ImGuiWindowFlags_NoMove;
//		flagsWindowsLocked1 |= ImGuiWindowFlags_NoSavedSettings;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoResize;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoTitleBar;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoCollapse;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoDecoration;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoBackground;
//		//flagsWindowsLocked1 |= ImGuiDockNodeFlags_AutoHideTabBar;
//		//flagsWindowsLocked1 |= ImGuiDockNodeFlags_NoTabBar;
//		//flagsWindowsLocked1 |= ImGuiDockNodeFlags_NoCloseButton;
//	}
//}
//else if (name == bModeLockControls.getName())
//{
//	if (!bModeLockControls)
//	{
//		flagsWindowsLocked2 = ImGuiWindowFlags_None;
//	}
//	else
//	{
//		flagsWindowsLocked2 = ImGuiWindowFlags_NoMove;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoResize;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoTitleBar;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoCollapse;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoDecoration;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoBackground;
//		//flagsWindowsLocked2 |= ImGuiDockNodeFlags_AutoHideTabBar;
//		//flagsWindowsLocked2 |= ImGuiDockNodeFlags_NoTabBar;
//		//flagsWindowsLocked2 |= ImGuiDockNodeFlags_NoCloseButton;
//	}
//}
