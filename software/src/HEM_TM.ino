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

/*
 * Turing Machine based on https://thonk.co.uk/documents/random%20sequencer%20documentation%20v2-1%20THONK%20KIT_LargeImages.pdf
 *
 * Thanks to Tom Whitwell for creating the concept, and for clarifying some things
 * Thanks to Jon Wheeler for the CV length and probability updates
 */

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"
#define TM_MAX_SCALE 63
#define TM_MIN_LENGTH 2
#define TM_MAX_LENGTH 16

class TM : public HemisphereApplet {
public:

    const char* applet_name() {
        return "ShiftReg";
    }

    void Start() {
        reg = random(0, 65535);
        p = 0;
        length = 16;
        cursor = 0;
        quantizer.Init();
        scale = OC::Scales::SCALE_SEMI;
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff); // Semi-tone
    }

    void Controller() {

        // CV 1 control over length
        int lengthCv = DetentedIn(0);
        if (lengthCv < 0) length = TM_MIN_LENGTH;        
        if (lengthCv > 0) {
            length = constrain(ProportionCV(lengthCv, TM_MAX_LENGTH + 1), TM_MIN_LENGTH, TM_MAX_LENGTH);
        }
      
        // CV 2 bi-polar modulation of probability
        int pCv = Proportion(DetentedIn(1), HEMISPHERE_MAX_CV, 100);
        
        if (Clock(0)) {
            // If the cursor is not on the p value, and Digital 2 is not gated, the sequence remains the same
            int prob = (cursor == 1 || Gate(1)) ? p + pCv : 0;
            prob = constrain(prob, 0, 100);

            // Grab the bit that's about to be shifted away
            int last = (reg >> (length - 1)) & 0x01;

            // Does it change?
            if (random(0, 99) < prob) last = 1 - last;

            // Shift left, then potentially add the bit from the other side
            reg = (reg << 1) + last;
        }
        
        // Send 5-bit quantized CV
        int note = reg & 0x1f;
        Out(0, quantizer.Lookup(note + 64));

        // Send 8-bit proportioned CV
        int cv = Proportion(reg & 0x00ff, 255, HEMISPHERE_MAX_CV);
        Out(1, cv);
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        DrawIndicator();
    }

    void OnButtonPress() {
        if (++cursor > 2) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) length = constrain(length += direction, TM_MIN_LENGTH, TM_MAX_LENGTH);
        if (cursor == 1) p = constrain(p += direction, 0, 100);
        if (cursor == 2) {
            scale += direction;
            if (scale >= TM_MAX_SCALE) scale = 0;
            if (scale < 0) scale = TM_MAX_SCALE - 1;
            quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, reg);
        Pack(data, PackLocation {16,7}, p);
        Pack(data, PackLocation {23,4}, length - 1);
        Pack(data, PackLocation {27,6}, scale);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        reg = Unpack(data, PackLocation {0,16});
        p = Unpack(data, PackLocation {16,7});
        length = Unpack(data, PackLocation {23,4}) + 1;
        scale = Unpack(data, PackLocation {27,6});
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=p Gate";
        help[HEMISPHERE_HELP_CVS]      = "1=Length 2=p Mod";
        help[HEMISPHERE_HELP_OUTS]     = "A=Quant5-bit B=CV8";
        help[HEMISPHERE_HELP_ENCODER]  = "Length/Prob/Scale";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int length; // Sequence length
    int cursor;  // 0 = length, 1 = p, 2 = scale
    braids::Quantizer quantizer;

    // Settings
    uint16_t reg; // 16-bit sequence register
    int p; // Probability of bit 15 changing on each cycle
    int8_t scale; // Scale used for quantized output

    void DrawSelector() {
        gfxBitmap(1, 14, 8, LOOP_ICON);
        gfxPrint(12 + pad(10, length), 15, length);
        gfxPrint(32, 15, "p=");
        if (cursor == 1 || Gate(1)) {
            int pCv = Proportion(DetentedIn(1), HEMISPHERE_MAX_CV, 100);
            int prob = constrain(p + pCv, 0, 100);
            if (cursor == 1) gfxCursor(45, 23, 18); // Probability Cursor
            gfxPrint(pad(100, prob), prob);
        } else {
            gfxBitmap(49, 14, 8, LOCK_ICON);
        }
        gfxBitmap(1, 24, 8, SCALE_ICON);
        gfxPrint(12, 25, OC::scale_names_short[scale]);
        if (cursor == 0) gfxCursor(13, 23, 12); // Length Cursor
        if (cursor == 2) gfxCursor(13, 33, 30); // Scale Cursor
    }

    void DrawIndicator() {
        gfxLine(0, 45, 63, 45);
        gfxLine(0, 62, 63, 62);
        for (int b = 0; b < 16; b++)
        {
            int v = (reg >> b) & 0x01;
            if (v) gfxRect(60 - (4 * b), 47, 3, 14);
        }
    }

    void AdvanceRegister(int prob) {
        // Before shifting, determine the fate of the last bit
        int last = (reg >> 15) & 0x01;
        if (random(0, 99) < prob) last = 1 - last;

        // Shift left, then potentially add the bit from the other side
        reg = (reg << 1) + last;
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to TM,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
TM TM_instance[2];

void TM_Start(bool hemisphere) {
    TM_instance[hemisphere].BaseStart(hemisphere);
}

void TM_Controller(bool hemisphere, bool forwarding) {
    TM_instance[hemisphere].BaseController(forwarding);
}

void TM_View(bool hemisphere) {
    TM_instance[hemisphere].BaseView();
}

void TM_OnButtonPress(bool hemisphere) {
    TM_instance[hemisphere].OnButtonPress();
}

void TM_OnEncoderMove(bool hemisphere, int direction) {
    TM_instance[hemisphere].OnEncoderMove(direction);
}

void TM_ToggleHelpScreen(bool hemisphere) {
    TM_instance[hemisphere].HelpScreen();
}

uint32_t TM_OnDataRequest(bool hemisphere) {
    return TM_instance[hemisphere].OnDataRequest();
}

void TM_OnDataReceive(bool hemisphere, uint32_t data) {
    TM_instance[hemisphere].OnDataReceive(data);
}
