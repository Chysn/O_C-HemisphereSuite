class TrigSeq16 : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "TrigSeq16";
    }

    void Start() {
        ForEachChannel(ch)
        {
            pattern[ch] = random(1, 255);
        }
        step = 0;
        end_step = 15;
        cursor = 0;
    }

    void Controller() {
        if (Clock(1)) step = 0;

        if (Clock(0)) {
            if (step < 8) {
                if ((pattern[0] >> step) & 0x01) ClockOut(0);
                else ClockOut(1);
            } else {
                if ((pattern[1] >> (step - 8)) & 0x01) ClockOut(0);
                else ClockOut(1);
            }
            step++;
            if (step > end_step) step = 0;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawDisplay();
    }

    void ScreensaverView() {
        DrawDisplay();
    }

    void OnButtonPress() {
        cursor++;
        if (cursor > 4) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        // Update end_step
        if (cursor == 4) {
            end_step = constrain(end_step += direction, 0, 15);
        } else {
            int ch = cursor > 1 ? 1 : 0;
            int this_cursor = cursor - (ch * 2);

            // Get the current pattern
            int curr_patt = pattern[ch];

            // Shift right based on which nybble is selcted
            uint8_t nybble = (curr_patt >> (this_cursor * 4)) & 0x0f;
            nybble += direction;
            nybble &= 0x0f;

            // Put the updated nybble back where it belongs
            if (this_cursor == 0) pattern[ch] = (curr_patt & 0xf0) + nybble;
            else pattern[ch] = (curr_patt & 0x0f) + (nybble << 4);
        }
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, pattern[0]);
        Pack(data, PackLocation {8,8}, pattern[1]);
        Pack(data, PackLocation {16,4}, end_step);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        pattern[0] = Unpack(data, PackLocation {0,8});
        pattern[1] = Unpack(data, PackLocation {8,8});
        end_step = Unpack(data, PackLocation {16,4});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "Trg A=Trg B=Cmp";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Set P=Select";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int step; // Current step
    uint8_t pattern[2];
    int end_step;
    int cursor; // 0=ch1 low, 1=ch1 hi, 2=ch2 low, 3=ch3 hi, 4=end_step
    
    void DrawDisplay() {
        bool stop = 0; // Stop displaying when end_step is reached

        ForEachChannel(ch)
        {
            int x = 10 + (31 * ch);

            // Draw the steps for this channel
            for (int s = 0; s < 8; s++)
            {
                if (!stop) {
                    int y = 18 + (6 * s);
                    gfxCircle(x, y, 3);
                    int value = (pattern[ch] >> s) & 0x01;
                    if (value) {
                        for (int r = 1; r < 3; r++) {
                            gfxCircle(x, y, r);
                            gfxCircle(x + 1, y, r);
                            gfxCircle(x + 2, y, r);
                        }
                    }

                    if (s + (ch * 8) == step) {
                        gfxLine(x + 4, y, x + 10, y);
                    }

                    // Draw the end_step cursor
                    if (s + (ch * 8) == end_step) {
                        if (cursor == 4) {
                            gfxLine(x - 8, y + 3, x + 5, y + 3);
                        }
                        stop = 1;
                    }
                }
            }

            // Draw the nybble cursor
            if (cursor < 4) {
                if (ch == (cursor > 1 ? 1 : 0) && CursorBlink()) {
                    int this_cursor = cursor - (ch * 2);
                    int y = 15 + (this_cursor * 24);
                    gfxLine(x - 5, y, x - 5, y + 24);
                }
            }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to TrigSeq16,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
TrigSeq16 TrigSeq16_instance[2];

void TrigSeq16_Start(int hemisphere) {
    TrigSeq16_instance[hemisphere].BaseStart(hemisphere);
}

void TrigSeq16_Controller(int hemisphere, bool forwarding) {
    TrigSeq16_instance[hemisphere].BaseController(forwarding);
}

void TrigSeq16_View(int hemisphere) {
    TrigSeq16_instance[hemisphere].BaseView();
}

void TrigSeq16_Screensaver(int hemisphere) {
    TrigSeq16_instance[hemisphere].BaseScreensaverView();
}

void TrigSeq16_OnButtonPress(int hemisphere) {
    TrigSeq16_instance[hemisphere].OnButtonPress();
}

void TrigSeq16_OnEncoderMove(int hemisphere, int direction) {
    TrigSeq16_instance[hemisphere].OnEncoderMove(direction);
}

void TrigSeq16_ToggleHelpScreen(int hemisphere) {
    TrigSeq16_instance[hemisphere].HelpScreen();
}

uint32_t TrigSeq16_OnDataRequest(int hemisphere) {
    return TrigSeq16_instance[hemisphere].OnDataRequest();
}

void TrigSeq16_OnDataReceive(int hemisphere, uint32_t data) {
    TrigSeq16_instance[hemisphere].OnDataReceive(data);
}
