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

class VectorLFO : public HemisphereApplet {
public:

    const char* applet_name() {
        return "VectorLFO";
    }

    void Start() {
        waveform_count = WaveformManager::WaveformCount();
        ForEachChannel(ch)
        {
            freq[ch] = 200;
            freq_mod[ch] = 0;
            waveform_number[ch] = 0;
            SwitchWaveform(ch, 0);
            last_clock[ch] = OC::CORE::ticks;
        }
    }

    void Controller() {
        ForEachChannel(ch)
        {
            if (Clock(ch)) {
                uint32_t ticks = OC::CORE::ticks - last_clock[ch];
                int new_freq = 1666666 / ticks;
                new_freq = constrain(new_freq, 3, 99900);
                osc[ch].SetFrequency(new_freq);
                freq[ch] = new_freq;
                osc[ch].Reset();
                last_clock[ch] = OC::CORE::ticks;
            }

            int mod = Proportion(DetentedIn(ch), HEMISPHERE_3V_CV, freq[ch]);
            if (freq_mod[ch] != mod && mod + freq[ch] > 3) {
                freq_mod[ch] = mod;
                osc[ch].SetFrequency(freq[ch] + freq_mod[ch]);
            }


            Out(ch, osc[ch].Next());
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
        byte c = cursor;
        byte ch = cursor < 2 ? 0 : 1;
        if (ch) c -= 2;

        if (c == 1) { // Waveform
            SwitchWaveform(ch, waveform_number[ch] + direction);
        }
        if (c == 0) { // Frequency
            if (freq[ch] > 100000) direction *= 1000;
            else if (freq[ch] > 10000) direction *= 100;
            else if (freq[ch] > 1000) direction *= 10;
            freq[ch] = constrain(freq[ch] + direction, 3, 99900);
            osc[ch].SetFrequency(freq[ch]);
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
        help[HEMISPHERE_HELP_DIGITALS] = "1,2=Sync";
        help[HEMISPHERE_HELP_CVS]      = "1,2=Freq. Mod";
        help[HEMISPHERE_HELP_OUTS]     = "A,B=Out";
        help[HEMISPHERE_HELP_ENCODER]  = "Wave/Freq.";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=Freq A; 1=Waveform A; 2=Freq B; 3=Waveform B
    int waveform_count;
    uint32_t last_clock[2];
    int freq_mod[2]; // Bipolar modulation
    VectorOscillator osc[2];

    // Settings
    int waveform_number[2];
    int freq[2];
    
    void DrawInterface() {
        byte c = cursor;
        byte ch = cursor < 2 ? 0 : 1;
        if (ch) c -= 2;

        // Show channel output
        gfxPos(1, 15);
        if (hemisphere == 0) gfxPrint(ch ? "B" : "A");
        else gfxPrint(ch ? "D" : "C");
        gfxInvert(1, 14, 7, 9);

        gfxPrint(10, 15, ones(freq[ch]));
        gfxPrint(".");
        int h = hundredths(freq[ch]);
        if (h < 10) gfxPrint("0");
        gfxPrint(h);
        gfxPrint(" Hz");
        DrawWaveform(ch);

        if (c == 0) gfxCursor(8, 23, 55);
        if (c == 1) gfxInvert(0, 24, 63, 63);
    }

    void DrawWaveform(byte ch) {
        uint16_t total_time = osc[ch].TotalTime();
        byte prev_x = 0; // Starting coordinates
        byte prev_y = 63;
        for (byte i = 0; i < osc[ch].SegmentCount(); i++)
        {
            VOSegment seg = osc[ch].GetSegment(i);
            byte y = 63 - Proportion(seg.level, 255, 40);
            byte seg_x = Proportion(seg.time, total_time, 64);
            byte x = prev_x + seg_x;
            gfxLine(prev_x, prev_y, x, y);
            prev_x = x;
            prev_y = y;
        }

        // Zero line
        gfxDottedLine(0, 43, 63, 43, 8);
    }

    void SwitchWaveform(byte ch, int waveform) {
        waveform = constrain(waveform, 0, waveform_count - 1);
        osc[ch] = WaveformManager::VectorOscillatorFromWaveform(waveform);
        waveform_number[ch] = waveform;
        osc[ch].SetFrequency(freq[ch]);
        osc[ch].SetScale((12 << 7) * 3);
    }

    int ones(int n) {return (n / 100);}
    int hundredths(int n) {return (n % 100);}
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to VectorLFO,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
VectorLFO VectorLFO_instance[2];

void VectorLFO_Start(bool hemisphere) {VectorLFO_instance[hemisphere].BaseStart(hemisphere);}
void VectorLFO_Controller(bool hemisphere, bool forwarding) {VectorLFO_instance[hemisphere].BaseController(forwarding);}
void VectorLFO_View(bool hemisphere) {VectorLFO_instance[hemisphere].BaseView();}
void VectorLFO_Screensaver(bool hemisphere) {VectorLFO_instance[hemisphere].BaseScreensaverView();}
void VectorLFO_OnButtonPress(bool hemisphere) {VectorLFO_instance[hemisphere].OnButtonPress();}
void VectorLFO_OnEncoderMove(bool hemisphere, int direction) {VectorLFO_instance[hemisphere].OnEncoderMove(direction);}
void VectorLFO_ToggleHelpScreen(bool hemisphere) {VectorLFO_instance[hemisphere].HelpScreen();}
uint32_t VectorLFO_OnDataRequest(bool hemisphere) {return VectorLFO_instance[hemisphere].OnDataRequest();}
void VectorLFO_OnDataReceive(bool hemisphere, uint32_t data) {VectorLFO_instance[hemisphere].OnDataReceive(data);}
