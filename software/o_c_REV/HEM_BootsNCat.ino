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

#define BNC_MAX_PARAM 63

class BootsNCat : public HemisphereApplet {
public:

    const char* applet_name() {
        return "BootsNCat";
    }

    void Start() {
        tone[0] = 32; // Bass drum freq
        decay[0] = 32; // Bass drum decay
        tone[1] = 55; // Snare low limit
        decay[1] = 16; // Snare decay
        noise_tone_countdown = 1;
        blend = 0;

        bass = WaveformManager::VectorOscillatorFromWaveform(HS::Triangle);
        SetBDFreq();
        bass.SetScale((12 << 7) * 3); // Audio signal is -3V to +3V due to DAC asymmetry

        ForEachChannel(ch)
        {
            levels[ch] = 0;
            eg[ch] = WaveformManager::VectorOscillatorFromWaveform(HS::Sawtooth);
            eg[ch].SetFrequency(decay[ch]);
            eg[ch].SetScale(ch ? HEMISPHERE_3V_CV : HEMISPHERE_MAX_CV);
            eg[ch].Offset(ch ? HEMISPHERE_3V_CV : HEMISPHERE_MAX_CV);
            eg[ch].Cycle(0);
            SetEGFreq(ch);
        }
    }

    void Controller() {
        // Bass and snare signals are calculated independently
        int32_t signal = 0;
        int32_t bd_signal = 0;
        int32_t sd_signal = 0;

        ForEachChannel(ch)
        {
            if (Changed(ch)) eg[ch].SetScale((ch ? HEMISPHERE_3V_CV : HEMISPHERE_MAX_CV) - In(ch));
            if (Clock(ch, 1)) eg[ch].Start(); // Use physical-only clocking
        }

        // Calculate bass drum signal
        if (!eg[0].GetEOC()) {
            levels[0] = eg[0].Next();
            bd_signal = Proportion(levels[0], HEMISPHERE_MAX_CV, bass.Next());
        }

        // Calculate snare drum signal
        if (--noise_tone_countdown == 0) {
            noise = random(0, (12 << 7) * 6) - ((12 << 7) * 3);
            noise_tone_countdown = BNC_MAX_PARAM - tone[1] + 1;
        }

        if (!eg[1].GetEOC()) {
            levels[1] = eg[1].Next();
            sd_signal = Proportion(levels[1], HEMISPHERE_MAX_CV, noise);
        }

        // Bass Drum Output
        signal = Proportion((BNC_MAX_PARAM - blend) + BNC_MAX_PARAM, BNC_MAX_PARAM * 2, bd_signal);
        signal += Proportion(blend, BNC_MAX_PARAM * 2, sd_signal); // Blend in snare drum
        Out(0, signal);

        // Snare Drum Output
        signal = Proportion((BNC_MAX_PARAM - blend) + BNC_MAX_PARAM, BNC_MAX_PARAM * 2, sd_signal);
        signal += Proportion(blend, BNC_MAX_PARAM * 2, bd_signal); // Blend in bass drum
        Out(1, signal);
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        if (++cursor > 4) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 4) { // Blend
            blend = constrain(blend + direction, 0, BNC_MAX_PARAM);
        } else {
            byte ch = cursor > 1 ? 1 : 0;
            byte c = cursor;
            if (ch) c -= 2;

            if (c == 0) { // Tone
                tone[ch] = constrain(tone[ch] + direction, 0, BNC_MAX_PARAM);
                if (ch == 0) SetBDFreq();
            }

            if (c == 1) { // Decay
                decay[ch] = constrain(decay[ch] + direction, 0, BNC_MAX_PARAM);
                SetEGFreq(ch);
            }
        }
        ResetCursor();
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,6}, tone[0]);
        Pack(data, PackLocation {6,6}, decay[0]);
        Pack(data, PackLocation {12,6}, tone[1]);
        Pack(data, PackLocation {18,6}, decay[1]);
        Pack(data, PackLocation {24,6}, blend);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        tone[0] = Unpack(data, PackLocation {0,6});
        decay[0] = Unpack(data, PackLocation {6,6});
        tone[1] = Unpack(data, PackLocation {12,6});
        decay[1] = Unpack(data, PackLocation {18,6});
        blend = Unpack(data, PackLocation {24,6});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1,2 Play";
        help[HEMISPHERE_HELP_CVS]      = "Atten. 1=BD 2=SD";
        help[HEMISPHERE_HELP_OUTS]     = "A=Left B=Right";
        help[HEMISPHERE_HELP_ENCODER]  = "Preset/Pan";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor = 0;
    VectorOscillator bass;
    VectorOscillator eg[2];
    int noise_tone_countdown = 0;
    uint32_t noise;
    int levels[2]; // For display
    
    // Settings
    int tone[2];
    int decay[2];
    int8_t blend;

    void DrawInterface() {
        gfxPrint(1, 15, "BD Tone");
        DrawKnobAt(15, tone[0], cursor == 0);

        gfxPrint(1, 25, "  Decay");
        DrawKnobAt(25, decay[0], cursor == 1);

        gfxPrint(1, 35, "SD Tone");
        DrawKnobAt(35, tone[1], cursor == 2);

        gfxPrint(1, 45, "  Decay");
        DrawKnobAt(45, decay[1], cursor == 3);

        gfxPrint(1, 55, "Blend");
        DrawKnobAt(55, blend, cursor == 4);

        // Level indicators
        ForEachChannel(ch) gfxInvert(1, 14 + (20 * ch), ProportionCV(levels[ch], 42), 9);
    }

    void DrawKnobAt(byte y, byte value, bool is_cursor) {
        byte x = 45;
        byte w = Proportion(value, BNC_MAX_PARAM, 16);
        byte p = is_cursor ? 1 : 3;
        gfxDottedLine(x, y + 4, 62, y + 4, p);
        gfxRect(x + w, y, 2, 7);
    }

    void SetBDFreq() {
        bass.SetFrequency(Proportion(tone[0], BNC_MAX_PARAM, 3000) + 3000);
    }

    void SetEGFreq(byte ch) {
        eg[ch].SetFrequency(1000 - Proportion(decay[ch], BNC_MAX_PARAM, 900));
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to BootsNCat,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
BootsNCat BootsNCat_instance[2];

void BootsNCat_Start(bool hemisphere) {BootsNCat_instance[hemisphere].BaseStart(hemisphere);}
void BootsNCat_Controller(bool hemisphere, bool forwarding) {BootsNCat_instance[hemisphere].BaseController(forwarding);}
void BootsNCat_View(bool hemisphere) {BootsNCat_instance[hemisphere].BaseView();}
void BootsNCat_OnButtonPress(bool hemisphere) {BootsNCat_instance[hemisphere].OnButtonPress();}
void BootsNCat_OnEncoderMove(bool hemisphere, int direction) {BootsNCat_instance[hemisphere].OnEncoderMove(direction);}
void BootsNCat_ToggleHelpScreen(bool hemisphere) {BootsNCat_instance[hemisphere].HelpScreen();}
uint32_t BootsNCat_OnDataRequest(bool hemisphere) {return BootsNCat_instance[hemisphere].OnDataRequest();}
void BootsNCat_OnDataReceive(bool hemisphere, uint32_t data) {BootsNCat_instance[hemisphere].OnDataReceive(data);}
