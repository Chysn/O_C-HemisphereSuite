class SumDiff : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Sum/Diff";
    }

    void Start() {
        sum = 0;
        diff = 0;
    }

    void Controller() {
        // Handle CV Inputs
        int cv1 = In(0);
        int cv2 = In(1);

        // Calculate min and max
        sum = (cv1 + cv2);
        diff = (cv1 - cv2);
        if (sum > HEMISPHERE_MAX_CV) sum = HEMISPHERE_MAX_CV;
        if (diff < 0) diff = -diff; // Difference is absolute value

        // Handle CV Outputs: min to 0, max to 1
        Out(0, sum);
        Out(1, diff);
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

    void OnButtonLongPress() {
    }

    void OnEncoderMove(int direction) {
    }

protected:
    void SetHelp() {
        // Each help section can have up to 18 characters. Be concise!
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS] = "1,2=CV";
        help[HEMISPHERE_HELP_OUTS] = "A=Sum B=abs(Diff)";
        help[HEMISPHERE_HELP_ENCODER] = "";
    }

private:
    int sum;
    int diff;

};

////////////////////////////////////////////////////////////////////////////////

SumDiff SumDiff_instance[2];

void SumDiff_Start(int hemisphere) {
    SumDiff_instance[hemisphere].SetHemisphere(hemisphere);
    SumDiff_instance[hemisphere].BaseStart();
}

void SumDiff_Controller(int hemisphere, bool forwarding) {
    SumDiff_instance[hemisphere].BaseController(forwarding);
}

void SumDiff_View(int hemisphere) {
    SumDiff_instance[hemisphere].BaseView();
}

void SumDiff_Screensaver(int hemisphere) {
    SumDiff_instance[hemisphere].ScreensaverView();
}

void SumDiff_OnButtonPress(int hemisphere) {
    SumDiff_instance[hemisphere].OnButtonPress();
}

void SumDiff_OnEncoderMove(int hemisphere, int direction) {
    SumDiff_instance[hemisphere].OnEncoderMove(direction);
}

void SumDiff_ToggleHelpScreen(int hemisphere) {
    SumDiff_instance[hemisphere].HelpScreen();
}
