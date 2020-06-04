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

#define HEM_EG_ATTACK 0
#define HEM_EG_DECAY 1
#define HEM_EG_SUSTAIN 2
#define HEM_EG_RELEASE 3
#define HEM_EG_NO_STAGE -1
#define HEM_EG_MAX_VALUE 255

#define HEM_SUSTAIN_CONST 35
#define HEM_EG_DISPLAY_HEIGHT 30

// About four seconds
#define HEM_EG_MAX_TICKS_AD 33333

// About eight seconds
#define HEM_EG_MAX_TICKS_R 133333

class ADSREG : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "ADSR EG";
    }

    void Start() {
        edit_stage = 0;
        attack = 20;
        decay = 30;
        sustain = 120;
        release = 25;
        ForEachChannel(ch)
        {
            stage_ticks[ch] = 0;
            gated[ch] = 0;
            stage[ch] = HEM_EG_NO_STAGE;
        }
    }

    void Controller() {
        // Look for CV modification of attack
        attack_mod = get_modification_with_input(0);
        release_mod = get_modification_with_input(1);

        ForEachChannel(ch)
        {
            if (Gate(ch)) {
                if (!gated[ch]) { // The gate wasn't on last time, so this is a newly-gated EG
                    stage_ticks[ch] = 0;
                    if (stage[ch] != HEM_EG_RELEASE) amplitude[ch] = 0;
                    stage[ch] = HEM_EG_ATTACK;
                    AttackAmplitude(ch);
                } else { // The gate is STILL on, so process the appopriate stage
                    stage_ticks[ch]++;
                    if (stage[ch] == HEM_EG_ATTACK) AttackAmplitude(ch);
                    if (stage[ch] == HEM_EG_DECAY) DecayAmplitude(ch);
                    if (stage[ch] == HEM_EG_SUSTAIN) SustainAmplitude(ch);
                }
                gated[ch] = 1;
            } else {
                if (gated[ch]) { // The gate was on last time, so this is a newly-released EG
                    stage[ch] = HEM_EG_RELEASE;
                    stage_ticks[ch] = 0;
                }

                if (stage[ch] == HEM_EG_RELEASE) { // Process the release stage, if necessary
                    stage_ticks[ch]++;
                    ReleaseAmplitude(ch);
                }
                gated[ch] = 0;
            }

            Out(ch, GetAmplitudeOf(ch));
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawIndicator();
        DrawADSR();
    }

    void OnButtonPress() {
        if (++edit_stage > HEM_EG_RELEASE) {edit_stage = HEM_EG_ATTACK;}
    }

    void OnEncoderMove(int direction) {
        int adsr[4] = {attack, decay, sustain, release};
        adsr[edit_stage] = constrain(adsr[edit_stage] += direction, 1, HEM_EG_MAX_VALUE);
        attack = adsr[HEM_EG_ATTACK];
        decay = adsr[HEM_EG_DECAY];
        sustain = adsr[HEM_EG_SUSTAIN];
        release = adsr[HEM_EG_RELEASE];
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, attack);
        Pack(data, PackLocation {8,8}, decay);
        Pack(data, PackLocation {16,8}, sustain);
        Pack(data, PackLocation {24,8}, release);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        attack = Unpack(data, PackLocation {0,8});
        decay = Unpack(data, PackLocation {8,8});
        sustain = Unpack(data, PackLocation {16,8});
        release = Unpack(data, PackLocation {24,8});

        if (attack == 0) Start(); // If empty data, initialize
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Gate 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "Mod 1=Att 2=Rel";
        help[HEMISPHERE_HELP_OUTS] = "Amp A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "A/D/S/R";
    }
    
private:
    int edit_stage;
    int attack; // Attack rate from 1-255 where 1 is fast
    int decay; // Decay rate from 1-255 where 1 is fast
    int sustain; // Sustain level from 1-255 where 1 is low
    int release; // Release rate from 1-255 where 1 is fast
    int attack_mod; // Modification to attack from CV1
    int release_mod; // Modification to release from CV2

    // Stage management
    int stage[2]; // The current ASDR stage of the current envelope
    int stage_ticks[2]; // Current number of ticks into the current stage
    bool gated[2]; // Gate was on in last tick
    simfloat amplitude[2]; // Amplitude of the envelope at the current position

    int GetAmplitudeOf(int ch) {
        return simfloat2int(amplitude[ch]);
    }

    void DrawIndicator() {
        ForEachChannel(ch)
        {
            int w = Proportion(GetAmplitudeOf(ch), HEMISPHERE_MAX_CV, 62);
            gfxRect(0, 15 + (ch * 10), w, 6);
        }
    }

    void DrawADSR() {
        int length = attack + decay + release + HEM_SUSTAIN_CONST; // Sustain is constant because it's a level
        int x = 0;
        x = DrawAttack(x, length);
        x = DrawDecay(x, length);
        x = DrawSustain(x, length);
        DrawRelease(x, length);
    }

    int DrawAttack(int x, int length) {
        int xA = x + Proportion(attack, length, 62);
        gfxLine(x, BottomAlign(0), xA, BottomAlign(HEM_EG_DISPLAY_HEIGHT), edit_stage != HEM_EG_ATTACK);
        return xA;
    }

    int DrawDecay(int x, int length) {
        int xD = x + Proportion(decay, length, 62);
        if (xD < 0) xD = 0;
        int yS = Proportion(sustain, HEM_EG_MAX_VALUE, HEM_EG_DISPLAY_HEIGHT);
        gfxLine(x, BottomAlign(HEM_EG_DISPLAY_HEIGHT), xD, BottomAlign(yS), edit_stage != HEM_EG_DECAY);
        return xD;
    }

    int DrawSustain(int x, int length) {
        int xS = x + Proportion(HEM_SUSTAIN_CONST, length, 62);
        int yS = Proportion(sustain, HEM_EG_MAX_VALUE, HEM_EG_DISPLAY_HEIGHT);
        if (yS < 0) yS = 0;
        if (xS < 0) xS = 0;
        gfxLine(x, BottomAlign(yS), xS, BottomAlign(yS), edit_stage != HEM_EG_SUSTAIN);
        return xS;
    }

    int DrawRelease(int x, int length) {
        int xR = x + Proportion(release, length, 62);
        int yS = Proportion(sustain, HEM_EG_MAX_VALUE, HEM_EG_DISPLAY_HEIGHT);
        gfxLine(x, BottomAlign(yS), xR, BottomAlign(0), edit_stage != HEM_EG_RELEASE);
        return xR;
    }

    void AttackAmplitude(int ch) {
        int effective_attack = constrain(attack + attack_mod, 1, HEM_EG_MAX_VALUE);
        int total_stage_ticks = Proportion(effective_attack, HEM_EG_MAX_VALUE, HEM_EG_MAX_TICKS_AD);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        if (effective_attack == 1) ticks_remaining = 0;
        if (ticks_remaining <= 0) { // End of attack; move to decay
            stage[ch] = HEM_EG_DECAY;
            stage_ticks[ch] = 0;
            amplitude[ch] = int2simfloat(HEMISPHERE_MAX_CV);
        } else {
            simfloat amplitude_remaining = int2simfloat(HEMISPHERE_MAX_CV) - amplitude[ch];
            simfloat increase = amplitude_remaining / ticks_remaining;
            amplitude[ch] += increase;
        }
    }

    void DecayAmplitude(int ch) {
        int total_stage_ticks = Proportion(decay, HEM_EG_MAX_VALUE, HEM_EG_MAX_TICKS_AD);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        simfloat amplitude_remaining = amplitude[ch] - int2simfloat(Proportion(sustain, HEM_EG_MAX_VALUE, HEMISPHERE_MAX_CV));
        if (sustain == 1) ticks_remaining = 0;
        if (ticks_remaining <= 0) { // End of decay; move to sustain
            stage[ch] = HEM_EG_SUSTAIN;
            stage_ticks[ch] = 0;
            amplitude[ch] = int2simfloat(Proportion(sustain, HEM_EG_MAX_VALUE, HEMISPHERE_MAX_CV));
        } else {
            simfloat decrease = amplitude_remaining / ticks_remaining;
            amplitude[ch] -= decrease;
        }
    }

    void SustainAmplitude(int ch) {
        amplitude[ch] = int2simfloat(Proportion(sustain - 1, HEM_EG_MAX_VALUE, HEMISPHERE_MAX_CV));
    }

    void ReleaseAmplitude(int ch) {
        int effective_release = constrain(release + release_mod, 1, HEM_EG_MAX_VALUE) - 1;
        int total_stage_ticks = Proportion(effective_release, HEM_EG_MAX_VALUE, HEM_EG_MAX_TICKS_R);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        if (effective_release == 0) ticks_remaining = 0;
        if (ticks_remaining <= 0 || amplitude[ch] <= 0) { // End of release; turn off envelope
            stage[ch] = HEM_EG_NO_STAGE;
            stage_ticks[ch] = 0;
            amplitude[ch] = 0;
        } else {
            simfloat decrease = amplitude[ch] / ticks_remaining;
            amplitude[ch] -= decrease;
        }
    }

    int get_modification_with_input(int in) {
        int mod = 0;
        mod = Proportion(DetentedIn(in), HEMISPHERE_MAX_CV, HEM_EG_MAX_VALUE / 2);
        return mod;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ADSREG,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ADSREG ADSREG_instance[2];

void ADSREG_Start(bool hemisphere) {
    ADSREG_instance[hemisphere].BaseStart(hemisphere);
}

void ADSREG_Controller(bool hemisphere, bool forwarding) {
    ADSREG_instance[hemisphere].BaseController(forwarding);
}

void ADSREG_View(bool hemisphere) {
    ADSREG_instance[hemisphere].BaseView();
}

void ADSREG_OnButtonPress(bool hemisphere) {
    ADSREG_instance[hemisphere].OnButtonPress();
}

void ADSREG_OnEncoderMove(bool hemisphere, int direction) {
    ADSREG_instance[hemisphere].OnEncoderMove(direction);
}

void ADSREG_ToggleHelpScreen(bool hemisphere) {
    ADSREG_instance[hemisphere].HelpScreen();
}

uint32_t ADSREG_OnDataRequest(bool hemisphere) {
    return ADSREG_instance[hemisphere].OnDataRequest();
}

void ADSREG_OnDataReceive(bool hemisphere, uint32_t data) {
    ADSREG_instance[hemisphere].OnDataReceive(data);
}
