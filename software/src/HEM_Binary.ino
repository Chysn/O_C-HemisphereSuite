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

#include "SegmentDisplay.h"

class Binary : public HemisphereApplet {
public:

    const char* applet_name() {
        return "BinaryCtr";
    }

    void Start() {
        segment.Init(SegmentSize::BIG_SEGMENTS);
    }

    void Controller() {
        bit[0] = Gate(0);
        bit[1] = Gate(1);
        bit[2] = In(0) > HEMISPHERE_3V_CV;
        bit[3] = In(1) > HEMISPHERE_3V_CV;

        // Output A is summed binary output, where
        // bit 0 = .25V, etc.
        int sum = (bit[3] * B0Val)
                + (bit[2] * (2 * B0Val))
                + (bit[1] * (4 * B0Val))
                + (bit[0] * (8 * B0Val));
        Out(0, sum);

        // Output B is a count of high binary states, with 1V per state
        int count = (static_cast<int>(bit[0] + bit[1] + bit[2] + bit[3])) * CVal;
        Out(1, count);
    }

    void View() {
        gfxHeader(applet_name());
        DrawDisplay();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        return data;
    }

    void OnDataReceive(uint32_t data) {
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Bit 3 2=Bit 2";
        help[HEMISPHERE_HELP_CVS]      = "1=Bit 1 2=Bit 0";
        help[HEMISPHERE_HELP_OUTS]     = "A=Binary B=Count";
        help[HEMISPHERE_HELP_ENCODER]  = "";
        //                               "------------------" <-- Size Guide
    }
    
private:
    bool bit[4];
    const int CVal = HEMISPHERE_MAX_CV / 4; // One-fourth of the count
    const int B0Val = HEMISPHERE_MAX_CV / 15; // Votage of Bit 0
    SegmentDisplay segment;
    
    void DrawDisplay() {
        segment.SetPosition(11 + (hemisphere * 64), 32);
        for (int b = 0; b < 4; b++)
        {
            segment.PrintDigit(static_cast<uint8_t>(bit[b]));
        }

        gfxRect(1, 15, ProportionCV(ViewOut(0), 62), 6);
        gfxRect(1, 58, ProportionCV(ViewOut(1), 62), 6);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Binary,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Binary Binary_instance[2];

void Binary_Start(bool hemisphere) {
    Binary_instance[hemisphere].BaseStart(hemisphere);
}

void Binary_Controller(bool hemisphere, bool forwarding) {
    Binary_instance[hemisphere].BaseController(forwarding);
}

void Binary_View(bool hemisphere) {
    Binary_instance[hemisphere].BaseView();
}

void Binary_OnButtonPress(bool hemisphere) {
    Binary_instance[hemisphere].OnButtonPress();
}

void Binary_OnEncoderMove(bool hemisphere, int direction) {
    Binary_instance[hemisphere].OnEncoderMove(direction);
}

void Binary_ToggleHelpScreen(bool hemisphere) {
    Binary_instance[hemisphere].HelpScreen();
}

uint32_t Binary_OnDataRequest(bool hemisphere) {
    return Binary_instance[hemisphere].OnDataRequest();
}

void Binary_OnDataReceive(bool hemisphere, uint32_t data) {
    Binary_instance[hemisphere].OnDataReceive(data);
}
