class Shuffle : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Shuffle";
    }

    void Start() {
        delay[0] = 0;
        delay[1] = 0;
        which = 0;
        cursor = 1;
        last_tick = 0;
    }

    void Controller() {
        uint32_t tick = OC::CORE::ticks;
        if (Clock(1)) {
            which = 0; // Reset (next trigger will be even clock)
            last_tick = tick;
        }

        if (Clock(0)) {
            which = 1 - which;
            if (last_tick) {
                tempo = tick - last_tick;
                uint32_t delay_ticks = Proportion(delay[which], 100, tempo);
                next_trigger = tick + delay_ticks;
            }
            last_tick = tick;
        }

        if (tick == next_trigger) ClockOut(0);
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
        delay[cursor] += direction;
        delay[cursor] = constrain(delay[cursor], 0, 99);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,7}, delay[0]);
        Pack(data, PackLocation {7,7}, delay[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        delay[0] = Unpack(data, PackLocation {0,7});
        delay[1] = Unpack(data, PackLocation {7,7});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=Clock";
        help[HEMISPHERE_HELP_ENCODER]  = "Odd/Even Delay";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    bool which; // The current clock state, 0=even, 1=odd
    uint32_t last_tick; // For calculating tempo
    uint32_t next_trigger; // The tick of the next scheduled trigger
    uint32_t tempo; // Calculated time between ticks

    // Icons
    const uint8_t x_note[8] = {0x00, 0xa0, 0x40, 0xa0, 0x1f, 0x02, 0x0c, 0x00};
    const uint8_t note[8] = {0xc0, 0xe0, 0xe0, 0xe0, 0x7f, 0x02, 0x14, 0x08};
    
    // Settings
    int16_t delay[2]; // Percentage delay for even (0) and odd (1) clock

    void DrawSelector() {
        for (int i = 0; i < 2; i++)
        {
            int16_t d = delay[i];
            gfxBitmap(i * 12, 15 + (i * 10), 8, x_note);
            gfxPrint(32 + (d < 10 ? 6 : 0), 15 + (i * 10), d);
            gfxPrint("%");
            if (cursor == i) gfxCursor(32, 23 + (i * 10), 18);
        }
    }

    void DrawIndicator() {
        // Draw some cool-looking barlines
        gfxLine(1, 40, 1, 62);
        gfxLine(57, 40, 57, 62);
        gfxLine(60, 40, 60, 62);
        gfxLine(61, 40, 61, 62);
        gfxCircle(53, 47, 1);
        gfxCircle(53, 55, 1);

        for (int n = 0; n < 2; n++)
        {
            int x = Proportion(delay[n], 100, 20) + (n * 20) + 4;
            gfxBitmap(x, 48 - (which == n ? 3 : 0), 8, which == n ? note : x_note);
        }

        int lx = Proportion(OC::CORE::ticks - last_tick, tempo, 20) + (which * 20) + 4;
        lx = constrain(lx, 1, 54);
        gfxDottedLine(lx, 42, lx, 60, 2);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Shuffle,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Shuffle Shuffle_instance[2];

void Shuffle_Start(int hemisphere) {
    Shuffle_instance[hemisphere].BaseStart(hemisphere);
}

void Shuffle_Controller(int hemisphere, bool forwarding) {
    Shuffle_instance[hemisphere].BaseController(forwarding);
}

void Shuffle_View(int hemisphere) {
    Shuffle_instance[hemisphere].BaseView();
}

void Shuffle_Screensaver(int hemisphere) {
    Shuffle_instance[hemisphere].BaseScreensaverView();
}

void Shuffle_OnButtonPress(int hemisphere) {
    Shuffle_instance[hemisphere].OnButtonPress();
}

void Shuffle_OnEncoderMove(int hemisphere, int direction) {
    Shuffle_instance[hemisphere].OnEncoderMove(direction);
}

void Shuffle_ToggleHelpScreen(int hemisphere) {
    Shuffle_instance[hemisphere].HelpScreen();
}

uint32_t Shuffle_OnDataRequest(int hemisphere) {
    return Shuffle_instance[hemisphere].OnDataRequest();
}

void Shuffle_OnDataReceive(int hemisphere, uint32_t data) {
    Shuffle_instance[hemisphere].OnDataReceive(data);
}
