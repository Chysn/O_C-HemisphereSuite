class BowTieSeq : public HemisphereApplet {
public:

    const char* applet_name() {
        return "BowTieSeq";
    }

    void Start() {
        step = -1;
    }

    void Controller() {
        if (Clock(1)) step = 0; // Reset
        
        uint16_t cv = (1000 * step) + 400;
        Out(0, cv);

        if (Clock(0)) if (++step > 7) step = 0;
    }

    void View() {
        gfxHeader(applet_name());
        DrawIndicator();
    }

    void ScreensaverView() {
        DrawIndicator();
    }

    void OnButtonPress() {
        step = 0;
    }

    void OnEncoderMove(int direction) {
        step += direction;
        if (step > 7) step = 0;
        if (step < 0) step = 7;
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
        help[HEMISPHERE_HELP_OUTS]     = "";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Step P=Reset";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int8_t step;
    
    void DrawIndicator() {
        for (uint8_t s = 0; s < 8; s++)
        {
            if (s == step) gfxRect(8 * s, 30, 7, 7);
            else gfxFrame(8 * s, 30, 7, 7);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to BowTieSeq,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
BowTieSeq BowTieSeq_instance[2];

void BowTieSeq_Start(int hemisphere) {
    BowTieSeq_instance[hemisphere].BaseStart(hemisphere);
}

void BowTieSeq_Controller(int hemisphere, bool forwarding) {
    BowTieSeq_instance[hemisphere].BaseController(forwarding);
}

void BowTieSeq_View(int hemisphere) {
    BowTieSeq_instance[hemisphere].BaseView();
}

void BowTieSeq_Screensaver(int hemisphere) {
    BowTieSeq_instance[hemisphere].BaseScreensaverView();
}

void BowTieSeq_OnButtonPress(int hemisphere) {
    BowTieSeq_instance[hemisphere].OnButtonPress();
}

void BowTieSeq_OnEncoderMove(int hemisphere, int direction) {
    BowTieSeq_instance[hemisphere].OnEncoderMove(direction);
}

void BowTieSeq_ToggleHelpScreen(int hemisphere) {
    BowTieSeq_instance[hemisphere].HelpScreen();
}

uint32_t BowTieSeq_OnDataRequest(int hemisphere) {
    return BowTieSeq_instance[hemisphere].OnDataRequest();
}

void BowTieSeq_OnDataReceive(int hemisphere, uint32_t data) {
    BowTieSeq_instance[hemisphere].OnDataReceive(data);
}
