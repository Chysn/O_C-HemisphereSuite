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

const char* const crusher_rate[8] = {
    "16.7", "8.3", "5.6", "4.2", "3.3", "2.1", "1", ".5"
};
const int crusher_ticks[8] = {1, 2, 3, 4, 5, 8, 16, 32};

class DrCrusher : public HemisphereApplet {
public:

    const char* applet_name() {
        return "DrCrusher";
    }

    void Start() {
    }

    void Controller() {
        if (!Gate(0)) {
            if (++count >= crusher_ticks[rate]) {
                count = 0;
                int cv = In(0);
                // Convert CV to positive, 16-bit
                int p_cv = cv + HEMISPHERE_MAX_CV;
                uint16_t p_cv16 = static_cast<uint16_t>(p_cv);
                p_cv16 = p_cv16 & mask;
                cv = static_cast<int>(p_cv16) - HEMISPHERE_MAX_CV;
                Out(0, cv);
            }
        } else Out(0, In(0));
        Out(1, In(0));
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        cursor = 1 - cursor;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) {
            rate = constrain(rate - direction, 0, 7);
        } else {
            depth = constrain(depth + direction, 1, 13);
            mask = get_mask();
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,3}, rate);
        Pack(data, PackLocation {3,4}, depth);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        rate = Unpack(data, PackLocation {0,3});
        depth = Unpack(data, PackLocation {3,4});
        mask = get_mask();
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Defeat";
        help[HEMISPHERE_HELP_CVS]      = "1=Input";
        help[HEMISPHERE_HELP_OUTS]     = "A=Crush B=Thru";
        help[HEMISPHERE_HELP_ENCODER]  = "Rate/Depth";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0 = Rate, 1 = Depth
    int rate = 0;
    int depth = 13;

    // Housekeeping
    byte count = 0;
    uint16_t mask = 0xffff;
    
    void DrawInterface() {
        gfxPrint(1, 15, crusher_rate[rate]);
        gfxPrint("kHz");
        gfxPrint(1 + pad(10, depth + 1), 25, depth + 1);
        gfxPrint("-bit");

        if (cursor == 0) gfxCursor(2, 23, 62);
        else gfxCursor(2, 33, 62);

        // Draw graphic indicator
        int cv = ViewOut(0);
        int w = Proportion(cv, HEMISPHERE_MAX_CV, 30);
        w = constrain(w, -30, 30);
        if (w > 0) gfxRect(32, 45, w, 10);
        else gfxFrame(32 + w, 45, -w, 10);
    }

    uint16_t get_mask() {
        uint16_t mask = 0x0000;
        for (byte b = 0; b <= depth; b++)
        {
            mask = mask << 1;
            mask |= 0x01;
        }
        mask = mask << (13 - depth);
        mask |= 0xc000; // Turn on the high two bits
        return mask;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to DrCrusher,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
DrCrusher DrCrusher_instance[2];

void DrCrusher_Start(bool hemisphere) {DrCrusher_instance[hemisphere].BaseStart(hemisphere);}
void DrCrusher_Controller(bool hemisphere, bool forwarding) {DrCrusher_instance[hemisphere].BaseController(forwarding);}
void DrCrusher_View(bool hemisphere) {DrCrusher_instance[hemisphere].BaseView();}
void DrCrusher_OnButtonPress(bool hemisphere) {DrCrusher_instance[hemisphere].OnButtonPress();}
void DrCrusher_OnEncoderMove(bool hemisphere, int direction) {DrCrusher_instance[hemisphere].OnEncoderMove(direction);}
void DrCrusher_ToggleHelpScreen(bool hemisphere) {DrCrusher_instance[hemisphere].HelpScreen();}
uint32_t DrCrusher_OnDataRequest(bool hemisphere) {return DrCrusher_instance[hemisphere].OnDataRequest();}
void DrCrusher_OnDataReceive(bool hemisphere, uint32_t data) {DrCrusher_instance[hemisphere].OnDataReceive(data);}
