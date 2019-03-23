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

class Metronome : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Metronome";
    }

    void Start() { }

    void Controller() {
        // Check the clock so that the little Metronome icon animates while
        // Metronome is selected
        Clock(0);

        // Outputs
        if (clock_m->IsRunning()) {
            if (clock_m->Tock()) {
                ClockOut(0);
                if (clock_m->EndOfBeat()) ClockOut(1);
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() { }

    void OnEncoderMove(int direction) {
        uint16_t bpm = clock_m->GetTempo();
        bpm += direction;
        clock_m->SetTempoBPM(bpm);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, clock_m->GetTempo());
        Pack(data, PackLocation {16,5}, clock_m->GetMultiply() - 1);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        clock_m->SetTempoBPM(Unpack(data, PackLocation {0,16}));
        clock_m->SetMultiply(Unpack(data, PackLocation {16,5}) + 1);
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=Multiply B=Beat";
        help[HEMISPHERE_HELP_ENCODER]  = "Tempo";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=Tempo, 1=Multiply, 2=Start/Stop
    ClockManager *clock_m = clock_m->get();
    
    void DrawInterface() {
        gfxIcon(1, 15, NOTE4_ICON);
        gfxPrint(9, 15, "= ");
        gfxPrint(pad(100, clock_m->GetTempo()), clock_m->GetTempo());
        gfxPrint(" BPM");

        DrawMetronome();
    }

    void DrawMetronome() {
        gfxLine(20,60,38,63); // Bottom Front
        gfxLine(38,62,44,55); // Bottom Right
        gfxLine(44,55,36,27); // Rear right edge
        gfxLine(38,63,33,29); // Front right edge
        gfxLine(20,60,29,27); // Front left edge
        gfxLine(22,49,36,51); // Front ledge
        gfxLine(29,27,31,25); // Point: front left
        gfxLine(33,29,31,25); // Point: front right
        gfxLine(36,27,31,25); // Point: rear right
        gfxLine(29,27,33,29); // Point base: front
        gfxLine(33,29,36,27); // Point base: right
        gfxDottedLine(29,50,31,28,3); // Tempo scale
        gfxDottedLine(30,50,32,28,3); // Tempo scale
        gfxCircle(40,51,1); // Winder

        // Pendulum arm
        if (clock_m->Cycle()) gfxLine(29,50,21,31);
        else gfxLine(29,50,37,32);
    }

    
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Metronome,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Metronome Metronome_instance[2];

void Metronome_Start(bool hemisphere) {Metronome_instance[hemisphere].BaseStart(hemisphere);}
void Metronome_Controller(bool hemisphere, bool forwarding) {Metronome_instance[hemisphere].BaseController(forwarding);}
void Metronome_View(bool hemisphere) {Metronome_instance[hemisphere].BaseView();}
void Metronome_OnButtonPress(bool hemisphere) {Metronome_instance[hemisphere].OnButtonPress();}
void Metronome_OnEncoderMove(bool hemisphere, int direction) {Metronome_instance[hemisphere].OnEncoderMove(direction);}
void Metronome_ToggleHelpScreen(bool hemisphere) {Metronome_instance[hemisphere].HelpScreen();}
uint32_t Metronome_OnDataRequest(bool hemisphere) {return Metronome_instance[hemisphere].OnDataRequest();}
void Metronome_OnDataReceive(bool hemisphere, uint32_t data) {Metronome_instance[hemisphere].OnDataReceive(data);}
