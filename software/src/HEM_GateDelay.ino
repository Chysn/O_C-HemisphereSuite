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

class GateDelay : public HemisphereApplet {
public:

    const char* applet_name() {
        return "GateDelay";
    }

    void Start() {
        ForEachChannel(ch)
        {
            for (int i = 0; i < 64; i++) tape[ch][i] = 0x0000;
            time[ch] = 1000;
            location[ch] = 0;
            last_gate[ch] = 0;
        }
        cursor = 0;
        ms_countdown = 0;
    }

    void Controller() {
        if (--ms_countdown < 0) {
            ForEachChannel(ch)
            {
                record(ch, Gate(ch));
                int mod_time = Proportion(DetentedIn(ch), HEMISPHERE_MAX_CV, 1000) + time[ch];
                mod_time = constrain(mod_time, 0, 2000);

                bool p = play(ch, mod_time);
                if (p) last_gate[ch] = OC::CORE::ticks;
                GateOut(ch, p);

                if (++location[ch] > 2047) location[ch] = 0;
            }
            ms_countdown = 16;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        cursor = 1 - cursor;
    }

    void OnEncoderMove(int direction) {
        if (time[cursor] > 100) direction *= 2;
        if (time[cursor] > 500) direction *= 2;
        if (time[cursor] > 1000) direction *= 2;
        time[cursor] = constrain(time[cursor] += direction, 0, 2000);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,11}, time[0]);
        Pack(data, PackLocation {11,11}, time[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        time[0] = Unpack(data, PackLocation {0,11});
        time[1] = Unpack(data, PackLocation {11,11});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Gate Ch1,2";
        help[HEMISPHERE_HELP_CVS]      = "Time Mod Ch1,Ch2";
        help[HEMISPHERE_HELP_OUTS]     = "Delay Ch1,2";
        help[HEMISPHERE_HELP_ENCODER]  = "Set time";
        //                               "------------------" <-- Size Guide
    }
    
private:
    uint32_t tape[2][64]; // 63 x 32 = 2016 bits per channel, or 2s at 1ms resolution
    int time[2]; // Length of each channel (in ms)
    uint16_t location[2]; // Location of record head (playback head = location + time)
    uint32_t last_gate[2]; // Time of last gate, for display of icon
    uint8_t cursor;
    int16_t ms_countdown; // Countdown for 1 ms

    void DrawInterface() {
        ForEachChannel(ch)
        {
            int y = 15 + (ch * 25);
            if (ch == cursor) gfxCursor(0, y + 8, 63);

            gfxPrint(1, y, time[ch]);
            gfxPrint("ms");

            if (OC::CORE::ticks - last_gate[ch] < 1667) gfxBitmap(54, y, 8, CLOCK_ICON);
        }
    }

    /* Write the gate state into the tape at the tape head */
    void record(int ch, bool gate) {
        uint16_t word = location[ch] / 32;
        uint8_t bit = location[ch] % 32;
        if (gate) {
            // Set the current bit
            tape[ch][word] |= (0x01 << bit);
        } else {
            // Clear the current bit
            tape[ch][word] &= (0xffff ^ (0x01 << bit));
        }
    }

    /* Get the status of the tape at the current play head location */
    bool play(int ch, int mod_time) {
        int play_location = location[ch] - mod_time;
        if (play_location < 0) play_location += 2048;
        uint16_t word = play_location / 32;
        uint8_t bit = play_location % 32;
        return ((tape[ch][word] >> bit) & 0x01);
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to GateDelay,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
GateDelay GateDelay_instance[2];

void GateDelay_Start(bool hemisphere) {
    GateDelay_instance[hemisphere].BaseStart(hemisphere);
}

void GateDelay_Controller(bool hemisphere, bool forwarding) {
    GateDelay_instance[hemisphere].BaseController(forwarding);
}

void GateDelay_View(bool hemisphere) {
    GateDelay_instance[hemisphere].BaseView();
}

void GateDelay_OnButtonPress(bool hemisphere) {
    GateDelay_instance[hemisphere].OnButtonPress();
}

void GateDelay_OnEncoderMove(bool hemisphere, int direction) {
    GateDelay_instance[hemisphere].OnEncoderMove(direction);
}

void GateDelay_ToggleHelpScreen(bool hemisphere) {
    GateDelay_instance[hemisphere].HelpScreen();
}

uint32_t GateDelay_OnDataRequest(bool hemisphere) {
    return GateDelay_instance[hemisphere].OnDataRequest();
}

void GateDelay_OnDataReceive(bool hemisphere, uint32_t data) {
    GateDelay_instance[hemisphere].OnDataReceive(data);
}
