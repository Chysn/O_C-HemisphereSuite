class Brancher : public HemisphereApplet {
public:

	/* Run when the Applet is selected */
    void Start() {
    	    prob = 50;
    	    last_index = 0;
    }

	/* Run during the interrupt service routine */
    void Controller() {
        for (int ch = 0; ch < 2; ch++) choices[ch] = In(ch);
    	    if (Clock(0)) {
    	        last_index = (random(1, 100) > prob) ? 0 : 1;
    	    }
	    Out(0, choices[last_index]);
    }

	/* Draw the screen */
    void View() {
        gfxHeader("Brancher");
        
        // Show the probability in the middle
        gfxPrint(26, 25, "p=");
        gfxPrint(26, 33, prob);
        
        // Show the choices along the right side,
        // with the chosen one
        for (int ch = 0; ch < 2; ch++)
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

	/* Draw the screensaver */
    void ScreensaverView() {
        for (int ch = 0; ch < 2; ch++)
        {
            int height = ProportionCV(choices[ch], 48);
            int y = (52 - height) / 2; // To put it in the center
            if (ch == last_index) {
                gfxRect(8 + (41 * ch), 12 + y, 3, height);
            } else {
                gfxFrame(8 + (41 * ch), 12 + y, 1, height);
            }
        }
    }

    /* On button press, flip the last index to the opposite */
    void OnButtonPress() {
    		last_index = 1 - last_index;
    }

	/* Called when the encoder button for this hemisphere is long-pressed */
    void OnButtonLongPress() {
    }

    /* Change the probability */
    void OnEncoderMove(int direction) {
        prob += (direction * 5);
        if (prob > 100) prob = 100;
        if (prob < 0) prob = 0;
    }

private:
	int prob;
	int choices[2];
	int last_index;
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
    Brancher_instance[hemisphere].SetHemisphere(hemisphere);
    Brancher_instance[hemisphere].Start();
}

void Brancher_Controller(int hemisphere) {
	Brancher_instance[hemisphere].IO();
    Brancher_instance[hemisphere].Controller();
}

void Brancher_View(int hemisphere) {
    Brancher_instance[hemisphere].View();
}

void Brancher_Screensaver(int hemisphere) {
    Brancher_instance[hemisphere].ScreensaverView();
}

void Brancher_OnButtonPress(int hemisphere) {
    Brancher_instance[hemisphere].OnButtonPress();
}

void Brancher_OnButtonLongPress(int hemisphere) {
    Brancher_instance[hemisphere].OnButtonLongPress();
}

void Brancher_OnEncoderMove(int hemisphere, int direction) {
    Brancher_instance[hemisphere].OnEncoderMove(direction);
}
