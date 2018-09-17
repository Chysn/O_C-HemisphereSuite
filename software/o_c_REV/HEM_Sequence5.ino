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

#include "HSMIDI.h"

class Sequence5 : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Sequence5";
    }

    void Start() {
        for (int s = 0; s < 5; s++) note[s] = random(0, 30);
        play = 1;
    }

    void Controller() {
        // Reset sequencer
        if (Clock(1)) {
            step = 0;
            ClockOut(1);
        }

        int transpose = 0;
        if (DetentedIn(0)) {
            transpose = In(0) / 128; // 128 ADC steps per semitone
        }
        int play_note = note[step] + 60 + transpose;
        play_note = constrain(play_note, 0, 127);

        if (Clock(0)) StartADCLag();

        if (EndOfADCLag()) {
            Advance(step);
            if (step == 0) ClockOut(1);
            play = 1;
        }

        if (play) {
            int cv = MIDIQuantizer::CV(play_note);
            Out(0, cv);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawPanel();
    }

    void OnButtonPress() {
        if (++cursor == 5) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (note[cursor] + direction < 0 && cursor > 0) {
            // If turning past zero, set the mute bit for this step
            muted |= (0x01 << cursor);
        } else {
            note[cursor] = constrain(note[cursor] += direction, 0, 30);
            muted &= ~(0x01 << cursor);
        }
        play = 1; // Replay the changed step in the controller, so it can be heard
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        for (int s = 0; s < 5; s++)
        {
            Pack(data, PackLocation {s * 5,5}, note[s]);
        }
        Pack(data, PackLocation{25,5}, muted);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        for (int s = 0; s < 5; s++)
        {
            note[s] = Unpack(data, PackLocation {s * 5,5});
        }
        muted = Unpack(data, PackLocation {25,5});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=Transpose";
        help[HEMISPHERE_HELP_OUTS]     = "A=CV B=Clk Step 1";
        help[HEMISPHERE_HELP_ENCODER]  = "Note";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor = 0;
    char muted = 0; // Bitfield for muted steps; ((muted >> step) & 1) means muted
    int note[5]; // Sequence value (0 - 30)
    int step = 0; // Current sequencer step
    bool play; // Play the note

    void Advance(int starting_point) {
        if (++step == 5) step = 0;
        // If all the steps have been muted, stay where we were
        if (step_is_muted(step) && step != starting_point) Advance(starting_point);
    }

    void DrawPanel() {
        // Sliders
        for (int s = 0; s < 5; s++)
        {
            int x = 6 + (12 * s);
            if (!step_is_muted(s)) {
                gfxLine(x, 25, x, 63);

                // When cursor, there's a heavier bar and a solid slider
                if (s == cursor) {
                    gfxLine(x + 1, 25, x + 1, 63);
                    gfxRect(x - 4, BottomAlign(note[s]), 9, 3);
                } else gfxFrame(x - 4, BottomAlign(note[s]), 9, 3);

                // When on this step, there's an indicator circle
                if (s == step) {gfxCircle(x, 20, 3);}
            } else if (s == cursor) {
                gfxLine(x, 25, x, 63);
                gfxLine(x + 1, 25, x + 1, 63);
             }
        }
    }

    bool step_is_muted(int step) {
        return (muted & (0x01 << step));
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Sequence5,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Sequence5 Sequence5_instance[2];

void Sequence5_Start(bool hemisphere) {
    Sequence5_instance[hemisphere].BaseStart(hemisphere);
}

void Sequence5_Controller(bool hemisphere, bool forwarding) {
    Sequence5_instance[hemisphere].BaseController(forwarding);
}

void Sequence5_View(bool hemisphere) {
    Sequence5_instance[hemisphere].BaseView();
}

void Sequence5_OnButtonPress(bool hemisphere) {
    Sequence5_instance[hemisphere].OnButtonPress();
}

void Sequence5_OnEncoderMove(bool hemisphere, int direction) {
    Sequence5_instance[hemisphere].OnEncoderMove(direction);
}

void Sequence5_ToggleHelpScreen(bool hemisphere) {
    Sequence5_instance[hemisphere].HelpScreen();
}

uint32_t Sequence5_OnDataRequest(bool hemisphere) {
    return Sequence5_instance[hemisphere].OnDataRequest();
}

void Sequence5_OnDataReceive(bool hemisphere, uint32_t data) {
    Sequence5_instance[hemisphere].OnDataReceive(data);
}
