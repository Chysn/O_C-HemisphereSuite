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

class TrigSeq : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "TrigSeq";
    }

    void Start() {
        ForEachChannel(ch)
        {
            pattern[ch] = random(1, 255);
            end_step[ch] = 7;
            step[ch] = 0;
        }
        cursor = 0;
    }

    void Controller() {
        if (Clock(0) || Clock(1)) {
            bool swap = In(0) >= HEMISPHERE_3V_CV;
            ForEachChannel(ch)
            {
                if (Clock(1) || step[ch] >= end_step[ch]) step[ch] = -1;
                step[ch]++;
                if ((pattern[ch] >> step[ch]) & 0x01) ClockOut(swap ? (1 - ch) : ch);
            }

        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawDisplay();
    }

    void OnButtonPress() {
        cursor++;
        if (cursor > 5) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        int ch = cursor > 2 ? 1 : 0;
        int this_cursor = cursor - (3 * ch);

        // Update end_step
        if (this_cursor == 2) {
            end_step[ch] = constrain(end_step[ch] += direction, 0, 7);
        } else {
            // Get the current pattern
            int curr_patt = pattern[ch];

            // Shift right based on which nybble is selcted
            uint8_t nybble = (curr_patt >> (this_cursor * 4)) & 0x0f;
            nybble += direction;
            nybble &= 0x0f;

            // Put the updated nybble back where it belongs
            if (this_cursor == 0) pattern[ch] = (curr_patt & 0xf0) + nybble;
            else pattern[ch] = (curr_patt & 0x0f) + (nybble << 4);
        }
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, pattern[0]);
        Pack(data, PackLocation {8,8}, pattern[1]);
        Pack(data, PackLocation {16,3}, end_step[0]);
        Pack(data, PackLocation {19,3}, end_step[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        pattern[0] = Unpack(data, PackLocation {0,8});
        pattern[1] = Unpack(data, PackLocation {8,8});
        end_step[0] = Unpack(data, PackLocation {16,3});
        end_step[1] = Unpack(data, PackLocation {19,3});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=Swap A&B @ 3V";
        help[HEMISPHERE_HELP_OUTS]     = "Trg A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Set P=Select";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int step[2]; // Current step of each channel
    uint8_t pattern[2];
    int end_step[2];
    int cursor; // 0=ch1 low, 1=ch1 hi, 2=c1 end_step,  3=ch2 low, 4=ch3 hi, 5=ch2 end_step
    
    void DrawDisplay() {
        ForEachChannel(ch)
        {
            int this_cursor = cursor - (3 * ch);
            int x = 10 + (31 * ch);

            // Draw the steps for this channel
            bool stop = 0; // Stop displaying when end_step is reached
            for (int s = 0; s < 8; s++)
            {
                if (!stop) {
                    int y = 18 + (6 * s);
                    gfxCircle(x, y, 3);
                    int value = (pattern[ch] >> s) & 0x01;
                    if (value) {
                        for (int r = 1; r < 3; r++) {
                            gfxCircle(x, y, r);
                            gfxCircle(x + 1, y, r);
                            gfxCircle(x + 2, y, r);
                        }
                    }

                    if (s == step[ch]) {
                        gfxLine(x + 4, y, x + 10, y);
                    }

                    // Draw the end_step cursor
                    if (s == end_step[ch]) {
                        if (this_cursor == 2 && CursorBlink()) {
                            gfxLine(x - 8, y + 3, x + 5, y + 3);
                        }
                        stop = 1;
                    }
                }
            }

            // Draw the nybble cursor
            if ((this_cursor == 0 || this_cursor == 1) && CursorBlink()) {
                int y = 15 + (this_cursor * 24);
                gfxLine(x - 5, y, x - 5, y + 24);
            }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to TrigSeq,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
TrigSeq TrigSeq_instance[2];

void TrigSeq_Start(bool hemisphere) {
    TrigSeq_instance[hemisphere].BaseStart(hemisphere);
}

void TrigSeq_Controller(bool hemisphere, bool forwarding) {
    TrigSeq_instance[hemisphere].BaseController(forwarding);
}

void TrigSeq_View(bool hemisphere) {
    TrigSeq_instance[hemisphere].BaseView();
}

void TrigSeq_OnButtonPress(bool hemisphere) {
    TrigSeq_instance[hemisphere].OnButtonPress();
}

void TrigSeq_OnEncoderMove(bool hemisphere, int direction) {
    TrigSeq_instance[hemisphere].OnEncoderMove(direction);
}

void TrigSeq_ToggleHelpScreen(bool hemisphere) {
    TrigSeq_instance[hemisphere].HelpScreen();
}

uint32_t TrigSeq_OnDataRequest(bool hemisphere) {
    return TrigSeq_instance[hemisphere].OnDataRequest();
}

void TrigSeq_OnDataReceive(bool hemisphere, uint32_t data) {
    TrigSeq_instance[hemisphere].OnDataReceive(data);
}
