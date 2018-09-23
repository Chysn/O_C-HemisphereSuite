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

class DIAGNOSTIC : public HemisphereApplet {
public:

    const char* applet_name() {
        return "DIAGNOSTIC";
    }

    void Start() {
        quantizer.Init();
        scale = OC::Scales::SCALE_SEMI;
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff); // Semi-tone
    }

    void Controller() {
        if (note != last_note) {
            cv = quantizer.Lookup(note);
            Out(0, cv);
            last_note = note;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        cursor = 1 - cursor;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) {
            scale = constrain(scale + direction, 0, 63);
            quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
            last_note = -1;
        } else {
            note = constrain(note + direction, 0, 127);
        }
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
    
    int note = 60;
    int last_note = 0;
    braids::Quantizer quantizer;
    int cv = 0;
    int scale = 0;

    void DrawInterface() {
        gfxPrint(1, 25, "Sc: ");
        gfxPrint(OC::scale_names_short[scale]);
        gfxPrint(1, 35, "Note: ");
        gfxPrint(note);
        gfxPrint(1, 45, "CV: ");
        gfxPrint(cv);
    }
    
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to DIAGNOSTIC,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
DIAGNOSTIC DIAGNOSTIC_instance[2];

void DIAGNOSTIC_Start(bool hemisphere) {DIAGNOSTIC_instance[hemisphere].BaseStart(hemisphere);}
void DIAGNOSTIC_Controller(bool hemisphere, bool forwarding) {DIAGNOSTIC_instance[hemisphere].BaseController(forwarding);}
void DIAGNOSTIC_View(bool hemisphere) {DIAGNOSTIC_instance[hemisphere].BaseView();}
void DIAGNOSTIC_Screensaver(bool hemisphere) {DIAGNOSTIC_instance[hemisphere].BaseScreensaverView();}
void DIAGNOSTIC_OnButtonPress(bool hemisphere) {DIAGNOSTIC_instance[hemisphere].OnButtonPress();}
void DIAGNOSTIC_OnEncoderMove(bool hemisphere, int direction) {DIAGNOSTIC_instance[hemisphere].OnEncoderMove(direction);}
void DIAGNOSTIC_ToggleHelpScreen(bool hemisphere) {DIAGNOSTIC_instance[hemisphere].HelpScreen();}
uint32_t DIAGNOSTIC_OnDataRequest(bool hemisphere) {return DIAGNOSTIC_instance[hemisphere].OnDataRequest();}
void DIAGNOSTIC_OnDataReceive(bool hemisphere, uint32_t data) {DIAGNOSTIC_instance[hemisphere].OnDataReceive(data);}
