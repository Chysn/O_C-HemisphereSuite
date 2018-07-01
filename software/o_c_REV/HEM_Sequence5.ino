class Sequence5 : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Sequence5";
    }

    void Start() {
        for (int s = 0; s < 5; s++) value[s] = random(0, 135);
    }

    void Controller() {
        // Reset sequencer
        if (Clock(1)) step = 0;

        if (Clock(0)) {
            Advance(step);
            if (step == 0) ClockOut(1);
        }

        Out(0, Proportion(value[step], 135, HEMISPHERE_MAX_CV));
    }

    void View() {
        gfxHeader(applet_name());
        DrawPanel();
    }

    void ScreensaverView() {
        DrawSkeleton();
    }

    void OnButtonPress() {
        if (++selected == 5) selected = 0;
    }

    void OnEncoderMove(int direction) {
        value[selected] = constrain(value[selected] += direction, -1, 135);
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        return data;
    }

    void OnDataReceive(uint32_t data) {
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=CV B=Clk Step 1";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Set P=Sel Step";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int selected = 0;
    int value[5]; // Sequence value (-1 - 135, and -1 means Off)
    int step = 0; // Current sequencer step

    void Advance(int starting_point) {
        if (++step == 5) step = 0;
        // If all the steps have been skipped, stay where we were
        if (value[step] < 0 && step != starting_point) Advance(starting_point);
    }

    void DrawPanel() {
        // Sliders
        for (int s = 0; s < 5; s++)
        {
            if (value[s] > -1) {
                int x = 6 + (12 * s);
                gfxLine(x, 25, x, 63);
                int p = Proportion(value[s], 135, 30) + 3;

                // When selected, there's a heavier bar and a solid slider
                if (s == selected) {
                    gfxLine(x + 1, 25, x + 1, 63);
                    gfxRect(x - 4, BottomAlign(p), 9, 3);
                } else gfxFrame(x - 4, BottomAlign(p), 9, 3);

                // When on this step, there's an indicator circle
                if (s == step) {gfxCircle(x, 20, 3);}
            }
        }
    }

    void DrawSkeleton() {
        for (int s = 0; s < 5; s++)
        {
            if (value[s] > -1) {
                int x = 6 + (12 * s);
                int p = Proportion(value[s], 135, 42);
                if (s == step) gfxRect(x - 4, BottomAlign(p), 9, 4);
            }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Sequence5,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Sequence5 Sequence5_instance[2];

void Sequence5_Start(int hemisphere) {
    Sequence5_instance[hemisphere].BaseStart(hemisphere);
}

void Sequence5_Controller(int hemisphere, bool forwarding) {
    Sequence5_instance[hemisphere].BaseController(forwarding);
}

void Sequence5_View(int hemisphere) {
    Sequence5_instance[hemisphere].BaseView();
}

void Sequence5_Screensaver(int hemisphere) {
    Sequence5_instance[hemisphere].ScreensaverView();
}

void Sequence5_OnButtonPress(int hemisphere) {
    Sequence5_instance[hemisphere].OnButtonPress();
}

void Sequence5_OnEncoderMove(int hemisphere, int direction) {
    Sequence5_instance[hemisphere].OnEncoderMove(direction);
}

void Sequence5_ToggleHelpScreen(int hemisphere) {
    Sequence5_instance[hemisphere].HelpScreen();
}

uint32_t Sequence5_OnDataRequest(int hemisphere) {
    return Sequence5_instance[hemisphere].OnDataRequest();
}

void Sequence5_OnDataReceive(int hemisphere, uint32_t data) {
    Sequence5_instance[hemisphere].OnDataReceive(data);
}
