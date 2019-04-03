// Copyright (c) 2018, Jason Justian
//
// Based on Braids Quantizer, Copyright 2015 Émilie Gillet.
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

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

class Squanch : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Squanch";
    }

    void Start() {
        quantizer.Init();
        scale = 5;
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
    }

    void Controller() {
        if (Clock(0)) {
            continuous = 0; // Turn off continuous mode if there's a clock
            StartADCLag(0);
        }

        if (continuous || EndOfADCLag(0)) {
            int32_t pitch = In(0);

            ForEachChannel(ch)
            {
                // For the B/D output, CV 2 is used to shift the output; for the A/C
                // output, the output is raised by one octave when Digital 2 is gated.
                int32_t shift_alt = (ch == 1) ? DetentedIn(1) : Gate(1) * (12 << 7);

                int32_t quantized = quantizer.Process(pitch, 0, shift[ch]);
                Out(ch, quantized + shift_alt);
                last_note[ch] = quantized;
            }
        }

    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        if (++cursor > 2) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 2) { // Scale selection
            scale += direction;
            if (scale >= OC::Scales::NUM_SCALES) scale = 0;
            if (scale < 0) scale = OC::Scales::NUM_SCALES - 1;
            quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
            continuous = 1; // Re-enable continuous mode when scale is changed
        } else {
            shift[cursor] = constrain(shift[cursor] + direction, -48, 48);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, scale);
        Pack(data, PackLocation {8,8}, shift[0] + 48);
        Pack(data, PackLocation {16,8}, shift[1] + 48);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        scale = Unpack(data, PackLocation {0,8});
        shift[0] = Unpack(data, PackLocation {8,8}) - 48;
        shift[1] = Unpack(data, PackLocation {16,8}) - 48;
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=+1 Oct A";
        help[HEMISPHERE_HELP_CVS]      = "1=Signal 2=Shift B";
        help[HEMISPHERE_HELP_OUTS]     = "A,B=Quantized";
        help[HEMISPHERE_HELP_ENCODER]  = "Shift/Scale";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=A shift, 1=B shift, 2=Scale
    bool continuous = 1;
    int last_note[2]; // Last quantized note
    braids::Quantizer quantizer;

    // Settings
    int scale;
    int16_t shift[2];

    void DrawInterface() {
        const uint8_t notes[2][8] = {{0xc0, 0xe0, 0xe0, 0xe0, 0x7f, 0x02, 0x14, 0x08},
                                     {0xc0, 0xa0, 0xa0, 0xa0, 0x7f, 0x00, 0x00, 0x00}};

        // Display icon if clocked
        if (!continuous) gfxIcon(56, 25, CLOCK_ICON);

        // Shift for A/C
        gfxIcon(1, 14, notes[0]);
        gfxPrint(11, 15, shift[0] > -1 ? "+" : "");
        gfxPrint(shift[0]);

        // Shift for B/D
        gfxIcon(32, 14, notes[1]);
        gfxPrint(43 + pad(10, shift[1]), 15, shift[1] > -1 ? "+" : "");
        gfxPrint(shift[1]);

        // Scale
        gfxBitmap(1, 24, 8, SCALE_ICON);
        gfxPrint(12, 25, OC::scale_names_short[scale]);

        // Cursors
        if (cursor == 0) gfxCursor(10, 23, 18);
        if (cursor == 1) gfxCursor(42, 23, 18);
        if (cursor == 2) gfxCursor(13, 33, 30); // Scale Cursor

        // Little note display

        // This is for relative visual purposes only, so I don't really care if this isn't a semitone
        // scale, or even if it has 12 notes in it:
        ForEachChannel(ch)
        {
            int semitone = (last_note[ch] / 128) % 12;
            int note_x = semitone * 4; // 4 pixels per semitone
            if (note_x < 0) note_x = 0;
            gfxIcon(10 + note_x, 41 + (10 * ch), notes[ch]);
        }

    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Squanch,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Squanch Squanch_instance[2];

void Squanch_Start(bool hemisphere) {Squanch_instance[hemisphere].BaseStart(hemisphere);}
void Squanch_Controller(bool hemisphere, bool forwarding) {Squanch_instance[hemisphere].BaseController(forwarding);}
void Squanch_View(bool hemisphere) {Squanch_instance[hemisphere].BaseView();}
void Squanch_OnButtonPress(bool hemisphere) {Squanch_instance[hemisphere].OnButtonPress();}
void Squanch_OnEncoderMove(bool hemisphere, int direction) {Squanch_instance[hemisphere].OnEncoderMove(direction);}
void Squanch_ToggleHelpScreen(bool hemisphere) {Squanch_instance[hemisphere].HelpScreen();}
uint32_t Squanch_OnDataRequest(bool hemisphere) {return Squanch_instance[hemisphere].OnDataRequest();}
void Squanch_OnDataReceive(bool hemisphere, uint32_t data) {Squanch_instance[hemisphere].OnDataReceive(data);}
