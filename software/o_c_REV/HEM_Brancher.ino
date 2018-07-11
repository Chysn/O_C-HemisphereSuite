class Brancher : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Brancher";
    }

	/* Run when the Applet is selected */
    void Start() {
    	    prob = 50;
    	    last_index = 0;
    }

	/* Run during the interrupt service routine */
    void Controller() {
        ForEachChannel(ch) choices[ch] = In(ch);
    	    if (Clock(0)) {
    	        last_index = (random(1, 100) > prob) ? 0 : 1;
    	    }
	    Out(0, choices[last_index]);
    }

	/* Draw the screen */
    void View() {
        gfxHeader("Brancher");
        DrawInterface();
    }

	/* Draw the screensaver */
    void ScreensaverView() {
        DrawInterface();
    }

    /* On button press, flip the last index to the opposite */
    void OnButtonPress() {
    		last_index = 1 - last_index;
    }

    /* Change the probability */
    void OnEncoderMove(int direction) {
        prob += (direction * 5);
        if (prob > 100) prob = 100;
        if (prob < 0) prob = 0;
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, prob);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        prob = Unpack(data, PackLocation {0,8});
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock, choose CV";
        help[HEMISPHERE_HELP_CVS] = "1,2=CV";
        help[HEMISPHERE_HELP_OUTS] = "A=CV based on p";
        help[HEMISPHERE_HELP_ENCODER] = "Probability";
    }

private:
	int prob;
	int choices[2];
	int last_index;

	void DrawInterface() {
        // Show the probability in the middle
        gfxPrint(26, 25, "p=");
        gfxPrint(23, 33, prob);
        gfxPrint("%");

        // Show the choices along the right side,
        // with the chosen one
        ForEachChannel(ch)
        {
            int height = ProportionCV(choices[ch], 48);
            int y = (52 - height) / 2; // To put it in the center
            if (ch == last_index) {
                gfxRect(4 + (46 * ch), 12 + y, 10, height);
            } else {
                gfxFrame(4 + (46 * ch), 12 + y, 10, height);
            }
        }
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
