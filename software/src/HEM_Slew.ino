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

#define HEM_SLEW_MAX_VALUE 200
#define HEM_SLEW_MAX_TICKS 64000

class Slew : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Slew";
    }

    void Start() {
        ForEachChannel(ch) signal[ch] = 0;
        rise = 50;
        fall = 50;
    }

    void Controller() {
        ForEachChannel(ch)
        {
            simfloat input = int2simfloat(In(ch));
            if (Gate(ch)) signal[ch] = input; // Defeat slew when channel's gate is high
            if (input != signal[ch]) {

                int segment = (input > signal[ch]) ? rise : fall;
                simfloat remaining = input - signal[ch];

                // The number of ticks it would take to get from 0 to HEMISPHERE_MAX_CV
                int max_change = Proportion(segment, HEM_SLEW_MAX_VALUE, HEM_SLEW_MAX_TICKS);

                // The number of ticks it would take to move the remaining amount at max_change
                int ticks_to_remaining = Proportion(simfloat2int(remaining), HEMISPHERE_MAX_CV, max_change);
                if (ticks_to_remaining < 0) ticks_to_remaining = -ticks_to_remaining;

                simfloat delta;
                if (ticks_to_remaining <= 0) {
                    delta = remaining;
                } else {
                    if (ch == 1) ticks_to_remaining /= 2;
                    delta = remaining / ticks_to_remaining;
                }
                signal[ch] += delta;
            }
            Out(ch, simfloat2int(signal[ch]));
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawIndicator();
    }

    void OnButtonPress() {
        cursor = 1 - cursor;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) {
            rise = constrain(rise += direction, 0, HEM_SLEW_MAX_VALUE);
            last_ms_value = Proportion(rise, HEM_SLEW_MAX_VALUE, HEM_SLEW_MAX_TICKS) / 17;
        }
        else {
            fall = constrain(fall += direction, 0, HEM_SLEW_MAX_VALUE);
            last_ms_value = Proportion(fall, HEM_SLEW_MAX_VALUE, HEM_SLEW_MAX_TICKS) / 17;
        }
        last_change_ticks = OC::CORE::ticks;
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, rise);
        Pack(data, PackLocation {8,8}, fall);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        rise = Unpack(data, PackLocation {0,8});
        fall = Unpack(data, PackLocation {8,8});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Defeat 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS]      = "Input 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_OUTS]     = "A=Linear B=Exp";
        help[HEMISPHERE_HELP_ENCODER]  = "Rise/Fall";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int rise; // Time to reach signal level if signal < 5V
    int fall; // Time to reach signal level if signal > 0V
    simfloat signal[2]; // Current signal level for each channel
    int cursor; // 0 = Rise, 1 = Fall
    int last_ms_value;
    int last_change_ticks;

    void DrawIndicator() {
        // Rise portion
        int r_x = Proportion(rise, 200, 31);
        gfxLine(0, 62, r_x, 33, cursor == 1);

        // Fall portion
        int f_x = 62 - Proportion(fall, 200, 31);
        gfxLine(f_x, 33, 62, 62, cursor == 0);

        // Center portion, if necessary
        gfxLine(r_x, 33, f_x, 33, 1);

        // Output indicators
        ForEachChannel(ch)
        {
            if (Gate(ch)) gfxFrame(1, 15 + (ch * 8), ProportionCV(ViewOut(ch), 62), 6);
            else gfxRect(1, 15 + (ch * 8), ProportionCV(ViewOut(ch), 62), 6);
        }

        // Change indicator, if necessary
        if (OC::CORE::ticks - last_change_ticks < 20000) {
            gfxPrint(15, 43, last_ms_value);
            gfxPrint("ms");
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Slew,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Slew Slew_instance[2];

void Slew_Start(bool hemisphere) {
    Slew_instance[hemisphere].BaseStart(hemisphere);
}

void Slew_Controller(bool hemisphere, bool forwarding) {
    Slew_instance[hemisphere].BaseController(forwarding);
}

void Slew_View(bool hemisphere) {
    Slew_instance[hemisphere].BaseView();
}

void Slew_OnButtonPress(bool hemisphere) {
    Slew_instance[hemisphere].OnButtonPress();
}

void Slew_OnEncoderMove(bool hemisphere, int direction) {
    Slew_instance[hemisphere].OnEncoderMove(direction);
}

void Slew_ToggleHelpScreen(bool hemisphere) {
    Slew_instance[hemisphere].HelpScreen();
}

uint32_t Slew_OnDataRequest(bool hemisphere) {
    return Slew_instance[hemisphere].OnDataRequest();
}

void Slew_OnDataReceive(bool hemisphere, uint32_t data) {
    Slew_instance[hemisphere].OnDataReceive(data);
}
