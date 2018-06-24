class SumDiff : public HemisphereApplet {
public:

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
        gfxHeader("SumDiff");
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
    int sum;
    int diff;

};

////////////////////////////////////////////////////////////////////////////////

SumDiff SumDiff_instance[2];

void SumDiff_Start(int hemisphere) {
    SumDiff_instance[hemisphere].SetHemisphere(hemisphere);
    SumDiff_instance[hemisphere].Start();
}

void SumDiff_Controller(int hemisphere, bool forwarding) {
    SumDiff_instance[hemisphere].IO(forwarding);
    SumDiff_instance[hemisphere].Controller();
}

void SumDiff_View(int hemisphere) {
    SumDiff_instance[hemisphere].View();
    SumDiff_instance[hemisphere].DrawNotifications();
}

void SumDiff_Screensaver(int hemisphere) {
    SumDiff_instance[hemisphere].ScreensaverView();
}

void SumDiff_OnButtonPress(int hemisphere) {
    SumDiff_instance[hemisphere].OnButtonPress();
}

void SumDiff_OnButtonLongPress(int hemisphere) {
    SumDiff_instance[hemisphere].OnButtonLongPress();
}

void SumDiff_OnEncoderMove(int hemisphere, int direction) {
    SumDiff_instance[hemisphere].OnEncoderMove(direction);
}

