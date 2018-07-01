class SampleAndHold : public HemisphereApplet {
public:

    const char* applet_name() {
        return "S&H/Rand";
    }

    void Start() {
    }

    void Controller() {
        ForEachChannel(ch)
        {
            if (Clock(ch) || (ch == 1 && ch1_normalize_ch2 && Clock(0))) {
                int cv = ch == 0 ? In(ch) : random(0, HEMISPHERE_MAX_CV);
                Out(ch, cv);
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        gfxPrint(1, 15, "S&H");
        gfxPrint(39, 15, "Rand");
        if (ch1_normalize_ch2) gfxPrint(28, 15, ">");
        gfxSkyline();
    }

    void ScreensaverView() {
        gfxButterfly_Channel();
    }

    /* Toggles the normalization of the channel 1 trigger to channel 2. This allows the
     * random output (B) to be sampled and held with the channel 1 digital input.
     */
    void OnButtonPress() {
        ch1_normalize_ch2 = 1 - ch1_normalize_ch2;
    }

    void OnEncoderMove(int direction) {
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,1}, ch1_normalize_ch2);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        ch1_normalize_ch2 = Unpack(data, PackLocation {0,1});
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Clk 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "Sample 1=Ch1";
        help[HEMISPHERE_HELP_OUTS] = "Hold A=Ch1 B=Rnd";
        help[HEMISPHERE_HELP_ENCODER] = "P=Clk Ch1 -> Ch2";
    }

private:
    int ch1_normalize_ch2 = 0;
};

////////////////////////////////////////////////////////////////////////////////

SampleAndHold SampleAndHold_instance[2];

void SampleAndHold_Start(int hemisphere) {
    SampleAndHold_instance[hemisphere].BaseStart(hemisphere);
}

void SampleAndHold_Controller(int hemisphere, bool forwarding) {
    SampleAndHold_instance[hemisphere].BaseController(forwarding);
}

void SampleAndHold_View(int hemisphere) {
    SampleAndHold_instance[hemisphere].BaseView();
}

void SampleAndHold_Screensaver(int hemisphere) {
    SampleAndHold_instance[hemisphere].ScreensaverView();
}

void SampleAndHold_OnButtonPress(int hemisphere) {
    SampleAndHold_instance[hemisphere].OnButtonPress();
}

void SampleAndHold_OnEncoderMove(int hemisphere, int direction) {
    SampleAndHold_instance[hemisphere].OnEncoderMove(direction);
}

void SampleAndHold_ToggleHelpScreen(int hemisphere) {
    SampleAndHold_instance[hemisphere].HelpScreen();
}

uint32_t SampleAndHold_OnDataRequest(int hemisphere) {
    return SampleAndHold_instance[hemisphere].OnDataRequest();
}

void SampleAndHold_OnDataReceive(int hemisphere, uint32_t data) {
    SampleAndHold_instance[hemisphere].OnDataReceive(data);
}
