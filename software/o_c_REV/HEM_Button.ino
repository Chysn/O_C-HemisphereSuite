// Copyright (c) 2022, Jason Justian
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

class Button : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Button";
    }

	/* Run when the Applet is selected */
    void Start() {
        toggle_st = 0; // Set toggle state to off
        trigger_out = 0; // Set trigger out queue to off
        trigger_countdown = 0;
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {
        if (Clock(0, 1)) OnButtonPress(); // Clock at Dig 0 emulates press (ignore forwarding)
        if (trigger_out) {
            ClockOut(0);
            trigger_out = 0; // Clear trigger queue
            trigger_countdown = 333; // Trigger display countdown
        }
        GateOut(1, toggle_st); // Send toggle state
    }

	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        DrawIndicator();
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
        trigger_out = 1; // Set trigger queue
        toggle_st = 1 - toggle_st; // Alternate toggle state when pressed
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
        /* Nothing for this applet */
    }
        
    /* No state data for this applet
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        return data;
    }

    /* No state data for this applet
     */
    void OnDataReceive(uint32_t data) {
        return;
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Press Button";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=Trg B=Toggle";
        help[HEMISPHERE_HELP_ENCODER]  = "P=Trg/Toggle";
        //                               "------------------" <-- Size Guide
    }
    
private:
    bool trigger_out; // Trigger output queue (output A/C)
    bool toggle_st; // Toggle state (output B/D)
    int trigger_countdown; // For momentary display of trigger output

    void DrawIndicator()
    {
        if (trigger_countdown > 0) {
            gfxBitmap(12, 35, 8, CLOCK_ICON);
            trigger_countdown--;
        }

        gfxBitmap(44, 43, 8, toggle_st ? CLOSED_ICON : OPEN_ICON);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Button,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Button Button_instance[2];

void Button_Start(bool hemisphere) {Button_instance[hemisphere].BaseStart(hemisphere);}
void Button_Controller(bool hemisphere, bool forwarding) {Button_instance[hemisphere].BaseController(forwarding);}
void Button_View(bool hemisphere) {Button_instance[hemisphere].BaseView();}
void Button_OnButtonPress(bool hemisphere) {Button_instance[hemisphere].OnButtonPress();}
void Button_OnEncoderMove(bool hemisphere, int direction) {Button_instance[hemisphere].OnEncoderMove(direction);}
void Button_ToggleHelpScreen(bool hemisphere) {Button_instance[hemisphere].HelpScreen();}
uint32_t Button_OnDataRequest(bool hemisphere) {return Button_instance[hemisphere].OnDataRequest();}
void Button_OnDataReceive(bool hemisphere, uint32_t data) {Button_instance[hemisphere].OnDataReceive(data);}
