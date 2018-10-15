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

class GatedVCA : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Gated VCA";
    }

    void Start() {
        amp_offset_pct = 0;
        amp_offset_cv = 0;
    }

    void Controller() {
        int signal = In(0);
        int amplitude = In(1);
        int output = ProportionCV(amplitude, signal);
        output += amp_offset_cv;
        output = constrain(output, -HEMISPHERE_MAX_CV, HEMISPHERE_MAX_CV);

        if (Gate(0)) Out(0, output); // Normally-off gated VCA output on A
        else Out(0, 0);

        if (Gate(1)) Out(1, 0); // Normally-on ungated VCA output on B
        else Out(1, output);
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
        amp_offset_pct = constrain(amp_offset_pct += direction, 0, 100);
        amp_offset_cv = Proportion(amp_offset_pct, 100, HEMISPHERE_MAX_CV);
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        return data;
    }

    void OnDataReceive(uint32_t data) {
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=A Gate 2=B Revrs";
        help[HEMISPHERE_HELP_CVS] = "1=CV signal 2=Amp";
        help[HEMISPHERE_HELP_OUTS] = "A=Norm off B=N. on";
        help[HEMISPHERE_HELP_ENCODER] = "T=Amp CV Offset";
    }
    
private:
    int amp_offset_pct; // Offset as percentage of max cv
    int amp_offset_cv; // Raw CV offset; calculated on encoder move

    void DrawInterface() {
        gfxPrint(0, 15, "Offset:");
        gfxPrint(pad(100, amp_offset_pct), amp_offset_pct);
        gfxSkyline();
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to GatedVCA,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
GatedVCA GatedVCA_instance[2];

void GatedVCA_Start(bool hemisphere) {
    GatedVCA_instance[hemisphere].BaseStart(hemisphere);
}

void GatedVCA_Controller(bool hemisphere, bool forwarding) {
    GatedVCA_instance[hemisphere].BaseController(forwarding);
}

void GatedVCA_View(bool hemisphere) {
    GatedVCA_instance[hemisphere].BaseView();
}

void GatedVCA_OnButtonPress(bool hemisphere) {
    GatedVCA_instance[hemisphere].OnButtonPress();
}

void GatedVCA_OnEncoderMove(bool hemisphere, int direction) {
    GatedVCA_instance[hemisphere].OnEncoderMove(direction);
}

void GatedVCA_ToggleHelpScreen(bool hemisphere) {
    GatedVCA_instance[hemisphere].HelpScreen();
}

uint32_t GatedVCA_OnDataRequest(bool hemisphere) {
    return GatedVCA_instance[hemisphere].OnDataRequest();
}

void GatedVCA_OnDataReceive(bool hemisphere, uint32_t data) {
    GatedVCA_instance[hemisphere].OnDataReceive(data);
}
