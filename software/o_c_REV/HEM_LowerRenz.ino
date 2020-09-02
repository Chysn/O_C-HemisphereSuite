// Copyright (c) 2018, Jason Justian
//
// Port of subset of Low Rents Copyright (c) 2016 Patrick Dowling,
// Copyright (c) 2014 Émilie Gillet, Copyright (c) 2016 Tim Churches
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

#include "streams_lorenz_generator.h"
#include "util/util_math.h"
#include "HSLorenzGeneratorManager.h" // Singleton Lorenz manager

class LowerRenz : public HemisphereApplet {
public:

    const char* applet_name() {
        return "LowerRenz";
    }

    void Start() {
        freq = 128;
        rho = 64;
    }

    void Controller() {
        if (!Gate(1)) { // Freeze if gated
            int freq_cv = Proportion(In(0), HEMISPHERE_MAX_CV, 63);
            int rho_cv = Proportion(In(1), HEMISPHERE_MAX_CV, 31);

            int32_t freq_h = SCALE8_16(constrain(freq + freq_cv, 0, 255));
            freq_h = USAT16(freq_h);
            lorenz_m->SetFreq(hemisphere, freq_h);

            int32_t rho_h = SCALE8_16(constrain(rho + rho_cv, 4, 127));
            lorenz_m->SetRho(hemisphere, USAT16(rho_h));

            if (Clock(0, true)) lorenz_m->Reset(hemisphere);
            lorenz_m->Process();

            // The scaling here is based on observation of the value range
            int x = Proportion(lorenz_m->GetOut(0 + (hemisphere * 2)) - 17000, 25000, HEMISPHERE_MAX_CV);
            int y = Proportion(lorenz_m->GetOut(1 + (hemisphere * 2)) - 17000, 25000, HEMISPHERE_MAX_CV);

            Out(0, x);
            Out(1, y);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawEditor();
        DrawOutput();
    }

    void OnButtonPress() {
        cursor = 1 - cursor;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) freq = constrain(freq += direction, 0, 255);
        if (cursor == 1) rho = constrain(rho += direction, 4, 127);
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, (uint8_t)freq);
        Pack(data, PackLocation {8,8}, (uint8_t)rho);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        freq = Unpack(data, PackLocation {0,8});
        rho = Unpack(data, PackLocation {8,8});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Reset 2=Freeze";
        help[HEMISPHERE_HELP_CVS]      = "1=Freq 2=Rho";
        help[HEMISPHERE_HELP_OUTS]     = "A=x B=y";
        help[HEMISPHERE_HELP_ENCODER]  = "Freq/Rho";
        //                               "------------------" <-- Size Guide
    }

private:
    LorenzGeneratorManager *lorenz_m = lorenz_m->get();
    int freq;
    int rho;
    int cursor; // 0 = Frequency, 1 = Rho

    void DrawEditor() {
        gfxPrint(1, 15, "Freq");
        gfxPrint(1, 24, freq);
        if (cursor == 0) gfxCursor(1, 32, 30);

        gfxPrint(45, 15, "Rho");
        gfxPrint(45 + (rho > 99 ? 0 : 6), 24, rho);
        if (cursor == 1) gfxCursor(32, 32, 31);
    }

    void DrawOutput() {
        gfxPrint(1, 38, "x");
        gfxPrint(1, 50, "y");
        ForEachChannel(ch)
        {
            int w = ProportionCV(ViewOut(ch), 62);
            gfxInvert(1, 38 + (12 * ch), w, 10);
        }
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to LowerRenz,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
LowerRenz LowerRenz_instance[2];

void LowerRenz_Start(bool hemisphere) {
    LowerRenz_instance[hemisphere].BaseStart(hemisphere);
}

void LowerRenz_Controller(bool hemisphere, bool forwarding) {
    LowerRenz_instance[hemisphere].BaseController(forwarding);
}

void LowerRenz_View(bool hemisphere) {
    LowerRenz_instance[hemisphere].BaseView();
}

void LowerRenz_OnButtonPress(bool hemisphere) {
    LowerRenz_instance[hemisphere].OnButtonPress();
}

void LowerRenz_OnEncoderMove(bool hemisphere, int direction) {
    LowerRenz_instance[hemisphere].OnEncoderMove(direction);
}

void LowerRenz_ToggleHelpScreen(bool hemisphere) {
    LowerRenz_instance[hemisphere].HelpScreen();
}

uint32_t LowerRenz_OnDataRequest(bool hemisphere) {
    return LowerRenz_instance[hemisphere].OnDataRequest();
}

void LowerRenz_OnDataReceive(bool hemisphere, uint32_t data) {
    LowerRenz_instance[hemisphere].OnDataReceive(data);
}
