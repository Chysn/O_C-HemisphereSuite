class SampleAndHold : public HemisphereApplet {
public:

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
        gfxHeader("Dual S&H");
        gfxButterfly(0);
    }

    void ScreensaverView() {
        gfxButterfly(1);
    }

    void OnButtonPress() {}
    void OnButtonLongPress() {}
    void OnEncoderMove(int direction) {}

private:
    int held_values[2];
};

////////////////////////////////////////////////////////////////////////////////

SampleAndHold SampleAndHold_instance[2];

void SampleAndHold_Start(int hemisphere) {
    SampleAndHold_instance[hemisphere].SetHemisphere(hemisphere);
    SampleAndHold_instance[hemisphere].Start();
}

void SampleAndHold_Controller(int hemisphere) {
    SampleAndHold_instance[hemisphere].IO();
    SampleAndHold_instance[hemisphere].Controller();
}

void SampleAndHold_View(int hemisphere) {
    SampleAndHold_instance[hemisphere].View();
}

void SampleAndHold_Screensaver(int hemisphere) {
    SampleAndHold_instance[hemisphere].ScreensaverView();
}

void SampleAndHold_OnButtonPress(int hemisphere) {
    SampleAndHold_instance[hemisphere].OnButtonPress();
}

void SampleAndHold_OnButtonLongPress(int hemisphere) {
    SampleAndHold_instance[hemisphere].OnButtonLongPress();
}

void SampleAndHold_OnEncoderMove(int hemisphere, int direction) {
    SampleAndHold_instance[hemisphere].OnEncoderMove(direction);
}

