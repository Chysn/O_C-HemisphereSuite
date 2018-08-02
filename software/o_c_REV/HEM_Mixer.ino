class ClassName : public HemisphereApplet {
public:

    const char* applet_name() {
        return "ClassName";
    }

    void Start() {
    }

    void Controller() {
    }

    void View() {
        gfxHeader(applet_name());
        gfxSkyline();
        // Add other view code as private methods
    }

    void ScreensaverView() {
        gfxSkyline();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name); 
        return data;
    }

    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8}); 
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "1,2 Signals";
        help[HEMISPHERE_HELP_OUTS]     = "A=Mix Out";
        help[HEMISPHERE_HELP_ENCODER]  = "P=Signal T=Level";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    
    
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ClassName,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ClassName ClassName_instance[2];

void ClassName_Start(int hemisphere) {
    ClassName_instance[hemisphere].BaseStart(hemisphere);
}

void ClassName_Controller(int hemisphere, bool forwarding) {
    ClassName_instance[hemisphere].BaseController(forwarding);
}

void ClassName_View(int hemisphere) {
    ClassName_instance[hemisphere].BaseView();
}

void ClassName_Screensaver(int hemisphere) {
    ClassName_instance[hemisphere].BaseScreensaverView();
}

void ClassName_OnButtonPress(int hemisphere) {
    ClassName_instance[hemisphere].OnButtonPress();
}

void ClassName_OnEncoderMove(int hemisphere, int direction) {
    ClassName_instance[hemisphere].OnEncoderMove(direction);
}

void ClassName_ToggleHelpScreen(int hemisphere) {
    ClassName_instance[hemisphere].HelpScreen();
}

uint32_t ClassName_OnDataRequest(int hemisphere) {
    return ClassName_instance[hemisphere].OnDataRequest();
}

void ClassName_OnDataReceive(int hemisphere, uint32_t data) {
    ClassName_instance[hemisphere].OnDataReceive(data);
}
