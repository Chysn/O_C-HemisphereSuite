#define SCHMITT_FLASH_SPEED 1667

class Schmitt : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "SchmittTr";
    }

    void Start() {
        low = 3200; // ~2.1V
        high = 3968; // ~2.6V
        cursor = 0;
        gate_countdown = 0;
    }

    void Controller() {
        ForEachChannel(ch)
        {
            if (!state[ch] && In(ch) > high) state[ch] = 1;
            if (state[ch] && In(ch) < low) state[ch] = 0;
            GateOut(ch, state[ch]);
        }

        if (--gate_countdown < -SCHMITT_FLASH_SPEED) gate_countdown = SCHMITT_FLASH_SPEED;
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void ScreensaverView() {
        DrawInterface();
    }

    // No controls
    void OnButtonPress() {
        if (++cursor == 3) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 1) {
            low += (64 * direction);
            low = constrain(low, 64, high - 64);
        }
        if (cursor == 2) {
            high += (64 * direction);
            high = constrain(high, low + 64, HEMISPHERE_MAX_CV);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, low);
        Pack(data, PackLocation {16,16}, high);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        low = Unpack(data, PackLocation {0,16});
        high = Unpack(data, PackLocation {16,16});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "CV Inputs 1,2";
        help[HEMISPHERE_HELP_OUTS]     = "Gate Ouputs A,B";
        help[HEMISPHERE_HELP_ENCODER]  = "High/Low Thresh";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0 = locked 1 = high threshold, 2 = low threshold

    // Housekeeping
    bool state[2];
    int gate_countdown;

    // Settings
    uint16_t low;
    uint16_t high;

    void DrawInterface() {
        // Draw two Schmitt Trigger symbols and inputs
        ForEachChannel(ch)
        {
            uint8_t x = 1 + (15 * ch);
            uint8_t y = 26 + (16 * ch);
            DrawSchmittTriggerAtPositionWithState(x, y, state[ch]);

            // Input monitor
            gfxLine(1, 22 + (38 * ch), ProportionCV(ViewIn(ch), 62), 22 + (38 * ch));
        }

        // Draw the threshold line
        int lx = ProportionCV(low, 62);
        int hx = ProportionCV(high, 62);
        gfxLine(lx, 15, hx, 15);
        if (cursor != 1 || CursorBlink()) gfxLine(lx, 15, lx, 18);
        if (cursor != 2 || CursorBlink()) gfxLine(hx, 15, hx, 18);

    }

    void DrawSchmittTriggerAtPositionWithState(uint8_t x, uint8_t y, bool state) {
        gfxLine(x, y + 7, x + 8, y + 7); // Input line
        gfxLine(x + 8, y, x + 8, y + 14); // Base of triangle
        gfxLine(x + 8, y, x + 30, y + 7); // Top angle
        gfxLine(x + 8, y + 14, x + 30, y + 7); // Bottom angle
        gfxLine(x + 30, y + 7, x + 38, y + 7); // Output line

        gfxLine(x + 13, y + 5, x + 19, y + 5); // Schmitt symbol, top
        gfxLine(x + 10, y + 9, x + 16, y + 9); // Schmitt symbol, bottom
        gfxLine(x + 13, y + 5, x + 13, y + 9); // Scmitt symbol, left
        gfxLine(x + 16, y + 5, x + 16, y + 9); // Schmitt symbol, right

        if (state) {
            if (gate_countdown > 0) gfxLine(x + 30, y + 7, x + 38, y + 4);
            else gfxLine(x + 30, y + 7, x + 38, y + 10);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Schmitt,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Schmitt Schmitt_instance[2];

void Schmitt_Start(int hemisphere) {
    Schmitt_instance[hemisphere].BaseStart(hemisphere);
}

void Schmitt_Controller(int hemisphere, bool forwarding) {
    Schmitt_instance[hemisphere].BaseController(forwarding);
}

void Schmitt_View(int hemisphere) {
    Schmitt_instance[hemisphere].BaseView();
}

void Schmitt_Screensaver(int hemisphere) {
    Schmitt_instance[hemisphere].BaseScreensaverView();
}

void Schmitt_OnButtonPress(int hemisphere) {
    Schmitt_instance[hemisphere].OnButtonPress();
}

void Schmitt_OnEncoderMove(int hemisphere, int direction) {
    Schmitt_instance[hemisphere].OnEncoderMove(direction);
}

void Schmitt_ToggleHelpScreen(int hemisphere) {
    Schmitt_instance[hemisphere].HelpScreen();
}

uint32_t Schmitt_OnDataRequest(int hemisphere) {
    return Schmitt_instance[hemisphere].OnDataRequest();
}

void Schmitt_OnDataReceive(int hemisphere, uint32_t data) {
    Schmitt_instance[hemisphere].OnDataReceive(data);
}
