class Switch : public HemisphereApplet {
public:

    void Start() {
    }

    void Controller() {
        for (int ch = 0; ch < 2; ch++)
        {
            if (Gate(ch)) {
                int cv = In(1);
                Out(ch, cv);
                gates[ch] = 1;
                outs[ch] = cv;
            } else {
                int cv = In(0);
                Out(ch, cv);
                gates[ch] = 0;
                outs[ch] = cv;
            }
        }
    }

    void View() {
        gfxHeader("Switch");
        gfxButterfly(0);
    }

    void ScreensaverView() {
        gfxButterfly(1);
    }

    void OnButtonPress() {
    }

    void OnButtonLongPress() {
    }

    void OnEncoderMove(int direction) {
    }

private:
    bool gates[2];
    int outs[2];
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Switch,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Switch Switch_instance[2];

void Switch_Start(int hemisphere) {
    Switch_instance[hemisphere].SetHemisphere(hemisphere);
    Switch_instance[hemisphere].Start();
}

void Switch_Controller(int hemisphere, bool forwarding) {
    Switch_instance[hemisphere].IO(forwarding);
    Switch_instance[hemisphere].Controller();
}

void Switch_View(int hemisphere) {
    Switch_instance[hemisphere].View();
    Switch_instance[hemisphere].DrawNotifications();
}

void Switch_Screensaver(int hemisphere) {
    Switch_instance[hemisphere].ScreensaverView();
}

void Switch_OnButtonPress(int hemisphere) {
    Switch_instance[hemisphere].OnButtonPress();
}

void Switch_OnButtonLongPress(int hemisphere) {
    Switch_instance[hemisphere].OnButtonLongPress();
}

void Switch_OnEncoderMove(int hemisphere, int direction) {
    Switch_instance[hemisphere].OnEncoderMove(direction);
}
