// Clock Divider:
//     Digital 1: Clock Source
//     Output 1: Clock Output 1
//     Output 2: Clock Output 2
//     Encoder: Set division between 1/8 and 8/1
//     Encoder Push: Toggle encoder function between Clock Output 1 and 2

#define HEMISPHERE_CLOCKDIV_MAX 8

class ClockDivider : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Clock Div";
    }

    void Start() {
        for (int ch = 0; ch < 2; ch++)
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

        if (Clock(0)) {
            // The input was clocked; set timing info
            cycle_time = this_tick - last_clock;

            // At the clock input, handle clock division
            for (int ch = 0; ch < 2; ch++)
            {
                count[ch]++;
                if (div[ch] > 0) { // Positive value indicates clock division
                    if (count[ch] == div[ch]) {
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
        for (int ch = 0; ch < 2; ch++)
        {
            if (div[ch] < 0) { // Negative value indicates clock multiplication
                if (this_tick == next_clock[ch]) {
                    int clock_every = (cycle_time / -div[ch]);
                    next_clock[ch] += clock_every;
                    ClockOut(ch);
                }
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        for (int ch = 0; ch < 2; ch++)
        {
            int y = 16 + (ch * 25);
            if (ch == selected) {
                // Cursor frame
                gfxFrame(0, y-2, 63, 11);
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

    void ScreensaverView() {
        int this_tick = OC::CORE::ticks;
        int ticks_since_clock = this_tick - last_clock;
        if (ticks_since_clock < 5000) {
            int r = 24 - (ticks_since_clock / 500);
            gfxCircle(31, 31, r);
        }
    }

    void OnButtonPress() {
        selected = 1 - selected;
    }

    void OnEncoderMove(int direction) {
        div[selected] += direction;
        if (div[selected] > HEMISPHERE_CLOCKDIV_MAX) div[selected] = HEMISPHERE_CLOCKDIV_MAX;
        if (div[selected] < -HEMISPHERE_CLOCKDIV_MAX) div[selected] = -HEMISPHERE_CLOCKDIV_MAX;
        if (div[selected] == 0) div[selected] = direction > 0 ? 1 : -2; // No such thing as 1/1 Multiple
        if (div[selected] == -1) div[selected] = 1; // Must be moving up to hit -1 (see previous line)
        count[selected] = 0; // Start the count over so things aren't missed
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS] = "";
        help[HEMISPHERE_HELP_OUTS] = "Clk A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "T=Set div P=Set Ch";
    }

private:
    int div[2]; // Division data for outputs. Positive numbers are divisions, negative numbers are multipliers
    int count[2]; // Number of clocks since last output (for clock divide)
    int next_clock[2]; // Tick number for the next output (for clock multiply)
    int last_clock; // The tick number of the last received clock
    int selected; // Which output is currently being edited
    int cycle_time; // Cycle time between the last two clock inputs
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
    ClockDivider_instance[hemisphere].SetHemisphere(hemisphere);
    ClockDivider_instance[hemisphere].Start();
}

void ClockDivider_Controller(int hemisphere, bool forwarding) {
    ClockDivider_instance[hemisphere].IO(forwarding);
    ClockDivider_instance[hemisphere].Controller();
}

void ClockDivider_View(int hemisphere) {
    ClockDivider_instance[hemisphere].BaseView();
}

void ClockDivider_Screensaver(int hemisphere) {
    ClockDivider_instance[hemisphere].ScreensaverView();
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
