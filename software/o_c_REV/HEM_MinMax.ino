class MinMax : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Min/Max";
    }

    void Start() {
        min = 0;
        max = 0;
    }

    void Controller() {
        // Handle CV Inputs
        int cv1 = In(0);
        int cv2 = In(1);

        // Calculate min and max
        min = (cv2 < cv1) ? cv2 : cv1;
        max = (cv2 > cv1) ? cv2 : cv1;

        // Handle CV Outputs: min to 0, max to 1
        Out(0, min);
        Out(1, max);
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
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS] = "1,2=CV";
        help[HEMISPHERE_HELP_OUTS] = "A=Min B=Max";
        help[HEMISPHERE_HELP_ENCODER] = "";
    }

private:
    int min;
    int max;

};

////////////////////////////////////////////////////////////////////////////////

MinMax MinMax_instance[2];

void MinMax_Start(int hemisphere) {
    MinMax_instance[hemisphere].SetHemisphere(hemisphere);
    MinMax_instance[hemisphere].BaseStart();
}

void MinMax_Controller(int hemisphere, bool forwarding) {
    MinMax_instance[hemisphere].BaseController(forwarding);
}

void MinMax_View(int hemisphere) {
    MinMax_instance[hemisphere].BaseView();
}

void MinMax_Screensaver(int hemisphere) {
    MinMax_instance[hemisphere].ScreensaverView();
}

void MinMax_OnButtonPress(int hemisphere) {
    MinMax_instance[hemisphere].OnButtonPress();
}

void MinMax_OnEncoderMove(int hemisphere, int direction) {
    MinMax_instance[hemisphere].OnEncoderMove(direction);
}

void MinMax_ToggleHelpScreen(int hemisphere) {
    MinMax_instance[hemisphere].HelpScreen();
}
