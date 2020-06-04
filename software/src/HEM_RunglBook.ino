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

class RunglBook : public HemisphereApplet {
public:

    const char* applet_name() {
        return "RunglBook";
    }

    void Start() {
        threshold = (12 << 7) * 2;
    }

    void Controller() {
        if (Clock(0)) {
            if (Gate(1)) {
                // Digital 2 freezes the buffer, so just rotate left
                reg = (reg << 1) | ((reg >> 7) & 0x01);
            } else {
                byte b0 = In(0) > threshold ? 0x01 : 0x00;
                reg = (reg << 1) | b0;
            }

            int rungle = Proportion(reg & 0x07, 0x07, HEMISPHERE_MAX_CV);
            int rungle_tap = Proportion((reg >> 5) & 0x07, 0x07, HEMISPHERE_MAX_CV);

            Out(0, rungle);
            Out(1, rungle_tap);
        }
    }

    void View() {
        gfxHeader(applet_name());
        gfxPrint(1, 15, "Thr:");
        gfxPrintVoltage(threshold);
        gfxSkyline();
    }

    void OnButtonPress() { }

    void OnEncoderMove(int direction) {
        threshold += (direction * 128);
        threshold = constrain(threshold, (12 << 7), (12 << 7) * 5); // 1V - 5V
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, threshold);
        return data;
    }
    void OnDataReceive(uint32_t data) {
        threshold = Unpack(data, PackLocation {0,16});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Freeze";
        help[HEMISPHERE_HELP_CVS]      = "1=Signal";
        help[HEMISPHERE_HELP_OUTS]     = "A=Rungle B=Alt";
        help[HEMISPHERE_HELP_ENCODER]  = "Threshold";
        //                               "------------------" <-- Size Guide
    }
    
private:
    byte reg;
    uint16_t threshold;
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to RunglBook,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
RunglBook RunglBook_instance[2];

void RunglBook_Start(bool hemisphere) {RunglBook_instance[hemisphere].BaseStart(hemisphere);}
void RunglBook_Controller(bool hemisphere, bool forwarding) {RunglBook_instance[hemisphere].BaseController(forwarding);}
void RunglBook_View(bool hemisphere) {RunglBook_instance[hemisphere].BaseView();}
void RunglBook_OnButtonPress(bool hemisphere) {RunglBook_instance[hemisphere].OnButtonPress();}
void RunglBook_OnEncoderMove(bool hemisphere, int direction) {RunglBook_instance[hemisphere].OnEncoderMove(direction);}
void RunglBook_ToggleHelpScreen(bool hemisphere) {RunglBook_instance[hemisphere].HelpScreen();}
uint32_t RunglBook_OnDataRequest(bool hemisphere) {return RunglBook_instance[hemisphere].OnDataRequest();}
void RunglBook_OnDataReceive(bool hemisphere, uint32_t data) {RunglBook_instance[hemisphere].OnDataReceive(data);}
