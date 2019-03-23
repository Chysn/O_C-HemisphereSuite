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

class ClockSetup : public HemisphereApplet {
public:

    const char* applet_name() {
        return "ClockSet";
    }

    void Start() { }

    // When the ClockSetup is active, the selected applets should continue to function, so
    // there's no need to have a controller for ClockSetup.
    void Controller() { }

    void View() {
        DrawInterface();
    }

    void OnButtonPress() {
        if (++cursor > 2) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) { // Source
            if (clock_m->IsRunning() || clock_m->IsPaused()) clock_m->Stop();
            else {
                clock_m->Start();
                clock_m->Pause();
            }
        }

        if (cursor == 1) { // Set tempo
            uint16_t bpm = clock_m->GetTempo();
            bpm += direction;
            clock_m->SetTempoBPM(bpm);
        }

        if (cursor == 2) { // Set multiplier
            int8_t mult = clock_m->GetMultiply();
            mult += direction;
            clock_m->SetMultiply(mult);
        }
    }
        
    uint32_t OnDataRequest() {return 0;}
    void OnDataReceive(uint32_t data) { }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "";
        help[HEMISPHERE_HELP_ENCODER]  = "";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=Source, 1=Tempo, 2=Multiply
    ClockManager *clock_m = clock_m->get();
    
    void DrawInterface() {
        // Header: This is sort of a faux applet, so its header
        // needs to extend across the screen
        graphics.setPrintPos(1, 2);
        graphics.print("Clock Setup");
        gfxLine(0, 10, 62, 10);
        gfxLine(0, 12, 62, 12);
        graphics.drawLine(0, 10, 127, 10);
        graphics.drawLine(0, 12, 127, 12);

        // Clock Source
        if (clock_m->IsRunning()) {
            gfxIcon(1, 15, PLAY_ICON);
            gfxPrint(16, 15, "Internal");
        } else if (clock_m->IsPaused()) {
            gfxIcon(1, 15, PAUSE_ICON);
            gfxPrint(16, 15, "Internal");
        } else {
            gfxIcon(1, 15, CLOCK_ICON);
            gfxPrint(16, 15, "Forward");
        }

        // Tempo
        gfxIcon(1, 25, NOTE4_ICON);
        gfxPrint(9, 25, "= ");
        gfxPrint(pad(100, clock_m->GetTempo()), clock_m->GetTempo());
        gfxPrint(" BPM");

        // Multiply
        gfxPrint(1, 35, "x");
        gfxPrint(clock_m->GetMultiply());

        if (cursor == 0) gfxCursor(16, 23, 46);
        if (cursor == 1) gfxCursor(23, 33, 18);
        if (cursor == 2) gfxCursor(8, 43, 12);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ClockSetup,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ClockSetup ClockSetup_instance[1];

void ClockSetup_Start(bool hemisphere) {ClockSetup_instance[hemisphere].BaseStart(hemisphere);}
void ClockSetup_Controller(bool hemisphere, bool forwarding) {ClockSetup_instance[hemisphere].BaseController(forwarding);}
void ClockSetup_View(bool hemisphere) {ClockSetup_instance[hemisphere].BaseView();}
void ClockSetup_OnButtonPress(bool hemisphere) {ClockSetup_instance[hemisphere].OnButtonPress();}
void ClockSetup_OnEncoderMove(bool hemisphere, int direction) {ClockSetup_instance[hemisphere].OnEncoderMove(direction);}
void ClockSetup_ToggleHelpScreen(bool hemisphere) {ClockSetup_instance[hemisphere].HelpScreen();}
uint32_t ClockSetup_OnDataRequest(bool hemisphere) {return ClockSetup_instance[hemisphere].OnDataRequest();}
void ClockSetup_OnDataReceive(bool hemisphere, uint32_t data) {ClockSetup_instance[hemisphere].OnDataReceive(data);}
