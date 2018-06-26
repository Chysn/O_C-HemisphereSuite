#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"

class DualQuant : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "DualQuant";
    }

    void Start() {
        selected = 0;
    }

    void Controller() {
    }

    void View() {
        gfxHeader(applet_name());



    }

    void ScreensaverView() {
        View();
    }

    void OnButtonPress() {
        selected = 1 - selected;
    }

    void OnEncoderMove(int direction) {

    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Clk 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "CV 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_OUTS] = "Pitch 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "T=Scale P=Sel Ch";
    }
    
private:
    braids::Quantizer quantizer;
    int selected;
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to DualQuant,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
DualQuant DualQuant_instance[2];

void DualQuant_Start(int hemisphere) {
    DualQuant_instance[hemisphere].SetHemisphere(hemisphere);
    DualQuant_instance[hemisphere].BaseStart();
}

void DualQuant_Controller(int hemisphere, bool forwarding) {
    DualQuant_instance[hemisphere].BaseController(forwarding);
}

void DualQuant_View(int hemisphere) {
    DualQuant_instance[hemisphere].BaseView();
}

void DualQuant_Screensaver(int hemisphere) {
    DualQuant_instance[hemisphere].ScreensaverView();
}

void DualQuant_OnButtonPress(int hemisphere) {
    DualQuant_instance[hemisphere].OnButtonPress();
}

void DualQuant_OnEncoderMove(int hemisphere, int direction) {
    DualQuant_instance[hemisphere].OnEncoderMove(direction);
}

void DualQuant_ToggleHelpScreen(int hemisphere) {
    DualQuant_instance[hemisphere].HelpScreen();
}
