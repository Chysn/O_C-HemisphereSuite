#define HEM_PALIMPSEST_MAX_VALUE 100

class Palimpsest : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Palimpsest";
    }

    void Start() {
        brush = 0;
        length = 16;
        step = 0;
        for (int s = 0; s < length; s++) accent[s] = 0;
    }

    void Controller() {
        if (Gate(1)) {
            // If this is the first time the brush was received during this step, compose the step,
            // unless the cursor is on the length parameter
            if (!brush && cursor != 2) {
                accent[step] += (HEMISPHERE_MAX_CV / HEM_PALIMPSEST_MAX_VALUE) * compose;
                accent[step] = constrain(accent[step], 0, HEMISPHERE_MAX_CV);
                brush = 1;
            }
        }

        if (Clock(0)) {
            // If the brush wasn't received during this step, decompose the step, unless
            // the cursor is on the length parameter
            if (!brush && cursor != 2) {
                accent[step] -= (HEMISPHERE_MAX_CV / HEM_PALIMPSEST_MAX_VALUE) * decompose;
                accent[step] = constrain(accent[step], 0, HEMISPHERE_MAX_CV);
            }

            brush = 0;
            Out(0, accent[step]);
            if (++step >= length) step = 0;
        } else {
            Out(0, accent[step]);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawControls();
        DrawSequence();
    }

    void ScreensaverView() {
        DrawControls();
        DrawSequence();
    }

    void OnButtonPress() {
        cursor++;
        if (cursor > 2) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) compose = constrain(compose += direction, 0, HEM_PALIMPSEST_MAX_VALUE);
        if (cursor == 1) decompose = constrain(decompose -= direction, 0, HEM_PALIMPSEST_MAX_VALUE);
        if (cursor == 2) length = constrain(length += direction, 2, 16);
        ResetCursor();
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,7}, compose);
        Pack(data, PackLocation {7,7}, decompose);
        Pack(data, PackLocation {14,4}, length - 1);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        compose = Unpack(data, PackLocation {0,7});
        decompose = Unpack(data, PackLocation {7,7});
        length = Unpack(data, PackLocation {14,4}) + 1;
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Brush";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=Output";
        help[HEMISPHERE_HELP_ENCODER]  = "Comp/Decomp/Length";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int accent[16]; // Accent data
    int length;
    int step;
    int compose;
    int decompose;
    int cursor;
    bool brush;

    void DrawControls() {
        int comp_w = Proportion(compose, HEM_PALIMPSEST_MAX_VALUE, 30);
        int decomp_w = Proportion(decompose, HEM_PALIMPSEST_MAX_VALUE, 30);
        gfxFrame(30 - decomp_w, 15, decomp_w, 7);
        gfxRect(32, 15, comp_w, 7);

        if (cursor == 0) gfxCursor(32, 23, 30);
        if (cursor == 1) gfxCursor(1, 23, 30);
        if (cursor == 2 && CursorBlink()) {
            gfxLine(0, 24, (length - 1) * 4 + 1, 24);
            gfxLine((length - 1) * 4 + 1, 24, (length - 1) * 4 + 1, 28);
            gfxLine(0, 24, 0, 28);
        }
    }

    void DrawSequence() {
        for (int s = 0; s < length; s++)
        {
            int index = s + step;
            if (index > length) index -= length;
            int h = ProportionCV(accent[index], 33);
            if (s == 0 && brush) gfxRect(s * 4 + 0, BottomAlign(h), 3, h);
            else gfxLine(s * 4 + 1, BottomAlign(h), s * 4 + 1, 62);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Palimpsest,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Palimpsest Palimpsest_instance[2];

void Palimpsest_Start(int hemisphere) {
    Palimpsest_instance[hemisphere].BaseStart(hemisphere);
}

void Palimpsest_Controller(int hemisphere, bool forwarding) {
    Palimpsest_instance[hemisphere].BaseController(forwarding);
}

void Palimpsest_View(int hemisphere) {
    Palimpsest_instance[hemisphere].BaseView();
}

void Palimpsest_Screensaver(int hemisphere) {
    Palimpsest_instance[hemisphere].BaseScreensaverView();
}

void Palimpsest_OnButtonPress(int hemisphere) {
    Palimpsest_instance[hemisphere].OnButtonPress();
}

void Palimpsest_OnEncoderMove(int hemisphere, int direction) {
    Palimpsest_instance[hemisphere].OnEncoderMove(direction);
}

void Palimpsest_ToggleHelpScreen(int hemisphere) {
    Palimpsest_instance[hemisphere].HelpScreen();
}

uint32_t Palimpsest_OnDataRequest(int hemisphere) {
    return Palimpsest_instance[hemisphere].OnDataRequest();
}

void Palimpsest_OnDataReceive(int hemisphere, uint32_t data) {
    Palimpsest_instance[hemisphere].OnDataReceive(data);
}
