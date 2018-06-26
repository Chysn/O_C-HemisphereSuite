class GatedVCA : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Gated VCA";
    }

	/* Run when the Applet is selected */
    void Start() {
        amp_offset_pct = 0;
        amp_offset_cv = 0;
    }

	/* Run during the interrupt service routine */
    void Controller() {
        int signal = In(0);
        int amplitude = In(1);
        int output = ProportionCV(amplitude, signal);
        output += amp_offset_cv;
        output = constrain(output, -HEMISPHERE_MAX_CV, HEMISPHERE_MAX_CV);

        Out(1, output); // Regular VCA output on B
        if (Gate(0)) Out(0, output); // Gated VCA output on A
        else Out(0, 0);
    }

	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        gfxPrint(0, 15, "Offset:");
        gfxPrint(amp_offset_pct);

        gfxInputBar(0, 25, 0);
        gfxInputBar(1, 35, 0);
        gfxOutputBar(1, 45, 0);
    }

	/* Draw the screensaver */
    void ScreensaverView() {
        gfxButterfly_Channel(1);
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
        amp_offset_pct = constrain(amp_offset_pct += direction, 0, 100);
        amp_offset_cv = Proportion(amp_offset_pct, 100, HEMISPHERE_MAX_CV);
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Gate Out1";
        help[HEMISPHERE_HELP_CVS] = "1=CV signal 2=Amp";
        help[HEMISPHERE_HELP_OUTS] = "A=Gated out B=Out";
        help[HEMISPHERE_HELP_ENCODER] = "T=Amp CV Offset";
    }
    
private:
    int amp_offset_pct; // Offset as percentage of max cv
    int amp_offset_cv; // Raw CV offset; calculated on encoder move
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to GatedVCA,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
GatedVCA GatedVCA_instance[2];

void GatedVCA_Start(int hemisphere) {
    GatedVCA_instance[hemisphere].BaseStart(hemisphere);
}

void GatedVCA_Controller(int hemisphere, bool forwarding) {
    GatedVCA_instance[hemisphere].BaseController(forwarding);
}

void GatedVCA_View(int hemisphere) {
    GatedVCA_instance[hemisphere].BaseView();
}

void GatedVCA_Screensaver(int hemisphere) {
    GatedVCA_instance[hemisphere].ScreensaverView();
}

void GatedVCA_OnButtonPress(int hemisphere) {
    GatedVCA_instance[hemisphere].OnButtonPress();
}

void GatedVCA_OnEncoderMove(int hemisphere, int direction) {
    GatedVCA_instance[hemisphere].OnEncoderMove(direction);
}

void GatedVCA_ToggleHelpScreen(int hemisphere) {
    GatedVCA_instance[hemisphere].HelpScreen();
}
