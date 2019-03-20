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
        cycle_time = 0;
        cursor = 0;
    }

    void Controller() {
        int this_tick = OC::CORE::ticks;

        // Set division via CV
        ForEachChannel(ch)
        {
            int input = DetentedIn(ch) - HEMISPHERE_CENTER_CV;
            if (input) {
                div[ch] = Proportion(input, HEMISPHERE_MAX_CV / 2, HEM_CLOCKDIV_MAX);
                div[ch] = constrain(div[ch], -HEM_CLOCKDIV_MAX, HEM_CLOCKDIV_MAX);
                if (div[ch] == 0 || div[ch] == -1) div[ch] = 1;
            }
        }

        if (Clock(1)) { // Reset
            ForEachChannel(ch) count[ch] = 0;
        }

        // The input was clocked; set timing info
        if (Clock(0)) {
        		cycle_time = ClockCycleTicks(0);
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

    void OnButtonPress() {
        cursor = 1 - cursor;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        div[cursor] += direction;
        if (div[cursor] > HEM_CLOCKDIV_MAX) div[cursor] = HEM_CLOCKDIV_MAX;
        if (div[cursor] < -HEM_CLOCKDIV_MAX) div[cursor] = -HEM_CLOCKDIV_MAX;
        if (div[cursor] == 0) div[cursor] = direction > 0 ? 1 : -2; // No such thing as 1/1 Multiple
        if (div[cursor] == -1) div[cursor] = 1; // Must be moving up to hit -1 (see previous line)
        count[cursor] = 0; // Start the count over so things aren't missed
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
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS] = "Div/Mult Ch1,Ch2";
        help[HEMISPHERE_HELP_OUTS] = "Clk A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "Div,Mult";
    }

private:
    int div[2]; // Division data for outputs. Positive numbers are divisions, negative numbers are multipliers
    int count[2]; // Number of clocks since last output (for clock divide)
    int next_clock[2]; // Tick number for the next output (for clock multiply)
    int cursor; // Which output is currently being edited
    int cycle_time; // Cycle time between the last two clock inputs

    void DrawSelector() {
        ForEachChannel(ch)
        {
            int y = 15 + (ch * 25);
            if (ch == cursor) gfxCursor(0, y + 8, 63);

            if (div[ch] > 0) {
                gfxPrint(1, y, "/");
                gfxPrint(div[ch]);
                gfxPrint(" Div");
            }
            if (div[ch] < 0) {
                gfxPrint(1, y, "x");
                gfxPrint(-div[ch]);
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

void ClockDivider_Start(bool hemisphere) {
    ClockDivider_instance[hemisphere].BaseStart(hemisphere);
}

void ClockDivider_Controller(bool hemisphere, bool forwarding) {
    ClockDivider_instance[hemisphere].BaseController(forwarding);
}

void ClockDivider_View(bool hemisphere) {
    ClockDivider_instance[hemisphere].BaseView();
}

void ClockDivider_OnButtonPress(bool hemisphere) {
    ClockDivider_instance[hemisphere].OnButtonPress();
}

void ClockDivider_OnEncoderMove(bool hemisphere, int direction) {
    ClockDivider_instance[hemisphere].OnEncoderMove(direction);
}

void ClockDivider_ToggleHelpScreen(bool hemisphere) {
    ClockDivider_instance[hemisphere].HelpScreen();
}

uint32_t ClockDivider_OnDataRequest(bool hemisphere) {
    return ClockDivider_instance[hemisphere].OnDataRequest();
}

void ClockDivider_OnDataReceive(bool hemisphere, uint32_t data) {
    ClockDivider_instance[hemisphere].OnDataReceive(data);
}
