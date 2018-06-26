// Skewed LFO:
//   Encoder: LFO rate or LFO skew
//   Button: Select rate or skew
//   Out 1: LFO
//   Out 2: Clock
//   Digital 1: Reset

#define HEMISPHERE_LFO_HIGH 40000
#define HEMISPHERE_LFO_LOW 800
#define LFO_MAX_CONTROL 62

class SkewedLFO : public HemisphereApplet {
public:

    const char* applet_name() {
        return "SkewedLFO";
    }

    void Start() {
        rate = 31;
        skew = 31;
        selected = 0;
        cycle_tick = 0;
    }

    void Controller() {
        cycle_tick++;

        // Handle reset trigger
        if (Clock(0)) cycle_tick = 0;

        // Handle LFO output
        if (cycle_tick >= TicksAtRate()) {
            cycle_tick = 0;
            ClockOut(1);
        }
        int cv = AmplitudeAtPosition(cycle_tick, HEMISPHERE_MAX_CV);
        Out(0, cv);
    }

    void View() {
        gfxHeader(applet_name());

        DrawCursor();
        DrawSkewedWaveform(0);
        DrawRateIndicator();
        DrawWaveformPosition();
    }

    void ScreensaverView() {
        DrawWaveformPosition();
    }

    void OnButtonPress() {
        selected = 1 - selected;
    }

    void OnEncoderMove(int direction) {
        if (selected == 0) {
            rate = constrain(rate += direction, 0, LFO_MAX_CONTROL);
        } else {
            skew = constrain(skew += direction, 0, LFO_MAX_CONTROL);
        }
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Reset";
        help[HEMISPHERE_HELP_CVS] = "";
        help[HEMISPHERE_HELP_OUTS] = "A=CV B=Clock";
        help[HEMISPHERE_HELP_ENCODER] = "T=Rate,Skew P=Set";
    }

private:
    int rate; // LFO rate between 0 and LFO_MAX_CONTROL, where 0 is slowest
    int skew; // LFO skew, where 0 is saw, LFO_MAX_CONTROL is ramp, and 31 is triangle
    int selected; // Whether knob is editing rate (0) or skew (1)
    int cycle_tick; // The current tick number within the cycle

    void DrawCursor() {
        if (selected == 0) {
            gfxRect(1, 16, rate, 4);
        } else {
            DrawSkewedWaveform(-1);
        }
    }

    void DrawRateIndicator() {
        gfxFrame(1, 15, 62, 6);
        gfxLine(rate, 15, rate, 20);
    }

    void DrawSkewedWaveform(int y_offset) {
        gfxLine(0, 62 + y_offset, skew, 33 + y_offset);
        gfxLine(skew, 33 + y_offset, 62, 62 + y_offset);
    }

    void DrawWaveformPosition() {
        int height = AmplitudeAtPosition(cycle_tick, 30);
        int x = Proportion(cycle_tick, TicksAtRate(), 62);
        gfxLine(x, 63, x, 63 - height);
    }

    int AmplitudeAtPosition(int position, int max_amplitude) {
        int amplitude = 0;
        int ticks_at_rate = TicksAtRate();
        int fall_point = Proportion(skew, LFO_MAX_CONTROL, ticks_at_rate);
        if (position < fall_point) {
            // Rise portion
            amplitude = Proportion(position, fall_point, max_amplitude);
        } else {
            // Fall portion
            amplitude = Proportion(ticks_at_rate - position, ticks_at_rate - fall_point, max_amplitude);
        }

        return amplitude;
    }

    int TicksAtRate() {
        int inv_rate = LFO_MAX_CONTROL - rate;
        int range = HEMISPHERE_LFO_HIGH - HEMISPHERE_LFO_LOW;
        int ticks_at_rate = Proportion(inv_rate, LFO_MAX_CONTROL, range) + HEMISPHERE_LFO_LOW;
        return ticks_at_rate;
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to SkewedLFO,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
SkewedLFO SkewedLFO_instance[2];

void SkewedLFO_Start(int hemisphere) {
    SkewedLFO_instance[hemisphere].SetHemisphere(hemisphere);
    SkewedLFO_instance[hemisphere].BaseStart();
}

void SkewedLFO_Controller(int hemisphere, bool forwarding) {
    SkewedLFO_instance[hemisphere].BaseController(forwarding);
}

void SkewedLFO_View(int hemisphere) {
    SkewedLFO_instance[hemisphere].BaseView();
}

void SkewedLFO_Screensaver(int hemisphere) {
    SkewedLFO_instance[hemisphere].ScreensaverView();
}

void SkewedLFO_OnButtonPress(int hemisphere) {
    SkewedLFO_instance[hemisphere].OnButtonPress();
}

void SkewedLFO_OnEncoderMove(int hemisphere, int direction) {
    SkewedLFO_instance[hemisphere].OnEncoderMove(direction);
}

void SkewedLFO_ToggleHelpScreen(int hemisphere) {
    SkewedLFO_instance[hemisphere].HelpScreen();
}
