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
    "Rising", "Falling", "Moving", "Steady", "ChgState", "ChgValue"
};

enum Trend {
    rising, falling, moving, steady, changedstate, changedvalue
};

class Trending : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Trending";
    }

    void Start() {
        ForEachChannel(ch)
        {
            assign[ch] = ch;
            result[ch] = 0;
            fire[ch] = 0;
        }
        sample_countdown = 0;
        sensitivity = 40;
    }

    void Controller() {
        if (--sample_countdown < 0) {
            sample_countdown = (TRENDING_MAX_SENS - sensitivity) * 20;
            if (sample_countdown < 96) sample_countdown = 96;

            ForEachChannel(ch)
            {
                int trend = GetTrend(ch);

                if (reset[ch]) {
                    Out(ch, 0);
                    reset[ch] = 0;
                }

                if (assign[ch] < Trend::changedstate) {
                    bool gate = 0;
                    if (assign[ch] == trend) gate = 1;
                    if (assign[ch] == Trend::moving && trend != Trend::steady) gate = 1;
                    GateOut(ch, gate);
                }
                if (assign[ch] == Trend::changedstate && trend != last_trend[ch]) fire[ch] = 1;
                if (fire[ch]) {
                    ClockOut(ch);
                    fire[ch] = 0;
                }
                last_trend[ch] = trend;
                result[ch] = 0;
            }
        } else {
            ForEachChannel(ch)
            {
                bool changed = Changed(ch);
                signal[ch] = In(ch);
                if (Observe(ch, signal[ch], last_signal[ch])) last_signal[ch] = signal[ch];
                if (assign[ch] == Trend::changedvalue && changed) fire[ch] = 1;
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
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
        else sensitivity = constrain(sensitivity + direction, 4, TRENDING_MAX_SENS);
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
        help[HEMISPHERE_HELP_CVS]      = "1,2=Signal";
        help[HEMISPHERE_HELP_OUTS]     = "A,B=Assignable";
        help[HEMISPHERE_HELP_ENCODER]  = "Assign/Sensitivity";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    int signal[2];
    int last_signal[2];
    int last_trend[2];
    int sample_countdown;
    int result[2];
    bool reset[2]; // When an encoder is changed, reset the outputs
    bool fire[2]; // Fire a trigger on next value check
    
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
        ForEachChannel(ch)
        {
            if (last_trend[ch] == Trend::rising) gfxDottedLine(5, 57 + (ch * 5), 57, 47 + (ch * 5), ch + 2);
            if (last_trend[ch] == Trend::falling) gfxDottedLine(5, 47 + (ch * 5), 57, 57 + (ch * 5), ch + 2);
            if (last_trend[ch] == Trend::steady) gfxDottedLine(5, 52 + (ch * 5), 57, 52 + (ch * 5), ch + 2);
        }
    }

    bool Observe(byte ch, int c_signal, int l_signal) {
        bool update = 0;
        if (abs(c_signal - l_signal) > 10) {
            if (c_signal > l_signal) result[ch] += 1;
            else result[ch] -= 1;
            update = 1;
        }
        return update;
    }

    int GetTrend(byte ch) {
        int trend = Trend::steady;
        if (result[ch] > 3) trend = Trend::rising;
        if (result[ch] < -3) trend = Trend::falling;
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

void Trending_Start(bool hemisphere) {Trending_instance[hemisphere].BaseStart(hemisphere);}
void Trending_Controller(bool hemisphere, bool forwarding) {Trending_instance[hemisphere].BaseController(forwarding);}
void Trending_View(bool hemisphere) {Trending_instance[hemisphere].BaseView();}
void Trending_OnButtonPress(bool hemisphere) {Trending_instance[hemisphere].OnButtonPress();}
void Trending_OnEncoderMove(bool hemisphere, int direction) {Trending_instance[hemisphere].OnEncoderMove(direction);}
void Trending_ToggleHelpScreen(bool hemisphere) {Trending_instance[hemisphere].HelpScreen();}
uint32_t Trending_OnDataRequest(bool hemisphere) {return Trending_instance[hemisphere].OnDataRequest();}
void Trending_OnDataReceive(bool hemisphere, uint32_t data) {Trending_instance[hemisphere].OnDataReceive(data);}
