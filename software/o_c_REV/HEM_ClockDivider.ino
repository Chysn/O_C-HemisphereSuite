#define HEM_CLOCKDIV_MAX 8

class ClockDivider : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Clock Div";
    }

    void Start() {
        ForEachChannel(ch)
        {
            div[ch] = ch + 1;
            count[ch] = 0;
            next_clock[ch] = 0;
        }
        last_clock = OC::CORE::ticks;
        cycle_time = 0;
        selected = 0;
    }

    void Controller() {
        int this_tick = OC::CORE::ticks;

        // Set division via CV
        ForEachChannel(ch)
        {
            if (DetentedIn(ch)) {
                div[ch] = Proportion(In(ch), HEMISPHERE_MAX_CV / 2, 8);
                div[ch] = constrain(div[ch], -8, 8);
                if (div[ch] == 0 || div[ch] == -1) div[ch] = 1;
            }
        }

        if (Clock(0)) {
            // The input was clocked; set timing info
            cycle_time = this_tick - last_clock;

            // At the clock input, handle clock division
            ForEachChannel(ch)
            {
                count[ch]++;
                if (div[ch] > 0) { // Positive value indicates clock division
                    if (count[ch] >= div[ch]) {
                        count[ch] = 0; // Reset
                        ClockOut(ch);
                    }
                } else {
                    // Calculate next clock for multiplication on each clock
                    int clock_every = (cycle_time / -div[ch]);
                    next_clock[ch] = this_tick + clock_every;
                    ClockOut(ch); // Sync
                }
            }

            last_clock = this_tick;
        }

        // Handle clock multiplication
        ForEachChannel(ch)
        {
            if (div[ch] < 0) { // Negative value indicates clock multiplication
                if (this_tick >= next_clock[ch]) {
                    int clock_every = (cycle_time / -div[ch]);
                    next_clock[ch] += clock_every;
                    ClockOut(ch);
                }
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
    }

    void ScreensaverView() {
        DrawSelector();
    }

    void OnButtonPress() {
        selected = 1 - selected;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        div[selected] += direction;
        if (div[selected] > HEM_CLOCKDIV_MAX) div[selected] = HEM_CLOCKDIV_MAX;
        if (div[selected] < -HEM_CLOCKDIV_MAX) div[selected] = -HEM_CLOCKDIV_MAX;
        if (div[selected] == 0) div[selected] = direction > 0 ? 1 : -2; // No such thing as 1/1 Multiple
        if (div[selected] == -1) div[selected] = 1; // Must be moving up to hit -1 (see previous line)
        count[selected] = 0; // Start the count over so things aren't missed
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, div[0] + 32);
        Pack(data, PackLocation {8,8}, div[1] + 32);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        div[0] = Unpack(data, PackLocation {0,8}) - 32;
        div[1] = Unpack(data, PackLocation {8,8}) - 32;
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS] = "Div/Mult Ch1,Ch2";
        help[HEMISPHERE_HELP_OUTS] = "Clk A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "Div,Mult";
    }

private:
    int div[2]; // Division data for outputs. Positive numbers are divisions, negative numbers are multipliers
    int count[2]; // Number of clocks since last output (for clock divide)
    int next_clock[2]; // Tick number for the next output (for clock multiply)
    int last_clock; // The tick number of the last received clock
    int selected; // Which output is currently being edited
    int cycle_time; // Cycle time between the last two clock inputs

    void DrawSelector() {
        ForEachChannel(ch)
        {
            int y = 16 + (ch * 25);
            if (ch == selected) {
                gfxCursor(0, y + 9, 63);
            }
            if (div[ch] > 0) {
                gfxPrint(2, y, "1/");
                gfxPrint(div[ch]);
                gfxPrint(" Div");
            }
            if (div[ch] < 0) {
                gfxPrint(2, y, -div[ch]);
                gfxPrint("/1");
                gfxPrint(" Mult");
            }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ClockDivider,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ClockDivider ClockDivider_instance[2];

void ClockDivider_Start(int hemisphere) {
    ClockDivider_instance[hemisphere].BaseStart(hemisphere);
}

void ClockDivider_Controller(int hemisphere, bool forwarding) {
    ClockDivider_instance[hemisphere].BaseController(forwarding);
}

void ClockDivider_View(int hemisphere) {
    ClockDivider_instance[hemisphere].BaseView();
}

void ClockDivider_Screensaver(int hemisphere) {
    ClockDivider_instance[hemisphere].BaseScreensaverView();
}

void ClockDivider_OnButtonPress(int hemisphere) {
    ClockDivider_instance[hemisphere].OnButtonPress();
}

void ClockDivider_OnEncoderMove(int hemisphere, int direction) {
    ClockDivider_instance[hemisphere].OnEncoderMove(direction);
}

void ClockDivider_ToggleHelpScreen(int hemisphere) {
    ClockDivider_instance[hemisphere].HelpScreen();
}

uint32_t ClockDivider_OnDataRequest(int hemisphere) {
    return ClockDivider_instance[hemisphere].OnDataRequest();
}

void ClockDivider_OnDataReceive(int hemisphere, uint32_t data) {
    ClockDivider_instance[hemisphere].OnDataReceive(data);
}
