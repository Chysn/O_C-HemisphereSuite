int const HEM_EG_ATTACK = 0;
int const HEM_EG_DECAY = 1;
int const HEM_EG_SUSTAIN = 2;
int const HEM_EG_RELEASE = 3;
int const HEM_EG_NO_STAGE = -1;

int const HEM_SUSTAIN_CONST = 35;
int const HEM_EG_DISPLAY_HEIGHT = 30;
int const HEM_EG_MAX_TICKS_AD = 66667; // About four seconds
int const HEM_EG_MAX_TICKS_R  = 133333; // About eight sconds

class ADSREG : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "ADSR EG";
    }

    void Start() {
        edit_stage = 0;
        attack = 10;
        decay = 15;
        sustain = 60;
        release = 15;
        ForEachChannel(ch)
        {
            stage_ticks[ch] = 0;
            gated[ch] = 0;
            stage[ch] = HEM_EG_NO_STAGE;
        }
    }

    void Controller() {
        ForEachChannel(ch)
        {
            if (Gate(ch)) {
                if (!gated[ch]) { // The gate wasn't on last time, so this is a newly-gated EG
                    stage_ticks[ch] = 0;
                    if (stage[ch] != HEM_EG_RELEASE) amplitude[ch] = 0;
                    stage[ch] = HEM_EG_ATTACK;
                    if (attack == 1) amplitude[ch] = int2simfloat(HEMISPHERE_MAX_CV); // Snappy!
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
        DrawADSR();

        ForEachChannel(ch)
        {
            int w = Proportion(GetAmplitudeOf(ch), HEMISPHERE_MAX_CV, 62);
            gfxRect(0, 15 + (ch * 10), w, 6);
        }
    }

    void ScreensaverView() {
        int h[2];
        ForEachChannel(ch)
        {
            h[ch] = Proportion(GetAmplitudeOf(ch), HEMISPHERE_MAX_CV, 40);
            gfxLine(0 + (37 * ch), BottomAlign(h[ch]), 25 + (37 * ch), BottomAlign(h[ch]));
        }
        gfxLine(25, BottomAlign(h[0]), 37, BottomAlign(h[1]));
    }

    void OnButtonPress() {
        if (++edit_stage > HEM_EG_RELEASE) {edit_stage = HEM_EG_ATTACK;}
    }

    void OnEncoderMove(int direction) {
        int adsr[4] = {attack, decay, sustain, release};
        adsr[edit_stage] = constrain(adsr[edit_stage] += direction, 1, 100);
        attack = adsr[HEM_EG_ATTACK];
        decay = adsr[HEM_EG_DECAY];
        sustain = adsr[HEM_EG_SUSTAIN];
        release = adsr[HEM_EG_RELEASE];
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Gate 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "";
        help[HEMISPHERE_HELP_OUTS] = "Amp 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "T=Set Stage P=Sel";
    }
    
private:
    int edit_stage;
    int attack; // Attack rate from 1-100 where 1 is fast
    int decay; // Decay rate from 1-100 where 1 is fast
    int sustain; // Sustain level from 1-100 where 1 is low
    int release; // Release rate from 1-100 where 1 is fast

    // Stage management
    int stage[2]; // The current ASDR stage of the current envelope
    int stage_ticks[2]; // Current number of ticks into the current stage
    bool gated[2]; // Gate was on in last tick
    simfloat amplitude[2]; // Amplitude of the envelope at the current position

    int GetAmplitudeOf(int ch) {
        return simfloat2int(amplitude[ch]);
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
        gfxLine(x, BottomAlign(0), xA, BottomAlign(HEM_EG_DISPLAY_HEIGHT));
        if (edit_stage == HEM_EG_ATTACK) {
            gfxLine(x, BottomAlign(1), xA, BottomAlign(HEM_EG_DISPLAY_HEIGHT + 1));
            gfxLine(x, BottomAlign(2), xA, BottomAlign(HEM_EG_DISPLAY_HEIGHT + 2));
            gfxLine(x + 1, BottomAlign(0), xA + 1, BottomAlign(HEM_EG_DISPLAY_HEIGHT));
        }
        return xA;
    }

    int DrawDecay(int x, int length) {
        int xD = x + Proportion(decay, length, 62);
        if (xD < 0) xD = 0;
        int yS = Proportion(sustain, 100, HEM_EG_DISPLAY_HEIGHT);
        gfxLine(x, BottomAlign(HEM_EG_DISPLAY_HEIGHT), xD, BottomAlign(yS));
        if (edit_stage == HEM_EG_DECAY) {
            gfxLine(x, BottomAlign(HEM_EG_DISPLAY_HEIGHT + 1), xD, BottomAlign(yS + 1));
            gfxLine(x, BottomAlign(HEM_EG_DISPLAY_HEIGHT + 2), xD, BottomAlign(yS + 2));
            gfxLine(x + 1, BottomAlign(HEM_EG_DISPLAY_HEIGHT), xD + 1, BottomAlign(yS));
        }
        return xD;
    }

    int DrawSustain(int x, int length) {
        int xS = x + Proportion(HEM_SUSTAIN_CONST, length, 62);
        int yS = Proportion(sustain, 100, HEM_EG_DISPLAY_HEIGHT);
        if (yS < 3) yS = 3;
        if (xS < 0) xS = 0;
        gfxLine(x, BottomAlign(yS), xS, BottomAlign(yS));
        if (edit_stage == HEM_EG_SUSTAIN) {
            gfxLine(x, BottomAlign(yS + 1), xS, BottomAlign(yS + 1));
            gfxLine(x, BottomAlign(yS + 2), xS, BottomAlign(yS + 2));
        }
        return xS;
    }

    int DrawRelease(int x, int length) {
        int xR = x + Proportion(release, length, 62);
        int yS = Proportion(sustain, 100, HEM_EG_DISPLAY_HEIGHT);
        gfxLine(x, BottomAlign(yS), xR, BottomAlign(0));
        if (edit_stage == HEM_EG_RELEASE) {
            gfxLine(x, BottomAlign(yS + 1), xR, BottomAlign(1));
            gfxLine(x, BottomAlign(yS + 2), xR, BottomAlign(2));
            gfxLine(x - 1, BottomAlign(yS), xR - 1, BottomAlign(0));
        }
        return xR;
    }

    void AttackAmplitude(int ch) {
        int total_stage_ticks = Proportion(attack, 100, HEM_EG_MAX_TICKS_AD);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        simfloat amplitude_remaining = int2simfloat(HEMISPHERE_MAX_CV) - amplitude[ch];

        if (ticks_remaining <= 0) { // End of attack; move to decay
            stage[ch] = HEM_EG_DECAY;
            stage_ticks[ch] = 0;
        } else {
            simfloat increase = amplitude_remaining / ticks_remaining;
            amplitude[ch] += increase;
        }
    }

    void DecayAmplitude(int ch) {
        int total_stage_ticks = Proportion(decay, 100, HEM_EG_MAX_TICKS_AD);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        simfloat amplitude_remaining = amplitude[ch] - int2simfloat(Proportion(sustain, 100, HEMISPHERE_MAX_CV));

        if (ticks_remaining <= 0) { // End of decay; move to sustain
            stage[ch] = HEM_EG_SUSTAIN;
            stage_ticks[ch] = 0;
        } else {
            simfloat decrease = amplitude_remaining / ticks_remaining;
            amplitude[ch] -= decrease;
        }
    }

    void SustainAmplitude(int ch) {
        amplitude[ch] = int2simfloat(Proportion(sustain, 100, HEMISPHERE_MAX_CV));
    }

    void ReleaseAmplitude(int ch) {
        int total_stage_ticks = Proportion(release, 100, HEM_EG_MAX_TICKS_R);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        if (ticks_remaining <= 0 || amplitude[ch] <= 0) { // End of release; turn off envelope
            stage[ch] = HEM_EG_NO_STAGE;
            stage_ticks[ch] = 0;
            amplitude[ch] = 0;
        } else {
            simfloat decrease = amplitude[ch] / ticks_remaining;
            amplitude[ch] -= decrease;
        }
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

void ADSREG_Start(int hemisphere) {
    ADSREG_instance[hemisphere].BaseStart(hemisphere);
}

void ADSREG_Controller(int hemisphere, bool forwarding) {
    ADSREG_instance[hemisphere].BaseController(forwarding);
}

void ADSREG_View(int hemisphere) {
    ADSREG_instance[hemisphere].BaseView();
}

void ADSREG_Screensaver(int hemisphere) {
    ADSREG_instance[hemisphere].ScreensaverView();
}

void ADSREG_OnButtonPress(int hemisphere) {
    ADSREG_instance[hemisphere].OnButtonPress();
}

void ADSREG_OnEncoderMove(int hemisphere, int direction) {
    ADSREG_instance[hemisphere].OnEncoderMove(direction);
}

void ADSREG_ToggleHelpScreen(int hemisphere) {
    ADSREG_instance[hemisphere].HelpScreen();
}
