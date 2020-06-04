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

const uint8_t HEM_PPQN_VALUES[] = {1, 2, 4, 8, 16, 24};

class Scope : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Scope";
    }

    void Start() {
        last_bpm_tick = OC::CORE::ticks;
        bpm = 0;
        sample_ticks = 320;
        freeze = 0;
        last_scope_tick = 0;
    }

    void Controller() {
        if (Clock(0)) {
            int this_tick = OC::CORE::ticks;
            int time = this_tick - last_bpm_tick;
            last_bpm_tick = this_tick;
            bpm = 1000000 / time;
            if (bpm > 9999) bpm = 9999;
        }

        if (Clock(1)) {
            if (last_scope_tick) {
                int cycle_ticks = OC::CORE::ticks - last_scope_tick;
                sample_ticks = cycle_ticks / 64;
                sample_ticks = constrain(sample_ticks, 2, 64000);
            }
            last_scope_tick = OC::CORE::ticks;
        }

        if (!freeze) {
            last_cv = In(1);

            if (--sample_countdown < 1) {
                sample_countdown = sample_ticks;
                if (++sample_num > 63) sample_num = 0;
                int sample = Proportion(In(0), HEMISPHERE_MAX_CV, 128);
                sample = constrain(sample, -128, 127) + 127;
                snapshot[sample_num] = (uint8_t)sample;
            }

            ForEachChannel(ch) Out(ch, In(ch));
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawBPM();
        DrawInput1();
        DrawInput2();
        if (freeze) {
            gfxInvert(0, 24, 64, 40);
        }
    }

    void OnButtonPress() {
        freeze = 1 - freeze;
    }

    void OnEncoderMove(int direction) {
        if (sample_ticks < 32) sample_ticks += direction;
        else sample_ticks += direction * 10;
        sample_ticks = constrain(sample_ticks, 2, 64000);
        last_encoder_move = OC::CORE::ticks;
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name); 
        return data;
    }

    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8}); 
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Clk 1=BPM 2=Cycle1";
        help[HEMISPHERE_HELP_CVS]      = "1=CV1 2=CV2";
        help[HEMISPHERE_HELP_OUTS]     = "A=CV1 B=CV2";
        help[HEMISPHERE_HELP_ENCODER]  = "T=SmplRate P=Freez";
        //                               "------------------" <-- Size Guide
    }
    
private:
    // BPM Calcultion
    int last_bpm_tick;
    int bpm;

    // CV monitor
    int last_cv;
    bool freeze;

    // Scope
    uint8_t snapshot[64];
    int sample_ticks; // Ticks between samples
    int sample_countdown; // Last time a sample was taken
    int sample_num; // Current sample number at the start
    int last_encoder_move; // The last the the sample_ticks value was changed
    int last_scope_tick; // Used to auto-calculate sample countdown

    // Icons

    void DrawBPM() {
        gfxPrint(9, 15, "BPM ");
        gfxPrint(bpm / 4);
        gfxLine(0, 24, 63, 24);

        if (OC::CORE::ticks - last_bpm_tick < 1666) gfxBitmap(1, 15, 8, CLOCK_ICON);
    }

    void DrawInput1() {
        for (int s = 0; s < 64; s++)
        {
            int x = s + sample_num;
            if (x > 63) x -= 64;
            int l = Proportion(snapshot[x], 255, 28);
            gfxPixel(x, (28 - l) + 24);
        }

        if (OC::CORE::ticks - last_encoder_move < 16667) {
            gfxPrint(1, 26, sample_ticks);
        }
    }

    void DrawInput2() {
        gfxLine(0, 53, 63, 53);
        gfxBitmap(1, 55, 8, CV_ICON);
        gfxPos(12, 55);
        gfxPrintVoltage(last_cv);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Scope,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Scope Scope_instance[2];

void Scope_Start(bool hemisphere) {
    Scope_instance[hemisphere].BaseStart(hemisphere);
}

void Scope_Controller(bool hemisphere, bool forwarding) {
    Scope_instance[hemisphere].BaseController(forwarding);
}

void Scope_View(bool hemisphere) {
    Scope_instance[hemisphere].BaseView();
}

void Scope_OnButtonPress(bool hemisphere) {
    Scope_instance[hemisphere].OnButtonPress();
}

void Scope_OnEncoderMove(bool hemisphere, int direction) {
    Scope_instance[hemisphere].OnEncoderMove(direction);
}

void Scope_ToggleHelpScreen(bool hemisphere) {
    Scope_instance[hemisphere].HelpScreen();
}

uint32_t Scope_OnDataRequest(bool hemisphere) {
    return Scope_instance[hemisphere].OnDataRequest();
}

void Scope_OnDataReceive(bool hemisphere, uint32_t data) {
    Scope_instance[hemisphere].OnDataReceive(data);
}
