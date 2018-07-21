class Brancher : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Brancher";
    }

    void Start() {
    	    p = 50;
    	    choice = 0;
    }

    void Controller() {
        if (Clock(0)) {
            int prob = p + Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 100);
            choice = (random(1, 100) <= prob) ? 0 : 1;
        }
        GateOut(choice, Gate(0));
    }

    void View() {
        gfxHeader("Brancher");
        DrawInterface();
    }

    void ScreensaverView() {
        DrawInterface();
    }

    void OnButtonPress() {
    		choice = 1 - choice;
    }

    /* Change the pability */
    void OnEncoderMove(int direction) {
        p = constrain(p += direction, 0, 100);
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,7}, p);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        p = Unpack(data, PackLocation {0,7});
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock/Gate";
        help[HEMISPHERE_HELP_CVS] = "1=p Mod";
        help[HEMISPHERE_HELP_OUTS] = "A,B=Clock/Gate";
        help[HEMISPHERE_HELP_ENCODER] = "Set p";
    }

private:
	int p;
	int choice;

	void DrawInterface() {
        // Show the probability in the middle
        gfxPrint(1, 15, "p=");
        gfxPrint(15, 15, p);
        gfxPrint(33, 15, hemisphere ? "% C" : "% A");
        gfxCursor(15, 23, 18);

        gfxPrint(12, 45, hemisphere ? "C" : "A");
        gfxPrint(44, 45, hemisphere ? "D" : "B");
        gfxFrame(9 + (32 * choice), 42, 13, 13);
	}
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Brancher,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Brancher Brancher_instance[2];

void Brancher_Start(int hemisphere) {
    Brancher_instance[hemisphere].BaseStart(hemisphere);
}

void Brancher_Controller(int hemisphere, bool forwarding) {
	Brancher_instance[hemisphere].BaseController(forwarding);
}

void Brancher_View(int hemisphere) {
    Brancher_instance[hemisphere].BaseView();
}

void Brancher_Screensaver(int hemisphere) {
    Brancher_instance[hemisphere].BaseScreensaverView();
}

void Brancher_OnButtonPress(int hemisphere) {
    Brancher_instance[hemisphere].OnButtonPress();
}

void Brancher_OnEncoderMove(int hemisphere, int direction) {
    Brancher_instance[hemisphere].OnEncoderMove(direction);
}

void Brancher_ToggleHelpScreen(int hemisphere) {
    Brancher_instance[hemisphere].HelpScreen();
}

uint32_t Brancher_OnDataRequest(int hemisphere) {
    return Brancher_instance[hemisphere].OnDataRequest();
}

void Brancher_OnDataReceive(int hemisphere, uint32_t data) {
    Brancher_instance[hemisphere].OnDataReceive(data);
}
