int const HEMISPHERE_ACTIVE_TICKS = 1500;

class TLNeuron : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "T.L.Neuron";
    }

    void Start() {
        selected = 0;
    }

    void Controller() {
        // Summing function: add up the three weights
        int sum = 0;
        ForEachChannel(ch)
        {
            if (Gate(ch)) {
                sum += dendrite_weight[ch];
                dendrite_activated[ch] = 1;
            } else {
                dendrite_activated[ch] = 0;
            }
        }
        if (In(0) > (HEMISPHERE_MAX_CV / 2)) {
            sum += dendrite_weight[2];
            dendrite_activated[2] = 1;
        } else {
            dendrite_activated[2] = 0;
        }

        // Threshold function: fire the axon if the sum is GREATER THAN the threshhold
        // Both outputs have the same signal, in case you want to feed an output back
        // to an input.
        if (!axon_activated) axon_radius = 5;
        axon_activated = (sum > threshold);
        ForEachChannel(ch) GateOut(ch, axon_activated);

        // Increase the axon radius via timer
        if (--axon_countdown < 0) {
            axon_countdown = HEMISPHERE_ACTIVE_TICKS;
            ++axon_radius;
            if (axon_radius > 14) axon_radius = 5;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawDendrites();
        DrawAxon();
        DrawStates();
    }

    void ScreensaverView() {
        DrawStates();
    }

    void OnButtonPress() {
        if (++selected > 3) selected = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (selected < 3) {
            dendrite_weight[selected] = constrain(dendrite_weight[selected] + direction, 0, 9);
        } else {
            threshold = constrain(threshold + direction, 0, 27);
        }
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1,2=Dendrites 1,2";
        help[HEMISPHERE_HELP_CVS]      = "2=Dendrite3";
        help[HEMISPHERE_HELP_OUTS]     = "A,B=Axon Output";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Set P=Select";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int selected; // Which thing is selected (Dendrite 1, 2, 3 weights; Axon threshold)
    int dendrite_weight[3] = {5, 5, 0};
    int threshold = 9;
    bool dendrite_activated[3];
    bool axon_activated;
    int axon_radius = 5;
    int axon_countdown;

    void DrawDendrites() {
        for (int d = 0; d < 3; d++)
        {
            gfxCircle(9, 21 + (16 * d), 8); // Dendrite
            gfxPrint(6, 18 + (16 * d), dendrite_weight[d]);
            if (selected == d) gfxCursor(7, 26 + (16 * d), 5);
        }
    }

    void DrawAxon() {
        gfxCircle(48, 37, 12);
        gfxPrint(41, 34, threshold);
        if (selected == 3) gfxCursor(42, 42, 10);
    }

    void DrawStates() {
        for (int d = 0; d < 3; d++)
        {
            if (dendrite_activated[d]) gfxLine(17, 21 + (16 * d), 36, 37); // Synapse
        }

        if (axon_activated) {
            gfxCircle(48, 37, 12);
            gfxCircle(48, 37, axon_radius);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to TLNeuron,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
TLNeuron TLNeuron_instance[2];

void TLNeuron_Start(int hemisphere) {
    TLNeuron_instance[hemisphere].BaseStart(hemisphere);
}

void TLNeuron_Controller(int hemisphere, bool forwarding) {
    TLNeuron_instance[hemisphere].BaseController(forwarding);
}

void TLNeuron_View(int hemisphere) {
    TLNeuron_instance[hemisphere].BaseView();
}

void TLNeuron_Screensaver(int hemisphere) {
    TLNeuron_instance[hemisphere].ScreensaverView();
}

void TLNeuron_OnButtonPress(int hemisphere) {
    TLNeuron_instance[hemisphere].OnButtonPress();
}

void TLNeuron_OnEncoderMove(int hemisphere, int direction) {
    TLNeuron_instance[hemisphere].OnEncoderMove(direction);
}

void TLNeuron_ToggleHelpScreen(int hemisphere) {
    TLNeuron_instance[hemisphere].HelpScreen();
}
