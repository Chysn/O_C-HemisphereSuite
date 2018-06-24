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
        int cv = HEM_SIMFLOAT2INT(AmplitudeAtPosition(cycle_tick) * HEMISPHERE_MAX_CV);
        Out(0, cv);
    }

    void View() {
        gfxHeader("SkewedLFO"); // 9-character maximum

        DrawCursor();
        DrawSkewedWaveform();
        DrawRateIndicator();
        DrawWaveformPosition();
    }

    void ScreensaverView() {
        DrawWaveformPosition();
    }

    void OnButtonPress() {
        selected = 1 - selected;
    }

    void OnButtonLongPress() {
    }

    void OnEncoderMove(int direction) {
        if (selected == 0) {
            rate = constrain(rate += direction, 0, LFO_MAX_CONTROL);
        } else {
            skew = constrain(skew += direction, 0, LFO_MAX_CONTROL);
        }
    }

private:
    int rate; // LFO rate between 0 and LFO_MAX_CONTROL, where 0 is slowest
    int skew; // LFO skew, where 0 is saw, LFO_MAX_CONTROL is ramp, and 31 is triangle
    int selected; // Whether knob is editing rate (0) or skew (1)
    int cycle_tick; // The current tick number within the cycle

    void DrawCursor() {
        if (selected == 0) {
            gfxFrame(0, 12, 63, 17);
        } else {
            gfxFrame(0, 31, 63, 33);
        }
    }

    void DrawRateIndicator() {
        int x = (64 - rate) / 2; // Center it
        gfxRect(x, 20, rate, 3);
    }

    void DrawSkewedWaveform() {
        gfxLine(0, 62, skew, 33);
        gfxLine(skew, 33, 62, 62);
    }

    void DrawWaveformPosition() {
        int height = HEM_SIMFLOAT2INT(AmplitudeAtPosition(cycle_tick) * 30);
        hem_simfloat coeff = HEM_SIMFLOAT(cycle_tick) / TicksAtRate();
        int x = HEM_SIMFLOAT2INT(coeff * 62);
        gfxLine(x, 63, x, 63 - height);
    }

    hem_simfloat AmplitudeAtPosition(int tick) {
        hem_simfloat amplitude;

        int ticks_at_rate = TicksAtRate();
        hem_simfloat coeff = HEM_SIMFLOAT(skew) / LFO_MAX_CONTROL;
        int fall_point = HEM_SIMFLOAT2INT(coeff * ticks_at_rate);
        if (tick < fall_point) {
            // Rise portion
            amplitude = HEM_SIMFLOAT(tick) / fall_point; // No div by 0 (fall point > 0 to get here)
        } else {
            // Fall portion
            amplitude = HEM_SIMFLOAT(ticks_at_rate - tick) / (ticks_at_rate - fall_point);
        }

        return amplitude;
    }

    int TicksAtRate() {
        int inv_rate = LFO_MAX_CONTROL - rate;
        int range = HEMISPHERE_LFO_HIGH - HEMISPHERE_LFO_LOW;
        hem_simfloat coeff = HEM_SIMFLOAT(inv_rate) / LFO_MAX_CONTROL;
        int ticks_at_rate = HEM_SIMFLOAT2INT(coeff * range) + HEMISPHERE_LFO_LOW;
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
    SkewedLFO_instance[hemisphere].Start();
}

void SkewedLFO_Controller(int hemisphere, bool forwarding) {
    SkewedLFO_instance[hemisphere].IO(forwarding);
    SkewedLFO_instance[hemisphere].Controller();
}

void SkewedLFO_View(int hemisphere) {
    SkewedLFO_instance[hemisphere].View();
    SkewedLFO_instance[hemisphere].DrawNotifications();
}

void SkewedLFO_Screensaver(int hemisphere) {
    SkewedLFO_instance[hemisphere].ScreensaverView();
}

void SkewedLFO_OnButtonPress(int hemisphere) {
    SkewedLFO_instance[hemisphere].OnButtonPress();
}

void SkewedLFO_OnButtonLongPress(int hemisphere) {
    SkewedLFO_instance[hemisphere].OnButtonLongPress();
}

void SkewedLFO_OnEncoderMove(int hemisphere, int direction) {
    SkewedLFO_instance[hemisphere].OnEncoderMove(direction);
}
