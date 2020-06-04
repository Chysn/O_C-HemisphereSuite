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

#define HEM_ENV_FOLLOWER_SAMPLES 166

class EnvFollow : public HemisphereApplet {
public:

    const char* applet_name() {
        return "EnvFollow";
    }

    void Start() {
        ForEachChannel(ch)
        {
            max[ch] = 0;
            gain[ch] = 10;
            duck[ch] = ch; // Default: one of each
        }
        countdown = HEM_ENV_FOLLOWER_SAMPLES;
    }

    void Controller() {
        if (--countdown == 0) {
            ForEachChannel(ch)
            {
                target[ch] = max[ch] * gain[ch];
                if (duck[ch]) target[ch] = HEMISPHERE_MAX_CV - target[ch]; // Handle ducking channel(s)
                target[ch] = constrain(target[ch], 0, HEMISPHERE_MAX_CV);
                max[ch] = 0;
            }
            countdown = HEM_ENV_FOLLOWER_SAMPLES;
        }

        ForEachChannel(ch)
        {
            if (In(ch) > max[ch]) max[ch] = In(ch);
            if (target[ch] > signal[ch]) signal[ch]++;
            else if (target[ch] < signal[ch]) signal[ch]--;
            Out(ch, signal[ch]);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
        gfxSkyline();
    }

    void OnButtonPress() {
        if (++cursor > 3) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor < 2) { // Gain per channel
            gain[cursor] = constrain(gain[cursor] + direction, 1, 31);
        } else {
            duck[cursor - 2] = 1 - duck[cursor - 2];
        }
        ResetCursor();
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,5}, gain[0]);
        Pack(data, PackLocation {5,5}, gain[1]);
        Pack(data, PackLocation {10,1}, duck[0]);
        Pack(data, PackLocation {11,1}, duck[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        gain[0] = Unpack(data, PackLocation {0,5});
        gain[1] = Unpack(data, PackLocation {5,5});
        duck[0] = Unpack(data, PackLocation {10,1});
        duck[1] = Unpack(data, PackLocation {11,1});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "Inputs 1,2";
        help[HEMISPHERE_HELP_OUTS]     = "Follow/Duck";
        help[HEMISPHERE_HELP_ENCODER]  = "Gain/Assign";
        //                               "------------------" <-- Size Guide
    }
    
private:
    uint8_t cursor;
    int max[2];
    uint8_t countdown;
    int signal[2];
    int target[2];

    // Setting
    uint8_t gain[2];
    bool duck[2]; // Choose between follow and duck per channel

    void DrawInterface() {
        ForEachChannel(ch)
        {
            // Duck
            gfxPrint(1 + (38 * ch), 15, duck[ch] ? "Duck" : "Foll");

            // Gain
            gfxFrame(32 * ch, 25, gain[ch], 3);

            if (cursor == ch && CursorBlink()) gfxRect(32 * ch, 25, gain[ch], 3);

        }
        if (cursor > 1) gfxCursor(1 + (38 * (cursor - 2)), 23, 24);
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to EnvFollow,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
EnvFollow EnvFollow_instance[2];

void EnvFollow_Start(bool hemisphere) {EnvFollow_instance[hemisphere].BaseStart(hemisphere);}
void EnvFollow_Controller(bool hemisphere, bool forwarding) {EnvFollow_instance[hemisphere].BaseController(forwarding);}
void EnvFollow_View(bool hemisphere) {EnvFollow_instance[hemisphere].BaseView();}
void EnvFollow_OnButtonPress(bool hemisphere) {EnvFollow_instance[hemisphere].OnButtonPress();}
void EnvFollow_OnEncoderMove(bool hemisphere, int direction) {EnvFollow_instance[hemisphere].OnEncoderMove(direction);}
void EnvFollow_ToggleHelpScreen(bool hemisphere) {EnvFollow_instance[hemisphere].HelpScreen();}
uint32_t EnvFollow_OnDataRequest(bool hemisphere) {return EnvFollow_instance[hemisphere].OnDataRequest();}
void EnvFollow_OnDataReceive(bool hemisphere, uint32_t data) {EnvFollow_instance[hemisphere].OnDataReceive(data);}
