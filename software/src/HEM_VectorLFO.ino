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
#include "vector_osc/WaveformManager.h"

class VectorLFO : public HemisphereApplet {
public:

    const char* applet_name() {
        return "VectorLFO";
    }

    void Start() {
        ForEachChannel(ch)
        {
            freq[ch] = 200;
            waveform_number[ch] = 0;
            SwitchWaveform(ch, 0);
            Out(ch, 0);
        }
    }

    void Controller() {
        // Input 1 is frequency modulation for channel 1
        if (Changed(0)) {
            int mod = Proportion(DetentedIn(0), HEMISPHERE_3V_CV, 3000);
            mod = constrain(mod, -3000, 3000);
            if (mod + freq[0] > 10) osc[0].SetFrequency(freq[0] + mod);
        }

        // Input 2 determines signal 1's attenuation on the B/D output mix; at 0V, signal 1
        // accounts for 50% of the B/D output. At 5V, signal 1 accounts for none of the
        // B/D output.
        int atten1 = DetentedIn(1);
        atten1 = constrain(atten1, 0, HEMISPHERE_MAX_CV);

        int signal = 0; // Declared here because the first channel's output is used in the second channel; see below
        ForEachChannel(ch)
        {
            if (Clock(ch)) {
                uint32_t ticks = ClockCycleTicks(ch);
                int new_freq = 1666666 / ticks;
                new_freq = constrain(new_freq, 3, 99900);
                osc[ch].SetFrequency(new_freq);
                freq[ch] = new_freq;
                osc[ch].Reset();
            }

            if (ch == 0) {
                // Out A is always just the first oscillator at full amplitude
                signal = osc[ch].Next();
            } else {
                // Out B can have channel 1 blended into it, depending on the value of atten1. At a value
                // of 0, Out B is a 50/50 mix of channels 1 and 2. At a value of 5V, channel 1 is absent
                // from Out B.
                signal = Proportion(HEMISPHERE_MAX_CV - atten1, HEMISPHERE_MAX_CV, signal); // signal from channel 1's iteration
                signal += osc[ch].Next();

                // Proportionally blend the signal, depending on attenuation. If atten1 is 0, then this
                // effectively divides the signal by 2. If atten1 is 5V, then the channel 2 signal will be
                // output at full amplitude.
                signal = Proportion(HEMISPHERE_MAX_CV, HEMISPHERE_MAX_CV + (HEMISPHERE_MAX_CV - atten1), signal);
            }
            Out(ch, signal);
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
            waveform_number[ch] = WaveformManager::GetNextWaveform(waveform_number[ch], direction);
            SwitchWaveform(ch, waveform_number[ch]);
            // Reset both waveform to provide a sync mechanism
            ForEachChannel(ch) osc[ch].Reset();
        }
        if (c == 0) { // Frequency
            if (freq[ch] > 100000) direction *= 10000;
            else if (freq[ch] > 10000) direction *= 1000;
            else if (freq[ch] > 1000) direction *= 100;
            else if (freq[ch] > 300) direction *= 10;
            freq[ch] = constrain(freq[ch] + direction, 10, 99900);
            osc[ch].SetFrequency(freq[ch]);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,6}, waveform_number[0]);
        Pack(data, PackLocation {6,6}, waveform_number[1]);
        Pack(data, PackLocation {12,10}, freq[0] & 0x03ff);
        Pack(data, PackLocation {22,10}, freq[1] & 0x03ff);
        return data;
    }
    void OnDataReceive(uint32_t data) {
        freq[0] = Unpack(data, PackLocation {12,10});
        freq[1] = Unpack(data, PackLocation {22,10});
        SwitchWaveform(0, Unpack(data, PackLocation {0,6}));
        SwitchWaveform(1, Unpack(data, PackLocation {6,6}));
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1,2=Sync";
        help[HEMISPHERE_HELP_CVS]      = "1=Freq1 2=Atten1@B";
        help[HEMISPHERE_HELP_OUTS]     = "Out A=1, B=2+1";
        help[HEMISPHERE_HELP_ENCODER]  = "Freq./Waveform";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=Freq A; 1=Waveform A; 2=Freq B; 3=Waveform B
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
        if (c == 1 && CursorBlink()) gfxFrame(0, 24, 63, 40);
    }

    void DrawWaveform(byte ch) {
        uint16_t total_time = osc[ch].TotalTime();
        VOSegment seg = osc[ch].GetSegment(osc[ch].SegmentCount() - 1);
        byte prev_x = 0; // Starting coordinates
        byte prev_y = 63 - Proportion(seg.level, 255, 38);
        for (byte i = 0; i < osc[ch].SegmentCount(); i++)
        {
            seg = osc[ch].GetSegment(i);
            byte y = 63 - Proportion(seg.level, 255, 38);
            byte seg_x = Proportion(seg.time, total_time, 62);
            byte x = prev_x + seg_x;
            x = constrain(x, 0, 62);
            y = constrain(y, 25, 62);
            gfxLine(prev_x, prev_y, x, y);
            prev_x = x;
            prev_y = y;
        }

        // Zero line
        gfxDottedLine(0, 44, 63, 44, 8);
    }

    void SwitchWaveform(byte ch, int waveform) {
        osc[ch] = WaveformManager::VectorOscillatorFromWaveform(waveform);
        waveform_number[ch] = waveform;
        osc[ch].SetFrequency(freq[ch]);
#ifdef BUCHLA_4U
        osc[ch].Offset((12 << 7) * 4);
        osc[ch].SetScale((12 << 7) * 4);
#else
        osc[ch].SetScale((12 << 7) * 3);
#endif
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
void VectorLFO_OnButtonPress(bool hemisphere) {VectorLFO_instance[hemisphere].OnButtonPress();}
void VectorLFO_OnEncoderMove(bool hemisphere, int direction) {VectorLFO_instance[hemisphere].OnEncoderMove(direction);}
void VectorLFO_ToggleHelpScreen(bool hemisphere) {VectorLFO_instance[hemisphere].HelpScreen();}
uint32_t VectorLFO_OnDataRequest(bool hemisphere) {return VectorLFO_instance[hemisphere].OnDataRequest();}
void VectorLFO_OnDataReceive(bool hemisphere, uint32_t data) {VectorLFO_instance[hemisphere].OnDataReceive(data);}
