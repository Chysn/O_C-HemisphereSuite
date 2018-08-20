// Copyright (c) 2018, Jason Justian
//
// Based on Braids Quantizer, Copyright 2015 Olivier Gillet.
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

class DualQuant : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "DualQuant";
    }

    void Start() {
        cursor = 0;
        ForEachChannel(ch)
        {
            quantizer[ch].Init();
            scale[ch] = ch + 5;
            quantizer[ch].Configure(OC::Scales::GetScale(scale[ch]), 0xffff);
            last_note[ch] = 0;
            continuous[ch] = 1;
        }
        adc_lag_countdown = 0;
    }

    void Controller() {
        ForEachChannel(ch)
        {
            if (Clock(ch)) {
                continuous[ch] = 0; // Turn off continuous mode if there's a clock
                StartADCLag(ch);
            }

            if (continuous[ch] || EndOfADCLag(ch)) {
                int32_t pitch = In(ch);
                int32_t quantized = quantizer[ch].Process(pitch, 0, 0);
                Out(ch, quantized);
                if (last_note[ch] != quantized) {
                    last_note[ch] = quantized;
                }
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
    }

    void ScreensaverView() {
        DrawSelector();
    }

    void OnButtonPress() {
        cursor = 1 - cursor;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        scale[cursor] += direction;
        if (scale[cursor] == OC::Scales::NUM_SCALES) scale[cursor] = 0;
        if (scale[cursor] < 0) scale[cursor] = OC::Scales::NUM_SCALES - 1;
        quantizer[cursor].Configure(OC::Scales::GetScale(scale[cursor]), 0xffff);
        continuous[cursor] = 1; // Re-enable continuous mode when scale is changed
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, scale[0]);
        Pack(data, PackLocation {16,16}, scale[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        scale[0] = Unpack(data, PackLocation {0,16});
        scale[1] = Unpack(data, PackLocation {16,16});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Clock 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "CV 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_OUTS] = "Pitch A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "Scale";
    }
    
private:
    braids::Quantizer quantizer[2];
    int scale[2]; // Scale per channel
    int last_note[2]; // Last quantized note
    bool continuous[2]; // Each channel starts as continuous and becomes clocked when a clock is received
    int cursor;
    int adc_lag_countdown;
    const uint8_t notes[2][8] = {{0xc0, 0xe0, 0xe0, 0xe0, 0x7f, 0x02, 0x14, 0x08},
                                 {0xc0, 0xa0, 0xa0, 0xa0, 0x7f, 0x00, 0x00, 0x00}};

    void DrawSelector()
    {
        ForEachChannel(ch)
        {
            gfxBitmap(0 + (31 * ch), 15, 8, notes[ch]);
            if (!continuous[ch]) gfxBitmap(10 + (31 * ch), 15,  8, CLOCK_ICON); // Display icon if clocked

            gfxPrint(0 + (31 * ch), 25, OC::scale_names_short[scale[ch]]);
            if (ch == cursor) gfxCursor(0 + (31 * ch), 33, 30);

            // Little notes
            gfxBitmap(ProportionCV(last_note[ch], 54), 41 + (10 * ch), 8, notes[ch]);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to DualQuant,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
DualQuant DualQuant_instance[2];

void DualQuant_Start(bool hemisphere) {
    DualQuant_instance[hemisphere].BaseStart(hemisphere);
}

void DualQuant_Controller(bool hemisphere, bool forwarding) {
    DualQuant_instance[hemisphere].BaseController(forwarding);
}

void DualQuant_View(bool hemisphere) {
    DualQuant_instance[hemisphere].BaseView();
}

void DualQuant_Screensaver(bool hemisphere) {
    DualQuant_instance[hemisphere].BaseScreensaverView();
}

void DualQuant_OnButtonPress(bool hemisphere) {
    DualQuant_instance[hemisphere].OnButtonPress();
}

void DualQuant_OnEncoderMove(bool hemisphere, int direction) {
    DualQuant_instance[hemisphere].OnEncoderMove(direction);
}

void DualQuant_ToggleHelpScreen(bool hemisphere) {
    DualQuant_instance[hemisphere].HelpScreen();
}

uint32_t DualQuant_OnDataRequest(bool hemisphere) {
    return DualQuant_instance[hemisphere].OnDataRequest();
}

void DualQuant_OnDataReceive(bool hemisphere, uint32_t data) {
    DualQuant_instance[hemisphere].OnDataReceive(data);
}
