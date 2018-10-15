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

class Switch : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Switch";
    }

    void Start() {
        active[0] = 1;
        active[1] = 1;
    }

    void Controller() {
        // Channel 1 is the sequential switch - When clocked, step between channel 1 and 2
        if (Clock(0)) {
            step = 1 - step;
            active[0] = step + 1;
        }
        Out(0, In(step));

        // Channel 2 is the gated switch - When gate is high, use channel 2, otherwise channel 1
        if (Gate(1)) {
            active[1] = 2;
            Out(1, In(1));
        } else {
            active[1] = 1;
            Out(1, In(0));
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawCaptions();
        DrawIndicator();
        gfxSkyline();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        return data;
    }

    void OnDataReceive(uint32_t data) {
    }

protected:
    void SetHelp() {
        // Each help section can have up to 18 characters. Be concise!
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Gate";
        help[HEMISPHERE_HELP_CVS] = "1,2=CV";
        help[HEMISPHERE_HELP_OUTS] = "A=Seq B=Gated Out";
        help[HEMISPHERE_HELP_ENCODER] = "";
    }

private:
    int step = 0;
    int active[2];

    void DrawIndicator()
    {
        ForEachChannel(ch)
        {
            // Selected input indicator
            gfxPrint(5 + (46 * ch), 40, active[ch]);
        }
    }

    void DrawCaptions()
    {
        gfxPrint(1, 15, "Seq");
        gfxPrint(36, 15, "Gate");
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Switch,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Switch Switch_instance[2];

void Switch_Start(bool hemisphere) {
    Switch_instance[hemisphere].BaseStart(hemisphere);
}

void Switch_Controller(bool hemisphere, bool forwarding) {
    Switch_instance[hemisphere].BaseController(forwarding);
}

void Switch_View(bool hemisphere) {
    Switch_instance[hemisphere].BaseView();
}

void Switch_OnButtonPress(bool hemisphere) {
    Switch_instance[hemisphere].OnButtonPress();
}

void Switch_OnEncoderMove(bool hemisphere, int direction) {
    Switch_instance[hemisphere].OnEncoderMove(direction);
}

void Switch_ToggleHelpScreen(bool hemisphere) {
    Switch_instance[hemisphere].HelpScreen();
}

uint32_t Switch_OnDataRequest(bool hemisphere) {
    return Switch_instance[hemisphere].OnDataRequest();
}

void Switch_OnDataReceive(bool hemisphere, uint32_t data) {
    Switch_instance[hemisphere].OnDataReceive(data);
}
