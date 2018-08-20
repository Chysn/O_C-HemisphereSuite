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

#define TRENDING_MAX_SENS 124

const char* const Trending_assignments[6] = {
    "Rising", "Falling", "Moving", "Steady", "Changed", "PassThru"
};

enum Trend {
    rising, falling, moving, steady, changed, passthru
};

class Trending : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Trending";
    }

    void Start() {
        assign[0] = 0;
        assign[1] = 1;
        sample_countdown = 0;
        result = 0;
        sensitivity = 40;
    }

    void Controller() {
        if (--sample_countdown < 0) {
            sample_countdown = (TRENDING_MAX_SENS - sensitivity) * 20;
            if (sample_countdown < 96) sample_countdown = 96;
            int trend = GetTrend();

            ForEachChannel(ch)
            {
                if (reset[ch]) {
                    Out(ch, 0);
                    reset[ch] = 0;
                }

                if (assign[ch] < changed) {
                    bool gate = 0;
                    if (assign[ch] == trend) gate = 1;
                    if (assign[ch] == Trend::moving && trend != Trend::steady) gate = 1;
                    GateOut(ch, gate);
                }
                if (assign[ch] == Trend::changed && trend != last_trend) ClockOut(ch);
                if (assign[ch] == Trend::passthru) Out(ch, signal);
            }

            last_trend = trend;
            result = 0;
        } else {
            signal = In(0);
            if (Observe(signal, last_signal)) last_signal = signal;
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
        if (++cursor > 2) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor < 2) {
            assign[cursor] = constrain(assign[cursor] + direction, 0, 5);
            reset[cursor] = 1;
        }
        else sensitivity = constrain(sensitivity + direction, 5, TRENDING_MAX_SENS);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,4}, assign[0]);
        Pack(data, PackLocation {4,4}, assign[1]);
        Pack(data, PackLocation {8,8}, sensitivity);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        assign[0] = Unpack(data, PackLocation {0,4});
        assign[1] = Unpack(data, PackLocation {4,4});
        sensitivity = Unpack(data, PackLocation {8,8});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "1=Signal";
        help[HEMISPHERE_HELP_OUTS]     = "A,B=Assignable";
        help[HEMISPHERE_HELP_ENCODER]  = "Assign/Sensitivity";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    int signal;
    int last_signal;
    int last_trend;
    int sample_countdown;
    int result;
    bool reset[2]; // When an encoder is changed, reset the outputs
    
    // Settings
    uint8_t assign[2];
    int sensitivity;

    void DrawSelector() {
        gfxPrint(1, 15, hemisphere ? "C:" : "A:");
        gfxPrint(Trending_assignments[assign[0]]);
        gfxPrint(1, 25, hemisphere ? "D:" : "B:");
        gfxPrint(Trending_assignments[assign[1]]);
        gfxFrame(1, 35, 62, 6);
        int px = Proportion(sensitivity, TRENDING_MAX_SENS, 62);
        if (cursor == 2) gfxRect(1, 35, px, 6);
        else {
            gfxLine(px, 35, px, 40);
            gfxCursor(12, 23 + (cursor * 10), 50);
        }
    }

    void DrawIndicator() {
        if (last_trend == Trend::rising) gfxLine(5, 62, 57, 52);
        if (last_trend == Trend::falling) gfxLine(5, 52, 57, 62);
        if (last_trend == Trend::steady) gfxLine(5, 57, 57, 57);
    }

    bool Observe(int c_signal, int l_signal) {
        bool update = 0;
        if (abs(c_signal - l_signal) > 10) {
            if (c_signal > l_signal) result += 1;
            else result -= 1;
            update = 1;
        }
        return update;
    }

    int GetTrend() {
        int trend = Trend::steady;
        if (result > 3) trend = Trend::rising;
        if (result < -3) trend = Trend::falling;
        return trend;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Trending,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Trending Trending_instance[2];

void Trending_Start(bool hemisphere) {
    Trending_instance[hemisphere].BaseStart(hemisphere);
}

void Trending_Controller(bool hemisphere, bool forwarding) {
    Trending_instance[hemisphere].BaseController(forwarding);
}

void Trending_View(bool hemisphere) {
    Trending_instance[hemisphere].BaseView();
}

void Trending_Screensaver(bool hemisphere) {
    Trending_instance[hemisphere].BaseScreensaverView();
}

void Trending_OnButtonPress(bool hemisphere) {
    Trending_instance[hemisphere].OnButtonPress();
}

void Trending_OnEncoderMove(bool hemisphere, int direction) {
    Trending_instance[hemisphere].OnEncoderMove(direction);
}

void Trending_ToggleHelpScreen(bool hemisphere) {
    Trending_instance[hemisphere].HelpScreen();
}

uint32_t Trending_OnDataRequest(bool hemisphere) {
    return Trending_instance[hemisphere].OnDataRequest();
}

void Trending_OnDataReceive(bool hemisphere, uint32_t data) {
    Trending_instance[hemisphere].OnDataReceive(data);
}
