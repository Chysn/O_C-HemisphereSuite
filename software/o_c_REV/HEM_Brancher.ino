// Copyright (c) 2018, Jason Justian
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
        bool master_clock = MasterClockForwarded();

        if (Clock(0)) {
            int prob = p + Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 100);
            choice = (random(1, 100) <= prob) ? 0 : 1;

            // If Master Clock Forwarding is enabled, respond to this clock by
            // sending a clock
            if (master_clock) ClockOut(choice);
        }

        // Pass along the gate state if Master Clock Forwarding is off. If it's on,
        // the clock is handled above
        if (!master_clock) GateOut(choice, Gate(0));
    }

    void View() {
        gfxHeader("Brancher");
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
        gfxPrint(15 + pad(100, p), 15, p);
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

void Brancher_Start(bool hemisphere) {
    Brancher_instance[hemisphere].BaseStart(hemisphere);
}

void Brancher_Controller(bool hemisphere, bool forwarding) {
	Brancher_instance[hemisphere].BaseController(forwarding);
}

void Brancher_View(bool hemisphere) {
    Brancher_instance[hemisphere].BaseView();
}

void Brancher_OnButtonPress(bool hemisphere) {
    Brancher_instance[hemisphere].OnButtonPress();
}

void Brancher_OnEncoderMove(bool hemisphere, int direction) {
    Brancher_instance[hemisphere].OnEncoderMove(direction);
}

void Brancher_ToggleHelpScreen(bool hemisphere) {
    Brancher_instance[hemisphere].HelpScreen();
}

uint32_t Brancher_OnDataRequest(bool hemisphere) {
    return Brancher_instance[hemisphere].OnDataRequest();
}

void Brancher_OnDataReceive(bool hemisphere, uint32_t data) {
    Brancher_instance[hemisphere].OnDataReceive(data);
}
