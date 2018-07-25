#define HEM_BURST_NUMBER_MAX 12
#define HEM_BURST_SPACING_MAX 500
#define HEM_BURST_SPACING_MIN 8

class Burst : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Burst";
    }

    void Start() {
        cursor = 0;
        number = 4;
        spacing = 50;
        bursts_to_go = 0;
    }

    void Controller() {
        // Settings over CV
        if (DetentedIn(0) > 0) number = ProportionCV(In(0), HEM_BURST_NUMBER_MAX - 1) + 1;
        if (DetentedIn(1) > 0) {
            spacing = ProportionCV(In(1), HEM_BURST_SPACING_MAX);
            if (spacing < HEM_BURST_SPACING_MIN) spacing = HEM_BURST_SPACING_MIN;
        }

        // Handle a burst set in progress
        if (bursts_to_go > 0) {
            if (--burst_countdown <= 0) {
                ClockOut(0, 34); // Short burst, about 2ms
                if (--bursts_to_go > 0) burst_countdown = spacing * 17; // Reset for next burst
                else GateOut(1, 0); // Turn off the gate
            }
        }

        // Handle a new burst set trigger
        if (Clock(0)) {
            GateOut(1, 1);
            bursts_to_go = number;
            burst_countdown = spacing * 17;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        DrawIndicator();
    }

    void ScreensaverView() {
        DrawSelector();
        DrawIndicator();
    }

    void OnButtonPress() {
        cursor = 1 - cursor;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) number = constrain(number += direction, 1, HEM_BURST_NUMBER_MAX);
        if (cursor == 1) spacing = constrain(spacing += direction, HEM_BURST_SPACING_MIN, HEM_BURST_SPACING_MAX);
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
        help[HEMISPHERE_HELP_DIGITALS] = "Trigger Ch1,Ch2";
        help[HEMISPHERE_HELP_CVS]      = "1=Number 2=Spacing";
        help[HEMISPHERE_HELP_OUTS]     = "1=Burst 2=Gate";
        help[HEMISPHERE_HELP_ENCODER]  = "Number/Spacing";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // Number and Spacing
    int burst_countdown; // Number of ticks to the next expected burst
    int bursts_to_go; // Counts down to end of burst set

    // Settings
    int number; // How many bursts fire at each trigger
    int spacing; // How many ms pass between each burst

    void DrawSelector() {
        // Number
        gfxPrint(1, 15, number);
        gfxPrint(20, 15, "bursts");

        // Spacing
        gfxPrint(1, 25, spacing);
        gfxPrint(20, 25, "ms");

        // Cursor
        gfxCursor(1, 23 + (cursor * 10), 62);
    }

    void DrawIndicator() {
        for (int i = 0; i < bursts_to_go; i++)
        {
            gfxFrame(1 + (i * 4), 40, 3, 12);
        }
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Burst,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Burst Burst_instance[2];

void Burst_Start(int hemisphere) {
    Burst_instance[hemisphere].BaseStart(hemisphere);
}

void Burst_Controller(int hemisphere, bool forwarding) {
    Burst_instance[hemisphere].BaseController(forwarding);
}

void Burst_View(int hemisphere) {
    Burst_instance[hemisphere].BaseView();
}

void Burst_Screensaver(int hemisphere) {
    Burst_instance[hemisphere].BaseScreensaverView();
}

void Burst_OnButtonPress(int hemisphere) {
    Burst_instance[hemisphere].OnButtonPress();
}

void Burst_OnEncoderMove(int hemisphere, int direction) {
    Burst_instance[hemisphere].OnEncoderMove(direction);
}

void Burst_ToggleHelpScreen(int hemisphere) {
    Burst_instance[hemisphere].HelpScreen();
}

uint32_t Burst_OnDataRequest(int hemisphere) {
    return Burst_instance[hemisphere].OnDataRequest();
}

void Burst_OnDataReceive(int hemisphere, uint32_t data) {
    Burst_instance[hemisphere].OnDataReceive(data);
}
