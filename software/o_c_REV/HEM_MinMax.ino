class MinMax : public HemisphereApplet {
public:

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
        gfxHeader("MinMax");
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
    int min;
    int max;

};

////////////////////////////////////////////////////////////////////////////////

MinMax MinMax_instance[2];

void MinMax_Start(int hemisphere) {
    MinMax_instance[hemisphere].SetHemisphere(hemisphere);
    MinMax_instance[hemisphere].Start();
}

void MinMax_Controller(int hemisphere) {
    MinMax_instance[hemisphere].IO();
    MinMax_instance[hemisphere].Controller();
}

void MinMax_View(int hemisphere) {
    MinMax_instance[hemisphere].View();
}

void MinMax_Screensaver(int hemisphere) {
    MinMax_instance[hemisphere].ScreensaverView();
}

void MinMax_OnButtonPress(int hemisphere) {
    MinMax_instance[hemisphere].OnButtonPress();
}

void MinMax_OnButtonLongPress(int hemisphere) {
    MinMax_instance[hemisphere].OnButtonLongPress();
}

void MinMax_OnEncoderMove(int hemisphere, int direction) {
    MinMax_instance[hemisphere].OnEncoderMove(direction);
}

