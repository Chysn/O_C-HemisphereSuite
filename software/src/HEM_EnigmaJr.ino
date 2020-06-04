// Hemisphere Applet Boilerplate. Follow these steps to add a Hemisphere app:
//
// Copyright (c) 2018, Jason Justian
//
// Based on Braids Quantizer, Copyright 2015 Émilie Gillet.
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

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"
#include "enigma/TuringMachine.h"
#include "enigma/TuringMachineState.h"
#include "enigma/EnigmaOutput.h"

class EnigmaJr : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Enigma Jr";
    }

    void Start() {
        ForEachChannel(ch)
        {
            output[ch].InitAs(ch);
        }
        SwitchTuringMachine(0);
    }

    void Controller() {
        if (Clock(1)) {
            tm_state.Reset();
            length = tm_state.GetLength();
        }

        // If a clock is present, advance the TuringMachineState
        if (Clock(0)) {
            uint16_t reg = tm_state.GetRegister();
            ForEachChannel(ch)
            {
                int transpose = In(0);
                output[ch].SendToDAC<EnigmaJr>(this, reg, transpose);
            }
            tm_state.Advance(p);

            // Handle Organize
            if (--length == 0) {
                if (DetentedIn(1) > 0) Organize(In(1));
                else length = tm_state.GetLength();
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        if (++cursor > 3) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) { // Switch TM
            byte prev_tm = tm_state.GetTMIndex();
            byte new_tm = constrain(prev_tm + direction, 0, HS::TURING_MACHINE_COUNT - 1);
            if (prev_tm != new_tm) SwitchTuringMachine(new_tm);
        } else if (cursor ==1) { // Probability
            p = constrain(p + direction, 0, 100);
        } else { // Output assign
            byte o = cursor - 2; // Which output
            if (output[o].type() > 0 || direction > 0)
                output[o].set_type(output[o].type() + direction);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,7}, p);
        Pack(data, PackLocation {7,4}, output[0].type());
        Pack(data, PackLocation {11,4}, output[1].type());
        Pack(data, PackLocation {15,16}, tm_state.GetTMIndex());
        return data;
    }

    void OnDataReceive(uint32_t data) {
        p = Unpack(data, PackLocation {0,7});
        output[0].set_type(Unpack(data, PackLocation {7,4}));
        output[1].set_type(Unpack(data, PackLocation {11,4}));
        SwitchTuringMachine(Unpack(data, PackLocation {15,16}));
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS]      = "1=Shift 2=Organize";
        help[HEMISPHERE_HELP_OUTS]     = "Assignable";
        help[HEMISPHERE_HELP_ENCODER]  = "Reg/Prob/Assign";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=Reg Select, 1=p, 2=Output A/C, 3=Output B/D
    byte length;

    // Settings
    int8_t p = 0;
    EnigmaOutput output[2];
    TuringMachineState tm_state;

    void DrawInterface() {
        // First line: TM and Probability
        char name[4];
        HS::TuringMachine::SetName(name, tm_state.GetTMIndex());
        gfxPrint(1, 15, name);
        if (tm_state.IsFavorite()) gfxIcon(20, 15, FAVORITE_ICON);
        if (cursor == 0) {
            // If the Turing Machine is being selected, display the length and favorite
            // status instead of the probability
            byte length = tm_state.GetLength();
            if (length > 0) {
                gfxIcon(42, 15, LOOP_ICON);
                gfxPrint(52 + pad(10, length), 15, length);
            }
            else gfxPrint("--");
        } else {
            gfxPrint(34, 15, "p=");
            gfxPrint(pad(100, p), p);
        }

        // Second and third lines: Outputs
        ForEachChannel(ch)
        {
            byte o = 'A' + static_cast<char>(ch);
            if (hemisphere) o += 2;
            char out_name[3] = {o, ':', '\0'};
            gfxPrint(1, 25 + (ch * 10), out_name);
            gfxPrint(enigma_type_short_names[output[ch].type()]);
        }

        // Cursor
        if (cursor == 0) gfxCursor(1, 23, 18);
        if (cursor == 1) gfxCursor(46, 23, 18);
        if (cursor == 2) gfxCursor(13, 33, 44);
        if (cursor == 3) gfxCursor(13, 43, 44);

        tm_state.DrawAt(hemisphere * 64, 45);
    }

    // When a new TM is selected, load it here
    void SwitchTuringMachine(byte ix) {
        tm_state.Init(ix);
        tm_state.SetWriteMode(0); // Write is not allowed in EnigmaJr
        length = tm_state.GetLength();
    }

    void Organize(int cv) {
        if (cv > HEMISPHERE_MAX_CV) cv = HEMISPHERE_MAX_CV;
        byte next_tm = Proportion(cv, HEMISPHERE_MAX_CV, HS::TURING_MACHINE_COUNT - 1);

        // Number of favorites
        byte favorites = 0;
        for (byte i = 0; i < HS::TURING_MACHINE_COUNT; i++) favorites += HS::user_turing_machines[i].favorite;

        if (favorites > 0) {
            byte pick = Proportion(cv, HEMISPHERE_MAX_CV, favorites) + 1;
            pick = constrain(pick, 1, favorites);
            favorites = 0;
            for (int i = 0; i < HS::TURING_MACHINE_COUNT; i++)
            {
                if (HS::user_turing_machines[i].favorite) {
                    if (++favorites == pick) {
                        next_tm = i;
                        break;
                    }
                    next_tm = i;
                }
            }
        }

        if (tm_state.GetTMIndex() != next_tm) SwitchTuringMachine(next_tm);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to EnigmaJr,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
EnigmaJr EnigmaJr_instance[2];

void EnigmaJr_Start(bool hemisphere) {EnigmaJr_instance[hemisphere].BaseStart(hemisphere);}
void EnigmaJr_Controller(bool hemisphere, bool forwarding) {EnigmaJr_instance[hemisphere].BaseController(forwarding);}
void EnigmaJr_View(bool hemisphere) {EnigmaJr_instance[hemisphere].BaseView();}
void EnigmaJr_OnButtonPress(bool hemisphere) {EnigmaJr_instance[hemisphere].OnButtonPress();}
void EnigmaJr_OnEncoderMove(bool hemisphere, int direction) {EnigmaJr_instance[hemisphere].OnEncoderMove(direction);}
void EnigmaJr_ToggleHelpScreen(bool hemisphere) {EnigmaJr_instance[hemisphere].HelpScreen();}
uint32_t EnigmaJr_OnDataRequest(bool hemisphere) {return EnigmaJr_instance[hemisphere].OnDataRequest();}
void EnigmaJr_OnDataReceive(bool hemisphere, uint32_t data) {EnigmaJr_instance[hemisphere].OnDataReceive(data);}
