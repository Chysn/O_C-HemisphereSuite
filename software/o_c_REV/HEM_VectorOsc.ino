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

#include "HSVectorOscillator.h"

class VectorOsc : public HemisphereApplet {
public:

    const char* applet_name() {
        return "VectorOsc";
    }

    void Start() {
//        osc.SetSegment(VOSegment {255,0});
        osc.SetSegment(VOSegment {255,1});
//        osc.SetSegment(VOSegment {0,0});
        osc.SetSegment(VOSegment {0,1});
        //osc.SetSegment(VOSegment {0,0});
        //osc.SetSegment(VOSegment {0,1});
        osc.SetFrequency(Hz);
        osc.SetScale((12 << 7) * 3);
    }

    void Controller() {
        Out(0, osc.Next());
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
        if (Hz < 500) Hz += direction;
        if (Hz > 500) Hz += (100 * direction);
        osc.SetFrequency(Hz);
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
        help[HEMISPHERE_HELP_DIGITALS] = "Digital in help";
        help[HEMISPHERE_HELP_CVS]      = "CV in help";
        help[HEMISPHERE_HELP_OUTS]     = "Out help";
        help[HEMISPHERE_HELP_ENCODER]  = "123456789012345678";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    VectorOscillator osc;
    uint32_t Hz = 440;
    
    void DrawInterface() {
        gfxPrint(1, 15, Hz);
        gfxPrint(1, 35, osc.Diagnostic());
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to VectorOsc,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
VectorOsc VectorOsc_instance[2];

void VectorOsc_Start(bool hemisphere) {VectorOsc_instance[hemisphere].BaseStart(hemisphere);}
void VectorOsc_Controller(bool hemisphere, bool forwarding) {VectorOsc_instance[hemisphere].BaseController(forwarding);}
void VectorOsc_View(bool hemisphere) {VectorOsc_instance[hemisphere].BaseView();}
void VectorOsc_Screensaver(bool hemisphere) {VectorOsc_instance[hemisphere].BaseScreensaverView();}
void VectorOsc_OnButtonPress(bool hemisphere) {VectorOsc_instance[hemisphere].OnButtonPress();}
void VectorOsc_OnEncoderMove(bool hemisphere, int direction) {VectorOsc_instance[hemisphere].OnEncoderMove(direction);}
void VectorOsc_ToggleHelpScreen(bool hemisphere) {VectorOsc_instance[hemisphere].HelpScreen();}
uint32_t VectorOsc_OnDataRequest(bool hemisphere) {return VectorOsc_instance[hemisphere].OnDataRequest();}
void VectorOsc_OnDataReceive(bool hemisphere, uint32_t data) {VectorOsc_instance[hemisphere].OnDataReceive(data);}
