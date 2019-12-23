// Copyright (c) 2019, Jason Justian
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

#define ATTENOFF_INCREMENTS 128

class AttenuateOffset : public HemisphereApplet {
public:

    const char* applet_name() {
        return "AttenOff";
    }

    void Start() {
        ForEachChannel(ch) level[ch] = 63;
    }

    void Controller() {
        ForEachChannel(ch)
        {
            int signal = Proportion(level[ch], 63, In(ch)) + (offset[ch] * ATTENOFF_INCREMENTS);
            signal = constrain(signal, -HEMISPHERE_3V_CV, HEMISPHERE_MAX_CV);
            Out(ch, signal);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        if (++cursor > 3) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        uint8_t ch = cursor / 2;
        if (cursor == 0 || cursor == 2) {
            // Change offset voltage
            int min = -HEMISPHERE_MAX_CV / ATTENOFF_INCREMENTS;
            int max = HEMISPHERE_MAX_CV / ATTENOFF_INCREMENTS;
            offset[ch] = constrain(offset[ch] + direction, min, max);
        } else {
            // Change level percentage
            level[ch] = constrain(level[ch] + direction, 0, 63);
        }

    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,9}, offset[0] + 256);
        Pack(data, PackLocation {10,9}, offset[1] + 256);
        Pack(data, PackLocation {19,6}, level[0]);
        Pack(data, PackLocation {25,6}, level[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        offset[0] = Unpack(data, PackLocation {0,9}) - 256;
        offset[1] = Unpack(data, PackLocation {10,9}) - 256;
        level[0] = Unpack(data, PackLocation {19,6});
        level[1] = Unpack(data, PackLocation {25,6});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "CV Inputs 1,2";
        help[HEMISPHERE_HELP_OUTS]     = "Outputs A,B";
        help[HEMISPHERE_HELP_ENCODER]  = "Offset V / Level %";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    int level[2];
    int offset[2];
    
    void DrawInterface() {
        ForEachChannel(ch)
        {
            gfxPrint(0, 15 + (ch * 20), (hemisphere ? (ch ? "D " : "C ") : (ch ? "B " : "A ")));
            int cv = offset[ch] * ATTENOFF_INCREMENTS;
            gfxPrintVoltage(cv);
            gfxPrint(16, 25 + (ch * 20), Proportion(level[ch], 63, 100));
            gfxPrint("%");
        }

        int ch = cursor / 2;
        if (cursor == 0 or cursor == 2) gfxCursor(13, 23 + (ch * 20), 36);
        else gfxCursor(13, 33 + (ch * 20), 36);
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to AttenuateOffset,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
AttenuateOffset AttenuateOffset_instance[2];

void AttenuateOffset_Start(bool hemisphere) {AttenuateOffset_instance[hemisphere].BaseStart(hemisphere);}
void AttenuateOffset_Controller(bool hemisphere, bool forwarding) {AttenuateOffset_instance[hemisphere].BaseController(forwarding);}
void AttenuateOffset_View(bool hemisphere) {AttenuateOffset_instance[hemisphere].BaseView();}
void AttenuateOffset_OnButtonPress(bool hemisphere) {AttenuateOffset_instance[hemisphere].OnButtonPress();}
void AttenuateOffset_OnEncoderMove(bool hemisphere, int direction) {AttenuateOffset_instance[hemisphere].OnEncoderMove(direction);}
void AttenuateOffset_ToggleHelpScreen(bool hemisphere) {AttenuateOffset_instance[hemisphere].HelpScreen();}
uint32_t AttenuateOffset_OnDataRequest(bool hemisphere) {return AttenuateOffset_instance[hemisphere].OnDataRequest();}
void AttenuateOffset_OnDataReceive(bool hemisphere, uint32_t data) {AttenuateOffset_instance[hemisphere].OnDataReceive(data);}
