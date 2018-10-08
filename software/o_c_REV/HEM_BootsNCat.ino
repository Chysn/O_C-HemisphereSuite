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

#include "vector_osc/HSVectorOscillator.h"

class BootsNCat : public HemisphereApplet {
public:

    const char* applet_name() {
        return "BootsNCat";
    }

    void Start() {
        ForEachChannel(ch)
        {
            sound[ch].SetWaveform(VO_TRIANGLE);
            sound[ch].SetFrequency(5000 + (4000 * ch));
            sound[ch].SetScale((12 << 7) * 3);

            eg[ch].SetFrequency(220);
            eg[ch].SetScale(1000);
            eg[ch].SetSegment(VOSegment {255,0});
            eg[ch].SetSegment(VOSegment {128,1});
            eg[ch].Cycle(0);
        }

    }

    void Controller() {
        ForEachChannel(ch)
        {
            if (Clock(ch)) {
                eg[ch].Start();
            }

            if (!eg[ch].GetEOC()) {
                int32_t signal = Proportion(eg[ch].Next(), 1000, sound[ch].Next());
                Out(ch, signal);
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
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
        help[HEMISPHERE_HELP_DIGITALS] = "1,2 Play";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=Left B=Right";
        help[HEMISPHERE_HELP_ENCODER]  = "Preset/Pan";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    VectorOscillator sound[2];
    VectorOscillator eg[2];
    
    void DrawInterface() {
        gfxSkyline();
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to BootsNCat,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
BootsNCat BootsNCat_instance[2];

void BootsNCat_Start(bool hemisphere) {BootsNCat_instance[hemisphere].BaseStart(hemisphere);}
void BootsNCat_Controller(bool hemisphere, bool forwarding) {BootsNCat_instance[hemisphere].BaseController(forwarding);}
void BootsNCat_View(bool hemisphere) {BootsNCat_instance[hemisphere].BaseView();}
void BootsNCat_Screensaver(bool hemisphere) {BootsNCat_instance[hemisphere].BaseScreensaverView();}
void BootsNCat_OnButtonPress(bool hemisphere) {BootsNCat_instance[hemisphere].OnButtonPress();}
void BootsNCat_OnEncoderMove(bool hemisphere, int direction) {BootsNCat_instance[hemisphere].OnEncoderMove(direction);}
void BootsNCat_ToggleHelpScreen(bool hemisphere) {BootsNCat_instance[hemisphere].HelpScreen();}
uint32_t BootsNCat_OnDataRequest(bool hemisphere) {return BootsNCat_instance[hemisphere].OnDataRequest();}
void BootsNCat_OnDataReceive(bool hemisphere, uint32_t data) {BootsNCat_instance[hemisphere].OnDataReceive(data);}
