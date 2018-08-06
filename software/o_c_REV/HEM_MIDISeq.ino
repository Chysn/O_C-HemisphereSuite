class MIDISeq : public HemisphereApplet {
public:

    const char* applet_name() {
        return "MIDISeq";
    }

    void Start() {
        step = 0;
        length = 8;
        for (int i = 0; i < 32; i++) note[i] = 0;
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
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=Note 2=Trg Write";
        help[HEMISPHERE_HELP_OUTS]     = "A=Pitch";
        help[HEMISPHERE_HELP_ENCODER]  = "Cursor/Edit";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    int step;
    int note[32];

    // Settings
    uint8_t channel; // MIDI channel
    uint8_t length;
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to MIDISeq,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
MIDISeq MIDISeq_instance[2];

void MIDISeq_Start(int hemisphere) {
    MIDISeq_instance[hemisphere].BaseStart(hemisphere);
}

void MIDISeq_Controller(int hemisphere, bool forwarding) {
    MIDISeq_instance[hemisphere].BaseController(forwarding);
}

void MIDISeq_View(int hemisphere) {
    MIDISeq_instance[hemisphere].BaseView();
}

void MIDISeq_Screensaver(int hemisphere) {
    MIDISeq_instance[hemisphere].BaseScreensaverView();
}

void MIDISeq_OnButtonPress(int hemisphere) {
    MIDISeq_instance[hemisphere].OnButtonPress();
}

void MIDISeq_OnEncoderMove(int hemisphere, int direction) {
    MIDISeq_instance[hemisphere].OnEncoderMove(direction);
}

void MIDISeq_ToggleHelpScreen(int hemisphere) {
    MIDISeq_instance[hemisphere].HelpScreen();
}

uint32_t MIDISeq_OnDataRequest(int hemisphere) {
    return MIDISeq_instance[hemisphere].OnDataRequest();
}

void MIDISeq_OnDataReceive(int hemisphere, uint32_t data) {
    MIDISeq_instance[hemisphere].OnDataReceive(data);
}
