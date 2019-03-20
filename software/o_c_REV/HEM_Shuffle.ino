// Copyright (c) 2018, Jason Justian
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
                int16_t d = delay[which] + Proportion(DetentedIn(which), HEMISPHERE_MAX_CV, 100);
                d = constrain(d, 0, 100);
                uint32_t delay_ticks = Proportion(d, 100, tempo);
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
        help[HEMISPHERE_HELP_CVS]      = "1=Odd Mod 2=Even";
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

    // Settings
    int16_t delay[2]; // Percentage delay for even (0) and odd (1) clock

    void DrawSelector() {
        for (int i = 0; i < 2; i++)
        {
            int16_t d = delay[i] + Proportion(DetentedIn(i), HEMISPHERE_MAX_CV, 100);
            d = constrain(d, 0, 100);
            gfxPrint(32 + pad(10, d), 15 + (i * 10), d);
            gfxPrint("%");
            if (cursor == i) gfxCursor(32, 23 + (i * 10), 18);
        }

        // Lines to the first parameter
        int x = Proportion(delay[0], 100, 20) + 8;
        gfxDottedLine(x, 41, x, 19, 3);
        gfxDottedLine(x, 19, 30, 19, 3);

        // Line to the second parameter
        gfxDottedLine(Proportion(delay[1], 100, 20) + 28, 45, 41, 33, 3);
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
            int16_t d = delay[n] + Proportion(DetentedIn(n), HEMISPHERE_MAX_CV, 100);
            d = constrain(d, 0, 100);
            int x = Proportion(d, 100, 20) + (n * 20) + 4;
            gfxBitmap(x, 48 - (which == n ? 3 : 0), 8, which == n ? NOTE_ICON : X_NOTE_ICON);
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

void Shuffle_Start(bool hemisphere) {
    Shuffle_instance[hemisphere].BaseStart(hemisphere);
}

void Shuffle_Controller(bool hemisphere, bool forwarding) {
    Shuffle_instance[hemisphere].BaseController(forwarding);
}

void Shuffle_View(bool hemisphere) {
    Shuffle_instance[hemisphere].BaseView();
}

void Shuffle_OnButtonPress(bool hemisphere) {
    Shuffle_instance[hemisphere].OnButtonPress();
}

void Shuffle_OnEncoderMove(bool hemisphere, int direction) {
    Shuffle_instance[hemisphere].OnEncoderMove(direction);
}

void Shuffle_ToggleHelpScreen(bool hemisphere) {
    Shuffle_instance[hemisphere].HelpScreen();
}

uint32_t Shuffle_OnDataRequest(bool hemisphere) {
    return Shuffle_instance[hemisphere].OnDataRequest();
}

void Shuffle_OnDataReceive(bool hemisphere, uint32_t data) {
    Shuffle_instance[hemisphere].OnDataReceive(data);
}
