class SampleAndHold : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Dual S&H";
    }

    void Start() {
        held_values[0] = 0;
        held_values[1] = 0;
    }

    void Controller() {
        for (int ch = 0; ch < 2; ch++)
        {
            if (Clock(ch)) {
                int cv = In(ch);
                Out(ch, cv);
                held_values[ch] = cv;
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        gfxButterfly(0);
    }

    void ScreensaverView() {
        gfxButterfly(1);
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Clk 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "Sample 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_OUTS] = "Hold A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "";
    }

private:
    int held_values[2];
};

////////////////////////////////////////////////////////////////////////////////

SampleAndHold SampleAndHold_instance[2];

void SampleAndHold_Start(int hemisphere) {
    SampleAndHold_instance[hemisphere].SetHemisphere(hemisphere);
    SampleAndHold_instance[hemisphere].Start();
}

void SampleAndHold_Controller(int hemisphere, bool forwarding) {
    SampleAndHold_instance[hemisphere].IO(forwarding);
    SampleAndHold_instance[hemisphere].Controller();
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
